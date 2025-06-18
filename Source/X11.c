#include <stdint.h>

bool windowCreate(const char *title)
{
    (void)title;
    return true;
}

void windowDestroy(void) {}

bool windowProcess(void) { return true; }

void windowGetSize(uint32_t *width, uint32_t *height)
{
    (void)width;
    (void)height;
}

void windowGetData(void **data) { (void)data; }
