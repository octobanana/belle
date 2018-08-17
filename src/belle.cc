#include "belle.hh"
#include "mime_type.hh"

#include "string.hh"
namespace String = OB::String;

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
namespace http = boost::beast::http;

#include <boost/asio.hpp>
#include <boost/asio/signal_set.hpp>
namespace ip = boost::asio::ip;
using tcp = boost::asio::ip::tcp;

#include <filesystem>
namespace fs = std::filesystem;

#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <memory>
#include <regex>
#include <functional>
#include <chrono>
#include <map>
#include <unordered_map>

namespace OB
{

namespace Belle
{

int const BUFFER_SIZE {8192}; // 8mb

http_connection::http_connection(tcp::socket socket, std::shared_ptr<Attr> attr):
  socket_ {std::move(socket)},
  attr_ {attr},
  buffer_ {BUFFER_SIZE},
  deadline_ {socket_.get_io_context(), std::chrono::seconds(15)}
{
}

void http_connection::start()
{
  read_request();
  check_deadline();
}

void http_connection::read_request()
{
  auto self = shared_from_this();

  http::async_read(
    socket_,
    buffer_,
    req_,
    [self](std::error_code ec, size_t bytes)
    {
      if(!ec)
      {
        self->process_request();
        self->write_response();
      }
    }
  );
}

void http_connection::process_request()
{
  // process request
  res_.result(http::status::ok);

  for (auto& e : attr_->cb_hook_pre_)
  {
    e(ctx_);
  }

  // Handles request errors
  auto const req_error = [&](unsigned int const e) {
    res_.result(e);
    if (attr_->cb_error_)
    {
      ctx_.error_code = e;
      attr_->cb_error_(ctx_);
      return;
    }
    else
    {
      res_.set(http::field::content_type, "text/plain");
      res_.body() = "Error: " + std::to_string(e);
      return;
    }
  };

  // Make sure we can handle the method
  if (req_.method() == http::verb::unknown)
  {
    req_error(400);
    return;
  }

  // serve dynamic content
  for (auto &regex_method : attr_->routes_)
  {
    bool method_match {false};
    auto match = regex_method.second.find(METHOD_ALL);
    if (match != regex_method.second.end())
    {
      method_match = true;
    }
    else
    {
      match = regex_method.second.find(static_cast<int>(req_.method()));
      if (match != regex_method.second.end())
      {
        method_match = true;
      }
    }

    if (method_match)
    {
      std::smatch sm_res;
      std::string url_req {req_.target().to_string()};
      auto url_params = String::delimit(url_req, "?");
      url_req = url_params.at(0);
      if (std::regex_match(url_req, sm_res, std::regex(regex_method.first, std::regex::icase)))
      {
        for (auto const& e : sm_res)
        {
          ctx_.url.emplace_back(e.str());
        }

        // set params
        if (url_params.size() == 2)
        {
          auto kv = String::delimit(url_params.at(1), "&");
          for (auto const& e : kv)
          {
            auto k_v = String::delimit_first(e, "=");
            if (k_v.empty())
            {
              ctx_.params[e] = "";
            }
            else if (k_v.size() == 2)
            {
              ctx_.params[k_v.at(0)] = k_v.at(1);
            }
            else
            {
              continue;
            }
          }
        }

        // set method
        ctx_.method = to_lower(req_.method_string().to_string());

        // set callback function
        cb_route user_func = match->second;

        // run user function
        try
        {
          user_func(ctx_);
        }
        catch (unsigned int const e)
        {
          // std::cerr << "Error:\n  Route: " << ctx_.method << " " << url_req << "\n  " << e << "\n";
          req_error(e);
        }
        catch (std::exception const& e)
        {
          // std::cerr << "Error:\n  Route: " << ctx_.method << " " << url_req << "\n  " << e.what() << "\n";
          req_error(500);
        }
        catch (...)
        {
          // std::cerr << "Error:\n  Route: " << ctx_.method << " " << url_req << "\n  " << "rapid unscheduled disassembly" << "\n";
          req_error(500);
        }
        return;
      }
    }
  }

  // serve static files
  try
  {
    if (req_.method() == http::verb::get)
    {
      auto v_file {static_file()};
      res_.set("content-type", v_file.at(1));
      res_.body() = v_file.at(0);
      return;
    }
  }
  catch (unsigned int const e)
  {
    req_error(e);
    return;
  }
  catch (...)
  {
    req_error(500);
    return;
  }

  // Handle all else
  req_error(400);
  return;
}

void http_connection::write_response()
{
  auto self = shared_from_this();

  res_.version(11);
  res_.set("server", "Belle");
  res_.keep_alive(req_.keep_alive());

  if (! res_.has_content_length())
  {
    res_.content_length(res_.body().size());
  }

  for (auto& e : attr_->cb_hook_post_)
  {
    e(ctx_);
  }

  http::async_write(
    socket_,
    res_,
    [self](boost::beast::error_code ec, size_t bytes)
    {
      self->socket_.shutdown(tcp::socket::shutdown_send, ec);
      self->deadline_.cancel();
    }
  );
}

void http_connection::check_deadline()
{
  auto self = shared_from_this();

  deadline_.async_wait(
    [self](boost::beast::error_code ec)
    {
      if(!ec)
      {
        // Close socket to cancel any outstanding operation.
        self->socket_.close(ec);
      }
    }
  );
}

std::vector<std::string> http_connection::static_file()
{
  if (attr_->static_paths_.empty())
  {
    throw 404u;
  }

  fs::path root_path {fs::canonical(attr_->static_paths_.at(0))};
  fs::path file_path {fs::canonical(fs::path(attr_->static_paths_.at(0) + req_.target().to_string()))};

  //Check if path is within web_root_path
  if (std::distance(root_path.begin(), root_path.end()) >
      std::distance(file_path.begin(), file_path.end()) ||
      !std::equal(root_path.begin(), root_path.end(), file_path.begin()))
  {
    throw 404u;
  }

  if (fs::is_directory(file_path))
  {
    file_path/="index.html";
  }

  // find out mime type of file
  const std::string mt {mime_type(file_path.string())};

  // open file to read
  std::ifstream file {file_path.string()};
  if (!file.is_open())
  {
    throw 500u;
  }
  std::stringstream os;
  os << file.rdbuf();
  file.close();
  std::string data {os.str()};

  std::vector<std::string> v_data {data, mt};
  return v_data;
}

Http::Http()
{
}

Http::~Http()
{
}

Http& Http::address(std::string _address)
{
  if (!_address.empty())
  {
    address_ = _address;
  }
  return *this;
}

Http& Http::port(uint16_t _port)
{
  if (_port > 0)
  {
    port_ = _port;
  }
  return *this;
}

Http& Http::static_dir(std::string dir_path)
{
  attr_->static_paths_.emplace_back(dir_path);
  return *this;
}

void Http::http_server(tcp::acceptor& acceptor, tcp::socket& socket)
{
  acceptor.async_accept(socket,
    [&](boost::beast::error_code ec)
    {
      if(!ec)
      {
        std::make_shared<http_connection>(std::move(socket), attr_)->start();
      }
      http_server(acceptor, socket);
    });
}

void Http::listen(std::string _address, uint16_t _port)
{
  address(_address);
  port(_port);

  auto address = ip::address::from_string(address_);
  auto port = static_cast<unsigned short>(port_);
  tcp::acceptor acceptor {ios_, {address, port}};
  tcp::socket socket {ios_};

  http_server(acceptor, socket);

  // Capture SIGINT and SIGTERM to perform a clean shutdown
  boost::asio::signal_set signals {ios_, SIGINT, SIGTERM};
  signals.async_wait([&](boost::system::error_code const& ec, int sig) {
    // std::cerr << "\nSignal: " << sig << "\n";
    ios_.stop();
  });

  ios_.run();

  return;
}

boost::asio::io_context& Http::io()
{
  return ios_;
}

void Http::hook_pre(cb_hook _callback)
{
  attr_->cb_hook_pre_.emplace_back(_callback);
}

void Http::hook_post(cb_hook _callback)
{
  attr_->cb_hook_post_.emplace_back(_callback);
}

void Http::error(cb_error _callback)
{
  attr_->cb_error_ = _callback;
}

void Http::route(std::string _route, http::verb _method, cb_route _callback)
{
  attr_->routes_[_route][static_cast<int>(_method)] = _callback;
}

void Http::route(std::string _route, std::vector<http::verb> _methods, cb_route _callback)
{
  for (const auto &m : _methods)
  {
    attr_->routes_[_route][static_cast<int>(m)] = _callback;
  }
}

void Http::route(std::string _route, cb_route _callback)
{
  attr_->routes_[_route][METHOD_ALL] = _callback;
}

std::map<std::string, std::vector<std::string>> Http::get_routes() const
{
  std::map<std::string, std::vector<std::string>> routes;

  for (auto const& e : attr_->routes_)
  {
    std::vector<std::string> methods;
    for (auto const& m : e.second)
    {
      if (m.first == METHOD_ALL)
      {
        methods.emplace_back("ALL");
      }
      else
      {
        methods.emplace_back(http::to_string(static_cast<http::verb>(m.first)));
      }
    }
    routes[e.first] = methods;
  }

  return routes;
}

bool Http::check_file(std::string dir, std::string file) const
{
  fs::path root_path {fs::canonical(dir)};
  fs::path file_path {fs::canonical(fs::path(dir) / fs::path(file))};
  if (std::distance(root_path.begin(), root_path.end()) >
      std::distance(file_path.begin(), file_path.end()) ||
      !std::equal(root_path.begin(), root_path.end(), file_path.begin()))
  {
    return false;
  }
  return true;
}

} // namespace Belle

} // namespace OB
