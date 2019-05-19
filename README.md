# kbnotify
Simple notification of keyboard indicators state:
Keyboard Layout, Caps Lock, Num Lock.

Build:

    gcc -O2 -s -lX11 `pkg-config --cflags --libs gio-2.0` -o kbnotify kbnotify.c
