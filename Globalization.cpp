//--------------------------------------------------------------------------------------
// File: Globalization.cpp
//
// Empty starting point for new Direct3D applications
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "dxstdafx.h"
#include "resource.h"

#include "mp3player.h"
#include "game.h"
#include "globaldefine.h"


//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------
Cmp3player                          g_mp3p; //! MP3 Player used to play music!
CGame*                              g_pGame; //! uhhh


//--------------------------------------------------------------------------------------
// Function Prototypes
//--------------------------------------------------------------------------------------
void            InitApp();   //! Initializes 
void            CleanupApp();   //! Cleans up memory allocated in InitApp()
//VOID            RenderText();
VOID            HandleGraphEvent(); //! Used to handle graph mp3 player
UINT            WCharStringToCharString(LPCWSTR fn, LPSTR pszFilename, int MAXLENGTH);


bool    CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext );
bool    CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps, void* pUserContext );
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
void    CALLBACK OnFrameMove( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
void    CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext );
void    CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
void    CALLBACK MouseProc(bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown, bool bSideButton1Down, bool bSideButton2Down, int nMouseWheelDelta, int xPos, int yPos, void* pUserContext);
void    CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );
void    CALLBACK OnLostDevice( void* pUserContext );
void    CALLBACK OnDestroyDevice( void* pUserContext );


//--------------------------------------------------------------------------------------
// Preloads the music and sound effects
//--------------------------------------------------------------------------------------
void InitApp()
{
    //! Play bg music
    g_mp3p.PlayFile(g_cstrThemeSong, DXUTGetHWND(), WM_GRAPHNOTIFY);

    //! Setup Sound manager and Preload soundfx
    g_pGame->InitApp(DXUTGetHWND());
    //TODO:
}

void CleanupApp()
{
    g_mp3p.CleanUp();
}

//--------------------------------------------------------------------------------------
// Used to handle mp3 player
//--------------------------------------------------------------------------------------
VOID HandleGraphEvent() {  
   if(!g_mp3p.IsEventInit())
      return;
   long evCode;
   LONG_PTR param1, param2;   
   while(SUCCEEDED(g_mp3p.pEvent->GetEvent(&evCode, &param1, &param2, 0))) {      
      g_mp3p.pEvent->FreeEventParams(evCode, param1, param2);
      switch(evCode) {
        case EC_COMPLETE:   // loop bg music
            g_mp3p.CleanUp();
            g_mp3p.PlayFile(g_cstrThemeSong, DXUTGetHWND(), WM_GRAPHNOTIFY);
            return;
        case EC_USERABORT: // Fall through.
        case EC_ERRORABORT:
           g_mp3p.CleanUp();
           return;           
      }      
   }
}

//! Converts a psz wchar to a psz char.
UINT WCharStringToCharString(LPCWSTR fn, LPSTR pszFilename, int MAXLENGTH) {
    UINT nLength;
    StringCchLength(fn, MAXLENGTH, &nLength);        
    WideCharToMultiByte(CP_ACP, NULL, fn, nLength, pszFilename, MAXLENGTH, NULL, NULL);
    pszFilename[nLength] = 0; // null terminate string
    return nLength;
}


//--------------------------------------------------------------------------------------
// Rejects any devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, 
                                  D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    // Typically want to skip backbuffer formats that don't support alpha blending
    IDirect3D9* pD3D = DXUTGetD3DObject(); 
    if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
                    AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, 
                    D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
        return false;

    return true;
}


//--------------------------------------------------------------------------------------
// Before a device is created, modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps, void* pUserContext )
{
    return true;
}


//--------------------------------------------------------------------------------------
// Create any D3DPOOL_MANAGED resources here 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    float width = (float)pBackBufferSurfaceDesc->Width;
    float height= (float)pBackBufferSurfaceDesc->Height;
    g_pGame = new CGame(pd3dDevice, width, height);

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3DPOOL_DEFAULT resources here 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice, 
                                const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    g_pGame->OnResetDevice();
    return S_OK;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    g_pGame->OnFrameMove(fElapsedTime);
}


//--------------------------------------------------------------------------------------
// Render the scene 
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    HRESULT hr;

    // Clear the render target and the zbuffer 
    V( pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0) );

    // Render the scene
    if( SUCCEEDED( pd3dDevice->BeginScene() ) )
    {
        V( g_pGame->OnFrameRender(pd3dDevice) );
        V( pd3dDevice->EndScene() );
    }
}


//--------------------------------------------------------------------------------------
// Handle messages to the application 
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, 
                          bool* pbNoFurtherProcessing, void* pUserContext )
{
    /*
    *pbNoFurtherProcessing = g_UI.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    *pbNoFurtherProcessing = g_NextLevelButton.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;
    */
    switch(uMsg)
    {
    case WM_GRAPHNOTIFY: // event by mp3player
      HandleGraphEvent();
      break;    
    case WM_MOUSEMOVE:
        g_pGame->MouseEvent(LOWORD(lParam), HIWORD(lParam), false, false, 0, true);
        break;
    }
    return 0;
}


//--------------------------------------------------------------------------------------
// Release resources created in the OnResetDevice callback here 
//--------------------------------------------------------------------------------------
void CALLBACK OnLostDevice( void* pUserContext )
{
    g_pGame->OnLostDevice();
}

//--------------------------------------------------------------------------------------
// Release resources created in the OnCreateDevice callback here
//--------------------------------------------------------------------------------------
void CALLBACK OnDestroyDevice( void* pUserContext )
{
    SAFE_DELETE(g_pGame);
}

//--------------------------------------------------------------------------------------
// Keyboard Event handler
//--------------------------------------------------------------------------------------
void  CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
    /*
    WCHAR wszInput[9];
    UINT pw;
    switch(nControlID)
    {
        
    case IDC_LOADPASSWORD:
        g_UI.GetEditBox( IDC_PASSWORDENTRY )->GetTextCopy(wszInput, 9);      
        
        char rawr[9];
        WCharStringToCharString(wszInput, rawr, 9);
        pw = static_cast<int>(strtol(rawr, NULL, 16));
        
        if(g_pJunkie->LoadPassword(pw))
            g_UI.GetEditBox( IDC_PASSWORDENTRY )->SetText(L"LEVEL LOADED");
        else
            g_UI.GetEditBox( IDC_PASSWORDENTRY )->SetText(L"Password Failed!");
        break;
    case IDC_LOADNEXTLEVEL:
        g_bDisplayNextLevelOption = false;
        g_pJunkie->LoadNextLevel();
        break;
        
    }
    */
}

//--------------------------------------------------------------------------------------
// Keyboard Event handler
//--------------------------------------------------------------------------------------
void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
    g_pGame->KeyboardEvent(nChar, bKeyDown);    
}

//--------------------------------------------------------------------------------------
// Mouse Event Handler
//--------------------------------------------------------------------------------------
void CALLBACK MouseProc(bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown, 
                        bool bSideButton1Down, bool bSideButton2Down, int nMouseWheelDelta, 
                        int xPos, int yPos, void* pUserContext)
{
    g_pGame->MouseEvent(xPos, yPos, bLeftButtonDown, bRightButtonDown, nMouseWheelDelta, false);    
}

//--------------------------------------------------------------------------------------
// Initialize everything and go into a render loop
//--------------------------------------------------------------------------------------
INT WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    // Set the callback functions
    DXUTSetCallbackDeviceCreated( OnCreateDevice );
    DXUTSetCallbackDeviceReset( OnResetDevice );
    DXUTSetCallbackDeviceLost( OnLostDevice );
    DXUTSetCallbackDeviceDestroyed( OnDestroyDevice );
    DXUTSetCallbackMsgProc( MsgProc );    
    DXUTSetCallbackKeyboard( KeyboardProc );
    DXUTSetCallbackMouse( MouseProc );
    DXUTSetCallbackFrameRender( OnFrameRender );
    DXUTSetCallbackFrameMove( OnFrameMove );

    // Initialize DXUT and create the desired Win32 window and Direct3D device for the application
    DXUTInit( true, false, true ); // Parse the command line, handle the default hotkeys, and show msgboxes
    DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
    DXUTCreateWindow( L"Globalization" );
    DXUTCreateDevice( D3DADAPTER_DEFAULT, true, 1024, 768, IsDeviceAcceptable, ModifyDeviceSettings );

    InitApp();

    // Start the render loop
    DXUTMainLoop();

    CleanupApp();

#if defined(DEBUG) || defined(_DEBUG)
    // Functions to pinpoint memory leaks
    // _CrtDumpMemoryLeaks ();
    _CrtCheckMemory();
#endif

    return DXUTGetExitCode();
}


