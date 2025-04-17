#pragma once

#include <kthook/kthook.hpp>
#include "PluginRender.h"

class c_plugin {
public:
    c_plugin(HMODULE hModule);
    ~c_plugin();

    HMODULE hModule;
private:
    c_pluginRender render;

    bool inited = false;

    using CTimerProto = void(__cdecl*)();
    kthook::kthook_simple<CTimerProto> hookCTimerUpdate{ reinterpret_cast<void*>(0x561B10) };
    void mainloop(const decltype(hookCTimerUpdate)& hook);
};