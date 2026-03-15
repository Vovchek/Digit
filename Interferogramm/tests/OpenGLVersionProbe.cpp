#include <windows.h>
#include <gl/GL.h>

#ifndef GL_SHADING_LANGUAGE_VERSION
#define GL_SHADING_LANGUAGE_VERSION 0x8B8C
#endif

#include <iostream>

#pragma comment(lib, "opengl32.lib")

namespace
{
    LPCSTR SafeGlString(GLenum name)
    {
        const GLubyte* value = glGetString(name);
        return value ? reinterpret_cast<LPCSTR>(value) : "<unavailable>";
    }
}

int main()
{
    WNDCLASSA wc{};
    wc.lpfnWndProc = DefWindowProcA;
    wc.hInstance = GetModuleHandleA(nullptr);
    wc.lpszClassName = "OpenGLVersionProbeWindow";

    if (!RegisterClassA(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        std::cerr << "Failed to register window class.\n";
        return 1;
    }

    HWND hwnd = CreateWindowA(
        wc.lpszClassName,
        "OpenGLVersionProbe",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        64,
        64,
        nullptr,
        nullptr,
        wc.hInstance,
        nullptr);

    if (!hwnd) {
        std::cerr << "Failed to create window.\n";
        return 2;
    }

    HDC hdc = GetDC(hwnd);
    if (!hdc) {
        std::cerr << "Failed to get device context.\n";
        DestroyWindow(hwnd);
        return 3;
    }

    PIXELFORMATDESCRIPTOR pfd{};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 24;
    pfd.iLayerType = PFD_MAIN_PLANE;

    const int pixelFormat = ChoosePixelFormat(hdc, &pfd);
    if (pixelFormat == 0 || !SetPixelFormat(hdc, pixelFormat, &pfd)) {
        std::cerr << "Failed to configure pixel format.\n";
        ReleaseDC(hwnd, hdc);
        DestroyWindow(hwnd);
        return 4;
    }

    HGLRC hglrc = wglCreateContext(hdc);
    if (!hglrc) {
        std::cerr << "Failed to create OpenGL context.\n";
        ReleaseDC(hwnd, hdc);
        DestroyWindow(hwnd);
        return 5;
    }

    if (!wglMakeCurrent(hdc, hglrc)) {
        std::cerr << "Failed to activate OpenGL context.\n";
        wglDeleteContext(hglrc);
        ReleaseDC(hwnd, hdc);
        DestroyWindow(hwnd);
        return 6;
    }

    std::cout << "OpenGL vendor  : " << SafeGlString(GL_VENDOR) << '\n';
    std::cout << "OpenGL renderer: " << SafeGlString(GL_RENDERER) << '\n';
    std::cout << "OpenGL version : " << SafeGlString(GL_VERSION) << '\n';
    std::cout << "GLSL version   : " << SafeGlString(GL_SHADING_LANGUAGE_VERSION) << '\n';
    std::cout << "\nNote: on hybrid laptops, the reported adapter depends on which GPU this process runs on.\n";

    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(hglrc);
    ReleaseDC(hwnd, hdc);
    DestroyWindow(hwnd);
    UnregisterClassA(wc.lpszClassName, wc.hInstance);

    return 0;
}
