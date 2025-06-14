#ifndef TKWIN_MAIN_H
#define TKWIN_MAIN_H

#include <stdint.h>

typedef enum
{
    TKWIN_NO_ERROR,
    TKWIN_FAILED_WAYLAND_CONNECT
} tkwin_error_t;

tkwin_error_t tkwin_createWindow(void);

void tkwin_destroyWindow(void);

bool tkwin_pollWindow(void);

void tkwin_closeWindow(void);

#endif // TKWIN_MAIN_H
