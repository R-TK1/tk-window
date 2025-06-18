#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wayland-client.h>

static struct wl_display *pDisplay = nullptr;
static struct wl_registry *pRegistry = nullptr;
static struct wl_compositor *pCompositor = nullptr;
static struct xdg_wm_base *pShell = nullptr;
static struct wl_output *pOutput = nullptr;

static struct wl_surface *pSurface = nullptr;
static struct xdg_surface *pShellSurface = nullptr;
static struct xdg_toplevel *pToplevel = nullptr;

static int32_t pScaleFactor = 0;
static uint32_t pWidth = 0;
static uint32_t pHeight = 0;

bool pClose = false;

extern const struct wl_interface wl_output_interface;
extern const struct wl_interface wl_seat_interface;
extern const struct wl_interface wl_surface_interface;
extern const struct wl_interface xdg_popup_interface;
extern const struct wl_interface xdg_positioner_interface;
extern const struct wl_interface xdg_surface_interface;
extern const struct wl_interface xdg_toplevel_interface;

static const struct wl_interface *xdg_shell_types[] = {
    NULL,
    NULL,
    NULL,
    NULL,
    &xdg_positioner_interface,
    &xdg_surface_interface,
    &wl_surface_interface,
    &xdg_toplevel_interface,
    &xdg_popup_interface,
    &xdg_surface_interface,
    &xdg_positioner_interface,
    &xdg_toplevel_interface,
    &wl_seat_interface,
    NULL,
    NULL,
    NULL,
    &wl_seat_interface,
    NULL,
    &wl_seat_interface,
    NULL,
    NULL,
    &wl_output_interface,
    &wl_seat_interface,
    NULL,
    &xdg_positioner_interface,
    NULL};

static const struct wl_message xdg_wm_base_requests[] = {
    {"destroy", "", xdg_shell_types + 0},
    {"create_positioner", "n", xdg_shell_types + 4},
    {"get_xdg_surface", "no", xdg_shell_types + 5},
    {"pong", "u", xdg_shell_types + 0},
};

static const struct wl_message xdg_wm_base_events[] = {
    {"ping", "u", xdg_shell_types + 0},
};

[[gnu::visibility("hidden")]]
const struct wl_interface xdg_wm_base_interface = {
    "xdg_wm_base", 7, 4, xdg_wm_base_requests, 1, xdg_wm_base_events,
};

static const struct wl_message xdg_positioner_requests[] = {
    {"destroy", "", xdg_shell_types + 0},
    {"set_size", "ii", xdg_shell_types + 0},
    {"set_anchor_rect", "iiii", xdg_shell_types + 0},
    {"set_anchor", "u", xdg_shell_types + 0},
    {"set_gravity", "u", xdg_shell_types + 0},
    {"set_constraint_adjustment", "u", xdg_shell_types + 0},
    {"set_offset", "ii", xdg_shell_types + 0},
    {"set_reactive", "3", xdg_shell_types + 0},
    {"set_parent_size", "3ii", xdg_shell_types + 0},
    {"set_parent_configure", "3u", xdg_shell_types + 0},
};

[[gnu::visibility("hidden")]]
const struct wl_interface xdg_positioner_interface = {
    "xdg_positioner", 7, 10, xdg_positioner_requests, 0, NULL,
};

static const struct wl_message xdg_surface_requests[] = {
    {"destroy", "", xdg_shell_types + 0},
    {"get_toplevel", "n", xdg_shell_types + 7},
    {"get_popup", "n?oo", xdg_shell_types + 8},
    {"set_window_geometry", "iiii", xdg_shell_types + 0},
    {"ack_configure", "u", xdg_shell_types + 0},
};

static const struct wl_message xdg_surface_events[] = {
    {"configure", "u", xdg_shell_types + 0},
};

[[gnu::visibility("hidden")]]
const struct wl_interface xdg_surface_interface = {
    "xdg_surface", 7, 5, xdg_surface_requests, 1, xdg_surface_events,
};

static const struct wl_message xdg_toplevel_requests[] = {
    {"destroy", "", xdg_shell_types + 0},
    {"set_parent", "?o", xdg_shell_types + 11},
    {"set_title", "s", xdg_shell_types + 0},
    {"set_app_id", "s", xdg_shell_types + 0},
    {"show_window_menu", "ouii", xdg_shell_types + 12},
    {"move", "ou", xdg_shell_types + 16},
    {"resize", "ouu", xdg_shell_types + 18},
    {"set_max_size", "ii", xdg_shell_types + 0},
    {"set_min_size", "ii", xdg_shell_types + 0},
    {"set_maximized", "", xdg_shell_types + 0},
    {"unset_maximized", "", xdg_shell_types + 0},
    {"set_fullscreen", "?o", xdg_shell_types + 21},
    {"unset_fullscreen", "", xdg_shell_types + 0},
    {"set_minimized", "", xdg_shell_types + 0},
};

static const struct wl_message xdg_toplevel_events[] = {
    {"configure", "iia", xdg_shell_types + 0},
    {"close", "", xdg_shell_types + 0},
    {"configure_bounds", "4ii", xdg_shell_types + 0},
    {"wm_capabilities", "5a", xdg_shell_types + 0},
};

[[gnu::visibility("hidden")]]
const struct wl_interface xdg_toplevel_interface = {
    "xdg_toplevel", 7, 14, xdg_toplevel_requests, 4, xdg_toplevel_events,
};

static const struct wl_message xdg_popup_requests[] = {
    {"destroy", "", xdg_shell_types + 0},
    {"grab", "ou", xdg_shell_types + 22},
    {"reposition", "3ou", xdg_shell_types + 24},
};

static const struct wl_message xdg_popup_events[] = {
    {"configure", "iiii", xdg_shell_types + 0},
    {"popup_done", "", xdg_shell_types + 0},
    {"repositioned", "3u", xdg_shell_types + 0},
};

[[gnu::visibility("hidden")]]
const struct wl_interface xdg_popup_interface = {
    "xdg_popup", 7, 3, xdg_popup_requests, 3, xdg_popup_events,
};

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

    pWidth = width;
    pHeight = height;
    printf("Window dimensions adjusted: %dx%d.\n", width, height);
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
    pScaleFactor = factor;
    printf("Monitor scale %d.\n", pScaleFactor);
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
            perror("Failed to connect to compositor. Code: ");
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
        if (pOutput == nullptr) perror("Failed to connect to output. Code: ");
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
    // TODO: Implment user-controlled Wayland server via command line
    // TODO: arguments given to the executable.
    pDisplay = wl_display_connect(nullptr);
    if (pDisplay == nullptr)
    {
        perror("Failed to connect to Wayland display server. Code: ");
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

    return true;
}

void windowDestroy(void) {}

bool windowProcess(void) { return wl_display_dispatch(pDisplay) != -1; }

void windowGetSize(uint32_t *width, uint32_t *height)
{
    *width = pWidth * pScaleFactor;
    *height = pHeight * pScaleFactor;
}

void windowGetData(void **data)
{
    data[0] = pDisplay;
    data[1] = pSurface;
}
