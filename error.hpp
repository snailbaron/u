#pragma once

#include <windows.h>

#include <concepts>
#include <exception>
#include <source_location>
#include <string>
#include <string_view>
#include <type_traits>

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

template <class T>
requires std::integral<T> || std::is_pointer_v<T>
T check(
    T value,
    std::string_view message,
    std::source_location sl = std::source_location::current())
{
    if (value == 0) {
        auto error = GetLastError();
        throw WindowsError{error, message, sl};
    }
    return value;
}

#define CHECK(E) check((E), #E)
