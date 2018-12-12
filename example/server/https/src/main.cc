// belle https example

#include "belle.hh"
namespace Belle = OB::Belle;

#include <ctime>

#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <mutex>

// prototypes
std::string http_routes(Belle::Server::Http_Routes& routes);

// return string containing all http routes
std::string http_routes(Belle::Server::Http_Routes& routes)
{
  std::stringstream res;

  for (auto const& e : routes)
  {
    res << "Route:  " << (*e).first << "\nMethod:";

    for (auto const& m : (*e).second)
    {
      if (static_cast<Belle::Method>(m.first) == Belle::Method::unknown)
      {
        res << " ALL";
      }
      else
      {
        res << " " << Belle::http::to_string(static_cast<Belle::Method>(m.first));
      }
    }

    res << "\n\n";
  }

  return res.str();
}

Belle::ssl::context get_ssl_context();
Belle::ssl::context get_ssl_context()
{
  // set the ssl context to use tls v1.2
  Belle::ssl::context ctx {Belle::ssl::context::tlsv12};

  // set the ssl context password callback
  ctx.set_password_callback(
  [](std::size_t, Belle::ssl::context_base::password_purpose)
  {
    return "test";
  });

  // set the ssl context options
  ctx.set_options(
    Belle::ssl::context::default_workarounds |
    Belle::ssl::context::no_sslv2 |
    Belle::ssl::context::single_dh_use
  );

  // set the ssl context certificate file to use
  ctx.use_certificate_chain_file("../../../cert/cert.pem");

  // set the ssl context private key file to use
  ctx.use_private_key_file("../../../cert/key.pem", Belle::ssl::context_base::file_format::pem);

  return ctx;
}

int main(int argc, char *argv[])
{
  // init the server
  Belle::Server app;

  // set the listening address
  std::string address {"127.0.0.1"};
  app.address(address);

  // set the listening port
  int port {8080};
  app.port(port);

  // set ssl
  app.ssl(true);

  // set the ssl context
  app.ssl_context(get_ssl_context());

  // set the number of threads
  // default value is 1 single thread
  app.threads(std::thread::hardware_concurrency());

  // multithreading can be enabled on an http only server
  // websocket upgrades must be disabled to use multithreading
  // the websocket channel implementation is not thread safe
  // default value is true
  app.websocket(false);

  // mutex for iostream
  std::mutex io_mutex;

  // enable serving static files from a public directory
  // if the path is relative, make sure to run the program
  // in the right working directory
  app.public_dir("../public");

  // set default http headers
  Belle::Headers headers;
  headers.set(Belle::Header::server, "Belle");
  headers.set(Belle::Header::cache_control, "private; max-age=0");
  app.http_headers(headers);

  // handle the following signals
  app.signals({SIGINT, SIGTERM});

  // set the on signal callback
  app.on_signal([&](auto ec, auto sig)
  {
    // print out the signal received
    std::cerr << "\nSignal " << sig << "\n";

    // get the io_context and safely stop the server
    app.io().stop();
  });

  // handle route GET '/'
  app.on_http("/", Belle::Method::get, [](Belle::Server::Http_Ctx& ctx)
  {
    // the response string
    std::string res {R"(
      <a href="/">home</a><br>
      <a href="/method">methods</a><br>
      <a href="/params?key=value&blank=&query=test&page=2">query parameters</a><br>
      <a href="/regex/hello">regex route</a><br>
      <a href="/error">custom error</a><br>
      <a href="/index.html">static page</a><br>
    )"};

    // set http response headers
    ctx.res.set("content-type", "text/html");

    // set the http status code
    // the http status code can be either an integer or
    // its respective enum representation
    // the default status is 200 OK
    ctx.res.result(Belle::Status::ok);

    // set the http body
    ctx.res.body() = std::move(res);
  });

  // handle route ALL '/method'
  // matches all methods
  app.on_http("/method", [](Belle::Server::Http_Ctx& ctx)
  {
  // get the request method type as a string
    std::stringstream res; res
    << "HTTP Method\n"
    << "method: " << ctx.req.method_string() << "\n";

    // set http response headers
    ctx.res.set("content-type", "text/plain");

    // set the http status code
    ctx.res.result(Belle::Status::ok);

    // echo back the matched http method
    ctx.res.body() = res.str();
  });

  // handle route POST '/post'
  // echo back the posted data
  app.on_http("/post", Belle::Method::post, [](Belle::Server::Http_Ctx& ctx)
  {
    // get the request body data
    std::stringstream res; res
    << "Post Data\n"
    << "Body: " << ctx.req.body() << "\n";

    // set http response headers
    ctx.res.set("content-type", "text/plain");

    // set the http status code
    ctx.res.result(Belle::Status::ok);

    // echo back the request body
    ctx.res.body() = res.str();
  });

  // handle route GET '/params'
  // with query parameters
  // ex. http://localhost:8080/params?q=test&page=2
  app.on_http("/params", Belle::Method::get, [](Belle::Server::Http_Ctx& ctx)
  {
    // stringstream to hold the response
    std::stringstream res;
    res << "Query Parameters\n";

    // access the query parameters
    for (auto const& [key, val] : ctx.req.params())
    {
      // add each key value pair to the response
      res << key << " | " << val << "\n";
    }

    // set http response headers
    ctx.res.set("content-type", "text/plain");

    // set the http status code
    ctx.res.result(Belle::Status::ok);

    // echo back the query parameters
    ctx.res.body() = res.str();
  });

  // handle route GET '/regex/([a-z]+)'
  // match a regex path
  // one or more lowercase characters in the range of a-z
  // ex. http://localhost:8080/regex/hello
  // ex. http://localhost:8080/regex/belle
  app.on_http("^/regex/([a-z]+)$", Belle::Method::get, [](Belle::Server::Http_Ctx& ctx)
  {
    // access the path regex capture groups
    // using req.path() which is a vector of strings
    // index 0 contains the matched path, minus any query parameters
    // index 1 to n contain the value of the capture groups if any
    // the full path with query parameters is in req.target()
    std::string path {ctx.req.path().at(0)};
    std::string match {ctx.req.path().at(1)};

    // stringstream to hold the response
    std::stringstream res; res
    << "Regex Captures\n"
    << "path:  " << path << "\n"
    << "match: " << match << "\n";

    // set http response headers
    ctx.res.set("content-type", "text/plain");

    // set the http status code
    ctx.res.result(Belle::Status::ok);

    // echo back the captured path parameter
    ctx.res.body() = res.str();
  });

  // trigger the custom error callback
  app.on_http("/error", Belle::Method::get, [](Belle::Server::Http_Ctx& ctx)
  {
    // the thrown value sets the http status code and
    // calls the Belle::Server::on_http_error callback
    // the http status code can be either an integer or
    // its respective enum representation
    throw Belle::Status::internal_server_error;
  });

  // set custom error callback
  app.on_http_error([](Belle::Server::Http_Ctx& ctx)
  {
    // stringstream to hold the response
    // get the http status code represented as an int and string
    std::stringstream res; res
    << "Custom Error\n"
    << "Status: " << ctx.res.result_int() << "\n"
    << "Reason: " << ctx.res.result() << "\n";

    // set http response headers
    ctx.res.set("content-type", "text/plain");

    // send the custom error response
    ctx.res.body() = res.str();
  });

  // set http connect callback
  // called at the beginning of every request
  app.on_http_connect([&io_mutex](Belle::Server::Http_Ctx& ctx)
  {
    // acquire lock
    std::scoped_lock lock {io_mutex};

    // print http request headers
    std::cerr << ctx.req.base();
  });

  // set http disconnect callback
  // called at the end of every request
  app.on_http_disconnect([&io_mutex](Belle::Server::Http_Ctx& ctx)
  {
    // access http request headers
    std::string ip {std::string(ctx.req["X-Real-IP"]).empty() ? "localhost" : ctx.req["X-Real-IP"]};
    std::string ua {std::string(ctx.req["user-agent"])};
    std::string rf {std::string(ctx.req["referer"])};

    // get the current time
    std::time_t t {std::time(nullptr)};
    std::tm tm {*std::localtime(&t)};

    // acquire lock
    std::scoped_lock lock {io_mutex};

    // log output
    std::cerr
    // the current timestamp
    << "[" << std::put_time(&tm, "%H:%M:%S %e %b %Y") << "] "
    // the ip address
    << "[" << ip << "] "
    // the http status code as an integer
    << "[" << ctx.res.result_int() << "] "
    // the http method as a string
    << "[" << ctx.req.method_string() << "] "
    // the full request path as a string
    << "[" << ctx.req.target().to_string() << "] "
    // the http referer header
    << "[" << rf << "] "
    // the http user-agent header
    << "[" << ua << "]\n\n";
  });

  // print out the address and port
  // along with all registered http routes
  // followed by the log output
  std::cout
  << "Server: " << address << ":" << port << "\n\n"
  << "Navigate to the following url:\n"
  << "  https://" << address << ":" << port << "/\n\n"
  << "Try running the following commands:\n"
  << "  curl -X PUT https://" << address << ":" << port << "/method\n"
  << "  curl -X POST --data 'post body message here' https://"
  << address << ":" << port << "/post\n\n"
  << "Begin Routes>\n\n"
  << http_routes(app.http_routes())
  << "Begin Log>\n\n";

  // start the server
  app.listen();

  // the server blocks until a signal is received

  return 0;
}
