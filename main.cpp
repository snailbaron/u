#include "error.hpp"
#include "hook.hpp"
#include "popup.hpp"

#include "resource.h"

#include <windows.h>

#include <array>
#include <cstdlib>
#include <cstring>
#include <format>
#include <iostream>
#include <utility>

constexpr int WM_U_TRAY = WM_APP + 1;

class HookGuard {
public:
    HookGuard(const HookGuard&) = delete;
    HookGuard& operator=(const HookGuard&) = delete;

    HookGuard() = default;

    HookGuard(int idHook, HOOKPROC hookProc)
    {
        hook(idHook, hookProc);
    }

    ~HookGuard()
    {
        if (_hook != NULL) {
            UnhookWindowsHookEx(_hook);
        }
    }

    HookGuard(HookGuard&& other) noexcept
    {
        swap(*this, other);
    }

    HookGuard& operator=(HookGuard&& other) noexcept
    {
        if (this != &other) {
            reset();
            swap(*this, other);
        }
        return *this;
    }

    void hook(int idHook, HOOKPROC hookProc)
    {
        reset();
        _hook = CHECK(SetWindowsHookEx(idHook, hookProc, NULL, 0));
    }

    void reset()
    {
        if (_hook != NULL) {
            CHECK(UnhookWindowsHookEx(_hook));
            _hook = NULL;
        }
    }

    friend void swap(HookGuard& lhs, HookGuard& rhs) noexcept
    {
        std::swap(lhs._hook, rhs._hook);
    }

private:
    HHOOK _hook = NULL;
};

void sendCodePointInput(uint16_t codePoint)
{
    auto inputs = std::array<INPUT, 2>{};
    std::memset(inputs.data(), 0, sizeof(INPUT) * inputs.size());

    inputs.at(0).type = INPUT_KEYBOARD;
    inputs.at(0).ki.wScan = codePoint;
    inputs.at(0).ki.dwFlags = KEYEVENTF_UNICODE;

    inputs.at(1).type = INPUT_KEYBOARD;
    inputs.at(1).ki.wScan = codePoint;
    inputs.at(1).ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;

    std::cerr << std::format("sending code point 0x{:x}\n", codePoint);
    CHECK(SendInput((UINT)inputs.size(), inputs.data(), sizeof(INPUT)));
}

LRESULT messageWindowProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
    if (umsg == WM_U_TRAY && lparam == WM_LBUTTONUP) {
        PostQuitMessage(0);
    }

    return DefWindowProc(hwnd, umsg, wparam, lparam);
}

int WINAPI WinMain(
    HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
try {
    auto messageWindowClassInfo = WNDCLASSEX{
        .cbSize = sizeof(WNDCLASSEX),
        .lpfnWndProc = messageWindowProc,
        .hInstance = hInstance,
        .lpszClassName = TEXT("MESSAGEWINDOW"),
    };
    auto popupWindowClass = CHECK(RegisterClassEx(&messageWindowClassInfo));

    auto messageWindow = CHECK(CreateWindowEx(
        0,
        (LPCTSTR)popupWindowClass,
        TEXT("U"),
        WS_MINIMIZE,
        0,
        0,
        0,
        0,
        HWND_MESSAGE,
        NULL,
        hInstance,
        NULL));

    auto appIcon = CHECK(LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON)));

    auto notifyIconData = NOTIFYICONDATA{
        .cbSize = sizeof(NOTIFYICONDATA),
        .hWnd = messageWindow,
        .uID = 0,
        .uFlags = NIF_MESSAGE | NIF_ICON,
        .uCallbackMessage = WM_U_TRAY,
        .hIcon = appIcon,

    };
    CHECK(Shell_NotifyIcon(NIM_ADD, &notifyIconData));

    auto pop = popup::Init{hInstance};

    auto hook = HookGuard{WH_KEYBOARD_LL, hookProc};

    MSG msg;
    BOOL ret;
    while ((ret = GetMessage(&msg, NULL, 0, 0))) {
        if (ret == -1) {
            throw WindowsError{GetLastError(), "GetMessage"};
        }

        if (msg.message == WM_USER) {
            auto codePoint = static_cast<uint16_t>(msg.wParam);
            sendCodePointInput(codePoint);
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return EXIT_SUCCESS;
} catch (const std::exception& e) {
    MessageBoxA(NULL, e.what(), NULL, 0);
    return EXIT_FAILURE;
} catch (...) {
    MessageBoxA(NULL, "unknown error", NULL, 0);
    return EXIT_FAILURE;
}
