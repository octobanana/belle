# Belle
A small http web server library in c++ using Boost Beast.  
Belle allows c++ apps to communicate over http using a simple api.  
It currently is not ment to be an edge server, as https is not yet implemented. The server is intended to run on localhost behind a reverse http proxy server, such as nginx, that would handle the outgoing https connection.  

### Features
* Simple api
* Serves static files
* Set and get http headers
* Set and get http body
* Use regular expressions to match url routes
* Routes can match a single, multiple, or any http methods.

### Dependencies
Requires Boost v1.66  
* Boost Beast
* Boost System
* Boost Filesystem

## Usage
The hello world of Belle:  
```cpp
#include "belle.hh"

int main()
{
  // init the server
  OB::Belle::Http app;

  // handle route GET '/'
  app.route("/", OB::Belle::Method::get, [](OB::Belle::Ctx& ctx) {
    auto& res = ctx.res;

    res.set("connection", "keep-alive");
    res.set("content-type", "text/plain");
    res.result(200);

    res.body = "Hello, World!";
  });

  // start the server
  app.listen("127.0.0.1", 8080);

  return 0;
}
```

## Examples
See the examples folder.  

## Build
Run the build script  
`$ ./build.sh -r`

## Install
Run the install script  
`$ ./install.sh -r`
