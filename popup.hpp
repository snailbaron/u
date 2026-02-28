#pragma once

#include <windows.h>

#include <string>

namespace popup {

class Init {
public:
    Init(const Init&) = delete;
    Init& operator=(const Init&) = delete;

    Init(Init&&) noexcept = delete;
    Init& operator=(Init&&) noexcept = delete;

    explicit Init(HINSTANCE hInstance);
    ~Init();
};

void show();
void hide();

void clear();
void addDigit(char digit);
void removeDigit();

} // namespace popup
