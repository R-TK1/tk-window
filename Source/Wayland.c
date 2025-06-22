/**
 * @file Wayland.c
 * @authors Israfil Argos
 * @brief This file provides the complete Wayland implementation of the WLWindow
 * interface. This only depends upon the default C-standard @c stdint.h and @c
 * string.h files.
 * @since v0.0.0.2
 *
 * @copyright (c) 2025 - the Waterlily Team
 * This source file is under the GNU General Public License v3.0. For licensing
 * and other information, see the @c LICENSE.md file that should have come with
 * your copy of the source code, or https://www.gnu.org/licenses/gpl-3.0.txt.
 */

#include <WLLogging.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

/**
 * @var bool pClose
 * @brief The global close variable, which is assigned in order to, well, close
 * the window. This does @b not instantly kill the window, it simply gives a
 * gentle nudge to begin resource deaquisition.
 * @since v0.0.0.20
 */
bool pClose = false;

static int pFileDescriptor = 0;

/**
 * @var int32_t pScale
 * @brief The monitor scale of screen coordinates to pixels. This is nearly
 * always one, unless on a display like the Apple Retina.
 * @since v0.0.0.2
 */
// static int32_t pScale = 0;

/**
 * @var uint32_t pWidth
 * @brief The width of the window in @b pixels. This value is recieved from the
 * display server and multiplied by @ref int32_t pScale to grab the actual pixel
 * value.
 * @since v0.0.0.2
 */
static uint32_t pWidth = 0;

/**
 * @var uint32_t pHeight
 * @brief The height of the window in @b pixels. This value is recieved from the
 * display server and multiplied by @ref int32_t pScale to grab the actual pixel
 * value.
 * @since v0.0.0.2
 */
static uint32_t pHeight = 0;

static bool waylandConnect(void)
{
    waterlily_log(VERBOSE_BEGIN, "Connecting to Wayland display server.");

    const char *const runtimeDirectory = getenv("XDG_RUNTIME_DIR");
    if (runtimeDirectory == nullptr)
    {
        waterlily_log(ERROR, "No XDG runtime directory found.");
        return false;
    }
    waterlily_log(VERBOSE_OK, "Found XDG_RUNTIME_DIR: '%s'.", runtimeDirectory);
    size_t runtimeDirectoryLength = strlen(runtimeDirectory);

    // Specifically a Unix socket address.
    struct sockaddr_un address = {.sun_family = AF_UNIX};
    if (runtimeDirectoryLength > sizeof(address.sun_path) - 1)
    {
        waterlily_log(ERROR,
                      "Cannot fit XDG runtime directory into socket path.");
        return false;
    }
    waterlily_log(VERBOSE_OK, "Socket address is legal.");

    (void)memcpy(address.sun_path, runtimeDirectory, runtimeDirectoryLength);
    address.sun_path[runtimeDirectoryLength++] = '/';

    const char *const waylandDisplay = getenv("WAYLAND_DISPLAY");
    // Either try to find the default display or run with the one we've been
    // provided.
    if (waylandDisplay == nullptr)
        (void)memcpy(address.sun_path + runtimeDirectoryLength, "wayland-0", 9);
    else
        (void)memcpy(address.sun_path + runtimeDirectoryLength, waylandDisplay,
                     strlen(waylandDisplay));
    waterlily_log(VERBOSE, "Selected Wayland display.");
    waterlily_log(VERBOSE, "Full socket path '%s'.", address.sun_path);

    pFileDescriptor = socket(AF_UNIX, SOCK_STREAM, 0);
    if (pFileDescriptor == -1)
    {
        waterlily_log(ERROR, "Failed to create socket.");
        close(pFileDescriptor);
        return false;
    }
    waterlily_log(VERBOSE_OK, "Created socket.");

    if (connect(pFileDescriptor, (struct sockaddr *)&address,
                sizeof(address)) == -1)
    {
        waterlily_log(ERROR, "Failed to connect to socket.");
        close(pFileDescriptor);
        return false;
    }
    waterlily_log(VERBOSE_OK, "Connected to socket.");

    waterlily_log(SUCCESS, "Connected to display server.");
    return true;
}

bool windowCreate(const char *title)
{
    if (!waylandConnect()) return false;

    (void)title;
    return true;
}

void windowDestroy(void) { close(pFileDescriptor); }

bool windowProcess(void) { return false; }

void windowGetSize(uint32_t *width, uint32_t *height)
{
    *width = pWidth;
    *height = pHeight;
}

void windowGetData(void **data) { (void)data; }
