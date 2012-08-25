#pragma once

#include "background.h"
#include "HUD.h"
#include "gameentities.h"
#include "numbertosprite.h"

#define ZOOMING_OUT     0x01
#define ZOOMING_IN      0x10
#define ZOOMING_NO      0x00

using std::vector;

class CGame
{
private:
    // --------------------------------
    // Camera stuff
    // --------------------------------
    D3DXMATRIX m_mPersp, m_mIdentity;
    D3DXVECTOR3 m_vEye, m_vAt, m_vUp;

    UCHAR m_cZoom;
    float m_CameraShiftX, m_CameraShiftY;
    float m_ScreenWidth, m_ScreenHeight;
    int nZoomDelta;

    // mouse pos in object space
    D3DXVECTOR3 m_MouseLoc;

    // --------------------------------
    // Background and hud items
    // --------------------------------
    CBackground m_bg, m_splash;
    CHUD m_HUD;
    CNumberToSprite m_n2sprite;

    bool m_bSplash;
    
    // --------------------------------
    // The Game internals
    // --------------------------------
    HWND m_hWnd;
    int m_nLevel; //! Determines # of enemies
    CGameEntities m_GameEntities;

private:
    VOID GetWorldMouse(D3DXVECTOR3& worldMouse, const D3DXMATRIX& mView, const D3DVIEWPORT9& viewport);

public:
    CGame(IDirect3DDevice9* pd3dDevice, float screenWidth, float screenHeight);
    ~CGame(void);

    VOID InitApp(HWND hWnd);    
    VOID CameraZoom(float fElapsedTime);

    VOID OnFrameMove(float fElapsedTime);
    HRESULT OnFrameRender(IDirect3DDevice9* pd3dDevice);

    VOID MouseEvent(int xpos, int ypos, bool bLMB, bool bRMB, int nWheelDelta, bool bMouseMove = false);
    VOID KeyboardEvent(UINT nChar, bool bKeyDown);

    VOID OnLostDevice();
    VOID OnResetDevice();
};
