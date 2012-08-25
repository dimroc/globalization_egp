#pragma once

class CBackground
{
private:
    IDirect3DVertexBuffer9* m_pVB;
    LPDIRECT3DTEXTURE9 m_pTex;

    float m_width, m_height;
public:
    CBackground(IDirect3DDevice9* pd3dDevice, LPCWSTR texFileName, float width, float height, float zfar, float texTile);
    ~CBackground();

    HRESULT Render(IDirect3DDevice9* pd3dDevice, bool bMipmap = false);

    float GetWidth() { return m_width; }
    float GetHeight() { return m_height; }
};
