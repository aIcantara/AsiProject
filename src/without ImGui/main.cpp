#include "main.h"

void Plugin::MainLoop(const decltype(hookCTimerUpdate)& hook) {
    static bool inited = false;
    if (!inited && samp::RefNetGame() && rakhook::initialize()) {
        StringCompressor::AddReference();

        samp::RefInputBox()->AddCommand("cmd", [](const char* param) {
            samp::RefChat()->AddMessage(0xFFFFFFFF, "Plugin cmd");
        });

        inited = true;
    }
    else {

    }

    return hook.get_trampoline()();
}

Plugin::Plugin(HMODULE hndl) : hModule(hndl) {
    using namespace std::placeholders;
    hookCTimerUpdate.set_cb(std::bind(&Plugin::MainLoop, this, _1));
    hookCTimerUpdate.install();
}

std::unique_ptr<Plugin> plugin;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
    switch (dwReason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        plugin = std::make_unique<Plugin>(hModule);
        break;
    case DLL_PROCESS_DETACH:
        rakhook::destroy();
        plugin.reset();
        break;
    }
    return TRUE;
}