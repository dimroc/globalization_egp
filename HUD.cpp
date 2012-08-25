#include "dxstdafx.h"
#include ".\hud.h"
#include "GlobalDefine.h"

CHUD::CHUD(IDirect3DDevice9* pd3dDevice)
{
    HRESULT hr; 
    V( D3DXCreateSprite(pd3dDevice, &m_pSprite) );
}

CHUD::~CHUD(void)
{
    SAFE_RELEASE(m_pSprite);

    for(int i=0; i<(int)m_vHUDItems.size(); i++)
        SAFE_DELETE(m_vHUDItems[i]);

    set<SVisionPlane*, VisionPlanePointerFunctor>::iterator it;
    for(it = m_sVisionPlane.begin(); it != m_sVisionPlane.end(); it++)
        SAFE_DELETE(*it);
}

VOID CHUD::AddItem(IDirect3DDevice9* pd3dDevice, D3DXVECTOR3 topLeftPos, LPCWSTR texFileName, DWORD color, const D3DXMATRIX* pmtx)
{
    SHUDItem* pHUDitem;
    pHUDitem = new SHUDItem(pd3dDevice, topLeftPos, texFileName, color, pmtx);

    m_vHUDItems.push_back(pHUDitem);
}

BOOL CHUD::AddVisionPlane(IDirect3DDevice9* pd3dDevice, D3DXVECTOR2 screenDim, LPCWSTR texFileName, int zOrder)
{
    SVisionPlane* pVisionPlane;
    pVisionPlane = new SVisionPlane(pd3dDevice, screenDim, texFileName, zOrder);

    return m_sVisionPlane.insert(pVisionPlane).second;
}

HRESULT CHUD::Render(IDirect3DDevice9* pd3dDevice)
{
    HRESULT hr;    

    V( pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE ) ); 
    // take alpha from alpha channel
    V( pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE) );
    V( pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1) );

    V( pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA) );
    V( pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA) );

    //V( pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATER) );
    //V( pd3dDevice->SetRenderState(D3DRS_ALPHAREF, (DWORD)0x00000003)); 
    
    V( pd3dDevice->SetFVF(BoxVertex::FVF) );

    //
    // Render VisionPlanes first
    //
    set<SVisionPlane*, VisionPlanePointerFunctor>::iterator it;
    for(it = m_sVisionPlane.begin(); it != m_sVisionPlane.end(); it++) {
        SVisionPlane* pvision = *it;
        V( pd3dDevice->SetTexture(0, pvision->m_pTex) );
        V( pd3dDevice->SetStreamSource(0, pvision->m_pVB, 0, sizeof(BoxVertex)) );
        V( pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2) );
    }

    DWORD flags = D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_TEXTURE;
    V( m_pSprite->Begin(flags) );

    // Render HUD Items
    for(int i=0; i<(int)m_vHUDItems.size(); i++) {
        SHUDItem* pitem = m_vHUDItems[i];
		V( m_pSprite->SetTransform(&pitem->mtx) );
        V( m_pSprite->Draw(pitem->m_pTex, NULL, NULL, &pitem->pos, pitem->color) );
    }

    V( m_pSprite->End() );
    return S_OK;
}


////////////////////////////// SHUDItem Functions //////////////////////////
CHUD::SHUDItem::SHUDItem(IDirect3DDevice9* pd3dDevice, D3DXVECTOR3 topLeftPos, LPCWSTR texFileName, DWORD _color, const D3DXMATRIX* pmtx) :
    m_pTex(NULL), pos(topLeftPos), color(_color)
{
    HRESULT hr;
    // Create texture.
    V(D3DXCreateTextureFromFile(pd3dDevice, texFileName, &m_pTex) );

    if(pmtx)	mtx = (*pmtx);
	else		D3DXMatrixIdentity(&mtx);
}

////////////////////////////// SVisionPlane Functions ///////////////////////////
CHUD::SVisionPlane::SVisionPlane(IDirect3DDevice9* pd3dDevice, D3DXVECTOR2 screenDim, LPCWSTR texFileName, int _zOrder) : 
    m_pVB(NULL), m_pTex(NULL), pos(screenDim), zOrder(_zOrder)
{
    HRESULT hr;    
    // Create texture.
    V(D3DXCreateTextureFromFile(pd3dDevice, texFileName, &m_pTex) );    

    int size  = 4*sizeof(BoxVertex);
    V( pd3dDevice->CreateVertexBuffer( size, D3DUSAGE_WRITEONLY, BoxVertex::FVF, D3DPOOL_MANAGED, &m_pVB, NULL) );

    // max tex coordinates
    float w = 0.7f, h = 0.5f; // ugly hacek, wtever.
    float maxu = 1, maxv = 1;
    float zFar = 1;

    // Lock the buffer to gain access to the vertices 
    BoxVertex* pVertices;
    V( m_pVB->Lock(0, size, (VOID**)&pVertices, 0 ) );

    pVertices[0] = BoxVertex(D3DXVECTOR3(-w, -h, zFar), 0, maxv);
    pVertices[1] = BoxVertex(D3DXVECTOR3(-w, h, zFar), 0, 0);    
    pVertices[2] = BoxVertex(D3DXVECTOR3(w, -h, zFar), maxu, maxv);    
    pVertices[3] = BoxVertex(D3DXVECTOR3(w, h, zFar), maxu, 0);

    V( m_pVB->Unlock() );
}
