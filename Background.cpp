#include "dxstdafx.h"
#include ".\background.h"
#include "GlobalDefine.h"

CBackground::CBackground(IDirect3DDevice9* pd3dDevice, LPCWSTR texFileName, float width, float height, float zFar, float texTile) :
    m_pVB(NULL), m_pTex(NULL), m_width(width), m_height(height)
{
    HRESULT hr;
    // Create the background!
    // Create texture.
    V(D3DXCreateTextureFromFile(pd3dDevice, texFileName, &m_pTex) );
    // Create background polygon.

    int size  = 4*sizeof(BoxVertex);
    V( pd3dDevice->CreateVertexBuffer( size, D3DUSAGE_WRITEONLY, BoxVertex::FVF, D3DPOOL_MANAGED, &m_pVB, NULL) );

    // max tex coordinates
    float maxu = texTile, maxv = texTile;

    // Lock the buffer to gain access to the vertices 
    BoxVertex* pVertices;
    V( m_pVB->Lock(0, size, (VOID**)&pVertices, 0 ) );

    pVertices[0] = BoxVertex(D3DXVECTOR3(-width/2, -height/2, zFar), 0, maxv);
    pVertices[1] = BoxVertex(D3DXVECTOR3(-width/2, height/2, zFar), 0, 0);    
    pVertices[2] = BoxVertex(D3DXVECTOR3(width/2, -height/2, zFar), maxu, maxv);    
    pVertices[3] = BoxVertex(D3DXVECTOR3(width/2, height/2, zFar), maxu, 0);

    V( m_pVB->Unlock() );
}

CBackground::~CBackground(void)
{
    SAFE_RELEASE(m_pVB);
    SAFE_RELEASE(m_pTex);    
}

HRESULT CBackground::Render(IDirect3DDevice9* pd3dDevice, bool bMipmap)
{
    HRESULT hr;        

    V( pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE ) );
    
    V( pd3dDevice->SetTexture(0, m_pTex) );
    V( pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP) );
    V( pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP) );

    V( pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR) );
    V( pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR) );
    
    if(bMipmap) {        
        V( pd3dDevice->SetSamplerState(0, D3DSAMP_MAXMIPLEVEL, 1) );
        V( pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR) );
    }
    else
    {
        V( pd3dDevice->SetSamplerState(0, D3DSAMP_MAXMIPLEVEL, 0) );
    }
    
    V( pd3dDevice->SetFVF(BoxVertex::FVF) );
    V( pd3dDevice->SetStreamSource(0, m_pVB, 0, sizeof(BoxVertex)) );
    V( pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2) );

    return S_OK;
}
