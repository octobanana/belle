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
  feed_max: 64
};

var init_data = function()
{
  document.title = location.pathname + ' | Chat++';

  let room = document.getElementById('room');
  room.innerHTML = location.pathname;
  room.setAttribute('href', location.pathname);
};

window.onload = function()
{
  init_data();
  websocket();
  handlers();
  messages();
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

var card = function(msg)
{
  let el_card =
  el('div', [['class', 'item']], '', [
    el('div', [['class', 'item-body']], msg, []),
    el('hr', [], '', [])
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

    // remove all old messages
    let feed = document.getElementById('feed');
    feed.innerHTML = '';

    // append connect message
    feed.appendChild(card("> Connected To Server"));

    // enable submit button
    let submit = document.getElementById('submit');
    submit.removeAttribute('disabled');
    submit.innerHTML = 'submit';
  };

  global.ws.val.onmessage = function(e)
  {
    let type = Number(e.data[0]);
    let res = e.data.substr(1);

    switch (type)
    {
      // new message
      case 0:
      {
        let feed = document.getElementById('feed');
        feed.appendChild(card(res));

        let items = document.querySelectorAll('.item');
        if (items.length > global.feed_max)
        {
          let num = items.length - global.feed_max;
          for (let i = 0; i < num; ++i)
          {
            items[i].parentNode.removeChild(items[i]);
          }
        }
      }
      break;

      // update user count
      case 1:
      {
        let count = document.getElementById('count');
        count.innerHTML = res;
        let users = document.getElementById('users');
        users.innerHTML = plural('user', 's', res);
      }
      break;

      // invalid type
      default:
      break;
    }
  };

  global.ws.val.onclose = function(e)
  {
    let submit = document.getElementById('submit');

    if (submit.getAttribute('disabled') == null)
    {
      // disable submit button
      submit.setAttribute('disabled', '');
      submit.innerHTML = 'disconnected';

      // append disconnect message
      let feed = document.getElementById('feed');
      feed.appendChild(card("> Disconnected From Server"));
    }

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

var handlers = function()
{
  let input = document.getElementById('msg');
  input.addEventListener('input', function(e) {
    input.setCustomValidity('');
  });
}

var messages = function()
{
  let input = document.getElementById('msg');
  let submit = document.getElementById('submit');

  submit.onclick = function(e)
  {
    e.preventDefault();

    let s = input.value.trim();

    if (! input.reportValidity() || s.length === 0)
    {
      input.setCustomValidity('Invalid, a comment must contain at least one character');
      input.value = '';
      input.focus();
      return false;
    }

    let req = '0' + s;
    global.ws.val.send(req);

    input.value = '';
    input.focus();
  }
}
