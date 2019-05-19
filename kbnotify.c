/*
Simple notification of keyboard indicators state:
Keyboard Layout, Caps Lock, Num Lock.
Thanks to everyone for their help and code examples.
Andrey Shashanov (2019)
gcc -O2 -s -lX11 `pkg-config --cflags --libs gio-2.0` -o kbnotify kbnotify.c
*/

#include <stdio.h>
#include <unctrl.h>
#include <string.h>
#include <X11/XKBlib.h>
#include <gio/gio.h>

int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused)))
{
    GDBusProxyFlags flags;
    GDBusProxy *proxy;
    Display *dpy;
    int dummy;
    uint32_t dbus_id = 0;

    if ((dpy = XOpenDisplay(NULL)) == NULL ||
        !XkbQueryExtension(dpy, &dummy, &dummy, &dummy, &dummy, &dummy))
        return 1;

    flags = G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES |
            G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS;

    proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION,
                                          flags,
                                          NULL,
                                          "org.freedesktop.Notifications",
                                          "/org/freedesktop/Notifications",
                                          "org.freedesktop.Notifications",
                                          NULL,
                                          NULL);

    if (proxy == NULL)
        return 1;

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
        GVariantBuilder *hints;
        GVariant *res;

        XkbGetState(dpy, XkbUseCoreKbd, &state);

        xkb = XkbGetKeyboard(dpy, XkbAllComponentsMask, XkbUseCoreKbd);
        summary = XGetAtomName(dpy, xkb->names->groups[state.group]);

        XkbGetIndicatorState(dpy, XkbUseCoreKbd, &indicator);

        body[0] = '\0';
        if (indicator & 1U)
            strcat(body, "Caps Lock    ");
        if (indicator & 2U)
            strcat(body, "Num Lock");

        hints = g_variant_builder_new((const GVariantType *)"a{sv}");
        g_variant_builder_add(hints, "{sv}",
                              "urgency", g_variant_new_byte(0));
        g_variant_builder_add(hints, "{sv}",
                              "transient", g_variant_new_boolean(TRUE));

        res = g_dbus_proxy_call_sync(proxy,
                                     "Notify",
                                     g_variant_new("(susssasa{sv}i)",
                                                   "",
                                                   dbus_id,
                                                   "input-keyboard",
                                                   summary,
                                                   body,
                                                   NULL,
                                                   hints,
                                                   -1),
                                     G_DBUS_CALL_FLAGS_NONE,
                                     -1,
                                     NULL,
                                     NULL);

        g_variant_builder_unref(hints);

        if (res != NULL)
        {
            if (g_variant_is_of_type(res, (const GVariantType *)"(u)"))
                g_variant_get(res, "(u)", &dbus_id);
            else
                dbus_id = 0;

            g_variant_unref(res);
        }
        else
            dbus_id = 0;

        XFree(summary);
        XkbFreeKeyboard(xkb, XkbAllComponentsMask, True);

        XNextEvent(dpy, &xev);
    }

    /* code will never be executed */
    XkbSelectEvents(dpy, XkbUseCoreKbd, XkbIndicatorStateNotifyMask, 0);
    XCloseDisplay(dpy);
    g_object_unref(proxy);
    return 0;
}
