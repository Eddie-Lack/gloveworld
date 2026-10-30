/* scriptinput: a mupen64plus input plugin whose buttons are set by the front end (emu.py).
 * Build: cc -shared -fPIC -O2 -I/usr/include/mupen64plus -o scriptinput.so scriptinput.c */
#define M64P_PLUGIN_PROTOTYPES 1
#include <string.h>
#include "m64p_types.h"
#include "m64p_plugin.h"

static unsigned int buttons[4];
static CONTROL *controls;

/* Set the button word of controller CTRL: the BUTTONS layout from m64p_plugin.h
 * (bits 0 to 15 buttons, bits 16 to 23 the stick's X, bits 24 to 31 its Y). */
EXPORT void CALL scriptinput_set(int ctrl, unsigned int value)
{
    if (ctrl >= 0 && ctrl < 4)
        buttons[ctrl] = value;
}

EXPORT m64p_error CALL PluginStartup(m64p_dynlib_handle core, void *context, void (*debug)(void *, int, const char *))
{
    return M64ERR_SUCCESS;
}

EXPORT m64p_error CALL PluginShutdown(void)
{
    return M64ERR_SUCCESS;
}

EXPORT m64p_error CALL PluginGetVersion(m64p_plugin_type *type, int *version, int *api, const char **name, int *caps)
{
    if (type) *type = M64PLUGIN_INPUT;
    if (version) *version = 0x010000;
    if (api) *api = 0x020100;
    if (name) *name = "scriptinput";
    if (caps) *caps = 0;
    return M64ERR_SUCCESS;
}

EXPORT void CALL InitiateControllers(CONTROL_INFO info)
{
    int i;

    controls = info.Controls;
    for (i = 0; i < 4; i++) {
        controls[i].Present = i == 0;
        controls[i].RawData = 0;
        controls[i].Plugin = PLUGIN_NONE;
    }
}

EXPORT void CALL GetKeys(int ctrl, BUTTONS *keys)
{
    keys->Value = buttons[ctrl];
}

EXPORT void CALL ControllerCommand(int ctrl, unsigned char *command) {}
EXPORT void CALL ReadController(int ctrl, unsigned char *command) {}
EXPORT int CALL RomOpen(void) { return 1; }
EXPORT void CALL RomClosed(void) {}
EXPORT void CALL SDL_KeyDown(int keymod, int keysym) {}
EXPORT void CALL SDL_KeyUp(int keymod, int keysym) {}
