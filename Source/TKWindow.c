#include <TKWindow.h>

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

tkwin_error_t tkwin_createWindow(void)
{
    tkwin_error_t error = TKWIN_NO_ERROR;

#ifdef WAYLAND
    error = tkwin_waylandCreateWindow();
#elifdef X11
    // TODO: X11 code
#endif

    return error;
}

void tkwin_destroyWindow(void)
{
#ifdef WAYLAND
    tkwin_waylandDestroyWindow();
#elifdef X11
    // TODO: X11 code
#endif
}

bool tkwin_pollWindow(void)
{
#ifdef WAYLAND
    window.shouldClose = tkwin_waylandPollWindow();
#elifdef X11
    // TODO: X11 code
#endif

    return window.shouldClose;
}

void tkwin_closeWindow(void) { window.shouldClose = true; }
