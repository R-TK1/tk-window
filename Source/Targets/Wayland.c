#include <TKWindow.h>
#include <Targets/Wayland.h>
#include <Targets/XDGShell.h>
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

static void handleConfigure(void *data, struct xdg_surface *shellSurface,
                            uint32_t serial)
{
    (void)data;
    xdg_surface_ack_configure(shellSurface, serial);
}

static const struct xdg_surface_listener pSurfaceListener = {
    .configure = handleConfigure};

static void handleTopClose(void *data, struct xdg_toplevel *toplevel)
{
    (void)data;
    (void)toplevel;
    rpgtk_windowClose();
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

static const struct xdg_toplevel_listener pToplevelListener = {
    .close = handleTopClose,
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
    xdg_wm_base_pong(shell, serial);
}

static struct xdg_wm_base_listener pShellListener = {.ping = handlePing};

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
    else if (strcmp(interface, xdg_wm_base_interface.name) == 0)
    {
        pShell = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(pShell, &pShellListener, NULL);
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

bool tkwin_waylandCreate(void)
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
    pShellSurface = xdg_wm_base_get_xdg_surface(pShell, pSurface);
    xdg_surface_add_listener(pShellSurface, &pSurfaceListener, nullptr);

    pToplevel = xdg_surface_get_toplevel(pShellSurface);
    xdg_toplevel_add_listener(pToplevel, &pToplevelListener, nullptr);

    xdg_toplevel_set_title(pToplevel, "Stormsinger");
    xdg_toplevel_set_app_id(pToplevel, "Stormsinger");
    xdg_toplevel_set_fullscreen(pToplevel, pOutput);

    wl_surface_commit(pSurface);
    wl_display_roundtrip(pDisplay);
    wl_surface_commit(pSurface);

    return true;
}

void tkwin_waylandDestroy(void) {}

bool tkwin_waylandPoll(void) { return wl_display_dispatch(pDisplay) != -1; }

void tkwin_waylandGetFramebufferSize(uint32_t *width, uint32_t *height)
{
    *width = pWidth * pScaleFactor;
    *height = pHeight * pScaleFactor;
}

void tkwin_waylandGetSurfaceData(void **data)
{
    data[0] = pDisplay;
    data[1] = pSurface;
}
