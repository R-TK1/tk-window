#ifndef TKWIN_WAYLAND_H
#define TKWIN_WAYLAND_H

#include <TKWindow.h>

tkwin_error_t tkwin_waylandCreateWindow(void);
void tkwin_waylandDestroyWindow(void);
bool tkwin_waylandPollWindow(void);

#endif // TKWIN_WAYLAND_H
