/**
 * @file TKWindow.h
 * @authors Israfil Argos
 * @brief This file provides the implementation for the public interface
 * declared in the @c TKWindow.h header file. This file is mostly a hub for all
 * the various platform-dependent interfaces.
 * @since v0.0.0.1
 *
 * @copyright (c) 2025 - the RPGtk Project
 * This source file is under the GNU General Public License v3.0. For licensing
 * and other information, see the @c LICENSE.md file that should have come with
 * your copy of the source code, or https://www.gnu.org/licenses/gpl-3.0.txt.
 */
#include <TKWindow.h>

#ifdef WAYLAND
#include <Targets/Wayland.h>
#elifdef X11
#include <Targets/X11.h>
#endif

struct
{
    const char *title;
    bool shouldClose;
} window = {nullptr, true};

inline bool rpgtk_windowCreate(const char *title)
{
#ifdef WAYLAND
    if (!tkwin_waylandCreate(title)) return false;
#elifdef X11
    // TODO: X11 code
#endif

    return true;
}

inline void rpgtk_windowDestroy(void)
{
#ifdef WAYLAND
    tkwin_waylandDestroy();
#elifdef X11
    // TODO: X11 code
#endif
}

inline bool rpgtk_windowProcess(void)
{
#ifdef WAYLAND
    window.shouldClose = tkwin_waylandPoll();
#elifdef X11
    // TODO: X11 code
#endif

    return window.shouldClose;
}

inline void rpgtk_windowClose(void) { window.shouldClose = true; }

inline void rpgtk_windowGetSize(uint32_t *width, uint32_t *height)
{
#ifdef WAYLAND
    tkwin_waylandGetFramebufferSize(width, height);
#elifdef X11
    (void)width;
    (void)height;
    // TODO: X11 code
#endif
}

inline void rpgtk_windowGetData(void **data)
{
#ifdef WAYLAND
    tkwin_waylandGetSurfaceData(data);
#elifdef X11
    (void)data;
    // TODO: X11 code
#endif
}
