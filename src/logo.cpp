// Chunky pixel lettering hosted in the Toolkit home window. Every visible
// pixel is an integer-sized square; no fonts, smoothing or gradients are used.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <commctrl.h>
#include <algorithm>
#include <array>
#include <new>

namespace {
constexpr wchar_t kClassName[] = L"OrderDeskPixelLogo";
constexpr int kWidth = 83;
constexpr int kHeight = 11;
using Pixels = std::array<std::array<unsigned char, kWidth>, kHeight>;
using Glyph = std::array<const char*, 9>;
HINSTANCE g_instance = nullptr;

const Glyph& Letter(char letter) {
    static const Glyph o = {".#####.", "#######", "##...##", "##...##", "##...##",
                            "##...##", "##...##", "#######", ".#####."};
    static const Glyph r = {"######.", "#######", "##...##", "##...##", "######.",
                            "#####..", "##.##..", "##..##.", "##...##"};
    static const Glyph d = {"#####..", "######.", "##..###", "##...##", "##...##",
                            "##...##", "##..###", "######.", "#####.."};
    static const Glyph e = {"#######", "#######", "##.....", "##.....", "######.",
                            "######.", "##.....", "#######", "#######"};
    static const Glyph s = {".######", "#######", "##.....", "##.....", ".#####.",
                            ".....##", ".....##", "#######", "######."};
    static const Glyph k = {"##...##", "##..##.", "##.##..", "####...", "####...",
                            "#####..", "##.###.", "##..###", "##...##"};
    switch (letter) {
    case 'O': return o;
    case 'R': return r;
    case 'D': return d;
    case 'E': return e;
    case 'S': return s;
    default: return k;
    }
}

Pixels MakePixels() {
    Pixels face{}, result{};
    constexpr char word[] = "ORDERDESK";
    for (int letter = 0; letter < 9; ++letter) {
        const auto& glyph = Letter(word[letter]);
        for (int y = 0; y < 9; ++y)
            for (int x = 0; x < 7; ++x)
                if (glyph[y][x] == '#') face[y + 2][letter * 9 + x + (8 - y) / 4] = 1;
    }
    // Resolve depth behind the complete face before adding the lit top edges.
    for (int y = 2; y < kHeight; ++y)
        for (int x = 0; x < kWidth - 2; ++x)
            if (face[y][x])
                for (int depth = 1; depth <= 2; ++depth) result[y - depth][x + depth] = 3;
    for (int y = 2; y < kHeight; ++y)
        for (int x = 0; x < kWidth; ++x)
            if (face[y][x]) result[y][x] = face[y - 1][x] ? 1 : 2;
    return result;
}

const Pixels kPixels = MakePixels();

struct Logo {
    HWND window = nullptr;
    HWND parent = nullptr;
    int rows = 1;
    int reveal = 0;
    bool owned = false;
    HBRUSH shades[4]{};
    ~Logo() { for (auto brush : shades) if (brush) DeleteObject(brush); }
};

void Paint(const Logo& logo, HDC dc, int width, int height) {
    const int pixel = std::max(1, std::min(width * 88 / (100 * kWidth),
                                          height * 88 / (100 * kHeight)));
    const int left = (width - kWidth * pixel) / 2;
    const int top = (height - kHeight * pixel) / 2;
    const int visible = (kWidth * logo.reveal + 999) / 1000;
    for (int y = 0; y < kHeight; ++y) {
        for (int x = 0; x < visible;) {
            const auto shade = kPixels[y][x];
            const int start = x++;
            while (x < visible && kPixels[y][x] == shade) ++x;
            if (shade) {
                RECT run{left + start * pixel, top + y * pixel,
                         left + x * pixel, top + (y + 1) * pixel};
                FillRect(dc, &run, logo.shades[shade]);
            }
        }
    }
}

void Layout(const Logo& logo) {
    RECT client{};
    GetClientRect(logo.parent, &client);
    SetWindowPos(logo.window, nullptr, 0, 0, client.right,
                 std::max(1, MulDiv(client.bottom, 6, logo.rows)), SWP_NOACTIVATE | SWP_NOZORDER);
}

LRESULT CALLBACK ParentProc(HWND window, UINT message, WPARAM wparam,
                            LPARAM lparam, UINT_PTR id, DWORD_PTR data) {
    auto* logo = reinterpret_cast<Logo*>(data);
    if (message == WM_NCDESTROY) {
        RemoveWindowSubclass(window, ParentProc, id);
        return DefSubclassProc(window, message, wparam, lparam);
    }
    const LRESULT result = DefSubclassProc(window, message, wparam, lparam);
    if (message == WM_SIZE) Layout(*logo);
    return result;
}

LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    auto* logo = reinterpret_cast<Logo*>(GetWindowLongPtrW(window, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
        logo = static_cast<Logo*>(reinterpret_cast<CREATESTRUCTW*>(lparam)->lpCreateParams);
        logo->window = window;
        SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(logo));
    }
    switch (message) {
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT: {
        PAINTSTRUCT paint{};
        HDC dc = BeginPaint(window, &paint);
        RECT client{};
        GetClientRect(window, &client);
        FillRect(dc, &client, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
        if (logo) Paint(*logo, dc, client.right, client.bottom);
        EndPaint(window, &paint);
        return 0;
    }
    case WM_SIZE:
        InvalidateRect(window, nullptr, FALSE);
        return 0;
    case WM_NCHITTEST:
        return HTTRANSPARENT;
    case WM_NCDESTROY:
        if (logo) {
            RemoveWindowSubclass(logo->parent, ParentProc, reinterpret_cast<UINT_PTR>(logo));
            SetWindowLongPtrW(window, GWLP_USERDATA, 0);
            if (logo->owned) delete logo;
        }
        break;
    }
    return DefWindowProcW(window, message, wparam, lparam);
}
} // namespace

extern "C" __declspec(dllexport) HWND WINAPI OrderLogoCreate(HWND parent, int legacy, int rows) {
    if (!IsWindow(parent) || rows < 6) return nullptr;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       reinterpret_cast<LPCWSTR>(&WindowProc), &g_instance);
    WNDCLASSW cls{};
    cls.lpfnWndProc = WindowProc;
    cls.hInstance = g_instance;
    cls.lpszClassName = kClassName;
    if (!RegisterClassW(&cls) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) return nullptr;
    auto* logo = new (std::nothrow) Logo;
    if (!logo) return nullptr;
    logo->parent = parent;
    logo->rows = rows;
    logo->shades[1] = CreateSolidBrush(legacy ? RGB(244, 187, 57) : RGB(78, 221, 105));
    logo->shades[2] = CreateSolidBrush(legacy ? RGB(255, 230, 129) : RGB(173, 255, 145));
    logo->shades[3] = CreateSolidBrush(legacy ? RGB(135, 80, 30) : RGB(18, 112, 62));
    if (!logo->shades[1] || !logo->shades[2] || !logo->shades[3]) { delete logo; return nullptr; }
    HWND window = CreateWindowExW(0, kClassName, L"", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
                                   0, 0, 1, 1, parent, nullptr, g_instance, logo);
    if (!window) { delete logo; return nullptr; }
    logo->owned = true;
    if (!SetWindowSubclass(parent, ParentProc, reinterpret_cast<UINT_PTR>(logo),
                           reinterpret_cast<DWORD_PTR>(logo))) {
        DestroyWindow(window);
        return nullptr;
    }
    Layout(*logo);
    return window;
}

extern "C" __declspec(dllexport) void WINAPI OrderLogoReveal(HWND window, int amount) {
    auto* logo = reinterpret_cast<Logo*>(GetWindowLongPtrW(window, GWLP_USERDATA));
    if (logo) {
        logo->reveal = std::clamp(amount, 0, 1000);
        InvalidateRect(window, nullptr, FALSE);
        UpdateWindow(window);
    }
}

extern "C" __declspec(dllexport) void WINAPI OrderLogoDestroy(HWND window) {
    if (IsWindow(window)) DestroyWindow(window);
}

// Toolkit owns the parent window. Unregister only after its children are gone.
extern "C" __declspec(dllexport) void WINAPI OrderLogoShutdown() {
    if (g_instance) UnregisterClassW(kClassName, g_instance);
}
