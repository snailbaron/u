#include "hook.hpp"

#include "error.hpp"
#include "popup.hpp"

namespace {

struct Keys {
    bool ctrlDown = false;
    bool shiftDown = false;
};

Keys _keys;
bool _active = false;
long long _number = 0;

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

    if (kbd.vkCode == 'U' && pressing && _keys.ctrlDown && _keys.shiftDown &&
        !_active) {
        _active = true;
        popup::show();
        return true;
    }

    if (!_active || !pressing) {
        return false;
    }

    if (kbd.vkCode >= '0' and kbd.vkCode <= '9') {
        popup::addDigit((char)kbd.vkCode);
        auto digit = kbd.vkCode - '0';
        _number = 16 * _number + digit;
    } else if (kbd.vkCode >= 'A' && kbd.vkCode <= 'F') {
        popup::addDigit((char)kbd.vkCode);
        auto digit = 10 + kbd.vkCode - 'A';
        _number = 16 * _number + digit;
    } else if (kbd.vkCode == VK_BACK) {
        popup::removeDigit();
        _number /= 16;
    } else if (kbd.vkCode == VK_RETURN) {
        uint16_t codePoint = 0;
        if (_number >= 0 && _number <= 0xffff) {
            codePoint = static_cast<uint16_t>(_number);
        }
        _active = false;
        _number = 0;
        popup::hide();
        popup::clear();

        if (codePoint > 0) {
            CHECK(
                PostThreadMessage(GetCurrentThreadId(), WM_USER, codePoint, 0));
        }
    }
    return true;
}

} // namespace

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
    if (processEvent(pressing, kbd)) {
        return 1;
    }

    return CallNextHookEx(NULL, nCode, wparam, lparam);
}
