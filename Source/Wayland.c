/**
 * @file Wayland.c
 * @authors Israfil Argos
 * @brief This file provides the complete Wayland implementation of the TKWindow
 * interface. This only depends upon the default C-standard @c stdint.h, and @c
 * string.h files, and the Wayland client header @c wayland-client.h.
 * @since v0.0.0.2
 *
 * @copyright (c) 2025 - the RPGtk Project
 * This source file is under the GNU General Public License v3.0. For licensing
 * and other information, see the @c LICENSE.md file that should have come with
 * your copy of the source code, or https://www.gnu.org/licenses/gpl-3.0.txt.
 */

#include <TKLogging.h>
#include <stdint.h>
#include <string.h>
#include <wayland-client.h>

/**
 * @var bool pClose
 * @brief The global close variable, which is assigned in order to, well, close
 * the window. This does @b not instantly kill the window, it simply gives a
 * gentle nudge to begin resource deaquisition.
 * @since v0.0.0.20
 */
bool pClose = false;

/**
 * @def REFREF(expr)
 * @brief Convert an interface into a double-referenced pointer via some casting
 * nonsense. This is for the sole purpose of creating other interfaces (whose
 * function signatures require an "array" of interfaces) and should not be used
 * outside of this purpose. For all intents and purposes, this macro does not
 * exist; ignore it.
 * @since v0.0.0.23
 *
 * @param[in] interface The interface to convert into a double-referenced
 * pointer, or "array".
 * @return A constant @c wl_interface double-pointer.
 */
#define REFREF(interface) (const struct wl_interface **)(&interface)

/**
 * @var const struct wl_interface xdg_toplevel_interface
 * @brief The XDG toplevel interface, including functions that can be called and
 * events that can be applied. This is the version seven interface.
 * @since v0.0.0.20
 *
 * @remark Most of the function definitions are missing to remove extraneous
 * data; the interface is still recognized as valid by the server, but we don't
 * store the strings in the executable.
 */
static const struct wl_interface xdg_toplevel_interface = {
    .name = "xdg_toplevel",
    .version = 7,
    .method_count = 14,
    .methods =
        (struct wl_message[]){
            {"destroy", "", nullptr},
            {0},
            {"set_title", "s", nullptr},
            {"set_app_id", "s", nullptr},
            {0},
            {0},
            {0},
            {0},
            {0},
            {0},
            {0},
            {"set_fullscreen", "?o", REFREF(wl_output_interface)},
            {0},
            {0},
        },
    .event_count = 4,
    .events =
        (struct wl_message[]){
            {"configure", "iia", nullptr},
            {"close", "", nullptr},
            {"configure_bounds", "4ii", nullptr},
            {"wm_capabilities", "5a", nullptr},
        },
};

/**
 * @var const struct wl_interface xdg_surface_interface
 * @brief The XDG surface interface, including functions that can be called and
 * events that can be applied. This is the version seven interface.
 * @since v0.0.0.20
 *
 * @remark Some of the function definitions are missing to remove extraneous
 * data; the interface is still recognized as valid by the server, but we don't
 * store the strings in the executable.
 */
static const struct wl_interface xdg_surface_interface = {
    .name = "xdg_surface",
    .version = 7,
    .method_count = 5,
    .methods =
        (struct wl_message[]){
            {"destroy", "", nullptr},
            {"get_toplevel", "n", REFREF(xdg_toplevel_interface)},
            {0},
            {0},
            {"ack_configure", "u", nullptr},
        },
    .event_count = 1,
    .events = (struct wl_message[]){{"configure", "u", nullptr}},
};

/**
 * @var const struct wl_interface xdg_wm_base_interface
 * @brief The XDG window manager base interface, including functions that can be
 * called and events that can be applied. This is the version seven interface.
 * @since v0.0.0.20
 *
 * @remark One of the function definitions are missing to remove extraneous
 * data; the interface is still recognized as valid by the server, but we don't
 * store the strings in the executable.
 */
static const struct wl_interface xdg_wm_base_interface = {
    .name = "xdg_wm_base",
    .version = 7,
    .method_count = 4,
    .methods =
        (struct wl_message[]){
            {"destroy", "", nullptr},
            {0},
            {"get_xdg_surface", "no", REFREF(xdg_surface_interface)},
            {"pong", "u", nullptr},
        },
    .event_count = 1,
    .events = (struct wl_message[]){{"ping", "u", nullptr}},
};

/**
 * @var struct wl_display *pDisplay
 * @brief The Wayland display server reference we've recieved. This is simply a
 * reference to the display server; the only thing it's useful for is syncing
 * processing and accessing the registry. All other information is accessed via
 * the registry.
 * @since v0.0.0.2
 */
static struct wl_display *pDisplay = nullptr;

/**
 * @var struct wl_registry *pRegistry
 * @brief The true core object for the Wayland protocol. We access all other
 * interfaces, like the compositor, monitors, and input devices through this
 * object.
 * @since v0.0.0.2
 */
static struct wl_registry *pRegistry = nullptr;

/**
 * @var struct wl_compositor *pCompositor
 * @brief The compositor reference object. We go through this compositor object
 * in order to grab @c wl_surface objects, which are smaller pixel buffers that
 * can be written to to paint contents onto the screen.
 * @since v0.0.0.2
 */
static struct wl_compositor *pCompositor = nullptr;

/**
 * @var struct wl_surface *pSurface
 * @brief The @c wl_surface object, basically just a buffer of pixels that will
 * be written to in order to paint content onto the screen.
 * @since v0.0.0.2
 */
static struct wl_surface *pSurface = nullptr;

/**
 * @var struct wl_output *pOutput
 * @brief The pixel output device, or monitor. This is the object we pull
 * dimensions from in order to size the window. This object is freed once we
 * recieve all required information from it.
 * @since v0.0.0.2
 */
static struct wl_output *pOutput = nullptr;

/**
 * @var struct xdg_wm_base *pShell
 * @brief A sort of second-level registry specifically for the XDG-shell
 * extension. This provides the ability to create toplevel interfaces and do
 * other manipulation tasks otherwise nigh impossible with default Wayland.
 * @since v0.0.0.2
 */
static struct xdg_wm_base *pShell = nullptr;

/**
 * @var struct xdg_surface *pShellSurface
 * @brief The shell surface, a relatively shallow wrapper around the default @c
 * wl_surface object, including configuration and ping events.
 * @since v0.0.0.2
 */
static struct xdg_surface *pShellSurface = nullptr;

/**
 * @var struct xdg_toplevel *pToplevel
 * @brief The toplevel XDG "window". This provides a much more complete wrapper
 * over @c wl_surface, including fullscreen capabilities, which this project
 * requires.
 * @since v0.0.0.2
 */
static struct xdg_toplevel *pToplevel = nullptr;

static int32_t pScale = 0;
static uint32_t pWidth = 0;
static uint32_t pHeight = 0;

static void handleConfigure(void *data, struct xdg_surface *shellSurface,
                            uint32_t serial)
{
    (void)data;

    // xdg_surface_ack_configure
    wl_proxy_marshal_flags(
        (struct wl_proxy *)shellSurface, 4, NULL,
        wl_proxy_get_version((struct wl_proxy *)shellSurface), 0, serial);
}

static const struct
{
    void (*configure)(void *data, struct xdg_surface *xdg_surface,
                      uint32_t serial);
} pShellSurfaceListener = {.configure = handleConfigure};

static void handleTopClose(void *data, struct xdg_toplevel *toplevel)
{
    (void)data;
    (void)toplevel;
    pClose = true;
}

static void handleTopConfigure(void *data, struct xdg_toplevel *toplevel,
                               int32_t width, int32_t height,
                               struct wl_array *states)
{
    (void)data;
    (void)toplevel;
    (void)states;

    pWidth = width * pScale;
    pHeight = height * pScale;
    rpgtk_log(VERBOSE, "Window dimensions adjusted: %dx%d.", pWidth, pHeight);
}

static void handleTopConfigureBounds(void *data, struct xdg_toplevel *toplevel,
                                     int32_t width, int32_t height)
{
    (void)data;
    (void)toplevel;
    (void)width;
    (void)height;
}

static void handleCapabilities(void *data, struct xdg_toplevel *toplevel,
                               struct wl_array *capabilities)
{
    (void)data;
    (void)toplevel;
    (void)capabilities;
}

static const struct
{
    void (*configure)(void *data, struct xdg_toplevel *xdg_toplevel,
                      int32_t width, int32_t height, struct wl_array *states);

    void (*close)(void *data, struct xdg_toplevel *xdg_toplevel);

    void (*configure_bounds)(void *data, struct xdg_toplevel *xdg_toplevel,
                             int32_t width, int32_t height);

    void (*wm_capabilities)(void *data, struct xdg_toplevel *xdg_toplevel,
                            struct wl_array *capabilities);
} pToplevelListener = {.close = handleTopClose,
                       .configure = handleTopConfigure,
                       .configure_bounds = handleTopConfigureBounds,
                       .wm_capabilities = handleCapabilities};

void handleGeometry(void *data, struct wl_output *output, int32_t x, int32_t y,
                    int32_t physical_width, int32_t physical_height,
                    int32_t subpixel, const char *make, const char *model,
                    int32_t transform)
{
    (void)data;
    (void)output;
    (void)x;
    (void)y;
    (void)physical_width;
    (void)physical_height;
    (void)subpixel;
    (void)make;
    (void)model;
    (void)transform;
}

void handleMode(void *data, struct wl_output *output, uint32_t flags,
                int32_t width, int32_t height, int32_t refresh)
{
    (void)data;
    (void)output;
    (void)flags;
    (void)width;
    (void)height;
    (void)refresh;
}

void handleFinish(void *data, struct wl_output *output)
{
    (void)data;
    (void)output;
}

void handleScale(void *data, struct wl_output *output, int32_t factor)
{
    (void)data;
    (void)output;
    pScale = factor;
    rpgtk_log(VERBOSE, "Monitor scale %d.", pScale);
}

void handleName(void *data, struct wl_output *output, const char *name)
{
    (void)data;
    (void)output;
    (void)name;
}

void handleDescription(void *data, struct wl_output *output,
                       const char *description)
{
    (void)data;
    (void)output;
    (void)description;
}

static const struct wl_output_listener outputListener = {
    .description = handleDescription,
    .done = handleFinish,
    .geometry = handleGeometry,
    .mode = handleMode,
    .name = handleName,
    .scale = handleScale};

static void handlePing(void *data, struct xdg_wm_base *shell, uint32_t serial)
{
    (void)data;

    // xdg_wm_base_pong
    wl_proxy_marshal_flags((struct wl_proxy *)shell, 3, nullptr,
                           wl_proxy_get_version((struct wl_proxy *)shell), 0,
                           serial);
}

static struct
{
    void (*ping)(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial);
} pShellListener = {.ping = handlePing};

static void handleGlobal(void *data, struct wl_registry *registry,
                         uint32_t name, const char *interface, uint32_t version)
{
    (void)data;
    (void)version;

    if (strcmp(interface, wl_compositor_interface.name) == 0)
    {
        pCompositor =
            wl_registry_bind(registry, name, &wl_compositor_interface, 1);
        if (pCompositor == nullptr)
            rpgtk_log(ERROR, "Failed to connect to compositor.");
    }
    else if (strcmp(interface, "xdg_wm_base") == 0)
    {
        pShell = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
        // xdg_wm_base_add_listener
        wl_proxy_add_listener((struct wl_proxy *)pShell,
                              (void (**)(void))&pShellListener, data);
    }
    else if (strcmp(interface, wl_output_interface.name) == 0)
    {
        pOutput = wl_registry_bind(registry, name, &wl_output_interface, 2);
        if (pOutput == nullptr)
            rpgtk_log(ERROR, "Failed to connect to output.");
        wl_output_add_listener(pOutput, &outputListener, nullptr);
    }
}

static void handleGlobalRemove(void *data, struct wl_registry *registry,
                               uint32_t name)
{
    (void)data;
    (void)registry;
    (void)name;
}

static const struct wl_registry_listener pRegistryListener = {
    .global = handleGlobal, .global_remove = handleGlobalRemove};

bool windowCreate(const char *title)
{
    pDisplay = wl_display_connect(nullptr);
    if (pDisplay == nullptr)
    {
        rpgtk_log(ERROR, "Failed to connect to display server.");
        return false;
    }

    // This call ostensibly cannot fail if display is properly gotten.
    pRegistry = wl_display_get_registry(pDisplay);
    (void)wl_registry_add_listener(pRegistry, &pRegistryListener, nullptr);
    (void)wl_display_roundtrip(pDisplay);

    pSurface = wl_compositor_create_surface(pCompositor);
    // xdg_wm_base_get_xdg_surface
    pShellSurface = (struct xdg_surface *)wl_proxy_marshal_flags(
        (struct wl_proxy *)pShell, 2, &xdg_surface_interface,
        wl_proxy_get_version((struct wl_proxy *)pShell), 0, nullptr, pSurface);

    // xdg_surface_add_listener
    wl_proxy_add_listener((struct wl_proxy *)pShellSurface,
                          (void (**)(void))&pShellSurfaceListener, nullptr);

    // xdg_surface_get_toplevel
    pToplevel = (struct xdg_toplevel *)wl_proxy_marshal_flags(
        (struct wl_proxy *)pShellSurface, 1, &xdg_toplevel_interface,
        wl_proxy_get_version((struct wl_proxy *)pShellSurface), 0, nullptr);
    // xdg_toplevel_add_listener
    wl_proxy_add_listener((struct wl_proxy *)pToplevel,
                          (void (**)(void))&pToplevelListener, nullptr);

    // xdg_toplevel_set_title
    wl_proxy_marshal_flags((struct wl_proxy *)pToplevel, 2, NULL,
                           wl_proxy_get_version((struct wl_proxy *)pToplevel),
                           0, title);
    // xdg_toplevel_set_app_id
    wl_proxy_marshal_flags((struct wl_proxy *)pToplevel, 3, NULL,
                           wl_proxy_get_version((struct wl_proxy *)pToplevel),
                           0, title);
    // xdg_toplevel_set_fullscreen
    wl_proxy_marshal_flags((struct wl_proxy *)pToplevel, 11, NULL,
                           wl_proxy_get_version((struct wl_proxy *)pToplevel),
                           0, pOutput);

    wl_surface_commit(pSurface);
    wl_display_roundtrip(pDisplay);
    wl_surface_commit(pSurface);

    wl_output_destroy(pOutput);

    return true;
}

void windowDestroy(void)
{
    // xdg_toplevel_destroy
    wl_proxy_marshal_flags((struct wl_proxy *)pToplevel, 0, NULL,
                           wl_proxy_get_version((struct wl_proxy *)pToplevel),
                           WL_MARSHAL_FLAG_DESTROY);
    // xdg_surface_destroy
    wl_proxy_marshal_flags(
        (struct wl_proxy *)pShellSurface, 0, NULL,
        wl_proxy_get_version((struct wl_proxy *)pShellSurface),
        WL_MARSHAL_FLAG_DESTROY);
    // xdg_wm_base_destroy
    wl_proxy_marshal_flags((struct wl_proxy *)pShell, 0, NULL,
                           wl_proxy_get_version((struct wl_proxy *)pShell),
                           WL_MARSHAL_FLAG_DESTROY);

    wl_surface_destroy(pSurface);
    wl_compositor_destroy(pCompositor);
    wl_registry_destroy(pRegistry);
    wl_display_disconnect(pDisplay);
}

bool windowProcess(void) { return wl_display_dispatch(pDisplay) != -1; }

void windowGetSize(uint32_t *width, uint32_t *height)
{
    *width = pWidth;
    *height = pHeight;
}

void windowGetData(void **data)
{
    data[0] = pDisplay;
    data[1] = pSurface;
}
