#include <TKWin.h>

#ifdef WAYLAND
#include <Targets/Wayland.h>
#elifdef X11
#include <Targets/X11.h>
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

bool awin_pollWindow(void)
{
#ifdef WAYLAND
    window.shouldClose = awin_waylandPollWindow();
#elifdef X11
    // TODO: X11 code
#endif

    return window.shouldClose;
}

void awin_closeWindow(void) { window.shouldClose = true; }
