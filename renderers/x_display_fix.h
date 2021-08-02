/**
 * RPiPlay - An open-source AirPlay mirroring server for Raspberry Pi
 * Copyright (C) 2019 Florian Draschbacher
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */

/* code from David Ventura https://github.com/DavidVentura/UxPlay */

#ifndef X_DISPLAY_FIX_H
#define X_DISPLAY_FIX_H

#ifdef __cplusplus
extern "C" {
#endif

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>

static Display* display;
static Window root, my_window;

void get_x_display_root() {
    display = XOpenDisplay(NULL);
    root = XDefaultRootWindow(display);  
}
  
Window enum_windows(Display* display, Window window, int depth) {
    int i;

    XTextProperty text;
    XGetWMName(display, window, &text);
    char* name;
    XFetchName(display, window, &name);

    if (name != 0 &&  strcmp((const char*) g_get_application_name(), name) == 0) {
        return window;
      }

    Window _root, parent;
    Window* children;
    unsigned int n;
    XQueryTree(display, window, &_root, &parent, &children, &n);
    if (children != NULL) {
      for (i = 0; i < n; i++) {
            Window w = enum_windows(display, children[i], depth + 1);
            if (w) return w;
        }
        XFree(children);
    }

    return (Window) NULL;
}

void fix_x_window_name() {
    if (!my_window) {
        my_window = enum_windows(display, root, 0);
        if (my_window) {
	    const char* str = g_get_application_name();
            Atom _NET_WM_NAME = XInternAtom(display, "_NET_WM_NAME", 0);
            Atom UTF8_STRING = XInternAtom(display, "UTF8_STRING",0);
            XChangeProperty(display, my_window, _NET_WM_NAME, UTF8_STRING, 8, 0, (const unsigned char *) str, strlen(str));
            XSync(display, False);
        }
    }
}


#ifdef __cplusplus
}
#endif

#endif 
