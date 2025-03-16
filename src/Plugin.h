#pragma once

#include "PluginRender.h"
#include <kthook/kthook.hpp>

class Plugin {
public:
    Plugin(HMODULE hModule);
    HMODULE hModule;
private:
    using CTimerProto = void( __cdecl* )();
    bool inited = false;
    PluginRender render;
    kthook::kthook_simple<CTimerProto> hookCTimerUpdate{ reinterpret_cast<void*>(0x561B10) };
    void MainLoop(const decltype(hookCTimerUpdate)& hook);
};