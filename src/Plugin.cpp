#include "main.h"

Plugin::Plugin(HMODULE hndl) : hModule(hndl) {
    using namespace std::placeholders;
    hookCTimerUpdate.set_cb(std::bind(&Plugin::MainLoop, this, _1));
    hookCTimerUpdate.install();
}

void Plugin::MainLoop(const decltype(hookCTimerUpdate)& hook) {
    if (!inited && samp::RefNetGame() && rakhook::initialize()) {
        StringCompressor::AddReference();

        samp::RefInputBox()->AddCommand("cmd", [](const char* param) {
            samp::RefChat()->AddMessage(-1, "Plugin cmd");
        });

        inited = true;
    }
    else {

    }

    return hook.get_trampoline()();
}