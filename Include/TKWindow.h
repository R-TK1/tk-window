#ifndef TKWIN_MAIN_H
#define TKWIN_MAIN_H

#include <stdint.h>

typedef enum
{
    TKWIN_NO_ERROR,
    TKWIN_FAILED_WAYLAND_CONNECT
} tkwin_error_t;

tkwin_error_t tkwin_create(void);

void tkwin_destroy(void);

bool tkwin_poll(void);

void tkwin_close(void);

#endif // TKWIN_MAIN_H
