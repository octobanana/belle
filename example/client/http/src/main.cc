// belle client http example

#include "belle.hh"
namespace Belle = OB::Belle;

#include <string>
#include <iostream>

// prototypes
void on_http_error(Belle::Client& app);
void http_get();
void http_post();

void on_http_error(Belle::Client& app)
{
  // set the http on error callback
  app.on_http_error([](auto& ctx)
  {
    std::cerr << "Error: " << ctx.ec.message() << "\n\n";
  });
}

void http_get()
{
  // init client with remote address and port
  Belle::Client app {"127.0.0.1", 8080};
  on_http_error(app);

  // http get request to path '/'
  app.on_http("/", [](auto& ctx)
  {
    // check http status code
    if (ctx.res.result() != Belle::Status::ok)
    {
      // print the response status code and reason
      std::cerr
      << "Error: " << ctx.res.result_int()
      << " " << ctx.res.reason() << "\n\n";

      return;
    }

    // print the response headers and body
    std::cerr << ctx.res.base() << ctx.res.body() << "\n\n";
  });

  // http get request to path '/method'
  app.on_http("/method", [](auto& ctx)
  {
    // check http status code
    if (ctx.res.result() != Belle::Status::ok)
    {
      // print the response status code and reason
      std::cerr
      << "Error: " << ctx.res.result_int()
      << " " << ctx.res.reason() << "\n\n";

      return;
    }

    // print the response headers and body
    std::cerr << ctx.res.base() << ctx.res.body() << "\n\n";
  });

  // init an http request object
  Belle::Request req;

  // set the target path
  req.target("/regex/hello");

  // set the method
  req.method(Belle::Method::get);

  // copy the request object
  app.on_http(req, [](auto& ctx)
  {
    // check http status code
    if (ctx.res.result() != Belle::Status::ok)
    {
      // print the response status code and reason
      std::cerr
      << "Error: " << ctx.res.result_int()
      << " " << ctx.res.reason() << "\n\n";

      return;
    }

    // print the full response
    std::cerr << ctx.res << "\n\n";
  });

  // set the target path
  req.target("/params");

  // set the query parameters
  req.params().emplace("pi", "π");
  req.params().emplace("page", "2");
  req.params().emplace("user", "François");
  req.params().emplace("q", "Hello,\nBelle!");
  req.params().emplace("q", "t#st spec!&l ch@r*ct=rs");

  // move the request object with Request::move
  // can also be moved using std::move
  app.on_http(req.move(), [](auto& ctx)
  {
    // check http status code
    if (ctx.res.result() != Belle::Status::ok)
    {
      // print the response status code and reason
      std::cerr
      << "Error: " << ctx.res.result_int()
      << " " << ctx.res.reason() << "\n\n";

      return;
    }

    // print the full response
    std::cerr << ctx.res << "\n\n";
  });

  // save the number of requests in the queue
  auto total = app.queue().size();

  // start the client and save the number of completed requests
  auto completed = app.connect();

  // print the number of completed requests
  std::cerr << "connect: " << completed << "/" << total << "\n\n";
}

void http_post()
{
  // init client with remote address and port
  Belle::Client app {"127.0.0.1", 8080};
  on_http_error(app);

  // set http headers
  Belle::Headers headers;
  headers.set(Belle::Header::user_agent, "Belle");
  headers.set(Belle::Header::content_type, "text/plain");

  // request parameters and response handler
  app.on_http(Belle::Method::post, "/post", headers, "Hello, Belle!", [](auto& ctx)
  {
    // check http status code
    if (ctx.res.result() != Belle::Status::ok)
    {
      // print the response status code and reason
      std::cerr
      << "Error: " << ctx.res.result_int()
      << " " << ctx.res.reason() << "\n\n";

      return;
    }

    // print the response body
    std::cerr << ctx.res.body() << "\n\n";
  });

  // save the number of requests in the queue
  auto total = app.queue().size();

  // start the client and save the number of completed requests
  auto completed = app.connect();

  // print the number of completed requests
  std::cerr << "connect: " << completed << "/" << total << "\n\n";
}

int main(int argc, char *argv[])
{
  // perform multiple http get requests to a single remote endpoint
  http_get();

  // perform an http post request to a single remote endpoint
  http_post();

  return 0;
}
