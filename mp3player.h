#pragma once

#ifdef MP3PLAYER_EXPORTS
#define MP3PLAYER_API __declspec(dllexport)
#else
#define MP3PLAYER_API __declspec(dllimport)
#endif

#include <dshow.h>
#include <dsound.h>

class MP3PLAYER_API Cmp3player {

private:
    IGraphBuilder *pGraph;
    IDirectSoundBuffer8 *pSound;
public:
   IMediaControl *pControl;
   IMediaEventEx   *pEvent;

public:
   Cmp3player(void) : pGraph(NULL), pControl(NULL), pEvent(NULL), pSound(NULL) {}
   
   HRESULT PlayFile(LPCWSTR, HWND, UINT);
   VOID CleanUp();
   BOOL IsEventInit() { return pEvent != NULL; }
};
