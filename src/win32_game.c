#include <windows.h>
#include <winuser.h>
#include <stdio.h>
#include <stdbool.h>
#include <wingdi.h>
#include <winuser.h>
#include <stdint.h>
#include <math.h>


#include <synchapi.h>
#include <initguid.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <audiopolicy.h>

//@TODO Consider switching to WASAPI
// instead of XAudio. This way we can let the game control the audio buffer
// and just use wasapi to play the buffer
// from a quick read, it seems like this will let us do something similar on linux using pulse audio

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



// REFERENCE_TIME is 100 nano-second units
// so 1 REFERENCE_TIME is 100 nano-seconds
// 1 milliseconds has 100,000 nano seconds
// so 1 milliseconds has 10,000 REFERENCE_TIME units

#define MsToReftimeUnits(Value) (Value * 10000)
#define SecToReftimeUnits(Value) (Value * 10000000)
// const int TEMP_HNS_BUF_DURATION = SecToReftimeUnits(0.020);

global_variable bool AppRunning;

global_variable HDC DeviceContext;

global_variable int WindowHeight, WindowWidth;
global_variable RECT WindowRect;

global_variable LARGE_INTEGER PerformanceQueryFrequency;
//global_variable XAUDIO2_BUFFER GlobalSoundBuffer;
global_variable IAudioClient *WasapiAudioClient;
global_variable IAudioRenderClient *WasapiRenderClient;
global_variable WAVEFORMATEX AudioFormat;

global_variable uint32_t WasapiNumTotalBufferFrames;
global_variable uint32_t WasapiNumTotalBufferBytes;
global_variable CG_Input GlobalInput;
global_variable CG_PlatformConfig Win32PlatformConfig;


global_variable float GlobalDeltaTime;

// missing from mingw64's xaudio2.h header
// copied from windows 10 sdk's xaudio2.h
// #define XAUDIO2_NO_VIRTUAL_AUDIO_CLIENT       0x10000   // Used in CreateMasteringVoice to create a virtual audio client

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

/* typedef struct Win32Voice { */
/*   IXAudio2SourceVoice* underlyingVoice; */

/* } Win32Voice; */

/* typedef struct Win32VoicePool{ */

/*   // @TODO : */
/*   // used voices will be at the start of the buffer, all voices */
/*   // after the Win32VoicePool.used index will be free */
/*   // When a new voice needs to be played */
/*   // Use the one at the end of the voices array  */
/*   // swap it with voices[used] so that the one that was at voices[used] */
/*   // is now at the end of the array */
/*   // and the new voices[used] is the one we just used. */
/*   // then increment used */
/*   // When a voice says it's free, swap it with voices[used-1] */
/*   // and then decreased used by 1 */

/*   uint32_t used; */
/*   Win32Voice *voices; */
/* } Win32VoicePool; */

/* global_variable Win32VoicePool GlobalVoicePool; */

typedef struct Win32OffscreenBuffer {
  BITMAPINFO Info;
  void *Memory;
  int Height, Width;
  int BytesPerPixel;

} Win32OffscreenBuffer;

global_variable Win32OffscreenBuffer GlobalOffscreenBuffer;

//#define XAUDIO2_CREATE_FUNC(name) HRESULT name(_Outptr_ IXAudio2 **ppXAudio2, UINT32 Flag, XAUDIO2_PROCESSOR XAudio2Processor)
//typedef XAUDIO2_CREATE_FUNC(xaudio2_create);

// #define XAUDIO2_CREATE_SOURCE_VOICE_FUNC(name) HRESULT name(IXAudio2SourceVoice        **ppSourceVoice,  const WAVEFORMATEX         *pSourceFormat,  UINT32                     Flags,  float                      MaxFrequencyRatio,  IXAudio2VoiceCallback      *pCallback,  const XAUDIO2_VOICE_SENDS  *pSendList,  const XAUDIO2_EFFECT_CHAIN *pEffectChain);
// typedef XAUDIO2_CREATE_SOURCE_VOICE_FUNC(xaudio2_create_source_voice);

// copied from win32 docs
HRESULT win32_find_riff_chunk(HANDLE hFile, char *fourcc, DWORD *dwChunkSize, DWORD *dwChunkDataPosition) {
  HRESULT hr = S_OK;
  if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
    return HRESULT_FROM_WIN32(GetLastError());

  BYTE dwChunkType[4];
  DWORD dwChunkDataSize;
  DWORD dwRIFFDataSize = 0;
  DWORD dwFileType;
  DWORD bytesRead = 0;
  DWORD dwOffset = 0;

  while (hr == S_OK) {
    DWORD dwRead;
    if (0 == ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL))
      hr = HRESULT_FROM_WIN32(GetLastError());

    if (0 == ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL))
      hr = HRESULT_FROM_WIN32(GetLastError());

    // this was holy sheet moment
    // the #define thing doesn't null terminate i think
    // and strcmp expects null termination
    // so it wasn't working when using the #define value directly in strcmp
    char *riffString = fourccRIFF;
    bool isRIFFChunk = strcmp(dwChunkType, riffString) == 0;
    if (isRIFFChunk) {

      dwRIFFDataSize = dwChunkDataSize;
      dwChunkDataSize = 4;
      if (0 == ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL))
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    else {
      if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, dwChunkDataSize, NULL, FILE_CURRENT))
        return HRESULT_FROM_WIN32(GetLastError());
    }

    dwOffset += sizeof(DWORD) * 2;

    bool isMatch = strcmp(fourcc, dwChunkType) == 0;

    if (isMatch) {
      *dwChunkSize = dwChunkDataSize;
      *dwChunkDataPosition = dwOffset;
      return S_OK;
    }

    dwOffset += dwChunkDataSize;

    if (bytesRead >= dwRIFFDataSize)
      return S_FALSE;
  }

  return S_OK;
}

HRESULT win32_read_riff_chunk_data(HANDLE hFile, void *buffer, DWORD buffersize, DWORD bufferoffset) {
  HRESULT hr = S_OK;
  if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, bufferoffset, NULL, FILE_BEGIN))
    return HRESULT_FROM_WIN32(GetLastError());
  DWORD dwRead;
  if (0 == ReadFile(hFile, buffer, buffersize, &dwRead, NULL))
    hr = HRESULT_FROM_WIN32(GetLastError());
  return hr;
}

int win32_get_window_height(HWND windowhandle) {
  RECT r;

  GetClientRect(windowhandle, &r);

  return r.bottom - r.top;
}
int win32_get_window_width(HWND windowhandle) {
  RECT r;

  GetClientRect(windowhandle, &r);

  return r.right - r.left;
}

internal void win32_resize_dib_section(Win32OffscreenBuffer *buffer, int _width, int _height) {

  if (buffer->Memory) {
    VirtualFree(buffer->Memory, 0, MEM_RELEASE);
  }

  buffer->Width = _width;
  buffer->Height = _height;
  buffer->BytesPerPixel = 4;

  buffer->Info.bmiHeader.biSize = sizeof(buffer->Info.bmiHeader);
  buffer->Info.bmiHeader.biWidth = _width;
  buffer->Info.bmiHeader.biHeight = _height;
  buffer->Info.bmiHeader.biPlanes = 1;
  buffer->Info.bmiHeader.biBitCount = buffer->BytesPerPixel * 8;
  buffer->Info.bmiHeader.biCompression = BI_RGB;

  buffer->Memory = VirtualAlloc(0, _width * _height * buffer->BytesPerPixel, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}


internal void win32_copy_buffer_to_window(Win32OffscreenBuffer buffer, HDC hdc, int winWidth, int winHeight)
{
  StretchDIBits(hdc, 0, 0,
		buffer.Width,
		buffer.Height,
		0, 0, buffer.Width, buffer.Height,
		buffer.Memory, &buffer.Info, DIB_RGB_COLORS, SRCCOPY);

}


/* void win32_play_audio_buffer(XAUDIO2_BUFFER _buffer){ */

/*   printf("Trying to play another sound, used before playing: %d\n", GlobalVoicePool.used); */
/*   if(GlobalVoicePool.used == MAX_SOURCE_VOICES){ */

/*     printf("No audio voices available, not going to play this audio\n"); */
/*     return; */
/*   } */

/*   Win32Voice poppedVoice = GlobalVoicePool.voices[MAX_SOURCE_VOICES-1]; */
/*   Win32Voice newEndVoice = GlobalVoicePool.voices[GlobalVoicePool.used]; */

/*   GlobalVoicePool.voices[MAX_SOURCE_VOICES-1] = newEndVoice; */
/*   GlobalVoicePool.voices[GlobalVoicePool.used] = poppedVoice; */
/*   GlobalVoicePool.used++; */

/*   printf("Just played another sound, total used voices: %d\n", GlobalVoicePool.used); */
/*   IXAudio2SourceVoice *v = poppedVoice.underlyingVoice; */
/*   v->lpVtbl->SubmitSourceBuffer(v,&_buffer, NULL); */
/*   v->lpVtbl->Start(v,0,XAUDIO2_COMMIT_NOW); */
/* } */

void win32_play_wave_file(char *path) {

  /* WAVEFORMATEXTENSIBLE wfx = {0}; */
  /* XAUDIO2_BUFFER buffer = {0}; */

  /* HANDLE hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL); */

  /* if (hFile == INVALID_HANDLE_VALUE) { */
  /*   printf("OPENING FILE FAILED\n"); */
  /*   return; */
  /* } */
  /* // read riff chunk */
  /* DWORD chunkSize = 0, chunkPos = 0; */
  /* win32_find_riff_chunk(hFile, fourccRIFF, &chunkSize, &chunkPos); */

  /* DWORD fileType; */
  /* win32_read_riff_chunk_data(hFile, &fileType, sizeof(DWORD), chunkPos); */

  /* // read FMT chunk */
  /* win32_find_riff_chunk(hFile, fourccFMT, &chunkSize, &chunkPos); */
  /* win32_read_riff_chunk_data(hFile, &wfx, chunkSize, chunkPos); */

  /* // read data chunk */
  /* win32_find_riff_chunk(hFile, fourccDATA, &chunkSize, &chunkPos); */
  /* BYTE *bufferData = (malloc(chunkSize)); */
  /* win32_read_riff_chunk_data(hFile, bufferData, chunkSize, chunkPos); */

  /* CloseHandle(hFile); */

  /* buffer.AudioBytes = chunkSize; */
  /* buffer.pAudioData = bufferData; */
  /* buffer.Flags = XAUDIO2_END_OF_STREAM; */

  /* //  win32_play_audio_buffer(buffer); */
}

//@NOTE Sample rate and all other format things need to be constant for source files, decide on the format and stick to it for all files
// otherwise the audio just cuts out, there seems to be no auto conversion going on
// if you want to do some conversion, perhaps do it when baking/importing assets

void win32_init_wasapi(uint32_t sampleRate, uint32_t bitDepth, float _bufferDurationSec, uint8_t numChannels) {
  IMMDevice *device;

  IMMDeviceEnumerator *enumerator;
  HRESULT hr = CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, &IID_IMMDeviceEnumerator, (void **)&enumerator);

    if (hr != S_OK) {
    printf("Sumtin wong CoCreateInstance enumerator\n");
    Assert(hr==S_OK);  
  }

  hr = enumerator->lpVtbl->GetDefaultAudioEndpoint(enumerator, eRender, eMultimedia, &device);

  void *ppInterface;
  device->lpVtbl->Activate(device, &IID_IAudioClient, CLSCTX_ALL, NULL, &ppInterface);

  int bytesInASample = numChannels * bitDepth / 8;
  WAVEFORMATEX format;

  format.wFormatTag = WAVE_FORMAT_PCM;
  format.nChannels = numChannels;
  format.nSamplesPerSec = sampleRate;
  format.nAvgBytesPerSec = bytesInASample * sampleRate;
  format.nBlockAlign = bytesInASample;
  format.wBitsPerSample = bitDepth;
  format.cbSize = 0;

  AudioFormat = format;

  IAudioClient *audioClient = (IAudioClient *)ppInterface;

  hr = audioClient->lpVtbl->Initialize(audioClient,

                                       AUDCLNT_SHAREMODE_SHARED, 0, SecToReftimeUnits(_bufferDurationSec), 0, &AudioFormat, NULL);

  if (hr != S_OK) {
    printf("Sumtin wong audioClient.Initialize\n");
    Assert(hr==S_OK);  
  }

  IAudioRenderClient *renderClient = NULL;
  audioClient->lpVtbl->GetService(audioClient, &IID_IAudioRenderClient, (void **)&renderClient);

  audioClient->lpVtbl->Start(audioClient);

  WasapiAudioClient = audioClient;
  WasapiRenderClient = renderClient;

  hr = WasapiAudioClient->lpVtbl->GetBufferSize(WasapiAudioClient, &WasapiNumTotalBufferFrames);

  WasapiNumTotalBufferBytes = AudioFormat.nBlockAlign * WasapiNumTotalBufferFrames;

  uint8_t *data;
  hr = WasapiRenderClient->lpVtbl->GetBuffer(WasapiRenderClient, WasapiNumTotalBufferFrames, &data);

  memset(data, 0, WasapiNumTotalBufferBytes);

  hr = WasapiRenderClient->lpVtbl->ReleaseBuffer(WasapiRenderClient, WasapiNumTotalBufferFrames, 0);
}
void win32_wasapi_run_test(){
  uint32_t numFramesPadding=0;
 

 
  WasapiAudioClient->lpVtbl->GetCurrentPadding(WasapiAudioClient, &numFramesPadding);

  uint32_t numFramesAvailable = WasapiNumTotalBufferFrames - numFramesPadding;

  //  printf("Num buffer frames: %d\n, padding: %d\n", WasapiNumTotalBufferFrames, numFramesPadding);

  //  printf("num frames available: %d\n", numFramesAvailable);
  if(numFramesAvailable == 0) return;
  uint8_t *data;
HRESULT hr=  WasapiRenderClient->lpVtbl->GetBuffer(WasapiRenderClient, numFramesAvailable,&data);

 float wave_frequency =100;
 float amplitude = 0.1;


 float numPhasesPerSec = wave_frequency;
 float phasePerSample = numPhasesPerSec / AudioFormat.nSamplesPerSec;
 local_persist float phase = 0.0;


 // printf("Phase per sample:%f\n", phasePerSample);
 // printf("Sample rate:%d\n", AudioFormat.nSamplesPerSec);

 int bytesInOneChannel = AudioFormat.nBlockAlign/AudioFormat.nChannels;
 if(Win32PlatformConfig.AudioBitDepth == 24){
    // 24 bit max value

   for(int i=0;i<numFramesAvailable;i++){
     uint8_t* p = data + i*AudioFormat.nBlockAlign;
     float val = (phase>0.5) ? 1.0 : -1.0;
     int32_t intAmplitude = (int32_t)(amplitude*8388607*val);

     float sinVal = phase;
     sinVal = sinf(sinVal*(44.0/7.0)); // 2 pi
     int32_t intAmplitudeSin = (int32_t)(amplitude*8388607*sinVal);
     for(int c=0;c<AudioFormat.nChannels;c++){
       p[0] =(intAmplitudeSin) & 0xFF;
       p[1] = (intAmplitudeSin >> 8) & 0xFF;
       p[2] = (intAmplitudeSin >> 16) & 0xFF;

       p+=3;
     }

     phase+=phasePerSample;

     if(phase>=1.0){
       phase -=1.0;
     }


   }
 }

 //   printf("Phase: %f\n", phase);
hr =  WasapiRenderClient->lpVtbl->ReleaseBuffer(WasapiRenderClient, numFramesAvailable,0);
 if(hr!=S_OK){
   exit(1);
 }
 
}
// end test
void win32_wasapi_copy_buffer(uint8_t *_sourceBuffer, uint32_t _writePosBytes, uint32_t _writeLengthBytes, uint32_t numFramesToWrite) {

  if (_writeLengthBytes == 0)
    return;

  uint32_t numWriteFrames = _writeLengthBytes / AudioFormat.nBlockAlign;

  uint8_t *data;
  HRESULT hr = WasapiRenderClient->lpVtbl->GetBuffer(WasapiRenderClient, numWriteFrames, &data);



  int dif  =(WasapiNumTotalBufferBytes) - (_writePosBytes +_writeLengthBytes);
  if( dif< 0){
    

    uint32_t writeLength1 = (WasapiNumTotalBufferBytes - _writePosBytes);
    uint32_t writeLength2 = (_writeLengthBytes - writeLength1);
    
    memcpy(data, _sourceBuffer + _writePosBytes, writeLength1);
    memcpy(data+writeLength1, _sourceBuffer, writeLength2);
  }

  else{
    memcpy(data, _sourceBuffer + _writePosBytes, _writeLengthBytes);
  }
  

  hr = WasapiRenderClient->lpVtbl->ReleaseBuffer(WasapiRenderClient, numWriteFrames, 0);
  if (hr != S_OK) {
    exit(1);
  }
}

void win32_init_com_api() {
  HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

  if (FAILED(hr)) {
    printf("Couldn't initial COM\n");
    return;
  };
  printf("Initialized COM \n");
}
/* void win32_init_xaudio2(uint32_t sampleRate, uint32_t bitDepth, uint32_t numVoices, Win32VoicePool* _data){ */

/*   HMODULE xaudio2 = LoadLibraryA("XAudio2_9.dll"); */
/*   if(xaudio2 == NULL){ */
/*     printf("XAudio2_9.dll not found\n"); */
/*     return; */
/*   } */

/*   IXAudio2 *xAudio; */
/*   xaudio2_create *XAudio2Create= (xaudio2_create*) GetProcAddress(xaudio2, "XAudio2Create"); */

/*   int numChannels =2; */
/*   int bytesInASample =  numChannels*bitDepth /8; */
/*   WAVEFORMATEX format= {0}; */
/*   format.wFormatTag=WAVE_FORMAT_PCM; */
/*   format.nChannels=numChannels; */
/*   format.nSamplesPerSec=sampleRate; */
/*   format.nAvgBytesPerSec = bytesInASample*sampleRate; */
/*   format.nBlockAlign=bytesInASample; */
/*   format.wBitsPerSample = bitDepth; */
/*   format.cbSize=0; */

/*   HRESULT res=  XAudio2Create(&xAudio, 0, XAUDIO2_DEFAULT_PROCESSOR); */
/*   if(res!=S_OK){ */
/*     printf("Couldn't create XAudio2\n"); */
/*   } */

/*   IXAudio2MasteringVoice* masteringVoice; */

/*   res=  xAudio->lpVtbl->CreateMasteringVoice( */
/* 					     xAudio, */
/* 					     &masteringVoice, */
/* 					     numChannels, */
/* 					     sampleRate, */
/* 					     XAUDIO2_NO_VIRTUAL_AUDIO_CLIENT, */
/* 					     NULL, */
/* 					     NULL, */
/* 					     // @NOTE: for now, just use other. I don't know the implications of using GameEffects */
/* 					     AudioCategory_Other); */

/*   if(res!=S_OK){ */
/*     printf("Couldn't create XAudio Mastering Voice\n"); */
/*   } */

/*   IXAudio2SubmixVoice * pSFXSubmixVoice; */
/*   IXAudio2_CreateSubmixVoice(xAudio, &pSFXSubmixVoice,numChannels,sampleRate,0,0,0,0); */

/*   XAUDIO2_SEND_DESCRIPTOR SFXSend = {0,(IXAudio2Voice*) pSFXSubmixVoice}; */
/*   XAUDIO2_VOICE_SENDS SFXSendList = {1, &SFXSend}; */

/*   if(_data->voices){ */
/*     VirtualFree(_data->voices,0, MEM_RELEASE); */
/*   } */
/*   _data->voices = VirtualAlloc( */
/* 			       0, */
/* 			       sizeof(Win32Voice)*numVoices, */
/* 			       MEM_COMMIT | MEM_RESERVE, */
/* 			       PAGE_READWRITE  */
/* 			       ); */
/*   _data->used = 0; */

/*   for(int i=0;i<numVoices;i++){ */
/*     IXAudio2SourceVoice* srcVoice; */
/*     if(FAILED(IXAudio2_CreateSourceVoice(xAudio, &srcVoice,&format,0,XAUDIO2_DEFAULT_FREQ_RATIO,NULL,&SFXSendList,NULL))){ */
/*       printf("Couldn't create source voice\n"); */
/*     } */
/*     Win32Voice voice; */
/*     voice.underlyingVoice = srcVoice; */
/*     _data->voices[i] = voice; */

/*   } */
/* } */

void temp_print_time_stamp(LARGE_INTEGER _compareToStamp) {

  LARGE_INTEGER perfTimeStamp;
  QueryPerformanceCounter(&perfTimeStamp);
  LARGE_INTEGER currentStamp = perfTimeStamp;

  int64_t dif = currentStamp.QuadPart - _compareToStamp.QuadPart;

  int64_t ElapsedSeconds = dif / PerformanceQueryFrequency.QuadPart;

  int64_t ElapsedMS = (dif * 1000) / PerformanceQueryFrequency.QuadPart;

  int64_t ElapsedMicroSeconds = (dif * 1000 * 1000) / PerformanceQueryFrequency.QuadPart;

  float FPS = 1.0 / ((float)ElapsedMS / 1000);

  printf("Elapsed:%dms, FPS:%f\n", ElapsedMS, FPS);
}

void win32_set_key_state(CG_InputKey *key, bool wasDownedThisFrame, bool isPressed, bool wasReleasedThisFrame) {
  key->WasDownedThisFrame = wasDownedThisFrame;
  key->IsPressed = isPressed;
  key->WasReleasedThisFrame = wasReleasedThisFrame;
}

void win32_reset_key_state(CG_InputKey *key, bool preserveIsPressed) {
  bool isPressed = key->IsPressed;
  if (!preserveIsPressed) {
    isPressed = false;
  }
  win32_set_key_state(key, false, isPressed, false);
};

LRESULT Win32CallbackFunc(HWND _window, UINT _msgId, WPARAM param3, LPARAM param4) {

  LRESULT result = 0;
  switch (_msgId) {
  case WM_CLOSE: {
    OutputDebugStringW(L"VM_CLOSE\n");
    AppRunning = false;
  } break;
  case WM_DESTROY: {
    OutputDebugStringW(L"VM_DESTROY\n");
    AppRunning = false;
  }
  case WM_SIZE: {

  } break;
  case WM_SIZING:{
    break;
  }
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
  case WM_SYSKEYUP: {
    bool wasDown = ((param4 & (1 << 30)) != 0);
    bool isDown = ((param4 & (1 << 31)) == 0);
    bool wasDownedThisFrame = !wasDown && isDown;
    bool wasReleasedThisFrame = !isDown && wasDown;

    uint32_t code = param3;

    switch (code) {
    case 'W': {
      //	printf("W was down %d, is down %d\n", wasDown, isDown);
      win32_set_key_state(&GlobalInput.Keyboard.w, wasDownedThisFrame, isDown, wasReleasedThisFrame);
    } break;
    case 'S': {
      win32_set_key_state(&GlobalInput.Keyboard.s, wasDownedThisFrame, isDown, wasReleasedThisFrame);
    } break;

    case 'A': {
      //	printf("W was down %d, is down %d\n", wasDown, isDown);
      win32_set_key_state(&GlobalInput.Keyboard.a, wasDownedThisFrame, isDown, wasReleasedThisFrame);
    } break;
    case 'D': {
      win32_set_key_state(&GlobalInput.Keyboard.d, wasDownedThisFrame, isDown, wasReleasedThisFrame);
    } break;

    case VK_F4: {
      bool alting = (param4 & (1 << 29)) != 0;
      if (alting) {
        AppRunning = false;
      }
    } break;
    default: {

    } break;
    }

  } break;
  default: {
    result = DefWindowProc(_window, _msgId, param3, param4);
  };
  };

  return result;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
  Win32PlatformConfig = cg_get_platform_config();
  QueryPerformanceFrequency(&PerformanceQueryFrequency);

  WNDCLASSW windclass = {0};
  win32_resize_dib_section(&GlobalOffscreenBuffer, Win32PlatformConfig.ScreenWidth, Win32PlatformConfig.ScreenHeight);

  windclass.style = CS_HREDRAW | CS_VREDRAW;
  windclass.lpfnWndProc = Win32CallbackFunc;
  windclass.hInstance = hInstance;
  // windowclass.hIcon=;
  windclass.hCursor = LoadCursor(NULL, IDC_ARROW);
  //  LPCWSTR   lpszMenuName;
  windclass.lpszClassName = L"cgame";

  RegisterClassW(&windclass);

  HWND hwnd = CreateWindowExW(0, // Optional window styles.

                              windclass.lpszClassName,          // Window class
                              L"cgame",                         // Window text
                              WS_OVERLAPPEDWINDOW | WS_VISIBLE, // Window style

                              // Size and position
                              CW_USEDEFAULT, CW_USEDEFAULT,
			      Win32PlatformConfig.ScreenWidth,
			      Win32PlatformConfig.ScreenHeight,

                              NULL,      // Parent window
                              NULL,      // Menu
                              hInstance, // Instance handle
                              NULL       // Additional application data
  );

  if (!hwnd) {
    OutputDebugStringW(L"Couldn't create window\n");
    return -1;
  }

  // set initial state of all keys here
  win32_reset_key_state(&GlobalInput.Keyboard.w, false);
  win32_reset_key_state(&GlobalInput.Keyboard.a, false);
  win32_reset_key_state(&GlobalInput.Keyboard.s, false);
  win32_reset_key_state(&GlobalInput.Keyboard.d, false);

  // INIT APPLICATION HERE

  // main game memory allocation
  CG_Memory gameMemory = {0};

#ifdef CGAME_DEVELOPMENT
  LPVOID baseAddress = (LPVOID)Terabytes((uint64_t)2);

#else
  LPVOID baseAddress = NULL;
#endif



  win32_init_com_api();
  win32_init_wasapi(Win32PlatformConfig.AudioSampleRate, Win32PlatformConfig.AudioBitDepth, Win32PlatformConfig.AudioBufferSizeInSeconds, Win32PlatformConfig.AudioChannelsCount);
  //  win32_init_xaudio2(SAMPLE_RATE_DEFAULT, BIT_DEPTH,MAX_SOURCE_VOICES, &GlobalVoicePool);
  AppRunning = true;



  gameMemory.AudioBufferCurrentWriteLengthFrames = 0;
  gameMemory.AudioBufferCurrentWritePositionFrames = 0;

  gameMemory.AudioBuffer = VirtualAlloc(NULL, WasapiNumTotalBufferBytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);


  // @TODO allow game to be played without audio
  Assert(gameMemory.AudioBuffer);

  gameMemory.AudioBufferTotalFrames = WasapiNumTotalBufferFrames;
  gameMemory.AudioBufferTotalBytes = WasapiNumTotalBufferBytes;

  //  printf("persistant: %d, volatile: %d\n", (uint32_t)baseAddress, gameMemory.VolatileStorage);

  cg_init();

  
  MSG msg = {0};
  printf("Created Window, starting app\n");

  LARGE_INTEGER previousFrameTimeStamp;
  QueryPerformanceCounter(&previousFrameTimeStamp);

  //  void* fileContents=  platform_read_whole_file(__FILE__);
  local_persist bool useWasapiTest = false;
  while (AppRunning) {
    LARGE_INTEGER perfTimeStamp;

    QueryPerformanceCounter(&perfTimeStamp);
    int64_t dif = perfTimeStamp.QuadPart - previousFrameTimeStamp.QuadPart;

    int64_t ElapsedMicroSeconds = (dif * 1000 * 1000) / PerformanceQueryFrequency.QuadPart;

    float ElapsedSeconds = ElapsedMicroSeconds / (1000.0 * 1000.0);
    //   printf("DeltaTime: %f\n", ElapsedSeconds);
    GlobalDeltaTime = ElapsedSeconds;

    previousFrameTimeStamp = perfTimeStamp;

    win32_reset_key_state(&GlobalInput.Keyboard.w, true);
    win32_reset_key_state(&GlobalInput.Keyboard.a, true);
    win32_reset_key_state(&GlobalInput.Keyboard.s, true);
    win32_reset_key_state(&GlobalInput.Keyboard.d, true);

    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    DeviceContext = GetDC(hwnd);

    CG_OffscreenBuffer buffer;
    buffer.Memory = GlobalOffscreenBuffer.Memory;
    buffer.Height = GlobalOffscreenBuffer.Height;
    buffer.Width = GlobalOffscreenBuffer.Width;
    buffer.BytesPerPixel = GlobalOffscreenBuffer.BytesPerPixel;
    cg_update(&gameMemory, &buffer, &GlobalInput, GlobalDeltaTime);
    
    uint32_t wasapiNumFramesPadding;
    WasapiAudioClient->lpVtbl->GetCurrentPadding(WasapiAudioClient, &wasapiNumFramesPadding);
    uint32_t numFramesToWrite = WasapiNumTotalBufferFrames - wasapiNumFramesPadding;

    gameMemory.AudioBufferCurrentWriteLengthFrames = numFramesToWrite;

    write_sound_test();
    //    gameMemory.AudioBufferCurrentWritePositionFrames = wasapiNumFramesPadding;



    win32_copy_buffer_to_window(GlobalOffscreenBuffer, DeviceContext, win32_get_window_width(hwnd), win32_get_window_height(hwnd));

    //      printf("Played %d bytes out of %d and %d samples out of %d\n", soundBufferBytesPlayedThisLoop,soundBufferSize, soundBufferSamplesPlayedThisLoop, samplesInBuffer);


    if(GlobalInput.Keyboard.d.WasDownedThisFrame){
      useWasapiTest = !useWasapiTest;
    }

    //win32_wasapi_run_test();
 
    win32_wasapi_copy_buffer(gameMemory.AudioBuffer, gameMemory.AudioBufferCurrentWritePositionFrames * AudioFormat.nBlockAlign, gameMemory.AudioBufferCurrentWriteLengthFrames * AudioFormat.nBlockAlign, gameMemory.AudioBufferCurrentWriteLengthFrames);

        

gameMemory.AudioBufferCurrentWritePositionFrames =
    (gameMemory.AudioBufferCurrentWritePositionFrames + numFramesToWrite)
  % WasapiNumTotalBufferFrames;


// printf("Buffer write pos/padding/num write frames: %d/%d/%d\n", gameMemory.AudioBufferCurrentWritePositionFrames, wasapiNumFramesPadding, gameMemory.AudioBufferCurrentWriteLengthFrames);
 if(GlobalDeltaTime >= Win32PlatformConfig.AudioBufferSizeInSeconds){
   printf("BIG MASSIVE FAT DANGEROUS BLACK HOLE LEVEL WARNING: Going to miss audio thing, dt: %f, audio buffer size seconds: %f\n", GlobalDeltaTime, Win32PlatformConfig.AudioBufferSizeInSeconds);
 }
    ReleaseDC(hwnd, DeviceContext);
   // temp_print_time_stamp(perfTimeStamp);
  }
  return 0;
}

// platform api implementation

void platform_play_wave_file(char *path) { win32_play_wave_file(path); }

void *platform_read_whole_file(char *path) {
  printf(path);
  HANDLE hnd = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
  void *content = NULL;
  if (hnd == INVALID_HANDLE_VALUE) {
    return NULL;
  }
  LARGE_INTEGER fileSize;
  if (GetFileSizeEx(hnd, &fileSize)) {

    content = VirtualAlloc(NULL, fileSize.QuadPart, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    // ReadFile can read a maximum of sizeof(DWORD) bytes
    // so fileSize.QuadPart should be less than that
    Assert(fileSize.QuadPart <= 0xFFFFFFFF);
    DWORD bytesRead;
    if (ReadFile(hnd, content, fileSize.QuadPart, &bytesRead, NULL) && (bytesRead == fileSize.QuadPart)) {

    } else {
      platform_free_file_memory(content);
      content = NULL;
    }

  } else {
  }

  CloseHandle(hnd);
  return content;
}

void platform_free_file_memory(void *memory) {
  if (memory != NULL) {
    VirtualFree(memory, 0, MEM_RELEASE);
  }
}


//@Incomplete - size is not used here, we're not writing the bytes to the file
void platform_write_or_overwrite_file(char *path, void *bytes, uint64_t size) { HANDLE hnd = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL); }



u64 platform_memory_get_page_size(){
  SYSTEM_INFO sysinfo = {0};
  GetSystemInfo(&sysinfo);

  return sysinfo.dwPageSize;
}
void *platform_memory_reserve(u64 _size){
  return VirtualAlloc(NULL, _size, MEM_RESERVE, PAGE_READWRITE);
}
b32 platform_memory_commit(void* _mem, u64 _size){
  return VirtualAlloc(_mem, _size, MEM_COMMIT, PAGE_READWRITE) !=NULL;
}; 
b32 platform_memory_decommit(void* _mem, u64 _size){
  return VirtualFree(_mem, _size, MEM_DECOMMIT);
 }
b32 platform_memory_free(void* _mem, u64 _size){
return VirtualFree(_mem, _size, MEM_RELEASE);
}
