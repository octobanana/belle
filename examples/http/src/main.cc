// belle http example

#include "../../../include/belle.hh"
namespace Belle = OB::Belle;

#include <ctime>

#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>

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
        res << " All";
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

  // disable websocket upgrades
  app.websocket(false);

  // set the number of threads to 2
  app.threads(2);

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
  app.on_http("/", Belle::Method::get, [&](Belle::Server::Http_Ctx& ctx)
  {
    // the response string
    std::string res {R"(
      <a href="/">home</a><br>
      <a href="/method">methods</a><br>
      <a href="/params?key=value&blank=&query=test&page=2">query parameters</a><br>
      <a href="/418">regex route</a><br>
      <a href="/error">custom error</a><br>
      <a href="/index.html">static page</a><br>
    )"};

    // set http response headers
    ctx.res.set("content-type", "text/html");

    // set the http status code
    ctx.res.result(200);

    // set the http body
    ctx.res.body() = res;
  });

  // handle route '/method'
  // matches all methods
  app.on_http("/method", [](Belle::Server::Http_Ctx& ctx)
  {
    // set http response headers
    ctx.res.set("content-type", "text/plain");

    // set the http status code
    ctx.res.result(200);

    // get the request method type
    std::string method {ctx.req.method_string()};

    // echo back the matched http method
    ctx.res.body() = "Method: " + method + "\n";
  });

  // handle route POST '/'
  app.on_http("/post", Belle::Method::post, [](Belle::Server::Http_Ctx& ctx)
  {
    // set http response headers
    ctx.res.set("content-type", "text/plain");

    // set the http status code
    ctx.res.result(200);

    // echo back the request body
    ctx.res.body() = "Body: " + ctx.req.body() + "\n";
  });

  // handle route GET '/params' with query parameters
  // ex. http://localhost:8080/?q=test&page=2
  app.on_http("/params", Belle::Method::get, [](Belle::Server::Http_Ctx& ctx)
  {
    // stringstream to hold the response
    std::stringstream res;
    res << "Query Parameters:\n";

    // access the query parameters
    for (auto const& e : ctx.req.params())
    {
      // add each key value pair to the response
      res << e.first << " | " << e.second << "\n";
    }

    // set http response headers
    ctx.res.set("content-type", "text/plain");

    // set the http status code
    ctx.res.result(200);

    // echo back the query parameters
    ctx.res.body() = res.str();
  });

  // handle route GET '/<400-500 errors>' with query parameters
  // match a regex url
  // ex. http://localhost:8080/404
  app.on_http("^/([45]{1}[0-9]{2})$", Belle::Method::get, [](Belle::Server::Http_Ctx& ctx)
  {
    // access the url regex capture groups
    // using req.url() which is a vector of strings
    // index 0 contains the full matched url
    // index 1 to n contain the value of the capture groups if any
    std::string match {ctx.req.url().at(1)};

    // set http response headers
    ctx.res.set("content-type", "text/plain");

    // set the http status code
    ctx.res.result(200);

    // echo back the captured url path
    ctx.res.body() = "match: " + match;
  });

  // handle an error
  app.on_http("/error", Belle::Method::get, [](Belle::Server::Http_Ctx& ctx)
  {
    // trigger the custom error callback
    throw 500;
  });

  // set custom error callback
  app.on_http_error([&](Belle::Server::Http_Ctx& ctx)
  {
    // stringstream to hold the response
    std::stringstream res; res
    << "Status: " << ctx.res.result_int() << "\n"
    << "Reason: " << ctx.res.result() << "\n";

    // set http response headers
    ctx.res.set("content-type", "text/plain");

    // echo the http status code
    ctx.res.body() = res.str();
  });

  // set http connect callback
  // called at the beginning of every request
  app.on_http_connect([&](Belle::Server::Http_Ctx& ctx)
  {
    // print notification
    std::cerr << "New Request!\n";
  });

  // set http disconnect callback
  // called at the end of every request
  app.on_http_disconnect([&](Belle::Server::Http_Ctx& ctx)
  {
    // access http request headers
    std::string ip {std::string(ctx.req["X-Real-IP"]).empty() ? "localhost" : ctx.req["X-Real-IP"]};
    std::string ua {std::string(ctx.req["user-agent"])};
    std::string rf {std::string(ctx.req["referer"])};

    // get the current time
    std::time_t t {std::time(nullptr)};
    std::tm tm {*std::localtime(&t)};

    // log output
    std::cerr
    << "[" << std::put_time(&tm, "%H:%M:%S %e %b %Y") << "] "
    << "[" << ip << "] "
    << "[" << ctx.res.result_int() << "] "
    << "[" << ctx.req.method() << "] "
    << "[" << ctx.req.target().to_string() << "] "
    << "[" << rf << "] "
    << "[" << ua << "]\n\n";
  });

  // print out the address and port
  // along with all registered http routes
  // followed by the log output
  std::cout
  << "Server: " << address << ":" << port << "\n\n"
  << "Navigate to the following url:\n"
  << "  http://127.0.0.1:8080/\n\n"
  << "Try running the following commands:\n"
  << "  curl -X PUT http://127.0.0.1:8080/method\n"
  << "  curl -X POST --data 'post body message here' http://127.0.0.1:8080/post\n\n"
  << "Begin Routes>\n\n"
  << http_routes(app.http_routes())
  << "Begin Log>\n\n";

  // start the server
  app.listen();

  // the server blocks until a signal is received

  return 0;
}
