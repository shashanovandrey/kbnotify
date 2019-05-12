# kbnotify
Simple notification (GNOME/Libnotify) of keyboard indicators state:
Keyboard Layout, Caps Lock, Num Lock.

Dependences (Debian): libnotify-bin, libnotify-dev

Build:

    gcc -O2 -s -lX11 `pkg-config --cflags --libs libnotify` -o kbnotify kbnotify.c
