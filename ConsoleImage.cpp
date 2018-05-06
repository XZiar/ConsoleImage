#include <cstdint>
#include <cstdio>
#include <string>
#include <cstring>
#include <vector>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "3rd/stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "3rd/stb_image_resize.h"
#define FMT_HEADER_ONLY
#include "3rd/format.h"

#if defined(_WIN32)
#   define NOMINMAX 1
#   define WIN32_LEAN_AND_MEAN 1
#   include <Windows.h>
#else
#   include <sys/ioctl.h>
#   include <unistd.h>
#endif

using std::string;
using std::vector;
using std::cin;

void PrintLine(const string& u8str)
{
#if defined(_WIN32)
    const auto u16str = fmt::internal::UTF8ToUTF16::UTF8ToUTF16(u8str);
    
#else
    printf("%s\x1b[39m\n", u8str.c_str());
#endif
}

int main()
{
#if defined(_WIN32)
    {
        const auto handle = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode;
        GetConsoleMode(handle, &mode);
        mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(handle, mode);
    }
#endif
    printf("input scale ratio:");
    double scale = 0.8;
    cin >> scale;
    fmt::BasicMemoryWriter<char> writer;
    while (true)
    {
        //read image
        printf("input image path:");
        string fpath;
        while (fpath.empty())
            std::getline(cin, fpath);
        int x, y, dummy;
        auto* data = stbi_load(fpath.c_str(), &x, &y, &dummy, 3);

        //determine image's new size
        int width, height, stride;
        {
            int wdRow = 0, wdCol = 0;
        #if defined(_WIN32)
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
            wdRow = csbi.srWindow.Right - csbi.srWindow.Left + 1;
            wdCol = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        #else
            struct winsize ws;
            ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
            wdRow = ws.ws_col;
            wdCol = ws.ws_row;
        #endif
            width = wdRow;
            stride = width * 3;
            height = ((uint32_t)(wdRow * y * scale) / x) / 2 * 2;
            printf("console is [%dx%d], resize img [%dx%d] to [%dx%d]\n", wdRow, wdCol, x, y, width, height);
        }
        
        //resize image
        std::vector<uint8_t> newImg(width*height * 3);
        stbir_resize_uint8(data, x, y, 0, newImg.data(), width, height, 0, 3);
        stbi_image_free(data);

        //show image --- row by row
        for (int y = 0; y < height; y += 2)
        {
            const auto* ptrUp = newImg.data() + stride * y;
            const auto* ptrDown = ptrUp + stride;
            for (int x = 0; x < width; ++x)
            {
                writer.write(u8"\x1b[38;2;{};{};{};48;2;{};{};{}m\u2580", ptrUp[0], ptrUp[1], ptrUp[2], ptrDown[0], ptrDown[1], ptrDown[2]);
                ptrUp += 3, ptrDown += 3;
            }
            writer.write("\x1b[39;49m\n");
        #if defined(_WIN32)
            const auto u16str = fmt::internal::UTF8ToUTF16::UTF8ToUTF16({ writer.data(),writer.size() });
            DWORD dummy2;
            WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), u16str.c_str(), u16str.size(), &dummy2, nullptr);
        #else
            printf("%s", writer.c_str());
        #endif
            writer.clear();
        }

    }
}