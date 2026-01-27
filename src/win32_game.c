#include <windows.h>
#include <winuser.h>
#include <stdio.h>
#include <stdbool.h>
#include <wingdi.h>
#include <winuser.h>
#include <stdint.h>
#include <synchapi.h>



// mingw64 headers need this macro defined
// if you want to use xaudio2 helper macros for calling functions from C instead of C++
#define COBJMACROS
#include <xaudio2.h>
#include <audiosessiontypes.h>

// copied from win32 docs

#define fourccRIFF "RIFF"
#define fourccDATA "data"
#define fourccFMT "fmt "
#define fourccWAVE "WAVE"
#define fourccXWMA "XWMA"
#define fourccDPDS "dpds"


#include "cgame.h"
#include "cgame.c"



global_variable bool AppRunning;




global_variable HDC DeviceContext;

global_variable int WindowHeight, WindowWidth;
global_variable RECT WindowRect;

global_variable const int SAMPLE_RATE_DEFAULT=48000;
global_variable const int BIT_DEPTH = 24;
global_variable const int SOUND_BUFFER_LENGTH_SECONDS = 2;
global_variable const int MAX_SOURCE_VOICES = 32;
global_variable  LARGE_INTEGER PerformanceQueryFrequency;
global_variable XAUDIO2_BUFFER GlobalSoundBuffer;


// missing from mingw64's xaudio2.h header
// copied from windows 10 sdk's xaudio2.h
#define XAUDIO2_NO_VIRTUAL_AUDIO_CLIENT       0x10000   // Used in CreateMasteringVoice to create a virtual audio client




// @NOTE: i checked in xaudio2.h and they have this COM interface thing that is supposed to be an object oriented thing to work with c++
// and you have to use it to use xaudio2
// but they do another thing where instead of giving you a class instance you get a struct instance in C
// which has a field called lpVtbl which has the pointer to the class method. For example in c++ you would do thing.poop
// but in C you would do thing.lpVtbl->poop

// finally, you actually don't need to do that directly, at least for xaudio2.h. You'll be able to find in that file that it has macros
// for supposedly making it "easier to use the XAudio2 COM interfaces in C code."
// but for some reason the macros aren't working, getting an implicit definition error
// UPDATE: probably because we're using mingw. It has its own xaudio2.h that it uses and
// the macros are behind a #ifndef COBJOBMACROS so you'll have to define t hat

typedef struct Win32Voice {
  IXAudio2SourceVoice* underlyingVoice;

} Win32Voice;

typedef struct Win32VoicePool{


  // @TODO :
  // used voices will be at the start of the buffer, all voices
  // after the Win32VoicePool.used index will be free
  // When a new voice needs to be played
  // Use the one at the end of the voices array 
  // swap it with voices[used] so that the one that was at voices[used]
  // is now at the end of the array
  // and the new voices[used] is the one we just used.
  // then increment used
  // When a voice says it's free, swap it with voices[used-1]
  // and then decreased used by 1
  
  bool used;
  Win32Voice *voices;
} Win32VoicePool;


global_variable Win32VoicePool GlobalVoicePool;

typedef struct Win32OffscreenBuffer {
  BITMAPINFO Info;
  void *Memory;
  int Height, Width;
  int BytesPerPixel;

}  Win32OffscreenBuffer;


global_variable Win32OffscreenBuffer GlobalOffscreenBuffer;



#define XAUDIO2_CREATE_FUNC(name) HRESULT name(_Outptr_ IXAudio2** ppXAudio2, UINT32 Flag, XAUDIO2_PROCESSOR XAudio2Processor)
typedef XAUDIO2_CREATE_FUNC(xaudio2_create);

//#define XAUDIO2_CREATE_SOURCE_VOICE_FUNC(name) HRESULT name(IXAudio2SourceVoice        **ppSourceVoice,  const WAVEFORMATEX         *pSourceFormat,  UINT32                     Flags,  float                      MaxFrequencyRatio,  IXAudio2VoiceCallback      *pCallback,  const XAUDIO2_VOICE_SENDS  *pSendList,  const XAUDIO2_EFFECT_CHAIN *pEffectChain);
//typedef XAUDIO2_CREATE_SOURCE_VOICE_FUNC(xaudio2_create_source_voice);

// copied from win32 docs
HRESULT win32_find_riff_chunk(HANDLE hFile, char* fourcc, DWORD *dwChunkSize, DWORD *dwChunkDataPosition)
{
    HRESULT hr = S_OK;
    if( INVALID_SET_FILE_POINTER == SetFilePointer( hFile, 0, NULL, FILE_BEGIN ) )
        return HRESULT_FROM_WIN32( GetLastError() );

    BYTE dwChunkType[4];
    DWORD dwChunkDataSize;
    DWORD dwRIFFDataSize = 0;
    DWORD dwFileType;
    DWORD bytesRead = 0;
    DWORD dwOffset = 0;

    while (hr == S_OK)
    {
        DWORD dwRead;
        if( 0 == ReadFile( hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL ) )
            hr = HRESULT_FROM_WIN32( GetLastError() );

        if( 0 == ReadFile( hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL ) )
            hr = HRESULT_FROM_WIN32( GetLastError() );


	// this was holy sheet moment
	// the #define thing doesn't null terminate i think
	// and strcmp expects null termination
	// so it wasn't working when using the #define value directly in strcmp
	char* riffString = fourccRIFF;
	bool isRIFFChunk = strcmp(dwChunkType, riffString)==0;
	if(isRIFFChunk)
        {

            dwRIFFDataSize = dwChunkDataSize;
            dwChunkDataSize = 4;
            if( 0 == ReadFile( hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL ) )
                hr = HRESULT_FROM_WIN32( GetLastError() );
	}

	  else{
            if( INVALID_SET_FILE_POINTER == SetFilePointer( hFile, dwChunkDataSize, NULL, FILE_CURRENT ) )
            return HRESULT_FROM_WIN32( GetLastError() );            
        }

        dwOffset += sizeof(DWORD) * 2;

	bool isMatch = strcmp(fourcc, dwChunkType)==0;

        if (isMatch)
        {
            *dwChunkSize = dwChunkDataSize;
            *dwChunkDataPosition = dwOffset;
            return S_OK;
        }

        dwOffset += dwChunkDataSize;

        if (bytesRead >= dwRIFFDataSize) return S_FALSE;

    }

    return S_OK;
}

HRESULT win32_read_riff_chunk_data(HANDLE hFile, void * buffer, DWORD buffersize, DWORD bufferoffset)
{
    HRESULT hr = S_OK;
    if( INVALID_SET_FILE_POINTER == SetFilePointer( hFile, bufferoffset, NULL, FILE_BEGIN ) )
        return HRESULT_FROM_WIN32( GetLastError() );
    DWORD dwRead;
    if( 0 == ReadFile( hFile, buffer, buffersize, &dwRead, NULL ) )
        hr = HRESULT_FROM_WIN32( GetLastError() );
    return hr;
}




int win32_get_window_height(HWND windowhandle){
  RECT r;

  GetClientRect(windowhandle, &r);

  return r.bottom - r.top;
}
int win32_get_window_width(HWND windowhandle){
  RECT r;

  GetClientRect(windowhandle, &r);

  return r.right - r.left;
}



internal void win32_resize_dib_section(Win32OffscreenBuffer* buffer, int _width, int _height){

  if(buffer->Memory){
    VirtualFree(buffer->Memory, 0,MEM_RELEASE);
  }

  buffer->Width = _width;
  buffer->Height = _height;
  buffer->BytesPerPixel = 4;

  buffer->Info.bmiHeader.biSize=sizeof(buffer->Info.bmiHeader);
  buffer->Info.bmiHeader.biWidth = _width;
  buffer->Info.bmiHeader.biHeight = -_height;
  buffer->Info.bmiHeader.biPlanes = 1;
  buffer->Info.bmiHeader.biBitCount = buffer->BytesPerPixel*8; 
  buffer->Info.bmiHeader.biCompression = BI_RGB;

buffer->Memory=  VirtualAlloc(
  0,
  _width*_height*buffer->BytesPerPixel,
  MEM_COMMIT | MEM_RESERVE,
  PAGE_READWRITE 
);



}

internal void win32_copy_buffer_to_window(Win32OffscreenBuffer buffer, HDC hdc, int winWidth, int winHeight){


StretchDIBits(
   hdc,
   0,0,winWidth, winHeight,
   0,0,buffer.Width, buffer.Height,
   buffer.Memory,
   &buffer.Info,
   DIB_RGB_COLORS,
   SRCCOPY
);
}



void win32_play_audio_buffer(XAUDIO2_BUFFER _buffer){
  if(GlobalVoicePool.used == MAX_SOURCE_VOICES){

    OutputDebugString(L"No audio voices available, not going to play this audio\n");
  }

  Win32Voice poppedVoice = GlobalVoicePool.voices[MAX_SOURCE_VOICES-1];
  Win32Voice newEndVoice = GlobalVoicePool.voices[GlobalVoicePool.used];

  GlobalVoicePool.voices[MAX_SOURCE_VOICES-1] = newEndVoice;
  GlobalVoicePool.voices[GlobalVoicePool.used] = poppedVoice;
  GlobalVoicePool.used++;


  IXAudio2SourceVoice *v = poppedVoice.underlyingVoice;
  v->lpVtbl->SubmitSourceBuffer(v,&_buffer, NULL);
  v->lpVtbl->Start(v,0,XAUDIO2_COMMIT_NOW);
}

void win32_play_wave_file(const char* path){

  

  WAVEFORMATEXTENSIBLE wfx = {0};
  XAUDIO2_BUFFER buffer = {0};


  HANDLE hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,0,NULL);

  if(hFile == INVALID_HANDLE_VALUE){
    printf("OPENING FILE FAILED\n");
  }
  // read riff chunk
  DWORD chunkSize=0, chunkPos=0;
  win32_find_riff_chunk(hFile, fourccRIFF, &chunkSize, &chunkPos);

  DWORD fileType;
  win32_read_riff_chunk_data(hFile, &fileType, sizeof(DWORD), chunkPos);


  // read FMT chunk
  win32_find_riff_chunk(hFile, fourccFMT, &chunkSize, &chunkPos);
  win32_read_riff_chunk_data(hFile, &wfx, chunkSize, chunkPos);

  // read data chunk
  win32_find_riff_chunk(hFile, fourccDATA, &chunkSize, &chunkPos);
  BYTE* bufferData = (malloc(chunkSize));
  win32_read_riff_chunk_data(hFile, bufferData, chunkSize, chunkPos);

  CloseHandle(hFile);

  
  buffer.AudioBytes = chunkSize;
  buffer.pAudioData = bufferData;
  buffer.Flags = XAUDIO2_END_OF_STREAM;


  win32_play_audio_buffer(buffer);

}



//@NOTE Sample rate and all other format things need to be constant for source files, decide on the format and stick to it for all files
// otherwise the audio just cuts out, there seems to be no auto conversion going on
// if you want to do some conversion, perhaps do it when baking/importing assets

void win32_init_xaudio2(uint32_t sampleRate, uint32_t bitDepth, uint32_t numVoices, Win32VoicePool* _data){



  HMODULE xaudio2 = LoadLibrary(L"XAudio2_9.dll");
  if(xaudio2 == NULL){
    printf("XAudio2_9.dll not found\n");
    return;
  }
  
  HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
  
  if (FAILED(hr)) {
    printf("Couldn't initial COM\n");
    return;
  };
  printf("Initialized COM \n");

  IXAudio2 *xAudio;
  xaudio2_create *XAudio2Create= (xaudio2_create*) GetProcAddress(xaudio2, "XAudio2Create");


 

  int numChannels =2;
  int bytesInASample =  numChannels*bitDepth /8;
  WAVEFORMATEX format= {};
  format.wFormatTag=WAVE_FORMAT_PCM;
  format.nChannels=numChannels;
  format.nSamplesPerSec=sampleRate;
  format.nAvgBytesPerSec = bytesInASample*sampleRate;
  format.nBlockAlign=bytesInASample;
  format.wBitsPerSample = bitDepth;
  format.cbSize=0;

  


  HRESULT res=  XAudio2Create(&xAudio, 0, XAUDIO2_DEFAULT_PROCESSOR);
  if(res!=S_OK){
    printf("Couldn't create XAudio2\n");
  }

  IXAudio2MasteringVoice* masteringVoice;


  res=  xAudio->lpVtbl->CreateMasteringVoice(
					     xAudio,
					     &masteringVoice,
					     numChannels,
					     sampleRate,
					     XAUDIO2_NO_VIRTUAL_AUDIO_CLIENT,
					     NULL,
					     NULL,
					     // @NOTE: for now, just use other. I don't know the implications of using GameEffects
					     AudioCategory_Other);

  if(res!=S_OK){
    printf("Couldn't create XAudio Mastering Voice\n");
  }

  IXAudio2SubmixVoice * pSFXSubmixVoice;
  IXAudio2_CreateSubmixVoice(xAudio, &pSFXSubmixVoice,numChannels,sampleRate,0,0,0,0);

  XAUDIO2_SEND_DESCRIPTOR SFXSend = {0,(IXAudio2Voice*) pSFXSubmixVoice};
  XAUDIO2_VOICE_SENDS SFXSendList = {1, &SFXSend};


  if(_data->voices){
    VirtualFree(_data->voices,0, MEM_RELEASE);
  }
  _data->voices = VirtualAlloc(
			       0,
			       sizeof(Win32Voice)*numVoices,
			       MEM_COMMIT | MEM_RESERVE,
			       PAGE_READWRITE 
			       );
  _data->used = 0;
 
  for(int i=0;i<numVoices;i++){
    IXAudio2SourceVoice* srcVoice;
    if(FAILED(IXAudio2_CreateSourceVoice(xAudio, &srcVoice,&format,0,XAUDIO2_DEFAULT_FREQ_RATIO,NULL,&SFXSendList,NULL))){
      printf("Couldn't create source voice\n");
    }
    Win32Voice voice;
    voice.underlyingVoice = srcVoice;
    _data->voices[i] = voice;


  }
}



void temp_print_time_stamp(LARGE_INTEGER _compareToStamp){

  LARGE_INTEGER perfTimeStamp;
    QueryPerformanceCounter(&perfTimeStamp);
    LARGE_INTEGER currentStamp = perfTimeStamp;

    int64_t dif = currentStamp.QuadPart - _compareToStamp.QuadPart;

    int32_t ElapsedSeconds = dif / PerformanceQueryFrequency.QuadPart;

    int32_t ElapsedMS = (dif*1000) / PerformanceQueryFrequency.QuadPart;

    int32_t ElapsedMicroSeconds =(dif *1000 * 1000) / PerformanceQueryFrequency.QuadPart;

    float FPS = 1.0/((float)ElapsedMS/1000);




    //    FPS = 1/FPS;
    //    printf("Elapsed:%dms, FPS:%f\n", ElapsedMS, FPS);
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

    RECT rect;
    GetClientRect(_window, &rect);
    WindowWidth = rect.right - rect.left;
    WindowHeight = rect.bottom - rect.top;
    WindowRect = rect;
    
    //    win32_copy_buffer_to_window(hdc,&rect, x,y,width,height);

    
    EndPaint(_window, &p);
  } break;

  case WM_KEYDOWN:
  case WM_KEYUP:
  case WM_SYSKEYDOWN:
  case WM_SYSKEYUP:
    {
      bool wasDown = ((param4 & (1<<30)) !=0);
      bool isDown = ((param4 & (1<<31)) ==0);
      uint32_t code = param3;


      switch(code) {
      case 'W' : {
	printf("W was down %d, is down %d\n", wasDown, isDown);
	win32_play_wave_file("g:/wave-file.wav");
      }break;
      case VK_F4 : {
	bool alting = (param4 & (1<<29)) !=0;
	if(alting){
	  AppRunning = false;
	}
      } break;
      default: {

      }break;
      }

      
    } break;
  default: {
    result = DefWindowProc(_window, _msgId, param3, param4);
  };
    

  };


  return result;
}
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow){



  QueryPerformanceFrequency(&PerformanceQueryFrequency);
  

  WNDCLASSW windclass = {0};
  win32_resize_dib_section(&GlobalOffscreenBuffer, 1280, 720);
  
  windclass.style=CS_HREDRAW|CS_VREDRAW;
  windclass.lpfnWndProc=Win32CallbackFunc;
  windclass.hInstance=hInstance;
  //windowclass.hIcon=;
  windclass.hCursor=LoadCursor(NULL,IDC_ARROW);
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


  win32_init_xaudio2(SAMPLE_RATE_DEFAULT, BIT_DEPTH,MAX_SOURCE_VOICES, &GlobalVoicePool);
  AppRunning = true;





  MSG msg={0};
  printf("Created Window, starting app\n");
  while(AppRunning){
    LARGE_INTEGER perfTimeStamp;
    QueryPerformanceCounter(&perfTimeStamp);

    while(PeekMessage(&msg,NULL,0,0,PM_REMOVE)){
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    
    }

    DeviceContext = GetDC(hwnd);
  

    temp_print_time_stamp(perfTimeStamp);

    CGameOffscreenBuffer buffer;
    buffer.Memory = GlobalOffscreenBuffer.Memory;
    buffer.Height = GlobalOffscreenBuffer.Height;
    buffer.Width = GlobalOffscreenBuffer.Width;
    buffer.BytesPerPixel = GlobalOffscreenBuffer.BytesPerPixel;
    cgame_update(&buffer);
    win32_copy_buffer_to_window(GlobalOffscreenBuffer, DeviceContext,win32_get_window_width(hwnd), win32_get_window_height(hwnd));



    //      printf("Played %d bytes out of %d and %d samples out of %d\n", soundBufferBytesPlayedThisLoop,soundBufferSize, soundBufferSamplesPlayedThisLoop, samplesInBuffer);
    ReleaseDC(hwnd, DeviceContext);





  }
  return 0;
}
 

