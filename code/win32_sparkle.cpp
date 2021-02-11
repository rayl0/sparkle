#include <windows.h>

#define internal static
#define global_variable static
#define local_persist static

global_variable int Running;
global_variable HBITMAP Bitmap;
global_variable BITMAPINFO BitmapInfo;
global_variable HDC   DeviceCompatibleDC;
global_variable void *BitmapMemory;

internal void
Win32ResizeDIBSection(int Width,
                      int Height)
{
    //TODO(rajat): Bulletproof this
    // Maybe first first, then free after if it failes

    if(Bitmap)
    {
        DeleteObject(Bitmap);
    }

    if(!DeviceCompatibleDC)
    {
        DeviceCompatibleDC = CreateCompatibleDC(0);
    }

    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = Width;
    BitmapInfo.bmiHeader.biHeight = Height;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;

    Bitmap = CreateDIBSection(
        DeviceCompatibleDC,
        &BitmapInfo,
        DIB_RGB_COLORS,
        &BitmapMemory,
        0,
        0);

    ReleaseDC(0, DeviceCompatibleDC);
}

internal void
Win32UpdateWindow(HDC DeviceContext, int X, int Y, int Width, int Height)
{
    StretchDIBits(
        DeviceContext,
        X, Y, Width, Height,
        X, Y, Width, Height,
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

        Win32UpdateWindow(DC, X, Y, Width, Height);
        PatBlt(DC, X, Y, Width, Height, Operation);

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
            while(Running)
            {
                MSG Message = {};

                BOOL Result = GetMessage(&Message, 0, 0, 0);
                if(Result > 0)
                {
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }
                else
                {
                    break;
                }
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
