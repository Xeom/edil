#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>
#include <X11/XKBlib.h>
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
        char c, ccase;
        KeyCode code;
        KeySym  upper, lower;

        c = chrs[ind];

        code = XKeysymToKeycode(disp, c);

        upper = XkbKeycodeToKeysym(disp, code, 0, 1);
        lower = XkbKeycodeToKeysym(disp, code, 0, 0);

        if (c == upper) ccase = 'u';
        else if (c == lower) ccase = 'l';
        else printf("I don't know what to do with '%c' (%d)", c, code);


        printf("%c %d%c\n", c, code, ccase);
    }

    XCloseDisplay(disp);
}