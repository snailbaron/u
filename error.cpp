#include "error.hpp"

#include <format>

namespace {

std::string errorMessage(DWORD errorCode)
{
    LPSTR buffer = nullptr;

    auto outSize = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        errorCode,
        LANG_ENGLISH,
        reinterpret_cast<LPSTR>(&buffer),
        0,
        nullptr);
    if (outSize == 0) {
        return std::format("Unknown Windows error: {}", errorCode);
    }

    auto message = std::string{buffer};
    LocalFree(buffer);
    return message;
}

} // namespace

WindowsError::WindowsError(
    DWORD errorCode, std::string_view message, std::source_location sl)
    : _message(std::format("{}:{}:{} ({}): {}: {} ({:x}): {}",
        sl.file_name(), sl.line(), sl.column(), sl.function_name(),
        message, errorCode, errorCode, errorMessage(errorCode)))
{ }

const char* WindowsError::what() const noexcept
{
    return _message.c_str();
}

void check(int returnValue, std::string_view message, std::source_location sl)
{
    if (!returnValue) {
        auto error = GetLastError();
        throw WindowsError{error, message, sl};
    }
}