#include <TKWin.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    awin_error_t error = awin_createWindow();
    if (error != AWIN_NO_ERROR)
    {
        fprintf(stderr, "Failed to create window. Code: %d.", error);
        return -1;
    }

    while (awin_pollWindow()) {}

    awin_destroyWindow();
    return 0;
}
