#include "Render.h"
#include <imgui.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

using namespace std::placeholders;

kthook::kthook_signal<HWND(__cdecl*)(HINSTANCE)> hookGameInstanceInit{ 0x745560 };
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

void c_render::pauseScreen(bool state) {
    static DWORD updateMouseProtection, rsMouseSetPosProtFirst, rsMouseSetPosProtSecond;

    if (state) {
        ::VirtualProtect(reinterpret_cast<void*>(0x53F3C6U), 5U, PAGE_EXECUTE_READWRITE, &updateMouseProtection);
        ::VirtualProtect(reinterpret_cast<void*>(0x53E9F1U), 5U, PAGE_EXECUTE_READWRITE, &rsMouseSetPosProtFirst);
        ::VirtualProtect(reinterpret_cast<void*>(0x748A1BU), 5U, PAGE_EXECUTE_READWRITE, &rsMouseSetPosProtSecond);

        *reinterpret_cast<uint8_t*>(0x53F3C6U) = 0xE9U;
        *reinterpret_cast<uint32_t*>(0x53F3C6U + 1U) = 0x15BU;

        memset(reinterpret_cast<void*>(0x53E9F1U), 0x90, 5U);
        memset(reinterpret_cast<void*>(0x748A1BU), 0x90, 5U);
    }
    else {
        memcpy(reinterpret_cast<void*>(0x53F3C6U), "\xE8\x95\x6C\x20\x00", 5U);
        memcpy(reinterpret_cast<void*>(0x53E9F1U), "\xE8\xAA\xAA\x0D\x00", 5U);
        memcpy(reinterpret_cast<void*>(0x748A1BU), "\xE8\x80\x0A\xED\xFF", 5U);

        using CPad_ClearMouseHistory_t = void(__cdecl*)();
        using CPad_UpdatePads_t = void(__cdecl*)();
        reinterpret_cast<CPad_ClearMouseHistory_t>(0x541BD0U)();
        reinterpret_cast<CPad_UpdatePads_t>(0x541DD0U)();

        ::VirtualProtect(reinterpret_cast<void*>(0x53F3C6U), 5U, updateMouseProtection, &updateMouseProtection);
        ::VirtualProtect(reinterpret_cast<void*>(0x53E9F1U), 5U, rsMouseSetPosProtFirst, &rsMouseSetPosProtFirst);
        ::VirtualProtect(reinterpret_cast<void*>(0x748A1BU), 5U, rsMouseSetPosProtSecond, &rsMouseSetPosProtSecond);
    }
}

std::uintptr_t c_render::findDevice(std::uint32_t len) {
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

void* c_render::getFunctionAddress(int VTableIndex) {
    return (*reinterpret_cast<void***>(findDevice(0x128000)))[VTableIndex];
}

std::optional<HRESULT> c_render::onPresent(const decltype(hookPresent)& hook, IDirect3DDevice9* pDevice, const RECT*, const RECT*, HWND, const RGNDATA*) {
    if (!ImGuiInited) {
        ImGui::CreateContext();
        ImGui_ImplWin32_Init(gameHwnd);
        ImGui_ImplDX9_Init(pDevice);
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
        ImGui::GetIO().IniFilename = nullptr;

        #pragma warning(push)
        #pragma warning(disable: 4996)
        std::string font{ getenv("WINDIR") }; font += "\\Fonts\\Arialbd.TTF";
        #pragma warning(pop)
        ImGui::GetIO().Fonts->AddFontFromFileTTF(font.c_str(), 15.f, NULL, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());

        hookWndProc.set_dest(GetWindowLongPtrA(gameHwnd, GWLP_WNDPROC));
        hookWndProc.set_cb(std::bind(&c_render::onWndProc, this, _1, _2, _3, _4, _5));
        hookWndProc.install();

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

std::optional<HRESULT> c_render::onLost(const decltype(hookReset)& hook, IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* parameters) {
    ImGui_ImplDX9_InvalidateDeviceObjects();
    return std::nullopt;
}

HRESULT c_render::onWndProc(const decltype(hookWndProc)& hook, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_KEYDOWN) {
        if (wParam == VK_F9 && (HIWORD(lParam) & KF_REPEAT) != KF_REPEAT) {
            ImGuiWindow = { !ImGuiWindow };
            pauseScreen(ImGuiWindow);
            ImGui::GetIO().MouseDrawCursor = ImGuiWindow;
        }
    }

    ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam);
    return hook.get_trampoline()(hwnd, uMsg, wParam, lParam);
}

c_render::c_render() {
    hookPresent.set_dest(getFunctionAddress(17));
    hookReset.set_dest(getFunctionAddress(16));
    hookPresent.before += std::bind(&c_render::onPresent, this, _1, _2, _3, _4, _5, _6);
    hookReset.before += std::bind(&c_render::onLost, this, _1, _2, _3);
    hookPresent.install();
    hookReset.install();
}

c_render::~c_render() {
    if (ImGui::GetCurrentContext()) {
        ImGui_ImplDX9_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }
}