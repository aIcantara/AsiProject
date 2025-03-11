#include "main.h"

bool ImGuiEnable{};

HRESULT WndProc(const decltype(WndProcHook)& hook, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_KEYUP) {
        if (wParam == VK_F9)
            ImGuiEnable = { !ImGuiEnable };
    }
    
    if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam)) {
        return 1;
    }
    return hook.get_trampoline()(hwnd, uMsg, wParam, lParam);
}

std::optional<HRESULT> D3D9Present(const decltype(PresentHook)& hook, IDirect3DDevice9* pDevice, CONST RECT* pSrcRect, CONST RECT* pDestRect, HWND hDestWindow, CONST RGNDATA* pDirtyRegion) {
    static bool ImGuiInit{};
    if (!ImGuiInit) {
        ImGui::CreateContext();
        ImGui_ImplWin32_Init(**reinterpret_cast<HWND**>(0xC17054));
        ImGui_ImplDX9_Init(pDevice);
        ImGui::GetIO().IniFilename = nullptr; 
        
        #pragma warning(push)
        #pragma warning(disable: 4996)
        std::string font{ getenv("WINDIR") }; font += "\\Fonts\\Arialbd.TTF";
        #pragma warning(pop)
        ImGui::GetIO().Fonts->AddFontFromFileTTF(font.c_str(), 14.0f, NULL, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());

        auto latest_wndproc_ptr = GetWindowLongPtrW(**reinterpret_cast<HWND**>(0xC17054), GWLP_WNDPROC);
        WndProcHook.set_dest(latest_wndproc_ptr);
        WndProcHook.set_cb(&WndProc);
        WndProcHook.install();

        ImGuiInit = { true };
    }

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (ImGuiEnable) {
        ImGui::SetNextWindowPos(ImVec2(100.f, 100.f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(300.f, 100.f), ImGuiCond_FirstUseEver);
        ImGui::Begin("Plugin Menu", &ImGuiEnable);

        ImGui::Text("Text");

        ImGui::End();
    }

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    return std::nullopt;
}

std::optional<HRESULT> D3D9Lost(const decltype(ResetHook)& hook, IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentParams) {
    ImGui_ImplDX9_InvalidateDeviceObjects();
    return std::nullopt;
}

void D3D9Reset(const decltype(ResetHook)& hook, HRESULT& return_value, IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentParams) {}

void setD3D9Hooks() {
    DWORD pDevice = *reinterpret_cast<DWORD*>(0xC97C28);
    void** vTable = *reinterpret_cast<void***>(pDevice);

    PresentHook.set_dest(vTable[17]);
    PresentHook.before.connect(&D3D9Present);
    PresentHook.install();

    ResetHook.set_dest(vTable[16]);
    ResetHook.before.connect(&D3D9Lost);
    ResetHook.after.connect(&D3D9Reset);
    ResetHook.install();
}

void CTimer__Update(const decltype(CTimerHook)& hook) {
    static bool init{};
    if (!init) {
        setD3D9Hooks();

        init = { true };
    }

    hook.get_trampoline()();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        CTimerHook.set_dest(0x561B10);
        CTimerHook.set_cb(&CTimer__Update);
        CTimerHook.install();
        break;
    case DLL_PROCESS_DETACH:
        WndProcHook.remove();
        ResetHook.remove();
        PresentHook.remove();
        CTimerHook.remove();
        break;
    }
    return TRUE;
}