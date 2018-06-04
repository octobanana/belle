#ifndef OB_BELLE_HH
#define OB_BELLE_HH

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

#include <string>
#include <vector>
#include <cstdint>
#include <regex>
#include <functional>
#include <map>
#include <memory>
#include <chrono>
#include <thread>

namespace OB
{
namespace Belle
{
  namespace fs = boost::filesystem;
  namespace http = boost::beast::http;
  namespace ip = boost::asio::ip;
  using tcp = boost::asio::ip::tcp;

  using Method = boost::beast::http::verb;
  using Status = boost::beast::http::status;
  using Header = boost::beast::http::field;

  struct Ctx
  {
    int error_code {0};
    std::vector<std::string> url;
    std::string method;
    http::request<http::string_body> req;
    http::response<http::string_body> res;
  };

  using cb_route =
    std::function<void(OB::Belle::Ctx&)>;

  using cb_error =
    std::function<void(OB::Belle::Ctx&)>;

  class Regex : public std::regex
  {
  public:
    Regex(const char* cstr) : std::regex {cstr}, str_ {cstr} {}
    Regex(std::string str) : std::regex {str}, str_ {std::move(str)} {}

    bool operator<(const Regex &rhs) const noexcept
    {
      return str_ < rhs.str_;
    }

    std::string str() const
    {
      return str_;
    }

    friend std::ostream& operator<<(std::ostream& os, const Regex& obj);

  private:
    std::string str_;
  }; // class Regex

  inline std::ostream& operator<<(std::ostream& os, const Regex& obj)
  {
    os << obj.str_;
    return os;
  }

  class Attr
  {
    public:
      std::vector<std::string> static_paths_;
      std::map<Regex, std::map<int, cb_route>> routes_;
      cb_error cb_error_;
  }; // class Attr

  class http_connection : public std::enable_shared_from_this<http_connection>
  {
  public:
    http_connection(tcp::socket socket, std::shared_ptr<Attr> attr);
    void start();

  private:
    tcp::socket socket_;
    std::shared_ptr<Attr> attr_;
    boost::beast::flat_buffer buffer_;
    boost::asio::basic_waitable_timer<std::chrono::steady_clock> deadline_;
    Ctx ctx_ {};
    http::request<http::string_body>& req_ {ctx_.req};
    http::response<http::string_body>& res_ {ctx_.res};
    int const METHOD_ALL {100};

    void read_request();
    void process_request();
    void create_response();
    void write_response();
    void check_deadline();
    std::vector<std::string> static_file();
  }; // class http_connection

  class Http
  {
  public:
    Http();
    ~Http();

    boost::asio::io_context& io();
    Http& address(std::string _address);
    Http& port(uint16_t _port);
    void listen(std::string _address = "", uint16_t _port = 0);
    Http& static_dir(std::string dir_path);

    void error(cb_route _callback);
    void route(std::string _route, http::verb _method, cb_route _callback);
    void route(std::string _route, std::vector<http::verb> _methods, cb_route _callback);
    void route(std::string _route, cb_route _callback);
    std::map<std::string, std::vector<std::string>> get_routes() const;

  private:
    boost::asio::io_context ios_ {1};
    std::string address_ {"127.0.0.1"};
    uint16_t port_ {4480};
    std::shared_ptr<Attr> attr_ {std::make_shared<Attr>()};
    int const METHOD_ALL {100};

    void http_server(tcp::acceptor& acceptor, tcp::socket& socket);
  }; // class Http
} // namespace Belle
} // namespace OB

#endif
