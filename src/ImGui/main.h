#pragma once

#include <Windows.h>
#include <memory>
#include <d3d9.h>

#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"

#include <kthook/kthook.hpp>

#include <RakHook/rakhook.hpp>
#include <RakNet/StringCompressor.h>

#include <sampapi/CNetGame.h>
#include <sampapi/CChat.h>
#include <sampapi/CInput.h>

namespace samp = sampapi::v037r1;

class Plugin {
private:
    using CTimerProto = void( __cdecl* )();
    kthook::kthook_simple<CTimerProto> hookCTimerUpdate{ reinterpret_cast<void*>(0x561B10) };
    void MainLoop(const decltype(hookCTimerUpdate)& hook);
public:
    Plugin(HMODULE hModule);
    HMODULE hModule;
};

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);