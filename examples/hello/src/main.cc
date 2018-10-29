// belle hello example

#include "../../../include/belle.hh"
namespace Belle = OB::Belle;

#include <string>
#include <iostream>

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

  // enable serving static files from a public directory
  // if the path is relative, make sure to run the program
  // in the right working directory
  app.public_dir("../public");

  // handle route GET '/'
  app.on_http("/", Belle::Method::get, [](Belle::Server::Http_Ctx& ctx)
  {
    // set http response headers
    ctx.res.set(Belle::Header::content_type, "text/plain");

    // set the http status code
    ctx.res.result(Belle::Status::ok);

    // set the http body
    ctx.res.body() = "Hello, Belle!";
  });

  // print out the address and port
  // along with the routes
  std::cout
  << "Server: " << address << ":" << port << "\n\n"
  << "Try out the following urls:\n"
  << "  http://127.0.0.1:8080/\n"
  << "  http://127.0.0.1:8080/index.html\n";

  // start the server
  app.listen();

  // the server blocks until a signal is received

  return 0;
}
