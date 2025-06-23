/**
 * @file Wayland.c
 * @authors Israfil Argos
 * @brief This file provides the complete Wayland implementation of the TKWindow
 * interface. This only depends upon the default C-standard @c stdint.h, and @c
 * string.h files, and the Wayland client header @c wayland-client.h.
 * @since v0.0.0.2
 *
 * @note This file contains material (the contents of the XDG-shell protocol)
 * copyrighted by the following people. All rights are reserved to their proper
 * owners.
 * Copyright © 2008-2013 Kristian Høgsberg
 * Copyright © 2013      Rafael Antognolli
 * Copyright © 2013      Jasper St. Pierre
 * Copyright © 2010-2013 Intel Corporation
 * Copyright © 2015-2017 Samsung Electronics Co., Ltd
 * Copyright © 2015-2017 Red Hat Inc.
 *
 * @copyright (c) 2025 - the Waterlily Project
 * This source file is under the GNU General Public License v3.0. For licensing
 * and other information, see the @c LICENSE.md file that should have come with
 * your copy of the source code, or https://www.gnu.org/licenses/gpl-3.0.txt.
 */

#include <WLLogging.h>
#include <stdint.h>
#include <stdlib.h>
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
 * dimensions from in order to size the window.
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

/**
 * @var int32_t pScale
 * @brief The monitor scale of screen coordinates to pixels. This is nearly
 * always one, unless on a display like the Apple Retina.
 * @since v0.0.0.2
 */
static int32_t pScale = 0;

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

/**
 * @copydoc xdg_wm_base_listener::ping
 */
static void ping(void *, struct xdg_wm_base *b, uint32_t s)
{
    // xdg_wm_base_pong
    wl_proxy_marshal_flags((struct wl_proxy *)b, 3, nullptr,
                           wl_proxy_get_version((struct wl_proxy *)b), 0, s);
}

/**
 * @struct xdg_wm_base_listener Wayland.c "Source/Wayland.c"
 * @brief An interface for handling events sent from the XDG "registry" object,
 * @c wm_base. This is basically a nerdy game of table tennis, we just recieve a
 * ping and send back a pong, over and over again so the WM doesn't think we're
 * a zombie.
 * @since v0.0.0.20
 */
static struct xdg_wm_base_listener
{
    /**
     * @property ping
     * @brief The ping event asks the client if it’s still alive. Pass the
     * serial specified in the event back to the compositor by sending a “pong”
     * request back with the specified serial. A compositor is free to ping in
     * any way it wants, but a client must always respond to any xdg_wm_base
     * object it created.
     * @since v0.0.0.20
     *
     * @remark It’s unspecified what will happen if the client doesn’t respond
     * to the ping request, or in what timeframe.
     *
     * @param[in] data Any data to be sent alongside events.
     * @param[in] base The window mananger base object that generated the event.
     * @param[in] serial The serial code of the event. The client must respond
     * to the ping with this serial code in its pong.
     */
    void (*ping)(void *data, struct xdg_wm_base *base, uint32_t serial);
}
/**
 * @var struct xdg_wm_base_listener pShellListener
 * @brief The listener for the XDG window manager base object, which is
 * basically the "registry" for the XDG Shell protocol.
 * @since v0.0.0.2
 *
 * @copydoc xdg_wm_base_listener
 */
pShellListener = {.ping = &ping};

/**
 * @copydoc xdg_surface_listener::configure
 */
static void configure(void *, struct xdg_surface *t, uint32_t s)
{
    waterlily_log(VERBOSE_OK, "Configure request completed.");
    // Acknowlege the configuration. (xdg_surface_ack_configure)
    wl_proxy_marshal_flags((struct wl_proxy *)t, 4, nullptr,
                           wl_proxy_get_version((struct wl_proxy *)t), 0, s);
}

/**
 * @struct xdg_surface_listener Wayland.c "Source/Wayland.c"
 * @brief An interface for handling events for the @c xdg_surface wrapper around
 * the @c wl_surface object.
 * @since v0.0.0.20
 */
static const struct xdg_surface_listener
{
    /**
     * @property configure
     * @brief The configure event marks the end of a configure sequence. A
     * configure sequence is a set of one or more events configuring the state
     * of the xdg_surface. Clients should send an ack_configure before
     * committing the new surface.
     * @since v0.0.0.20
     *
     * @remark Where applicable, xdg_surface surface roles will during a
     * configure sequence extend this event as a latched state sent as events
     * before the xdg_surface.configure event. If the client receives multiple
     * configure events before it can respond to one, it is free to discard all
     * but the last event it received.
     *
     * @param[in] data Any user-defined data sent alongside the surface.
     * @param[in] surface The surface who is being configured.
     * @param[in] serial The serial ID of this event. This should be sent
     * alongside a configuration acknowledgement event.
     */
    void (*configure)(void *data, struct xdg_surface *surface, uint32_t serial);
}
/**
 * @var struct xdg_surface_listener pShellSurfaceListener
 * @brief The listener for the XDG surface object. This handles exclusvely
 * surface-level configuration-end events sent by the server.
 * @since v0.0.0.2
 *
 * @copydoc xdg_surface_listener
 */
pShellSurfaceListener = {&configure};

/**
 * @copydoc xdg_toplevel_listener::topConfigure
 */
static void topConfigure(void *, struct xdg_toplevel *, int32_t w, int32_t h,
                         struct wl_array *s)
{
    waterlily_log(VERBOSE_BEGIN, "Configure request recieved.");

    pWidth = (uint32_t)(w * pScale);
    pHeight = (uint32_t)(h * pScale);
    wl_surface_commit(pSurface);
    waterlily_log(VERBOSE, "Window dimensions adjusted: %dx%d.", pWidth,
                  pHeight);

    int32_t *states = s->data;
    for (size_t i = 0; i < s->size / sizeof(int32_t); i++)
    {
        switch (states[i])
        {
            case 2:
                waterlily_log(VERBOSE, "The window is now fullscreened.");
                break;
            case 9: waterlily_log(NOTE, "The window is now suspended."); break;
            default:
                waterlily_log(WARNING, "Got unknown state value '%d'.",
                              states[i]);
                break;
        }
    }
}

/**
 * @copydoc xdg_toplevel_listener::close
 */
static void close(void *, struct xdg_toplevel *)
{
    waterlily_log(LOG, "Closing window.");
    pClose = true;
}

/**
 * @copydoc xdg_toplevel_listener::bounds
 */
static void bounds(void *, struct xdg_toplevel *, int32_t w, int32_t h)
{
    waterlily_log(VERBOSE, "Recommended dimensions: %dx%d.", w, h);
}

/**
 * @copydoc xdg_toplevel_listener::capabilities
 */
static void capabilities(void *, struct xdg_toplevel *, struct wl_array *c)
{
    int32_t *states = c->data;
    for (size_t i = 0; i < c->size / sizeof(int32_t); i++)
    {
        if (states[i] == 3)
        {
            waterlily_log(VERBOSE_OK, "Found fullscreen support.");
            return;
        }
    }

    waterlily_log(ERROR, "No fullscreen support available.");
}

static const struct
{
    void (*topConfigure)(void *data, struct xdg_toplevel *toplevel,
                         int32_t width, int32_t height,
                         struct wl_array *states);

    void (*close)(void *data, struct xdg_toplevel *toplevel);

    void (*bounds)(void *data, struct xdg_toplevel *toplevel, int32_t width,
                   int32_t height);

    void (*capabilities)(void *data, struct xdg_toplevel *toplevel,
                         struct wl_array *capabilities);
} pToplevelListener = {&topConfigure, &close, &bounds, &capabilities};

static void geometry(void *, struct wl_output *, int32_t, int32_t, int32_t,
                     int32_t, int32_t, const char *, const char *, int32_t)
{
}

static void mode(void *, struct wl_output *, uint32_t, int32_t, int32_t,
                 int32_t)
{
}

static void finish(void *, struct wl_output *) {}

static void scale(void *, struct wl_output *, int32_t s)
{
    pScale = s;
    waterlily_log(VERBOSE, "Monitor scale %d.", pScale);
}

static void name(void *, struct wl_output *, const char *) {}

static void description(void *, struct wl_output *, const char *) {}

static const struct wl_output_listener pOutputListener = {
    &geometry, &mode, &finish, &scale, &name, &description};

static void handleGlobal(void *data, struct wl_registry *registry,
                         uint32_t name, const char *interface, uint32_t version)
{
    (void)data;
    (void)version;

    if (strcmp(interface, wl_compositor_interface.name) == 0)
    {
        pCompositor =
            wl_registry_bind(registry, name, &wl_compositor_interface, version);
        if (pCompositor == nullptr)
        {
            waterlily_log(ERROR, "Failed to connect to compositor.");
            return;
        }
        waterlily_log(VERBOSE_OK, "Connected to compositor v%d.", version);
    }
    else if (strcmp(interface, "xdg_wm_base") == 0)
    {
        pShell =
            wl_registry_bind(registry, name, &xdg_wm_base_interface, version);
        if (pShell == nullptr)
        {
            waterlily_log(ERROR, "Failed to connect to window manager.");
            return;
        }

        // xdg_wm_base_add_listener
        wl_proxy_add_listener((struct wl_proxy *)pShell,
                              (void (**)(void))&pShellListener, data);
        waterlily_log(VERBOSE_OK, "Connected to window manager v%d.", version);
    }
    else if (strcmp(interface, wl_output_interface.name) == 0)
    {
        pOutput =
            wl_registry_bind(registry, name, &wl_output_interface, version);
        if (pOutput == nullptr)
        {
            waterlily_log(ERROR, "Failed to connect to output.");
            return;
        }
        wl_output_add_listener(pOutput, &pOutputListener, nullptr);
        waterlily_log(VERBOSE_OK, "Connected to output v%d.", version);
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
    // setenv("WAYLAND_DEBUG", "1", true);
    pDisplay = wl_display_connect(nullptr);
    if (pDisplay == nullptr)
    {
        waterlily_log(ERROR, "Failed to connect to display server.");
        return false;
    }

    // This call ostensibly cannot fail if display is properly gotten.
    pRegistry = wl_display_get_registry(pDisplay);
    (void)wl_registry_add_listener(pRegistry, &pRegistryListener, nullptr);
    (void)wl_display_roundtrip(pDisplay);
    if (pCompositor == nullptr || pShell == nullptr)
    {
        waterlily_log(ERROR, "Could not find compositor and/or shell.");
        return false;
    }

    pSurface = wl_compositor_create_surface(pCompositor);
    // xdg_wm_base_get_xdg_surface
    struct wl_proxy *id = wl_proxy_marshal_flags(
        (struct wl_proxy *)pShell, 2, &xdg_surface_interface,
        wl_proxy_get_version((struct wl_proxy *)pShell), 0, nullptr, pSurface);
    pShellSurface = (struct xdg_surface *)id;
    // xdg_surface_add_listener
    wl_proxy_add_listener((struct wl_proxy *)pShellSurface,
                          (void (**)(void))&pShellSurfaceListener, nullptr);

    // xdg_surface_get_toplevel
    pToplevel = (struct xdg_toplevel *)wl_proxy_marshal_flags(
        (struct wl_proxy *)pShellSurface, 1, &xdg_toplevel_interface,
        wl_proxy_get_version((struct wl_proxy *)pShellSurface), 0, nullptr);

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
    // xdg_toplevel_add_listener
    wl_proxy_add_listener((struct wl_proxy *)pToplevel,
                          (void (**)(void))&pToplevelListener, nullptr);

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
    wl_output_release(pOutput);
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
