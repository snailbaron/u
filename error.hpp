#pragma once

#include <windows.h>

#include <exception>
#include <source_location>
#include <string>
#include <string_view>

class WindowsError : public std::exception {
public:
    WindowsError(
        DWORD errorCode,
        std::string_view message,
        std::source_location sl = std::source_location::current());

    const char* what() const noexcept override;

private:
    std::string _message;
};

void check(
    int returnValue,
    std::string_view message,
    std::source_location sl = std::source_location::current());

template <class T>
[[nodiscard]] T* check(
    T* ptr,
    std::string_view message,
    std::source_location sl = std::source_location::current())
{
    if (ptr == nullptr) {
        throw WindowsError{GetLastError(), message, sl};
    }
    return ptr;
}

#define CHECK(E) check((E), #E)