/**
 * @file WLWindow.h
 * @authors Israfil Argos
 * @brief This file provides the public interface for the WLWindow library, a
 * tiny wrapper around many different windowing systems. This file requires no
 * dependencies beyond the standard C @c stdint.h include.
 * @since v0.0.0.1
 *
 * @copyright (c) 2025 - the RPGtk Project
 * This source file is under the GNU General Public License v3.0. For licensing
 * and other information, see the @c LICENSE.md file that should have come with
 * your copy of the source code, or https://www.gnu.org/licenses/gpl-3.0.txt.
 */
#ifndef WLWINDOW_MAIN_H
#define WLWINDOW_MAIN_H

#include <stdint.h>

/**
 * @fn bool waterlily_windowCreate(void)
 * @brief Create the main window object of the engine. This should only be
 * called once, to prevent resource wasting and other undesirable behavior.
 * There are no checks in this function for anything but internal failure, call
 * only when you're certain there is no other window created.
 * @since v0.0.0.1
 *
 * @remark The created window is always fullscreen, undecorated, and focused off
 * the bat.
 *
 * @param[in] title The title you wish your window to have. This must be
 * NUL-terminated, it is not edited in any way during the course of the
 * function.
 * @return A boolean value representing whether or not the window was created
 * successfully. This can be for any number of reasons, all of which are
 * documented in depth in the platform-dependent versions of this function.
 * However, a message will always be logged to an attatched @c tty explaining
 * any errors.
 */
[[nodiscard]] [[gnu::nonnull(1)]] [[gnu::always_inline]]
inline bool waterlily_windowCreate(const char *title)
{
    extern bool windowCreate(const char *title);
    return windowCreate(title);
}

/**
 * @fn void waterlily_windowDestroy(void)
 * @brief Destroy the main window object of the engine. This should only be
 * called when a window is truly created, to prevent double-frees and other
 * undefined behaviors. There are no checks in this function for anything but
 * internal failure, call only when you're certain there is a window created.
 * @since v0.0.0.1
 *
 * @remark This function kills the window without asking for permission from any
 * other running processes. All other dependents (like graphical processes)
 * should be deinitialized before this function is called. To close the window,
 * please see @ref waterlily_windowClose(void).
 */
[[gnu::always_inline]]
inline void waterlily_windowDestroy(void)
{
    extern void windowDestroy(void);
    windowDestroy();
}

/**
 * @fn bool waterlily_windowProcess(void)
 * @brief Process any and all window events and clear the queue. This should be
 * called each time you wish to paint a "frame" for the window. This will
 * process things like user input, close events, etc.
 * @since v0.0.0.2
 *
 * @return A boolean value representing whether or not event processing
 * succeeded. If false is returned, the window should close, no questions asked.
 * The window processing failing does not necessarily mean an error has
 * occurred, simply that processing cannot continue.
 */
[[nodiscard]] [[gnu::hot]] [[gnu::always_inline]]
inline bool waterlily_windowProcess(void)
{
    extern bool windowProcess(void);
    return windowProcess();
}

/**
 * @fn void waterlily_windowClose(void)
 * @brief Close the window. This sends a bullet directly into the windowing
 * processes, and should alert its dependents to clean themselves up as well.
 * @since v0.0.0.2
 *
 * @remark This does not destroy the window. For that you should see @ref
 * waterlily_windowDestroy(void), which should be called after this function.
 */
[[gnu::always_inline]]
inline void waterlily_windowClose(void)
{
    extern bool pClose;
    pClose = true;
}

/**
 * @fn void waterlily_windowGetSize(uint32_t *width, uint32_t *height)
 * @brief Get the size of the window's framebuffer in pixels.
 * @since v0.0.0.9
 *
 * @remark On most platforms, this will equal the size in screen coordinates,
 * but on platforms like the Apple Retina display, this is not the case.
 *
 * @param[out] width The storage for the width of the framebuffer in pixels.
 * @param[out] height The storage for the height of the framebuffer in pixels.
 */
[[gnu::nonnull(1, 2)]] [[gnu::always_inline]]
inline void waterlily_windowGetSize(uint32_t *width, uint32_t *height)
{
    extern void windowGetSize(uint32_t *width, uint32_t *height);
    windowGetSize(width, height);
}

/**
 * @fn void waterlily_windowGetData(void **data)
 * @brief Get the native data specific to this window. Each platform has its own
 * set of data, discussed in detail in the platform-specific version of this
 * function.
 *
 * @remark the following data is provided for each platform, in the order
 * specified; Wayland: @c wl_display, @c wl_surface, X11: N/A.
 *
 * @param[out] data The data array. This is not touched by the function, so it
 * must contain enough space to properly handle all items passed to it.
 */
[[gnu::nonnull(1)]] [[gnu::always_inline]]
inline void waterlily_windowGetData(void **data)
{
    extern void windowGetData(void **data);
    windowGetData(data);
}

#endif // WLWINDOW_MAIN_H
