#pragma once

#include <d3d9.h>
#include <kthook/kthook.hpp>

class c_render {
public:
    c_render();
    ~c_render();
private:
    using PresentSignature = HRESULT(__stdcall*)(IDirect3DDevice9*, const RECT*, const RECT*, HWND, const RGNDATA*);
    using ResetSignature = HRESULT(__stdcall*)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);

    bool ImGuiInited = false;
    bool ImGuiWindow = false;

    void pauseScreen(bool state);

    std::uintptr_t findDevice(std::uint32_t Len);
    void* getFunctionAddress(int VTableIndex);

    kthook::kthook_signal<PresentSignature> hookPresent{};
    kthook::kthook_signal<ResetSignature> hookReset{};
    std::optional<HRESULT> onPresent(const decltype(hookPresent)& hook, IDirect3DDevice9* pDevice, const RECT*, const RECT*, HWND, const RGNDATA*);
    std::optional<HRESULT> onLost(const decltype(hookReset)& hook, IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* parameters);

    kthook::kthook_simple<WNDPROC> hookWndProc{};
    HRESULT onWndProc(const decltype(hookWndProc)& hook, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};