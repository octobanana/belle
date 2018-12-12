var global =
{
  ws:
  {
    val: null,
    timeout:
    {
      val: 1,
      step: 2,
      max: 90,
      total: 0
    },
    retry:
    {
      val: 0,
      max: 10
    }
  },
  feed_max: 64,
  rooms: {}
};

window.onload = function()
{
  websocket();
};

var plural = function(str, end, num)
{
  num = Number(num);
  if (num === 1) return str;
  return str + end;
};

var el = function(tag, attr, text, children)
{
  let root = document.createElement(tag);

  if (text.length > 0)
  {
    let t = document.createTextNode(text);
    root.appendChild(t);
  }

  for (e of attr)
  {
    if (e.length == 1)
    {
      root.setAttribute(e[0], '');
    }
    else if (e.length == 2)
    {
      root.setAttribute(e[0], e[1]);
    }
  }

  for (e of children)
  {
    if (typeof e === 'string' || e instanceof String)
    {
      root.innerHTML += e;
    }
    else
    {
      root.appendChild(e);
    }
  }

  return root;
}

var card = function(room, count)
{
  let el_card =
  el('div', [['id', room], ['class', 'item']], '', [
    el('a', [['class', 'item-link'], ['href', room]], '', [
      el('div', [['class', 'item-body']], '[' + count + '] ' + room, [
      ]),
    ])
  ]);

  return el_card;
}

var websocket = function()
{
  global.ws.val = new WebSocket((location.protocol == 'https:' ? 'wss:' : 'ws:') + '//' + location.hostname + (location.port ? ':' + location.port : '') + location.pathname);

  global.ws.val.onopen = function()
  {
    // reset values
    global.ws.retry.val = 0;
    global.ws.timeout.total = 0;
    global.ws.timeout.val = 1;
  };

  global.ws.val.onmessage = function(e)
  {
    let type = Number(e.data[0]);
    let res = e.data.substr(1);

    switch (type)
    {
      // update room count
      case 0:
      {
        let room_count = document.getElementById('room-count');
        let rooms = document.getElementById('rooms');
        let count = Number(res);
        room_count.innerHTML = count;
        rooms.innerHTML = plural('room', 's', count);
      }
      break;

      // update user count
      case 1:
      {
        let user_count = document.getElementById('user-count');
        let users = document.getElementById('users');
        let count = Number(res);
        user_count.innerHTML = count;
        users.innerHTML = plural('user', 's', count);
      }
      break;

      // update room card
      case 2:
      {
        let data = res.split('/', 2);
        let count = Number(data[0]);
        let room = '/' + data[1];

        global.rooms[room] = {room: room, count: count};
        let arr = [];
        for (e in global.rooms)
        {
          arr.push(global.rooms[e]);
        }
        arr.sort((lhs, rhs) => {
          if (lhs.count < rhs.count) return -1;
          if (lhs.count > rhs.count) return 1;
          return 0;
        });

        let feed = document.getElementById('feed');
        while (feed.firstChild)
        {
          feed.removeChild(feed.firstChild);
        }
        for (let i = 0; i < arr.length; ++i)
        {
          feed.appendChild(card(arr[i].room, arr[i].count));
        }
      }
      break;

      // invalid type
      default:
      break;
    }
  };

  global.ws.val.onclose = function(e)
  {
    if (global.ws.retry.val > 0)
    {
      global.ws.timeout.val = Math.floor(global.ws.timeout.val * global.ws.timeout.step);
    }

    global.ws.retry.val += 1;

    if (global.ws.timeout.val > global.ws.timeout.max)
    {
      global.ws.timeout.val = global.ws.timeout.max;
    }

    if (global.ws.retry.val > global.ws.retry.max)
    {
      return;
    }

    global.ws.timeout.total += global.ws.timeout.val;

    setTimeout(function()
    {
      websocket();
    }, global.ws.timeout.val * 1000);
  };

  global.ws.val.onerror = function(e)
  {
    global.ws.val.close();
  };
}
