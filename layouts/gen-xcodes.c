#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>
#include <xcb/xcb.h>
#include <xcb/xcb_event.h>

char chrs[] =
    "0123456789"
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";

int main(int argc, char **argv)
{
    int ind;
    xcb_connection_t *xconn;

    Display *disp = XOpenDisplay(NULL);

    xconn = XGetXCBConnection(disp);

    for (ind = 0; ind < sizeof(chrs) - 1; ++ind)
    {
        char c;
        KeyCode code;

        c = chrs[ind];

        code = XKeysymToKeycode(disp, c);
        printf("%c %d\n", c, code);
    }

    XCloseDisplay(disp);
}