#include "main.h"
#include <imgui.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

using namespace std::placeholders;
using InitGameInstance = HWND(__cdecl*)(HINSTANCE);
kthook::kthook_signal<InitGameInstance> hookGameInstanceInit{ 0x745560 };
HWND gameHwnd = []() {
    HWND* pHwnd = *reinterpret_cast<HWND**>(0xC17054);
    if (pHwnd != nullptr) {
        return *pHwnd;
    }
    else {
        hookGameInstanceInit.after += [](const auto& hook, HWND& returnValue, HINSTANCE inst) {
            gameHwnd = returnValue;
        };
        return HWND(0);
    }
}();

PluginRender::PluginRender() : ImGuiInited(false) {
    hookPresent.set_dest(getFunctionAddress(17));
    hookReset.set_dest(getFunctionAddress(16));
    hookPresent.before += std::bind(&PluginRender::onPresent, this, _1, _2, _3, _4, _5, _6);
    hookReset.before += std::bind(&PluginRender::onLost, this, _1, _2, _3);
    hookReset.after += std::bind(&PluginRender::onReset, this, _1, _2, _3, _4);
    hookPresent.install();
    hookReset.install();
}

PluginRender::~PluginRender() {
    hookPresent.remove();
    hookReset.remove();
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

std::uintptr_t PluginRender::findDevice(std::uint32_t len) {
    static std::uintptr_t base = [](std::size_t len) {
        std::string pathTo(MAX_PATH, '\0');
        if (auto size = GetSystemDirectoryA(pathTo.data(), MAX_PATH)) {
            pathTo.resize(size);
            pathTo += "\\d3d9.dll";
            std::uintptr_t dwObjBase = reinterpret_cast<std::uintptr_t>(LoadLibraryA(pathTo.c_str()));
            while (dwObjBase++ < dwObjBase + len) {
                if (*reinterpret_cast<std::uint16_t*>(dwObjBase + 0x00) == 0x06C7 &&
                    *reinterpret_cast<std::uint16_t*>(dwObjBase + 0x06) == 0x8689 &&
                    *reinterpret_cast<std::uint16_t*>(dwObjBase + 0x0C) == 0x8689) {
                    dwObjBase += 2;
                    break;
                }
            }
            return dwObjBase;
        }
        return std::uintptr_t(0);
    }(len);
    return base;
}

void* PluginRender::getFunctionAddress(int VTableIndex) {
    return (*reinterpret_cast<void***>(findDevice(0x128000)))[VTableIndex];
}

std::optional<HRESULT> PluginRender::onPresent(const decltype(hookPresent)& hook, IDirect3DDevice9* pDevice, const RECT*, const RECT*, HWND, const RGNDATA*) {
    if (!ImGuiInited) {
        ImGui::CreateContext();
        ImGui_ImplWin32_Init(gameHwnd);
        ImGui_ImplDX9_Init(pDevice);
        ImGui::GetIO().IniFilename = nullptr;

        #pragma warning(push)
        #pragma warning(disable: 4996)
        std::string font{ getenv("WINDIR") }; font += "\\Fonts\\Arialbd.TTF";
        #pragma warning(pop)
        ImGui::GetIO().Fonts->AddFontFromFileTTF(font.c_str(), 15.f, NULL, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());

        auto latest_wndproc_ptr = GetWindowLongPtrA(gameHwnd, GWLP_WNDPROC);
        hookWndproc.set_dest(latest_wndproc_ptr);
        hookWndproc.set_cb(std::bind(&PluginRender::onWndproc, this, _1, _2, _3, _4, _5));
        hookWndproc.install();

        ImGuiInited = true;
    }

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (ImGuiWindow) {
        ImGui::SetNextWindowPos(ImVec2(100.f, 100.f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(300.f, 300.f), ImGuiCond_FirstUseEver);
        ImGui::Begin("AsiProject", &ImGuiWindow);

        ImGui::Text("Text");

        ImGui::End();
    }

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

    return std::nullopt;
}

std::optional<HRESULT> PluginRender::onLost(const decltype(hookReset)& hook, IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* parameters) {
    ImGui_ImplDX9_InvalidateDeviceObjects();
    return std::nullopt;
}

void PluginRender::onReset(const decltype(hookReset)& hook, HRESULT& returnValue, IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* parameters) {}

HRESULT __stdcall PluginRender::onWndproc(const decltype(hookWndproc)& hook, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_KEYDOWN) {
        if (wParam == VK_F9 && (HIWORD(lParam) & KF_REPEAT) != KF_REPEAT) {
            ImGuiWindow = { !ImGuiWindow };
            samp::RefGame()->SetCursorMode(ImGuiWindow ? samp::CURSOR_LOCKCAM : samp::CURSOR_NONE, !ImGuiWindow);
        }
    }
    if (uMsg == WM_CHAR) {
        wchar_t wch;
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, reinterpret_cast<char*>(&wParam), 1, &wch, 1);
        wParam = wch;
    }

    ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam);
    return hook.get_trampoline()(hwnd, uMsg, wParam, lParam);
}