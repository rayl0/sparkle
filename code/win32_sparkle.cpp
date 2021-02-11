#include <windows.h>
#include <stdint.h>

#define internal static
#define global_variable static
#define local_persist static

global_variable int Running;
global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable int BitmapWidth;
global_variable int BitmapHeight;
global_variable int BytesPerPixel = 4;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

internal void 
RenderWeirdGraphics(int XOffset, int YOffset)
{
    int Pitch = BitmapWidth * BytesPerPixel;

    char* Row = (char*)BitmapMemory;
    for (int Y = 0; Y < BitmapHeight; ++Y)
    {
        u32* Pixel = (u32*)Row;
        for (int X = 0; X < BitmapWidth; ++X)
        {
            u8 B = X + XOffset;
            u8 G = Y + YOffset;

            *Pixel = ((G << 8) | B);

            ++Pixel;
        }

        Row += Pitch;
    }
}

internal void
Win32ResizeDIBSection(int Width,
                      int Height)
{
    //TODO(rajat): Bulletproof this
    // Maybe first first, then free after if it failes

    if (BitmapMemory)
    {
        VirtualFree(BitmapMemory, 0, MEM_RELEASE);
    }

    BitmapWidth = Width;
    BitmapHeight = Height;

    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = BitmapWidth;
    BitmapInfo.bmiHeader.biHeight = -BitmapHeight;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;

    u32 BitmapMemorySize = 4 * (BitmapWidth * BitmapHeight);
    BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

internal void
Win32UpdateWindow(HDC DeviceContext, RECT *WindowRect)
{
    s32 WindowWidth = WindowRect->right - WindowRect->left;
    s32 WindowHeight = WindowRect->bottom - WindowRect->top;

    StretchDIBits(
        DeviceContext,
        0, 0, WindowWidth, WindowHeight,
        0, 0, BitmapWidth, BitmapHeight,
        BitmapMemory,
        &BitmapInfo,
        DIB_RGB_COLORS,
        SRCCOPY);

}

LRESULT CALLBACK
SparkleWindowCallback (
    HWND Window,
    UINT Message,
    WPARAM WParam,
    LPARAM LParam
)
{
    LRESULT Result = 0;

    switch(Message)
    {

    case WM_SIZE:
    {
        RECT WindowRect;
        GetClientRect(Window, &WindowRect);

        int Width = WindowRect.right - WindowRect.left;
        int Height = WindowRect.bottom - WindowRect.top;

        Win32ResizeDIBSection(Width, Height);
  
        OutputDebugStringA("WM_SIZE\n");
    }break;

    case WM_CLOSE:
    {
        Running = false;
    }break;

    case WM_DESTROY:
    {
        // TODO(rajat): Handle this as a error!
        Running = false;
    }break;

    case WM_ACTIVATEAPP:
    {
        OutputDebugStringA("WM_ACTIVEAPP\n");
    }break;

    case WM_PAINT:
    {
        PAINTSTRUCT Struct;
        HDC DC = BeginPaint(Window, &Struct);

        int X = Struct.rcPaint.left;
        int Y = Struct.rcPaint.top;
        int Width = Struct.rcPaint.right - Struct.rcPaint.left;
        int Height = Struct.rcPaint.bottom - Struct.rcPaint.top;

        static DWORD Operation = WHITENESS;
        PatBlt(DC, X, Y, Width, Height, Operation);

        RenderWeirdGraphics(0, 0);
        Win32UpdateWindow(GetDC(Window), &Struct.rcPaint);

        if (Operation == WHITENESS)
        {
            Operation = BLACKNESS;
        }
        else
        {
            Operation = WHITENESS;
        }

        EndPaint(Window, &Struct);
    }break;

    default:
    {
        Result = DefWindowProc(Window, Message, WParam, LParam);
    }break;

    }

    return(Result);
}

int WinMain(
  HINSTANCE Instance,
  HINSTANCE PrevInstance,
  LPSTR     CommandLine,
  int       ShowCommand
)
{
    WNDCLASS WindowClass = {};
    WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc = SparkleWindowCallback;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = "SparkleWindowClass";

    if(RegisterClass(&WindowClass))
    {
        HWND Window = CreateWindowEx(
            0,
            WindowClass.lpszClassName,
            "Sparkle",
            WS_OVERLAPPEDWINDOW|WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            0,
            0,
            Instance,
            0);

        if(Window)
        {
            Running = true;

            int XOffset = 0;
            int YOffset = 0;
            while(Running)
            {
                MSG Message = {};

                while (PeekMessage(&Message, Window, 0, 0, PM_REMOVE))
                {
                    if (Message.message == WM_QUIT)
                    {
                        Running = false;
                    }

                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }

                RenderWeirdGraphics(XOffset, YOffset);

                HDC DC = GetDC(Window);
                RECT WindowRect;
                GetClientRect(Window, &WindowRect);

                Win32UpdateWindow(DC, &WindowRect);
                ReleaseDC(Window, DC);

                ++XOffset;
                ++YOffset;
            }
        }
        else
        {
            //TODO(rajat): Logging
        }
    }
    else
    {
        //TODO(rajat): Logging
    }

   return(0);
}
