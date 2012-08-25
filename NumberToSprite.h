#pragma once

#include "GlobalDefine.h"

using std::vector;

class CNumberToSprite
{
private:
    static const int m_nSYMBOLS = 13; //! number of symbols horizontal in texture
    bool m_bCurrencyDisplay;

    LPD3DXSPRITE m_pSprite;
    vector<LPDIRECT3DTEXTURE9> m_vpTex;

    vector<float> m_vSpacing;
    int m_nDecimals;
public:
    CNumberToSprite(IDirect3DDevice9* pd3dDevice);
    ~CNumberToSprite(void);

    VOID SetCurrencyDisplay(bool b) { m_bCurrencyDisplay = b; }

    // returns the index this texture is in class
    int AddTexture(IDirect3DDevice9* pd3dDevice, LPCWSTR numFileName);
    VOID SetNumDecimals(int nDecs) { m_nDecimals = nDecs < 14? nDecs : 14; }

    RECT CreateSymbolRect(char c, int width);

    HRESULT DrawNumberToScreen(float num, const D3DXVECTOR3& screenPos, DWORD color, float scale = 1, UINT texIndex = 0);
    //HRESULT DrawNumberToScreen(int num); // TODO:

    HRESULT OnLostDevice() { return m_pSprite->OnLostDevice(); }
    HRESULT OnResetDevice() { return m_pSprite->OnResetDevice(); }
};
