#include <windows.h>
#include <winuser.h>
#include <stdio.h>
#include <stdbool.h>
#include <wingdi.h>
#include <winuser.h>



#define file_scope static
#define local_persist static
#define global_variable static

global_variable bool AppRunning;

global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable HBITMAP BitmapHandle;
global_variable HDC BitmapDeviceContext;

file_scope void Win32ResizeDIBSection(int _width, int _height){
  if(BitmapHandle){
    DeleteObject(BitmapHandle);
  }
  if(!BitmapDeviceContext){
    BitmapDeviceContext = CreateCompatibleDC(0);  
  }

  BitmapInfo.bmiHeader.biSize=sizeof(BitmapInfo.bmiHeader);
  BitmapInfo.bmiHeader.biWidth = _width;
  BitmapInfo.bmiHeader.biHeight = _height;
  BitmapInfo.bmiHeader.biPlanes = 1;
  BitmapInfo.bmiHeader.biBitCount = sizeof(float)*4*8; // 4*8 instead of 3*8 because of alignment or something
  BitmapInfo.bmiHeader.biCompression = BI_RGB;


  BitmapHandle = CreateDIBSection(
   BitmapDeviceContext,
   &BitmapInfo,
   DIB_RGB_COLORS,
   &BitmapMemory,
   NULL,
   0 
);

}

file_scope void Win32UpdateWindow(HDC hdc, int x, int y, int width, int height){
StretchDIBits(
   hdc,
   x,y,width,height,
   x,y,width,height,
   BitmapMemory,
   &BitmapInfo,
   DIB_RGB_COLORS,
   SRCCOPY
);
}
LRESULT Win32CallbackFunc(HWND _window, UINT _msgId, WPARAM param3, LPARAM param4){
  LRESULT result = 0;
  switch(_msgId) {
  case WM_CLOSE: {
    OutputDebugStringW(L"VM_CLOSE\n");
    AppRunning = false;
  } break;
  case WM_DESTROY:{
    OutputDebugStringW(L"VM_DESTROY\n");
    AppRunning = false;
  }
  case WM_SIZE:{
    OutputDebugStringW(L"VM_SIZE\n");
    RECT rect;
    GetClientRect(_window, &rect);
    
    Win32ResizeDIBSection(rect.right - rect.left, rect.bottom - rect.top);
  } break;
  case WM_ACTIVATEAPP: {
    OutputDebugStringW(L"VM_ACTIVATEAPP\n");
  } break;
  case WM_PAINT: {
    PAINTSTRUCT p;
    HDC hdc = BeginPaint(_window, &p);
    int x = p.rcPaint.left;
    int y = p.rcPaint.top;
    int width = p.rcPaint.right - p.rcPaint.left;
    int height = p.rcPaint.bottom - p.rcPaint.top;

    Win32UpdateWindow(hdc, x,y,width,height);

    
    EndPaint(_window, &p);
  } break;
  default: {
    result = DefWindowProc(_window, _msgId, param3, param4);
  };
    

  };


  return result;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow){

  WNDCLASSW windclass = {0};
  
  
  windclass.style=CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
  windclass.lpfnWndProc=Win32CallbackFunc;
  windclass.hInstance=hInstance;
  //windowclass.hIcon=;
  //  windowclass.hCursor;
  //  LPCWSTR   lpszMenuName;
  windclass.lpszClassName=L"cgame";
  

  RegisterClass(&windclass);

  HWND hwnd = CreateWindowEx(
			     0,                              // Optional window styles.
			     windclass.lpszClassName,                     // Window class
			     L"cgame",    // Window text
			     WS_OVERLAPPEDWINDOW|WS_VISIBLE,            // Window style

			     // Size and position
			     CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

			     NULL,       // Parent window    
			     NULL,       // Menu
			     hInstance,  // Instance handle
			     NULL        // Additional application data
			     );

  if(!hwnd) {
    OutputDebugStringW(L"Couldn't create window\n");
    return -1;
  }

  AppRunning = true;
  
  MSG msg={0};
  printf("Created Window, starting app\n");
  while(AppRunning){

    BOOL suc=GetMessage(&msg,NULL,0,0);
    if(suc>0){
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    
    } else {
      AppRunning = false;
    }
    


  }
  return 0;
}
 

