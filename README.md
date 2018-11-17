# Belle
An HTTP / Websocket library in C++17 using Boost.Beast and Boost.ASIO.

Belle enables C++ programs to communicate asynchronously over HTTP and Websockets.
It aims to have an intuitive API, reasonable defaults, and great performance.

## A Brief Tour
An HTTP server listening on 127.0.0.1:8080
that responds to a GET request to the path '/'.
```cpp
#include "belle.hh"
namespace Belle = OB::Belle;

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

## About
Belle is a single header C++17 library for working with HTTP and Websockets.
It utilizes the HTTP and Websocket functionalities of Boost.Beast,
along with the asynchronous networking capabilities of Boost.ASIO.

### Features
* HTTP 1.0 / 1.1 server
* HTTP 1.0 / 1.1 client
* Websocket server
* Serve static content
* Serve dynamic content
* Use lambdas as route handlers
* Routes can match a single, multiple, or all HTTP methods
* Use regular expressions with capture groups to match URL
* URL query parameters are automatically parsed
* Handle HTTP and Websocket requests on the same port
* Group Websocket connections into channels for broadcasting
* Multithreaded HTTP server

### Future Features
* Add HTTP 1.1 client support
* Add an async Websocket client
* Add SSL / TLS server support
* add Websocket ping, pong, and timeout
* Add an interface for a custom logger
* Chunked encoding support
* Request and response streaming support
* Add a generic TCP interface
* General optimizations to improve performance

## Install
Belle is a single header file.
It can either be installed into your systems include directory,
or copied directly into your projects folder.

The default install location is `/usr/local/include/ob/belle.hh`.

To install on your system:
```
$ ./install.sh
```

To copy to your project:
```
$ cp ./include/belle.hh ./<project>
```

## Build
### Requirements
* C++17 compiler
* Boost >= 1.67
* OpenSSL >= 1.1.0 (if SSL is enabled)
* CMake >= 3.8 (to build examples)

### Dependencies
* ssl (libssl)
* crypto (libcrypto)
* pthread (libpthread)
* boost (libboost_system)

### Flags
Use the following define flags at compile time to alter the library:
* -D OB_BELLE_CONFIG_SSL_OFF
  * Disable SSL support
* -D OB_BELLE_CONFIG_CLIENT_OFF
  * Disable client support
* -D OB_BELLE_CONFIG_SERVER_OFF
  * Disable server support

### Usage
If Belle is installed on your system:
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

## Documentation
The source code contains helpful comments and explanations.

Basic documentation can be generated using Doxygen:
```
$ doxygen ./.doxyfile
```

## Examples

There are several examples in the `./examples` folder:
* __hello__: a basic HTTP server with a single dynamic endpoint and static file handling
* __http__: a multithreaded HTTP server with multiple dynamic endpoints, static file handling, signal handling, and error handling
* __chat__: a single threaded HTTP / Websocket chat room server with multiple dynamic endpoints, static file handling, signal handling, and error handling, with a basic html/css/js client interface

Each example has its own CMake build file.
