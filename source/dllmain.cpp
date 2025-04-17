#include "Plugin.h"
#include <memory>

std::unique_ptr<c_plugin> plugin;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
    if (dwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        plugin = std::make_unique<c_plugin>(hModule);
    }
    else if (dwReason == DLL_PROCESS_DETACH) {
        plugin->~c_plugin();
        plugin.reset();
    }
    return TRUE;
}