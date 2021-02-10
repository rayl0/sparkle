#include <windows.h>

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
         OutputDebugStringA("WM_SIZE\n");
    }break;

    case WM_CLOSE:
    {
        OutputDebugStringA("WM_CLOSE\n");
    }break;

    case WM_DESTROY:
    {
        OutputDebugStringA("WM_DESTROY\n");
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
            for(;;)
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
