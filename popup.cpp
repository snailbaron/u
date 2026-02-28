#include "popup.hpp"

#include "error.hpp"

#include <utility>

namespace popup {

namespace {

HWND _popup = NULL;
std::string _popupText;

LRESULT popupWindowProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
    if (umsg == WM_PAINT) {
        auto ps = PAINTSTRUCT{};
        auto hdc = CHECK(BeginPaint(_popup, &ps));

        auto rect = RECT{};
        CHECK(GetClientRect(_popup, &rect));

        CHECK(FillRect(hdc, &rect, (HBRUSH)(COLOR_WINDOW + 1)));
        CHECK(DrawTextA(hdc, _popupText.c_str(), -1, &rect, 0));

        EndPaint(_popup, &ps);
        return 0;
    }

    return DefWindowProc(hwnd, umsg, wparam, lparam);
}

void redraw()
{
    CHECK(InvalidateRect(_popup, NULL, FALSE));
    CHECK(UpdateWindow(_popup));
}

} // namespace

Init::Init(HINSTANCE hInstance)
{
    auto popupWindowClassInfo = WNDCLASSEX{
        .cbSize = sizeof(WNDCLASSEX),
        .style = 0,
        .lpfnWndProc = popupWindowProc,
        .cbClsExtra = 0,
        .cbWndExtra = 0,
        .hInstance = hInstance,
        .hIcon = NULL,
        .hCursor = NULL,
        .hbrBackground = NULL,
        .lpszMenuName = NULL,
        .lpszClassName = TEXT("UPOPUP"),
        .hIconSm = NULL,
    };
    auto popupWindowClass = CHECK(RegisterClassEx(&popupWindowClassInfo));

    _popup = CHECK(CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
        (LPCTSTR)popupWindowClass,
        TEXT("U"),
        WS_POPUP | WS_BORDER,
        0,
        0,
        100,
        30,
        NULL,
        NULL,
        hInstance,
        NULL));
}

Init::~Init() { }

void show()
{
    ShowWindow(_popup, SW_SHOWNOACTIVATE);
}

void hide()
{
    ShowWindow(_popup, SW_HIDE);
}

void clear()
{
    _popupText = "";
    redraw();
}

void addDigit(char digit)
{
    _popupText += digit;
    redraw();
}

void removeDigit()
{
    if (!_popupText.empty()) {
        _popupText.pop_back();
        redraw();
    }
}

} // namespace popup
