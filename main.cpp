#include "error.hpp"

#include <windows.h>

#include <array>
#include <cstdlib>
#include <cstring>
#include <format>
#include <iostream>

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

class Hooker {
public:
    bool processEvent(bool pressing, const KBDLLHOOKSTRUCT& kbd)
    {
        if (kbd.vkCode == VK_LCONTROL) {
            _keys.ctrlDown = pressing;
            return false;
        }
        if (kbd.vkCode == VK_LSHIFT) {
            _keys.shiftDown = pressing;
            return false;
        }

        if (kbd.vkCode == 'U' && pressing && _keys.ctrlDown &&
            _keys.shiftDown && !_active) {
            _active = true;
            std::cerr << "activate\n";
            return true;
        }

        if (!_active || !pressing) {
            return false;
        }

        if (kbd.vkCode >= '0' and kbd.vkCode <= '9') {
            std::cerr << "got " << (char)kbd.vkCode << "\n";
            auto digit = kbd.vkCode - '0';
            _number = 16 * _number + digit;
        } else if (kbd.vkCode >= 'A' && kbd.vkCode <= 'F') {
            std::cerr << "got " << (char)kbd.vkCode << "\n";
            auto digit = 10 + kbd.vkCode - 'A';
            _number = 16 * _number + digit;
        } else if (kbd.vkCode == VK_RETURN) {
            uint16_t codePoint = 0;
            std::cerr << std::format("number: 0x{:x}\n", _number);
            if (_number >= 0 && _number <= 0xffff) {
                codePoint = static_cast<uint16_t>(_number);
            }
            std::cerr << "deactivate\n";
            _active = false;
            _number = 0;

            std::cerr << std::format("code point: 0x{:x}\n", codePoint);
            if (codePoint > 0) {
                std::cerr << std::format("will send {:x}\n", codePoint);
                CHECK(PostThreadMessage(
                    GetCurrentThreadId(), WM_USER, codePoint, 0));
            }
        }
        return true;
    }

private:
    struct Keys {
        bool ctrlDown = false;
        bool shiftDown = false;
    };

    Keys _keys;
    bool _active = false;
    long long _number = 0;
};

Hooker _hooker;

LRESULT CALLBACK hookProc(int nCode, WPARAM wparam, LPARAM lparam)
{
    if (nCode < 0) {
        return CallNextHookEx(nullptr, nCode, wparam, lparam);
    }

    bool pressing = false;
    if (wparam == WM_KEYDOWN || wparam == WM_SYSKEYDOWN) {
        pressing = true;
    } else if (wparam == WM_KEYUP || wparam == WM_SYSKEYUP) {
        pressing = false;
    } else {
        return CallNextHookEx(nullptr, nCode, wparam, lparam);
    }

    const auto& kbd = *reinterpret_cast<const KBDLLHOOKSTRUCT*>(lparam);
    if (_hooker.processEvent(pressing, kbd)) {
        return 1;
    }

    return CallNextHookEx(NULL, nCode, wparam, lparam);
}

int main()
try {
    auto hook = CHECK(SetWindowsHookEx(WH_KEYBOARD_LL, hookProc, NULL, 0));

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

    CHECK(UnhookWindowsHookEx(hook));
} catch (const std::exception& e) {
    std::cerr << e.what() << "\n";
    return EXIT_FAILURE;
} catch (...) {
    std::cerr << "unknown error\n";
    return EXIT_FAILURE;
}
