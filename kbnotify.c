/*
Simple notification (GNOME/Libnotify) of keyboard indicators state:
Keyboard Layout, Caps Lock, Num Lock.
Thanks to everyone for their help and code examples.
Andrey Shashanov (2019)
Dependences (Debian): libnotify-bin, libnotify-dev
gcc -O2 -s -lX11 `pkg-config --cflags --libs libnotify` -o kbnotify kbnotify.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/XKBlib.h>
#include <libnotify/notify.h>

/* get short program name */
char *program_name(char *s)
{
    char *p;
    if (s == NULL)
        return NULL;
    for (p = s; *s; ++s)
        if (*s == '/')
            p = s + 1U;
    return p;
}

int main(int argc __attribute__((unused)), char *argv[])
{
    Display *dpy;
    NotifyNotification *notification;
    int dummy;

    if ((dpy = XOpenDisplay(NULL)) == NULL)
    {
        fprintf(stderr, "%s: Error open DISPLAY\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (!XkbQueryExtension(dpy, &dummy, &dummy, &dummy, &dummy, &dummy))
    {
        fprintf(stderr, "%s: XKB extension is not present\n", argv[0]);
        return EXIT_FAILURE;
    }

    notify_init(program_name(argv[0]));
    notification = notify_notification_new(NULL, NULL, NULL);
    notify_notification_set_urgency(notification, NOTIFY_URGENCY_LOW);
    notify_notification_set_hint(notification,
                                 "transient",
                                 g_variant_new_boolean(TRUE));

    XkbSelectEvents(dpy, XkbUseCoreKbd, XkbIndicatorStateNotifyMask,
                    XkbIndicatorStateNotifyMask);

    for (;;)
    {
        XkbStateRec state;
        XkbDescPtr xkb;
        XEvent xev;
        unsigned int indicator;
        char *summary;
        char body[64];

        XkbGetState(dpy, XkbUseCoreKbd, &state);

        xkb = XkbGetKeyboard(dpy, XkbAllComponentsMask, XkbUseCoreKbd);
        summary = XGetAtomName(dpy, xkb->names->groups[state.group]);

        XkbGetIndicatorState(dpy, XkbUseCoreKbd, &indicator);

        body[0] = '\0';
        if (indicator & 1U)
            strcat(body, "Caps Lock  ");
        if (indicator & 2U)
            strcat(body, "Num Lock");

        notify_notification_update(notification,
                                   summary,
                                   body,
                                   "input-keyboard");
        notify_notification_show(notification, NULL);

        XFree(summary);
        XkbFreeKeyboard(xkb, XkbAllComponentsMask, True);

        XNextEvent(dpy, &xev);
    }

    /* code will never be executed */
    XkbSelectEvents(dpy, XkbUseCoreKbd, XkbIndicatorStateNotifyMask, 0);
    XCloseDisplay(dpy);
    notify_uninit();
    return EXIT_SUCCESS;
}
