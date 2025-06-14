#ifndef TKWIN_WAYLAND_H
#define TKWIN_WAYLAND_H

#include <TKWindow.h>

awin_error_t awin_waylandCreateWindow(void);
void awin_waylandDestroyWindow(void);
bool awin_waylandPollWindow(void);

#endif // TKWIN_WAYLAND_H
