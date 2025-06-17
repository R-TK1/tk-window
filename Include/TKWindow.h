/**
 * @file TKWindow.h
 * @authors Israfil Argos
 * @brief This file provides the public interface for the TKWindow library, a
 * tiny wrapper around many different windowing systems. This file requires no
 * dependencies beyond the standard C `stdint` include.
 * @version 0.0.0.17
 *
 * @copyright (c) 2025 - the RPGtk Project
 * This source file is under the GNU General Public License v3.0. For licensing
 * and other information, see the `LICENSE.md` file that should have come with
 * your copy of the source code, or <https://www.gnu.org/licenses/gpl-3.0.txt>.
 */
#ifndef TKWINDOW_MAIN_H
#define TKWINDOW_MAIN_H

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

void tkwin_getFramebufferSize(uint32_t *width, uint32_t *height);

void tkwin_getSurfaceData(void **data);

#endif // TKWINDOW_MAIN_H
