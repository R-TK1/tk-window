#ifndef TKWIN_WAYLAND_H
#define TKWIN_WAYLAND_H

#include <TKWindow.h>

tkwin_error_t tkwin_waylandCreate(void);
void tkwin_waylandDestroy(void);
bool tkwin_waylandPoll(void);

#endif // TKWIN_WAYLAND_H
