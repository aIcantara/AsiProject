#include "Plugin.h"
#include <RakHook/rakhook.hpp>
#include <RakNet/StringCompressor.h>
#include <sampapi/CNetGame.h>
#include <sampapi/CChat.h>
#include <sampapi/CInput.h>

namespace samp = sampapi::v037r1;

void c_plugin::mainloop(const decltype(hookCTimerUpdate)& hook) {
    if (!inited && samp::RefNetGame() && rakhook::initialize()) {
        StringCompressor::AddReference();

        samp::RefInputBox()->AddCommand("cmd", [](const char* param) {
            samp::RefChat()->AddMessage(-1, "plugin cmd");
        });

        samp::RefChat()->AddMessage(-1, "plugin loaded");

        inited = true;
    }

    return hook.get_trampoline()();
}

c_plugin::c_plugin(HMODULE hndl) : hModule(hndl) {
    using namespace std::placeholders;
    hookCTimerUpdate.set_cb(std::bind(&c_plugin::mainloop, this, _1));
    hookCTimerUpdate.install();
}

c_plugin::~c_plugin() {
    rakhook::destroy();
    render.~c_pluginRender();
}