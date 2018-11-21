# Belle
An HTTP / Websocket library in C++17 using Boost.Beast and Boost.ASIO.

Belle enables C++ programs to communicate asynchronously over HTTP and Websockets.
It aims to have an intuitive API, reasonable defaults, and great performance.

## A Brief Tour
An HTTP server listening on 127.0.0.1:8080
that responds to a POST request to the path '/'.
```cpp
#include "belle.hh"
namespace Belle = OB::Belle;

#include <string>

int main()
{
  // init the server with local address and port
  Belle::Server app {"127.0.0.1", 8080};

  // handle route POST '/'
  app.on_http("/", Belle::Method::post, [](auto& ctx)
  {
    // echo back the request body
    ctx.res.body() = ctx.req.body();
  });

  // start the server
  app.listen();

  return 0;
}
```

An HTTP client connecting to 127.0.0.1:8080
that sends a POST request to the path '/'.
```cpp
#include "belle.hh"
namespace Belle = OB::Belle;

#include <string>
#include <iostream>

int main()
{
  // init the client with remote address and port
  Belle::Client app {"127.0.0.1", 8080};

  // request parameters and response handler
  app.on_http(Belle::Method::post, "/", "Hello, Belle!", [](auto& ctx)
  {
    // print the response body
    std::cout << ctx.res.body() << "\n";
  });

  // start the client
  app.connect();

  return 0;
}
```

## About
Belle is a single header C++17 library for working with HTTP and Websockets.
It utilizes the HTTP and Websocket functionalities of Boost.Beast,
along with the asynchronous networking capabilities of Boost.ASIO.

The design goals for Belle are the following:
* __Intuitive API__: Interactions with the library should be as straightforward and concise as that of a modern scripting language.
* __Reasonable Defaults__: The default state of data and behaviour should be logical and expected, allowing customization when needed.
* __Great Performance__: Speed and memory usage is important and sought after, but not at the cost of diminishing the goals listed above.

There are many HTTP and Websocket libraries currently available, each with their own goals and advantages.
As the networking layer is often a crucial component in your program,
make sure to browse around and select a library that is going to be the most beneficial for you.

This library is undergoing active development and is working its way towards a 1.0.0 release.
Be aware that while the major version is zero, _0.y.z_, the public API should __not__ be considered stable, and is subject to change at any time.

__Belle welcomes and encourages your thoughts and feedback!__

The following lists describe an overview of the current implemented features, along with the planned future features:

### Server Features
* HTTP 1.0 / 1.1 server (async, multithreaded)
* Websocket server (async)
* Serve static content
* Serve dynamic content
* Use lambdas as handlers
* Routes can match a single, multiple, or all HTTP methods
* Use regular expressions with capture groups to match and tokenize routes
* Parse query parameters
* Percent decode query parameters
* Handle HTTP and Websocket on the same port
* Group Websocket connections into channels for broadcasting

### Client Features
* HTTP 1.0 / 1.1 client (async, SSL / TLS)
* Use lambdas as handlers
* Serialize query parameters
* Percent encode query parameters

### Future Features
* Add an async, SSL / TLS Websocket client
* Add SSL / TLS server support
* add Websocket ping, pong, and timeout
* Add an interface for a custom logger
* Add connection manager for clean shutdown
* Chunked encoding support
* Request and response streaming support
* General optimizations to improve performance

## Install
Belle is a single header file.
It can either be installed into your systems include directory,
or copied directly into your projects directory.

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
### Environment
* Linux (supported)
* BSD (untested)
* macOS (untested)
* Windows (untested)

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

If Belle is copied to your project directory:
```cpp
#include "belle.hh"
namespace Belle = OB::Belle;
```

Make sure to link the __Dependencies__, along with meeting the __Requirements__ listed above.

## Documentation
The source code contains helpful comments and explanations.
A detailed set of documentation and guides are planned for the future.

Basic documentation can be generated using Doxygen:
```
$ doxygen ./.doxyfile
```

## Examples
There are several examples located in the `./example` directory.
Each example has its own CMake build file, and can be built in either __Debug__ or __Release__ mode by passing the appropriate CMake flag, `-DCMAKE_BUILD_TYPE=Debug` or `-DCMAKE_BUILD_TYPE=Release`.
Several examples rely on relative paths, therefore, they expect to be executed
from a specific path. The example programs should be executed in their respective build directory.

Within an examples directory, the following will build and run it in Debug mode.
```
$ mkdir -p ./build
$ cd ./build
$ cmake ../ -DCMAKE_BUILD_TYPE=Debug
$ make
$ ./app
```

The examples depend on the Belle source file in the `./include` directory.
To use one of the examples as a starting point, copy the selected example
to a new directory, and then follow the __Install__ and __Build__
instructions listed above.

### Server
* __hello__: an HTTP server with a get endpoint, post endpoint, and static file handling
* __http__: a multithreaded HTTP server with multiple endpoints, static file handling, signal handling, and error handling
* __chat__: a single threaded HTTP / Websocket chat room server with multiple endpoints, static file handling, signal handling, and error handling, with a basic html/css/js client interface

### Client
* __http__: an HTTP client that makes multiple requests to a remote endpoint
* __https__: an HTTPS client that makes multiple requests to a remote endpoint

## Tests
There are currently no tests at this time, but there are plans to add them in the future.

## Benchmarks
There are currently no benchmarks at this time, but there are plans to add them in the future.

## Versioning
This project uses [Semantic Versioning](https://semver.org/).

## License
This project is licensed under the [MIT License](https://opensource.org/licenses/MIT).

Copyright (c) 2018 [Brett Robinson](https://octobanana.com/)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
