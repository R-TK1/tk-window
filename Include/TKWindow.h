#ifndef TKWIN_MAIN_H
#define TKWIN_MAIN_H

#include <stdint.h>

typedef enum
{
    AWIN_NO_ERROR,
    AWIN_FAILED_WAYLAND_CONNECT
} awin_error_t;

awin_error_t awin_createWindow(void);

void awin_destroyWindow(void);

bool awin_pollWindow(void);

void awin_closeWindow(void);

#endif // TKWIN_MAIN_H
