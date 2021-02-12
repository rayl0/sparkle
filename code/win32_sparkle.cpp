#include <windows.h>
#include <stdint.h>

#define internal static
#define global_variable static
#define local_persist static

struct win32_offscreen_buffer
{
    // NOTE(rajat): Pixels will always be 32 bit wide.

    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
}; 

global_variable int Running;
global_variable win32_offscreen_buffer GlobalBitmapBuffer;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

struct win32_window_dimensions
{
    int Width;
    int Height;
};

internal win32_window_dimensions
Win32GetWindowDimensions(HWND Window)
{
    win32_window_dimensions Dimensions;

    RECT WindowRect;
    GetClientRect(Window, &WindowRect);

    Dimensions.Width = WindowRect.right - WindowRect.left;
    Dimensions.Height = WindowRect.bottom - WindowRect.top;

    return Dimensions;
}; 

internal void
RenderWeirdGraphics(win32_offscreen_buffer Buffer, int XOffset, int YOffset)
{
    int BytesPerPixel = 4;
    int Pitch = Buffer.Width * BytesPerPixel;

    char* Row = (char*)Buffer.Memory;
    for (int Y = 0; Y < Buffer.Height; ++Y)
    {
        u32* Pixel = (u32*)Row;
        for (int X = 0; X < Buffer.Width; ++X)
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
Win32ResizeDIBSection(
    win32_offscreen_buffer *Buffer,
    int Width,
    int Height)
{
    //TODO(rajat): Bulletproof this
    // Maybe first first, then free after if it failes

    if(Buffer->Memory)
    {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }

    Buffer->Width = Width;
    Buffer->Height = Height;

    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;

    u32 BitmapMemorySize = 4 * (Buffer->Width * Buffer->Height);
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

internal void
Win32DisplayBuffer(win32_offscreen_buffer Buffer,
                   HDC DeviceContext, int WindowWidth, int WindowHeight)
{
    StretchDIBits(
        DeviceContext,
        0, 0, WindowWidth, WindowHeight,
        0, 0, Buffer.Width, Buffer.Height,
        Buffer.Memory,
        &Buffer.Info,
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

        win32_window_dimensions Dimensions = Win32GetWindowDimensions(Window);

        RenderWeirdGraphics(GlobalBitmapBuffer, 0, 0);
        Win32DisplayBuffer(GlobalBitmapBuffer, GetDC(Window),
                           Dimensions.Width, Dimensions.Height);

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
    WindowClass.style = CS_HREDRAW|CS_VREDRAW;
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


        win32_window_dimensions Dimensions =
            Win32GetWindowDimensions(Window);

        Win32ResizeDIBSection(&GlobalBitmapBuffer,
                              Dimensions.Width, Dimensions.Height);

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

                RenderWeirdGraphics(GlobalBitmapBuffer, XOffset, YOffset);

                HDC DC = GetDC(Window);
                win32_window_dimensions Dimensions = Win32GetWindowDimensions(Window);

                Win32DisplayBuffer(GlobalBitmapBuffer, DC,
                                   Dimensions.Width,
                                   Dimensions.Height);

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
