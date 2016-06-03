# fish-clock
Animated and customisable on-screen clock, with fast Cairo-rendered graphics in C, real X transparency, and socket-driven commands.

By default it comes with 13 ticks, but if I hadn't told you, you wouldn't
have noticed.

Author: Allen Haim <allen@netherrealm.net>, © 2013-2016.
Source: github.com/misterfish/fish-clock
Licence: GPL 2.0

Version: 1.0

You will need the `aosd` package from atheme. On Debian-like systems
`libaosd2` should be enough.

You will also need the perl module `X11::Aosd`. If you have cpanm
(Debian-like: `cpanminus`) installed, `sudo cpanm X11::Aosd` should do it,
though it can be a bit finicky.

And, you probably want some kind of compositing window manager, such as `xcompmgr`. Debian-like: the package and the binary are called `xcompmgr` and there is no need to restart X.

Then:

`./install
./fish-clock -v
`

One way to talk to the socket is to install `socat`. Then:

`echo toggle | socat STDIN UNIX-CONNECT:/tmp/.fishclock-socket`
`echo toggle-fill | socat STDIN UNIX-CONNECT:/tmp/.fishclock-socket`

Edit the config file to taste (no need to restart the binary).

And set up some kind of keyboard shortcut. With the `awesome` window manager, this works, in the keys table:

`   { 'toggle clock',          { modkey,           }, "c", function() awful.util.spawn('socat SYSTEM:"echo toggle; read _" UNIX-CONNECT:/tmp/.fishclock-socket') end },`

`   { 'toggle clock (filled)', { modkey, "Control" }, "c", function() awful.util.spawn('socat SYSTEM:"echo toggle-fill; read _" UNIX-CONNECT:/tmp/.fishclock-socket') end },`



