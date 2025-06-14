#ifndef AWIN_WAYLAND_H
#define AWIN_WAYLAND_H

#include <TKWin.h>

awin_error_t awin_waylandCreateWindow(void);
void awin_waylandDestroyWindow(void);
bool awin_waylandPollWindow(void);

#endif // AWIN_WAYLAND_H
