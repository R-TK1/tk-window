#ifndef TKWINDOW_WAYLAND_H
#define TKWINDOW_WAYLAND_H

#include <stdint.h>

bool tkwin_waylandCreate(void);
void tkwin_waylandDestroy(void);
bool tkwin_waylandPoll(void);

void tkwin_waylandGetFramebufferSize(uint32_t *width, uint32_t *height);
void tkwin_waylandGetSurfaceData(void **data);

#endif // TKWINDOW_WAYLAND_H
