#include "dxstdafx.h"
#include ".\numbertosprite.h"


CNumberToSprite::CNumberToSprite(IDirect3DDevice9* pd3dDevice) :
    m_pSprite(NULL), m_nDecimals(0), m_bCurrencyDisplay(false)
{
    HRESULT hr;
    V( D3DXCreateSprite(pd3dDevice, &m_pSprite) );    
}

CNumberToSprite::~CNumberToSprite(void)
{
    SAFE_RELEASE(m_pSprite);
    for(size_t i=0;i<m_vpTex.size();i++) {
        SAFE_RELEASE(m_vpTex[i]);
    }
}

// TODO: Precalc all the Rects and then just look it up instead of doing it every frame.
HRESULT CNumberToSprite::DrawNumberToScreen(float num, const D3DXVECTOR3& screenPos, DWORD color, float scale, UINT texIndex)
{
    if(texIndex >= m_vpTex.size() || num > 1000000000000.0f) // to stop buffer overflows. trillion limit
        return S_FALSE;    

    char digitbuf[14], decimalbuf[14];
    bool bNegative = false;
    if(num < 0) {
        bNegative = true;
        num = -num;
    }

    UINT tempnum = (UINT)num;
    itoa(tempnum, digitbuf, 10);

    num -= tempnum;
    for(int i=0;i<m_nDecimals;i++)
        num *= 10; // get decimal numbers

    tempnum = (int)num; // get decimals
    if(tempnum > 0) {
        itoa(tempnum, decimalbuf, 10);                
    }
    else
    {   // just put in 0's
        int i;
        for(i=0;i<m_nDecimals;i++)
            decimalbuf[i] = '0';
        decimalbuf[i] = NULL;
    }    

    //
    // Ready to Draw Number to Sprite!
    //
    D3DXVECTOR3 pos(screenPos);

    HRESULT hr;
    RECT rect;
    V( m_pSprite->Begin( 0 ) );

    // output dollar sign
    if(m_bCurrencyDisplay)
    {
        rect = CreateSymbolRect('$', m_vSpacing[texIndex]);
        V( m_pSprite->Draw(m_vpTex[texIndex], &rect, NULL, &pos, color) );
        pos.x += m_vSpacing[texIndex];
    }
    
    // output sign if any
    if(bNegative) {
        rect = CreateSymbolRect('-', m_vSpacing[texIndex]);
        V( m_pSprite->Draw(m_vpTex[texIndex], &rect, NULL, &pos, color) );
        pos.x += m_vSpacing[texIndex];
    }

    // out integer number part
    for(size_t i=0; i<strlen(digitbuf); i++)
    {
        rect = CreateSymbolRect(digitbuf[i], m_vSpacing[texIndex]);
        V( m_pSprite->Draw(m_vpTex[texIndex], &rect, NULL, &pos, color) );
        pos.x += m_vSpacing[texIndex];
    }

    // output decimal point if decimals
    if(m_nDecimals > 0) {
        rect = CreateSymbolRect('.', m_vSpacing[texIndex]);
        V( m_pSprite->Draw(m_vpTex[texIndex], &rect, NULL, &pos, color) );
        pos.x += m_vSpacing[texIndex];
    }

    // output decimals
    for(size_t i=0; i<strlen(decimalbuf); i++)
    {
        rect = CreateSymbolRect(decimalbuf[i], m_vSpacing[texIndex]);
        V( m_pSprite->Draw(m_vpTex[texIndex], &rect, NULL, &pos, color) );
        pos.x += m_vSpacing[texIndex];
    }

    V( m_pSprite->End() );
    return S_OK;
}

RECT CNumberToSprite::CreateSymbolRect(char c, int width)
{
    static float height = 64;

    RECT rect;
    ZeroMemory(&rect, sizeof(RECT));

    rect.bottom = (int)height;

    if(isdigit(c))
    {
        rect.left = width*atoi(&c);                                        
        rect.right = width*( atoi(&c) + 1 );
        return rect;
    }
    switch(c)
    {
    case '-':
        rect.left = width*11;
        rect.right = width*12;
        return rect;
    case '.':
        rect.left = width*10;
        rect.right = width*11;
        return rect;
    case '$':
    default:    // have '$' fall thru and be the fault for all other misc input.
        rect.left = width*12;
        rect.right = width*13;
        return rect;
    }
}

// returns the index this texture is in class
int CNumberToSprite::AddTexture(IDirect3DDevice9* pd3dDevice, LPCWSTR numberFileName)
{
    HRESULT hr;
    LPDIRECT3DTEXTURE9 pTex;
    V( D3DXCreateTextureFromFile(pd3dDevice, numberFileName, &pTex) );

    // get length
    D3DSURFACE_DESC desc;
    V( pTex->GetLevelDesc(0, &desc) );

    m_vSpacing.push_back((float)desc.Width / m_nSYMBOLS);
    m_vpTex.push_back(pTex);

    return (int)(m_vpTex.size() - 1);
}
