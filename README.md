# `u` is for Unicode ![](https://github.com/user-attachments/assets/836076b0-5134-4a3a-9246-a4a267733e43)

_The hiragana ゆ, which you can see in the icon, is also read "yu". It also somewhat resembles the Cyrillic letter Ю, also read "yu", which is kind of hilarious._

This is a tool that allows you to input Unicode characters. Press `Ctrl+Shift+U`, type the code point in hex, press `Enter`. Backspace also works.

Only 2-byte code points are supported right now (so, 4 hex digits).

The digits are displayed in a small window in the top left corder of your screen.

The program will present itself in the notification area. Clicking the icon closes it.

## Installation

Download the binary from the latest [release](https://github.com/snailbaron/u/releases). Run.

## How it works

The project is built with "Visual Studio 2022 (v143)" Platform Toolset, whatever that means. If you can run binaries built with VS 2022, you are probably fine.

The program uses a [hook](https://learn.microsoft.com/en-us/windows/win32/winmsg/hooks) to intercept `Ctrl+Shift+U`.
