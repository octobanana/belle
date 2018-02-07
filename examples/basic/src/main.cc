#include <ob/belle.hh>

#include <string>
#include <sstream>
#include <iostream>

using Ctx = OB::Belle::Ctx;
using Method = OB::Belle::Method;
using Status = OB::Belle::Status;
using Header = OB::Belle::Header;

std::string str_routes(std::map<std::string, std::vector<std::string>> const& routes)
{
  std::string routes_str;
  for (auto const& e : routes)
  {
    routes_str += "Route:  " + e.first + "\n" + "Method:";
    for (auto const& m : e.second)
    {
      routes_str += " " + m;
    }
    routes_str += "\n\n";
  }
  return routes_str;
}

int main(int argc, char *argv[])
{
  // init the server
  OB::Belle::Http app;

  // set the listening address
  std::string address {"127.0.0.1"};
  app.address("127.0.0.1");

  // set the listening port
  int port {8080};
  app.port(8080);

  // enable serving static files
  // set the relative path to the public directory
  // if file 'hello.txt' is in public, url access would be /hello.txt
  app.static_dir("public");

  // handle route GET '/'
  app.route("/", Method::get, [](Ctx& ctx) {
    auto& req = ctx.req;
    auto& res = ctx.res;

    // set the 'content-type' header to 'text/plain'
    res.set("content-type", "text/plain");

    // set the 'connection' header to 'keep-alive'
    res.set("connection", "keep-alive");

    // set the http status code to '200'
    res.result(200);

    // send hello world plain text
    res.body = "Hello, World!";
  });

  // handle all methods to route '/method'
  app.route("/method", [](Ctx& ctx) {
    auto& req = ctx.req;
    auto& res = ctx.res;

    res.set("content-type", "text/plain");
    res.set("connection", "keep-alive");
    res.result(200);

    // return the http method
    res.body = "Method: " + ctx.method;
  });

  // handle route GET '/redirect'
  app.route("/redirect", Method::get, [](Ctx& ctx) {
    auto& res = ctx.res;

    res.set("connection", "keep-alive");
    res.set("location", "/");
    res.result(303);
  });

  // handle route POST '/'
  app.route("/", Method::post, [](Ctx& ctx) {
    auto& req = ctx.req;
    auto& res = ctx.res;

    res.set(Header::content_type, "text/plain");
    res.keep_alive(req.keep_alive());
    res.result(Status::ok);

    // echo back request body
    res.body = req.body;
  });

  // handle route GET '/routes'
  app.route("/routes", Method::get, [&app](Ctx& ctx) {
    auto& req = ctx.req;
    auto& res = ctx.res;

    res.set(Header::content_type, "text/plain");
    res.keep_alive(req.keep_alive());
    res.result(Status::ok);

    res.body = str_routes(app.get_routes());
  });

  // handle route GET '/regex/<letters_only>/<numbers_only>/'
  // using regex url capture groups
  app.route("^/regex/([a-zA-Z]*)/([0-9]*)/$", Method::get, [](Ctx& ctx) {
    auto& req = ctx.req;
    auto& res = ctx.res;
    auto& url = ctx.url;

    res.set("content-type", "text/plain");
    res.keep_alive(req.keep_alive());
    res.result(200);

    std::stringstream ss;
    // url[0] contains the full url regex match
    ss << "url:  " << url.at(0) << "\n";
    // url[1] contains capture group 1
    ss << "arg1: " << url.at(1) << "\n";
    // url[2] contains capture group 2
    ss << "arg2: " << url.at(2) << "\n";

    res.body = ss.str();
  });

  // set error callback
  app.error([&](Ctx& ctx)
  {
    auto& req = ctx.req;
    auto& res = ctx.res;
    auto& err = ctx.error_code;

    res.set("content-type", "text/plain");
    res.keep_alive(req.keep_alive());
    res.result(err);

    try
    {
      res.body = "Custom Error: " + std::to_string(err);
    }
    catch (...)
    {
      res.result(500);
      res.body = "Custom Error: 500";
    }
  });

  // print out the list of registered routes and methods
  std::cout << str_routes(app.get_routes());

  // print out the address and port
  std::cout << "Running on: " << address << ":" << port << "\n";

  // start the server
  app.listen();

  return 0;
}
