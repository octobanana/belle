# Belle
An HTTP / Websocket library in C++17 using Boost.Beast and Boost.ASIO.

Belle enables C++ programs to communicate asynchronously over HTTP and Websockets.
It aims to have an intuitive API, reasonable defaults, and great performance.

## A Brief Tour
The hello world of Belle:
```cpp
#include "belle.hh"
namespace Belle = OB::Belle;

#include <string>

int main()
{
  // init the server
  Belle::Server app;

  // handle route GET '/'
  app.on_http("/", Belle::Method::get, [](auto& ctx)
  {
    // set http response headers
    ctx.res.set(Belle::Header::content_type, "text/plain");

    // set the http status code
    ctx.res.result(Belle::Status::ok);

    // set the http body
    ctx.res.body() = "Hello, Belle!";
  });

  // start the server
  app.listen("127.0.0.1", 8080);

  return 0;
}
```

### Features
* HTTP 1.0 / 1.1 server
* Websocket server
* Intuitive API
* Serve static content
* Serve dynamic content
* Use lambdas as route handlers
* Routes can match a single, multiple, or all HTTP methods
* Use regular expressions with capture groups to match URL
* URL query parameters are automatically parsed
* Handle HTTP and Websocket requests on the same port
* Group Websocket connections into channels for broadcasting
* Multithreaded (only for HTTP server)

### Requirements
* C++17 compiler
* boost >= 1.67
* cmake >= 3.5 (to build examples)

### Dependencies
* pthread (libpthread)
* boost (libboost_system)

## Build
Belle is a single header file.
It can either be installed into your systems include directory, or copied directly into your projects folder.

## Install
The default install location is `/usr/local/include/ob/belle.hh`.
Run the install script:  
`$ ./install.sh`

## Usage
If Belle is installed to your system:  
```cpp
#include <ob/belle.hh>
namespace Belle = OB::Belle;
```

If Belle is copied to your project folder:  
```cpp
#include "belle.hh"
namespace Belle = OB::Belle;
```

Make sure to link the dependencies, along with meeting the requirements listed above.

## Documentation and Examples
Basic documentation can be generated using Doxygen:  
`$ doxygen ./.doxyfile`

There are several examples in the `./examples` folder:
* __hello__: a basic HTTP server with a single dynamic endpoint and static file handling
* __http__: a multithreaded HTTP server with multiple dynamic endpoints, static file handling, signal handling, and error handling
* __chat__: a single threaded HTTP / Websocket chat room server with multiple dynamic endpoints, static file handling, signal handling, and error handling, with a basic html/css/js client interface

Each example has its own CMake build file.

### Future Features
* Chunked encoding support
* Add SSL / TLS support
* Add an interface for a custom logger
* Add an async and sync HTTP / Websocket clients
* Add a generic TCP interface with callbacks
* Take greater advantage of Boost.Beast
* General optimizations to improve performance
