#pragma once

class Plugin {
public:
    Plugin(HMODULE hModule);
    HMODULE hModule;
private:
    bool inited = false;

    using CTimerProto = void(__cdecl*)();
    kthook::kthook_simple<CTimerProto> hookCTimerUpdate{ reinterpret_cast<void*>(0x561B10) };
    void MainLoop(const decltype(hookCTimerUpdate)& hook);
};