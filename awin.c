#include "awin.h"

#ifdef WAYLAND
#include <wayland.h>
#elifdef X11
#include <x11.h>
#endif

struct
{
    const char *title;
    bool shouldClose;
} window = {nullptr, true};

awin_error_t awin_createWindow(void)
{
    awin_error_t error = AWIN_NO_ERROR;

#ifdef WAYLAND
    error = awin_waylandCreateWindow();
#elifdef X11
    // TODO: X11 code
#endif

    return error;
}

void awin_destroyWindow(void)
{
#ifdef WAYLAND
    awin_waylandDestroyWindow();
#elifdef X11
    // TODO: X11 code
#endif
}

bool awin_pollWindow(void) { return window.shouldClose; }
