#include "main.h"
#include <memory>

std::unique_ptr<Plugin> plugin;

PluginRender render;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
    if (dwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        plugin = std::make_unique<Plugin>(hModule);        
    }
    else if (dwReason == DLL_PROCESS_DETACH) {
        rakhook::destroy();
        plugin.reset();
        render.~PluginRender();
    }
    return TRUE;
}