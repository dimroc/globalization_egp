#pragma once

using std::vector;
using std::set;

class CHUD
{    
private:
    struct SHUDItem {    
        D3DXVECTOR3 pos;    
        LPDIRECT3DTEXTURE9 m_pTex;
		
		D3DXMATRIX mtx;
		DWORD color;

        SHUDItem(IDirect3DDevice9* pd3dDevice, D3DXVECTOR3 topLeftPos, LPCWSTR texFileName, 
				 DWORD color, const D3DXMATRIX* mtx);
        ~SHUDItem() { SAFE_RELEASE(m_pTex); }
    };

    struct SVisionPlane {
    
        static const UINT FVF = D3DFVF_XYZ | D3DFVF_TEX1;
        D3DXVECTOR2 pos;
        int zOrder;

        IDirect3DVertexBuffer9* m_pVB;
        LPDIRECT3DTEXTURE9 m_pTex;

        SVisionPlane(IDirect3DDevice9* pd3dDevice, D3DXVECTOR2 screenDim, LPCWSTR texFileName, int zOrder);
        ~SVisionPlane() { SAFE_RELEASE(m_pVB); SAFE_RELEASE(m_pTex);  }
        int GetZOrder() const { return zOrder; }
        
    };
    class VisionPlanePointerFunctor {
    public:
        bool operator()(const SVisionPlane* lhs, const SVisionPlane* rhs ) const {
            return lhs->GetZOrder() < rhs->GetZOrder();
        }
    };

    LPD3DXSPRITE m_pSprite;
    vector<SHUDItem*> m_vHUDItems;
    set<SVisionPlane*, VisionPlanePointerFunctor> m_sVisionPlane;

public:
    CHUD(IDirect3DDevice9* pd3dDevice);    
    ~CHUD(void);

    HRESULT OnLostDevice() { return m_pSprite->OnLostDevice(); }
    HRESULT OnResetDevice() { return m_pSprite->OnResetDevice(); }

    //! Adds a static Item to HUD
    VOID AddItem(IDirect3DDevice9* pd3dDevice, D3DXVECTOR3 topLeftPos, LPCWSTR texFileName, 
							DWORD color = 0xFFFFFFFF, const D3DXMATRIX* mtx = NULL);

    //! Adds a vision plane immediately in front of the camera in the order zOrder with other Vision Planes
    BOOL AddVisionPlane(IDirect3DDevice9* pd3dDevice, D3DXVECTOR2 screenDim, LPCWSTR texFileName, int zOrder);

    HRESULT Render(IDirect3DDevice9* pd3dDevice);
};
