#include <windows.h>
#include <math.h>
#include <time.h>

const double PI = 3.14159265358979323846;
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
    const wchar_t CLASS_NAME[] = L"Clock Window Class";
    WNDCLASS wc = { };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TOPMOST,
        CLASS_NAME,
        NULL,
        WS_POPUP,
        100, 100, 400, 400,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) { return 0; }

    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 150, LWA_COLORKEY | LWA_ALPHA);
    ShowWindow(hwnd, nCmdShow);
    SetTimer(hwnd, 1, 1000, NULL);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static bool isDragging = false;
    static POINT cursorPosStartDrag;
    static RECT windowRectStartDrag;

    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_LBUTTONDOWN:
        isDragging = true;
        SetCapture(hwnd);
        GetCursorPos(&cursorPosStartDrag);
        GetWindowRect(hwnd, &windowRectStartDrag);
        return 0;
    case WM_LBUTTONUP:
        isDragging = false;
        ReleaseCapture();
        return 0;
    case WM_MOUSEMOVE:
        if (isDragging) // Ensure dragging flag is set
        {
            POINT cursorPos;
            GetCursorPos(&cursorPos);
            int deltaX = cursorPos.x - cursorPosStartDrag.x;
            int deltaY = cursorPos.y - cursorPosStartDrag.y;

            SetWindowPos(hwnd, 0,
                windowRectStartDrag.left + deltaX,
                windowRectStartDrag.top + deltaY,
                0, 0, SWP_NOSIZE | SWP_NOZORDER);
        }
        return 0;
    case WM_SETCURSOR:
        if (LOWORD(lParam) == HTCLIENT)
        {
            SetCursor(LoadCursor(nullptr, IDC_ARROW));
            return TRUE;
        }
        break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        SetTextAlign(hdc, TA_CENTER | TA_BASELINE);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(255, 255, 255));

        SYSTEMTIME time;
        GetLocalTime(&time);
        RECT rect;
        GetClientRect(hwnd, &rect);
        int xCenter = (rect.right - rect.left) / 2;
        int yCenter = (rect.bottom - rect.top) / 2;
        int radius = min(xCenter, yCenter);

        // Gradient Background
        TRIVERTEX vertex[2];
        vertex[0].x = 0;
        vertex[0].y = 0;
        vertex[0].Red = 0x4000;
        vertex[0].Green = 0x4000;
        vertex[0].Blue = 0x4000;
        vertex[0].Alpha = 0x0000;

        vertex[1].x = rect.right;
        vertex[1].y = rect.bottom;
        vertex[1].Red = 0x8000;
        vertex[1].Green = 0x8000;
        vertex[1].Blue = 0x8000;
        vertex[1].Alpha = 0x0000;

        GRADIENT_RECT gRect;
        gRect.UpperLeft = 0;
        gRect.LowerRight = 1;
        GradientFill(hdc, vertex, 2, &gRect, 1, GRADIENT_FILL_RECT_H);

        // Draw numbers
        for (int i = 1; i <= 12; i++)
        {
            int x = xCenter + (int)(0.87 * radius * sin(i * PI / 6));
            int y = yCenter - (int)(0.87 * radius * cos(i * PI / 6));
            wchar_t numStr[3];
            wsprintf(numStr, L"%d", i);
            TextOut(hdc, x, y, numStr, 2);
        }

        // Draw clock hands
        double hourAngle = (time.wHour % 12 + time.wMinute / 60.0) * PI / 6;
        double minuteAngle = time.wMinute * PI / 30;
        double secondAngle = time.wSecond * PI / 30;
        int hourX = xCenter + (int)(0.5 * radius * sin(hourAngle));
        int hourY = yCenter - (int)(0.5 * radius * cos(hourAngle));
        int minuteX = xCenter + (int)(0.75 * radius * sin(minuteAngle));
        int minuteY = yCenter - (int)(0.75 * radius * cos(minuteAngle));
        int secondX = xCenter + (int)(0.9 * radius * sin(secondAngle));
        int secondY = yCenter - (int)(0.9 * radius * cos(secondAngle));

        HPEN hPenHour = CreatePen(PS_SOLID, 8, RGB(255, 0, 0));
        HPEN hPenMinute = CreatePen(PS_SOLID, 6, RGB(0, 255, 0));
        HPEN hPenSecond = CreatePen(PS_SOLID, 4, RGB(0, 0, 255));
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPenHour);

        MoveToEx(hdc, xCenter, yCenter, NULL);
        LineTo(hdc, hourX, hourY);
        SelectObject(hdc, hPenMinute);
        MoveToEx(hdc, xCenter, yCenter, NULL);
        LineTo(hdc, minuteX, minuteY);
        SelectObject(hdc, hPenSecond);
        MoveToEx(hdc, xCenter, yCenter, NULL);
        LineTo(hdc, secondX, secondY);

        SelectObject(hdc, hOldPen);
        DeleteObject(hPenHour);
        DeleteObject(hPenMinute);
        DeleteObject(hPenSecond);
        EndPaint(hwnd, &ps);
    } return 0;
    case WM_TIMER:
    {
        InvalidateRect(hwnd, NULL, TRUE);
    } return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
