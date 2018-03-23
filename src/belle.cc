#include "belle.hh"
#include "mime_type.hh"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
namespace http = boost::beast::http;

#include <boost/asio.hpp>
namespace ip = boost::asio::ip;
using tcp = boost::asio::ip::tcp;

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

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

namespace OB
{
namespace Belle
{
  int const BUFFER_SIZE {4096}; // 4mb

  http_connection::http_connection(tcp::socket socket, std::shared_ptr<Attr> attr):
    socket_ {std::move(socket)},
    attr_ {attr},
    buffer_ {BUFFER_SIZE},
    deadline_ {socket_.get_io_context(), std::chrono::seconds(60)}
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
        }
      }
    );
  }

  void http_connection::process_request()
  {
    // process request

    // Handles request errors
    auto const req_error =
    [&](const int& e)
    {
      if (attr_->cb_error_)
      {
        ctx_.error_code = e;
        attr_->cb_error_(ctx_);
        return;
      }
      else
      {
        res_.result(static_cast<unsigned int>(e));
        res_.set(http::field::content_type, "text/plain");
        res_.keep_alive(req_.keep_alive());
        res_.body() = "Error: " + std::to_string(e);
        return;
      }
    };

    // Make sure we can handle the method
    if (req_.method() == http::verb::unknown)
    {
      req_error(400);
      res_.version(11);
      res_.set("server", "Belle");
      res_.keep_alive(req_.keep_alive());
      write_response();
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
        const std::string url_req {req_.target().to_string()};
        if (std::regex_match(url_req, sm_res, regex_method.first))
        {
          for (auto const& e : sm_res)
          {
            ctx_.url.emplace_back(e.str());
          }

          // set method
          ctx_.method = to_lower(req_.method_string().to_string());

          // set callback function
          cb_route user_func = match->second;

          // run user function
          user_func(ctx_);
          res_.version(11);
          res_.set("server", "Belle");

          write_response();
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
        res_.result(http::status::ok);
        res_.set("content-type", v_file.at(1));
        res_.body() = v_file.at(0);
        res_.version(11);
        res_.set("server", "Belle");
        res_.keep_alive(req_.keep_alive());
        write_response();
        return;
      }
    }
    catch (int const& e)
    {
      req_error(e);
      write_response();
      return;
    }

    // handle url not found
    req_error(400);
    write_response();
    return;
  }

  void http_connection::write_response()
  {
    auto self = shared_from_this();

    res_.set(http::field::content_length, res_.body().size());

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
    try
    {
      fs::path root_path;
      if (attr_->static_paths_.empty())
      {
        throw 404;
      }
      root_path = fs::canonical(attr_->static_paths_.at(0));

      fs::path file_path = fs::canonical(root_path/req_.target().to_string());

      //Check if path is within web_root_path
      if (std::distance(root_path.begin(), root_path.end()) >
          std::distance(file_path.begin(), file_path.end()) ||
          !std::equal(root_path.begin(), root_path.end(), file_path.begin()))
      {
        throw 404;
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
        throw 500;
      }
      std::stringstream os;
      os << file.rdbuf();
      file.close();
      std::string data {os.str()};

      std::vector<std::string> v_data {data, mt};
      return v_data;
    }
    catch (std::exception const& e)
    {
      throw 404;
    }
    catch (int const& e)
    {
      throw e;
    }
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
    ios_.run();

    return;
  }

  boost::asio::io_context& Http::ios()
  {
    return ios_;
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
      routes[e.first.str()] = methods;
    }

    return routes;
  }
} // namespace Belle
} // namespace OB
