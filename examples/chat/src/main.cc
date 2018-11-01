// belle chat example

#include "../../../include/belle.hh"
namespace Belle = OB::Belle;

#include <ctime>

#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <deque>

using namespace std::string_literals;

// basic ring buffer
template<class T>
class Ringbuf
{
public:

  Ringbuf(size_t size = 64):
    _size {size}
  {
  }

  ~Ringbuf()
  {
  }

  Ringbuf& push(T const& t)
  {
    _que.emplace_back(t);

    if (_que.size() > _size)
    {
      _que.pop_front();
    }

    return *this;
  }

  Ringbuf& shrink_to_fit()
  {
    while (_que.size() > _size)
    {
      _que.pop_front();
    }

    return *this;
  }

  std::deque<T> const& get() const
  {
    return _que;
  }

  size_t max_size() const
  {
    return _size;
  }

  Ringbuf max_size(size_t size)
  {
    _size = size;

    if (_que.size() > _size)
    {
      shrink_to_fit();
    }

    return *this;
  }

  Ringbuf& clear()
  {
    _que.clear();

    return *this;
  }

  size_t size() const
  {
    return _que.size();
  }

  bool empty() const
  {
    return _que.empty();
  }

  T& operator[](size_t n)
  {
    return _que.at(n);
  }

  T const& operator[](size_t n) const
  {
    return _que.at(n);
  }

  T& at(size_t n)
  {
    return _que.at(n);
  }

  T const& at(size_t n) const
  {
    return _que.at(n);
  }

private:

  size_t _size;
  std::deque<T> _que;
}; // class Ringbuf

// convert object into a string
template<class T>
std::string to_string(T t)
{
  std::stringstream ss;
  ss << t;

  return ss.str();
}

// read a file into a string
std::optional<std::string> file(std::string const& str);
std::optional<std::string> file(std::string const& str)
{
  std::ifstream file {str};

  if (! file.is_open())
  {
    return {};
  }

  file.seekg(0, std::ios::end);
  size_t size (static_cast<size_t>(file.tellg()));
  std::string content (size, ' ');
  file.seekg(0);
  file.read(&content[0], static_cast<std::streamsize>(size));

  return content;
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

  // enable serving static files from a public directory
  // if the path is relative, make sure to run the program
  // in the right working directory
  app.public_dir("../public");

  // serve static content from public dir
  // default value is true
  app.http_static(true);

  // serve dynamic content
  // default value is true
  app.http_dynamic(true);

  // accept websocket upgrade requests
  // default value is true
  app.websocket(true);

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

  // store received messages
  std::unordered_map<std::string, Ringbuf<std::string>> chat;

  // total number of connected users
  int user_count {0};

  // init some default channels
  app.channels()["/"] = Belle::Server::Channel();
  app.channels()["/new"] = Belle::Server::Channel();

  // add some default messages
  chat["/new"].push("'/' shows an overview of the rooms");
  chat["/new"].push("'/<room_name>' go to an existing room or create a new one");
  chat["/new"].push("Try creating a new room called '/dev'");
  chat["/new"].push("The index page, '/', will now show 3 rooms");
  chat["/new"].push("The most popular room gets moved to the top of the index, try playing around with several tabs open");
  chat["/new"].push("The newest comments appear at the top of the page");

  // handle ws connections to index room '/'
  app.on_websocket("/",
  // on data function: called after every websocket read
  [&](Belle::Server::Websocket_Ctx& ctx)
  {
    // register the route
    // data will be broadcasted in the websocket connect and disconnect handlers
  });

  // handle ws connections to chat rooms '/<chat_room>'
  app.on_websocket("^(/[a-z]+)$",
  // on begin function: called once after connected
  [&](Belle::Server::Websocket_Ctx& ctx)
  {
    // broadcast the total number of connected users to the channel
    ctx.channels.at(ctx.req.url().at(0)).broadcast("1" + std::to_string(ctx.channels.at(ctx.req.url().at(0)).size()));

    // check if there is any messages stored
    if (chat.find(ctx.req.url().at(0)) != chat.end())
    {
      // send out all previous messages to new user
      for (auto const& e : chat[ctx.req.url().at(0)].get())
      {
        ctx.send("0" + e);
      }
    }

    // send welcome message
    ctx.send("0"s + "> welcome to "s + ctx.req.url().at(0));
  },

  // on data function: called after every websocket read
  [&](Belle::Server::Websocket_Ctx& ctx)
  {
    // a simple protocol:
    // in the received message,
    // the first character holds an int from 0-9,
    // the remaining characters are the message

    // get the message type
    int type {std::stoi(to_string(ctx.msg.at(0)))};

    // determine action
    switch (type)
    {
      case 0:
      chat[ctx.req.url().at(0)].push(ctx.msg.substr(1));
      ctx.channels.at(ctx.req.url().at(0)).broadcast("0" + ctx.msg.substr(1));
      break;

      default:
      break;
    }
  },

  // on end function: called once after disconnected
  [&](Belle::Server::Websocket_Ctx& ctx)
  {
    // a user has disconnected
    // broadcast the total number of connected users to the channel
    ctx.channels.at(ctx.req.url().at(0))
      .broadcast("1" + std::to_string(ctx.channels.at(ctx.req.url().at(0)).size()));
  }
  );

  // called once at the very beginning after connected
  app.on_websocket_connect([&](Belle::Server::Websocket_Ctx& ctx)
  {
    // increase total user count
    ++user_count;

    // send room count
    ctx.channels.at("/").broadcast("0" + std::to_string(ctx.channels.size()));
    // send user count
    ctx.channels.at("/").broadcast("1" + std::to_string(user_count));
    for (auto const& e : ctx.channels)
    {
      // send count and room info
      ctx.channels.at("/").broadcast("2" + std::to_string(e.second.size()) + e.first);
    }
  });

  // called once at the very end after disconnected
  app.on_websocket_disconnect([&](Belle::Server::Websocket_Ctx& ctx)
  {
    // decrease total user count
    --user_count;

    // send room count
    ctx.channels.at("/").broadcast("0" + std::to_string(ctx.channels.size()));
    // send user count
    ctx.channels.at("/").broadcast("1" + std::to_string(user_count));
    for (auto const& e : ctx.channels)
    {
      // send count and room info
      ctx.channels.at("/").broadcast("2" + std::to_string(e.second.size()) + e.first);
    }
  });

  // handle route GET '/'
  // with no set dynamic route for a route ending in a '/' character,
  // the default action is to look for a static file named 'index.html'
  // in the corresponding public directory

  // handle route GET '/<chat_room>'
  app.on_http("^(/[a-z]+)$", Belle::Method::get, [&](Belle::Server::Http_Ctx& ctx)
  {
    // set http response headers
    ctx.res.set("content-type", "text/html");

    // send the file contents
    if (auto res = file(app.public_dir() + "/chat.html"))
    {
      ctx.res.body() = std::move(res.value());
    }
    else
    {
      throw 404;
    }
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

  // print out the address and port
  std::cout
  << "Server: " << address << ":" << port << "\n\n"
  << "Navigate to the following url:\n"
  << "  http://" << address << ":" << port << "/new\n\n";

  // start the server
  app.listen();

  return 0;
}
