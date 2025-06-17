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

bool tkwin_create(void)
{
#ifdef WAYLAND
    if (!tkwin_waylandCreate()) return false;
#elifdef X11
    // TODO: X11 code
#endif

    return true;
}

void tkwin_destroy(void)
{
#ifdef WAYLAND
    tkwin_waylandDestroy();
#elifdef X11
    // TODO: X11 code
#endif
}

bool tkwin_poll(void)
{
#ifdef WAYLAND
    window.shouldClose = tkwin_waylandPoll();
#elifdef X11
    // TODO: X11 code
#endif

    return window.shouldClose;
}

void tkwin_close(void) { window.shouldClose = true; }

void tkwin_getFramebufferSize(uint32_t *width, uint32_t *height)
{
#ifdef WAYLAND
    tkwin_waylandGetFramebufferSize(width, height);
#elifdef X11
    (void)width;
    (void)height;
    // TODO: X11 code
#endif
}

void tkwin_getSurfaceData(void **data)
{
#ifdef WAYLAND
    tkwin_waylandGetSurfaceData(data);
#elifdef X11
    (void)data;
    // TODO: X11 code
#endif
}
