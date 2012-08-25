#include "dxstdafx.h"
#include ".\game.h"

#include "globaldefine.h"

CGame::CGame(IDirect3DDevice9* pd3dDevice, float screenWidth, float screenHeight) :
        m_bg(pd3dDevice, g_wstrBackgroundTex, g_bgWidth, g_bgHeight, g_bgZFar, g_bgTexTile),
        m_splash(pd3dDevice, g_wstrSplashTex, g_bgWidth, g_bgHeight, 50.0f, 1),
        m_bSplash(true),
        m_cZoom(ZOOMING_NO), nZoomDelta(0),
        m_CameraShiftX(0.0f), m_CameraShiftY(0.0f),
        m_ScreenWidth(screenWidth), m_ScreenHeight(screenHeight),
        m_vUp(0.0f, 1.0f, 0.0f),
        m_vAt(0.0f, 0.0f, 1),
        m_vEye(0.0f, 0.0f, g_fMinZ/1.5f),
        m_HUD(pd3dDevice),
        m_GameEntities(pd3dDevice),
        m_n2sprite(pd3dDevice),
        m_nLevel(1)
{
    float fAspectRatio = screenWidth / screenHeight;
    D3DXMatrixPerspectiveFovLH( &m_mPersp, D3DX_PI/4, fAspectRatio, 0.1f, 1000.0f);

    D3DXMatrixIdentity(&m_mIdentity);

    m_HUD.AddItem(pd3dDevice, D3DXVECTOR3(0.0f, 0.0f, 0.0f), g_wstrLogoTex);
    m_HUD.AddItem(pd3dDevice, D3DXVECTOR3(g_hudCashX, 0.0f, 0.0f), g_wstrCashTex);
    m_HUD.AddItem(pd3dDevice, D3DXVECTOR3(g_hudCashX, 30.0f, 0.0f), g_wstrCostTex);	
    m_HUD.AddItem(pd3dDevice, D3DXVECTOR3(-20.0f, screenHeight - 60.0f, 0.0f), g_wstrLevelTex);	

    m_HUD.AddVisionPlane(pd3dDevice, D3DXVECTOR2(screenWidth, screenHeight), g_wstrVisionTex, 0);

    m_n2sprite.AddTexture(pd3dDevice, g_wstrNumberTex);
}

CGame::~CGame(void)
{
}

VOID CGame::OnLostDevice()
{
    HRESULT hr;
    V( m_HUD.OnLostDevice() );
    V( m_GameEntities.OnLostDevice() );
	V( m_n2sprite.OnLostDevice() );
}

VOID CGame::OnResetDevice()
{
    HRESULT hr;
    V( m_HUD.OnResetDevice() );
    V( m_GameEntities.OnResetDevice() );
	V( m_n2sprite.OnResetDevice() );
}

VOID CGame::InitApp(HWND hWnd)
{
    m_hWnd = hWnd;
    m_GameEntities.InitGame(hWnd, m_nLevel);
}

VOID CGame::OnFrameMove(float fElapsedTime)
{
    if(m_bSplash)
        return;
    CameraZoom(fElapsedTime);
    bool bLevelDone = m_GameEntities.OnFrameMove(fElapsedTime);

    if(bLevelDone) {
        m_nLevel++;        
        m_GameEntities.RestartEntities(m_nLevel+1);
    }
}

HRESULT CGame::OnFrameRender(IDirect3DDevice9* pd3dDevice)
{
    HRESULT hr;   
    
    //
    // Set up Matrices
    //
    D3DXMATRIX mView;
    D3DXMatrixLookAtLH(&mView, &m_vEye, &m_vAt, &m_vUp);
    V( pd3dDevice->SetTransform(D3DTS_PROJECTION, &m_mPersp) );
    V( pd3dDevice->SetTransform( D3DTS_VIEW, &mView ) );

    if(m_bSplash) {
        V_RETURN( m_splash.Render(pd3dDevice) );
        return S_OK;
    }

    //
    // Project Mouse location to world space
    //     
    D3DVIEWPORT9 viewport;
    D3DXVECTOR3 worldMouse;

    V( pd3dDevice->GetViewport(&viewport) );
    GetWorldMouse(worldMouse, mView, viewport);

    // to tell where to draw the mall under cursor        
    m_GameEntities.SetDrawMouseLocation(worldMouse); 

    //
    // Draw semi 2D stuff
    //
    V_RETURN( m_bg.Render(pd3dDevice) );
    
    V_RETURN( m_GameEntities.OnFrameRender(pd3dDevice) );

    // Draw screenspace stuff
    V( pd3dDevice->SetTransform( D3DTS_VIEW, &m_mIdentity ) );
    V( pd3dDevice->SetTransform( D3DTS_WORLD, &m_mIdentity ) );
    V_RETURN( m_HUD.Render(pd3dDevice) );

    // Player Money
    m_n2sprite.SetCurrencyDisplay(true);

    D3DXVECTOR3 moneypos(g_hudCashX+125.0f, 0.0f, 0.0f);
    V_RETURN( m_n2sprite.DrawNumberToScreen(m_GameEntities.GetPlayerMoney(), moneypos, 0xFFFFFFFF) );    
    moneypos.y = 30.0f;
    V_RETURN( m_n2sprite.DrawNumberToScreen(m_GameEntities.GetPlayerMallCost(), moneypos, 0xFFFFFFFF) );   

    // Computer money
    moneypos.y = 0.0f;
    moneypos.x = 180.0f;
    for(int i=0; i<m_GameEntities.GetNumComputerPlayers(); i++)
    {
        float money = m_GameEntities.GetComputerPlayerMoney(i);
        float cost = m_GameEntities.GetComputerPlayerCost(i);
        DWORD color = m_GameEntities.GetComputerPlayerColor(i);

        moneypos.y = 0.0f;
        V_RETURN( m_n2sprite.DrawNumberToScreen(money, moneypos, color) );
        moneypos.y = 30.0f;
        V_RETURN( m_n2sprite.DrawNumberToScreen(cost, moneypos, color) );

        moneypos.x += 125.0f;
    }

    m_n2sprite.SetCurrencyDisplay(false);
    // number of houses on map.
    /*
    moneypos.y = m_ScreenHeight - 100.0f;
    moneypos.x = 0.0f;
    V_RETURN( m_n2sprite.DrawNumberToScreen((float)m_GameEntities.GetNumHouses(), moneypos, 0xFFFFFFFF) );    
    */

    // level
    V_RETURN( m_n2sprite.DrawNumberToScreen((float)m_nLevel, D3DXVECTOR3(100.0f, m_ScreenHeight - 60.0f, 0), 0xFFFFFFFF) );

    return S_OK;
}

VOID CGame::MouseEvent(int xpos, int ypos, bool bLMB, bool bRMB, int nWheelDelta, bool bMouseMove)
{
    if(m_bSplash && !bMouseMove) {
        m_bSplash = false;
        return ;
    }
    // hack to get responsive zoom
    if(nZoomDelta > 0.0f && nWheelDelta > 0.0f || nZoomDelta < 0.0f && nWheelDelta < 0.0f)
        nZoomDelta += -nWheelDelta/5; // Zoom based on mwheel delta
    else 
        nZoomDelta = -nWheelDelta/5; // Zoom based on mwheel delta

    if(!bMouseMove)
    {   // check for clicks on gui and send entity requests to game entities.
        if(bLMB)
            m_GameEntities.RequestMallAtMouse();
        else if(bRMB)
            m_GameEntities.DropCultureBombAtMouse();
    }
    else
    {   // Scroll screen based on the mouse movement
        if(xpos < g_MovementThreshX)    m_CameraShiftX = -g_MovementSpeed;
        else if(xpos > m_ScreenWidth - g_MovementThreshX)    m_CameraShiftX = g_MovementSpeed;
        else    m_CameraShiftX = 0.0f;

        if(ypos < g_MovementThreshY)    m_CameraShiftY = g_MovementSpeed;
        else if(ypos > m_ScreenHeight - g_MovementThreshY)    m_CameraShiftY = -g_MovementSpeed;
        else    m_CameraShiftY = 0.0f;
    }

    m_MouseLoc.x = (float)xpos;
    m_MouseLoc.y = (float)ypos;    // to place mall under cursor  
    m_MouseLoc.z = 0.0f;
}

VOID CGame::KeyboardEvent(UINT nChar, bool bKeyDown)
{
    if(m_bSplash) {
        m_bSplash = false;
        return;
    }
    if(nChar == 'Z') {
        if(bKeyDown)
            m_cZoom = ZOOMING_IN;
        else
            m_cZoom = ZOOMING_NO;
    }
    else if(nChar == 'X') {
        if(bKeyDown)
            m_cZoom = ZOOMING_OUT;
        else
            m_cZoom = ZOOMING_NO;
    }
    /*
    else if(nChar == 'Q' && bKeyDown) {
        m_nLevel++;
        m_GameEntities.RestartEntities(m_nLevel+1);
    } 
    */
}

VOID CGame::CameraZoom(float fElapsedTime)
{
    // Deal with the Zoom
    if(m_cZoom == ZOOMING_IN || nZoomDelta < 0) {
        if(m_vEye.z < g_fMaxZ) {
            m_vEye.z += g_ZoomSpeed * fElapsedTime;            
            if(nZoomDelta) nZoomDelta++;
        }
        else
            nZoomDelta = 0;
    }
    else if(m_cZoom == ZOOMING_OUT || nZoomDelta > 0) {
        if(m_vEye.z > g_fMinZ) {
            m_vEye.z -= g_ZoomSpeed * fElapsedTime;            
            if(nZoomDelta) nZoomDelta--;
        }
        else
            nZoomDelta = 0;
    }

    // Deal with the shift
    m_vEye.x += m_CameraShiftX*fElapsedTime;
    m_vEye.y += m_CameraShiftY*fElapsedTime;

    if(m_vEye.x > g_fMaxX || m_vEye.x < -g_fMaxX)  m_vEye.x -= m_CameraShiftX*fElapsedTime;
    if(m_vEye.y > g_fMaxY || m_vEye.y < -g_fMaxY)  m_vEye.y -= m_CameraShiftY*fElapsedTime;

    m_vAt = m_vEye;
    m_vAt.z = m_vEye.z + 1;
}

VOID CGame::GetWorldMouse(D3DXVECTOR3& worldMouse, const D3DXMATRIX& mView, const D3DVIEWPORT9& viewport)
{
    // project mouse to world space. normalize direction from camera to world space. add to camera.    
    D3DXVec3Unproject(&worldMouse, &m_MouseLoc, &viewport, &m_mPersp, &mView, &m_mIdentity);
    worldMouse = worldMouse - m_vEye;
    D3DXVec3Normalize(&worldMouse, &worldMouse);

    // Remember, eneities are @ z = g_zDist = ~10.0f;
    float multiplier = (m_vEye.z - g_zDist + 0.1f) / worldMouse.z;
    worldMouse = m_vEye - worldMouse * ( multiplier );
}
