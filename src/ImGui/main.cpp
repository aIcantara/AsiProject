#include "main.h"

using PresentSignature = HRESULT(__stdcall*)(IDirect3DDevice9*, const RECT*, const RECT*, HWND, const RGNDATA*);
using ResetSignature = HRESULT(__stdcall*)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
using InitGameInstance = HWND(__cdecl*)(HINSTANCE);

bool ImGuiWindow{};

std::uintptr_t find_device(std::uint32_t Len) {
    static std::uintptr_t base = [](std::size_t Len) {
        std::string path_to(MAX_PATH, '\0');
        if (auto size = GetSystemDirectoryA(path_to.data(), MAX_PATH)) {
            path_to.resize(size);
            path_to += "\\d3d9.dll";
            std::uintptr_t dwObjBase = reinterpret_cast<std::uintptr_t>(LoadLibraryA(path_to.c_str()));
            while (dwObjBase++ < dwObjBase + Len) {
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
        }(Len);
    return base;
}

void* get_function_address(int VTableIndex) {
    return (*reinterpret_cast<void***>(find_device(0x128000)))[VTableIndex];
}

kthook::kthook_signal<InitGameInstance> game_instance_init_hook{ 0x745560 };

HWND game_hwnd = []() {
    HWND* hwnd_ptr = *reinterpret_cast<HWND**>(0xC17054);
    if (hwnd_ptr != nullptr) {
        return *hwnd_ptr;
    }
    else {
        game_instance_init_hook.after += [](const auto& hook, HWND& return_value, HINSTANCE inst) {
            game_hwnd = return_value;
            };
        return HWND(0);
    }
}();

kthook::kthook_signal<PresentSignature> present_hook{ get_function_address(17) };
kthook::kthook_signal<ResetSignature> reset_hook{ get_function_address(16) };
kthook::kthook_simple<WNDPROC> wndproc_hook{};

HRESULT __stdcall on_wndproc(const decltype(wndproc_hook)& hook, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_KEYDOWN: {
        if (wParam == VK_INSERT && (HIWORD(lParam) & KF_REPEAT) != KF_REPEAT) {
            ImGuiWindow = { !ImGuiWindow };
        }
        break;
    }
    }
    if (uMsg == WM_CHAR) {
        wchar_t wch;
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, reinterpret_cast<char*>(&wParam), 1, &wch, 1);
        wParam = wch;
    }
    ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam);
    auto& io = ImGui::GetIO();
    if (io.WantCaptureKeyboard || io.WantCaptureMouse) {
        return 1;
    }

    return hook.get_trampoline()(hwnd, uMsg, wParam, lParam);
}

std::optional<HRESULT> on_present(const decltype(present_hook)& hook, IDirect3DDevice9* device_ptr, const RECT*, const RECT*, HWND, const RGNDATA*) {
    static bool ImGui_inited = false;
    if (!ImGui_inited) {
        ImGui::CreateContext();
        ImGui_ImplWin32_Init(game_hwnd);
        ImGui_ImplDX9_Init(device_ptr);
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
        ImGui::GetIO().IniFilename = nullptr;

        #pragma warning(push)
        #pragma warning(disable: 4996)
        std::string font{ getenv("WINDIR") }; font += "\\Fonts\\Arialbd.TTF";
        #pragma warning(pop)
        ImGui::GetIO().Fonts->AddFontFromFileTTF(font.c_str(), 15.f, NULL, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());

        auto latest_wndproc_ptr = GetWindowLongPtrA(game_hwnd, GWLP_WNDPROC);
        wndproc_hook.set_dest(latest_wndproc_ptr);
        wndproc_hook.set_cb(&on_wndproc);
        wndproc_hook.install();
        ImGui_inited = true;
    }

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (ImGuiWindow) {
        ImGui::SetNextWindowPos(ImVec2(100.f, 100.f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(250.f, 300.f), ImGuiCond_FirstUseEver);
        ImGui::Begin("AsiProject", &ImGuiWindow);

        ImGui::Text("Text");

        ImGui::End();
    }

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

    return std::nullopt;
}

std::optional<HRESULT> on_lost(const decltype(reset_hook)& hook, IDirect3DDevice9* device_ptr, D3DPRESENT_PARAMETERS* parameters) {
    ImGui_ImplDX9_InvalidateDeviceObjects();
    return std::nullopt;
}

void on_reset(const decltype(reset_hook)& hook, HRESULT& return_value, IDirect3DDevice9* device_ptr, D3DPRESENT_PARAMETERS* parameters) {}

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
        present_hook.before += on_present;
        reset_hook.before += on_lost;
        reset_hook.after += on_reset;
        break;
    case DLL_PROCESS_DETACH:
        rakhook::destroy();
        plugin.reset();
        present_hook.remove();
        reset_hook.remove();
        ImGui_ImplDX9_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        break;
    }
    return TRUE;
}