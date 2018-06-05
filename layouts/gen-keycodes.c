#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>
#include <xcb/xcb.h>
#include <xcb/xcb_event.h>

char chrs[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";

int main(int argc, char **argv)
{
    int ind;
    xcb_connection_t *xconn;

    Display *dpy = XOpenDisplay(NULL);

    xconn = XGetXCBConnection(dpy);

    for (ind = 0; ind < sizeof(chrs); ++ind)
    {
        char str[2];
        KeySym sym;
        KeyCode code;
        str[0] = chrs[ind];
        str[1] = '\0';
        sym = XStringToKeysym(str);
        code = XKeysymToKeycode(dpy, str[0]);
        printf("%d\n", code);
    }
    XCloseDisplay(dpy);
}