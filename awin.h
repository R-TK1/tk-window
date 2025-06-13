#ifndef AWIN_MAIN_H
#define AWIN_MAIN_H

#include <stdint.h>

typedef enum
{
    AWIN_NO_ERROR
} awin_error_t;

awin_error_t awin_createWindow(void);

void awin_destroyWindow(void);

bool awin_pollWindow(void);

#endif // AWIN_MAIN_H
