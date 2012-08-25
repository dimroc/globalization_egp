#pragma once

#include "globaldefine.h"

using std::vector;

class CGameEntities
{
private:

    struct SPlayer;
    struct SHouse;
    struct SMall;

    //---------------------------------------------------------------------------
    // Game Entities!
    //---------------------------------------------------------------------------

    //! Interface containing all Entity attributes. Inherited by other elements.
    struct IEntity {
        int nFrame; //! Which frame in the Texture Group it's in (sprites).
        float fScale; //! scale of entity!        
        float length; //! for collision detection
        float timeElapsed;
        float speed; //! movement speed

        DWORD color;
        RECT srcRect;
        D3DXMATRIX mWorld; //! Every Entity has it's own world mtx based on it's 2D position.

        D3DXVECTOR3 pos;
        LPDIRECT3DTEXTURE9 pTex; // not owner
        CSound* pSound; // not owner               

        IEntity(const D3DXVECTOR3& _pos, LPDIRECT3DTEXTURE9 _pTex, CSound* _pSound);            
        ~IEntity();
        //virtual VOID OnFrameMove(float elapsedTime)                         =   0;
        //virtual VOID RenderThruSprite(LPD3DXSPRITE pSprite)                 =   0;

        VOID UpdateWorldMatrix(bool bFlipHorizontal = false);
    };

    struct SPerson : public IEntity {    
        SHouse* pHome;
        SMall* pDestMall;
        float fMoney;        
        D3DXVECTOR3 curDir;

        SPerson(LPDIRECT3DTEXTURE9 _pTex, CSound* _pSound, SHouse* _pHome);

        VOID OnFrameMove(float elapsedTime);
        VOID RenderThruSprite(LPD3DXSPRITE pSprite);
    };

    struct SMall : public IEntity {
        SPlayer* pOwner;
        D3DXVECTOR3 vEntrances[4];
        int nHousesUsing; //! Number of houses using this mall

        D3DXVECTOR3 center;        
        bool bFull;

        SMall(const D3DXVECTOR3& _pos, LPDIRECT3DTEXTURE9 pTex, CSound* _pSound, SPlayer* _player);

        VOID OnFrameMove(float elapsedTime);
        VOID RenderThruSprite(LPD3DXSPRITE pSprite);
    };

    struct SHouse : public IEntity  {
        SMall* pClosestMall;
        vector<SPerson*> vpPeople;
        D3DXVECTOR3 center;
        int nVisits;        
        int nClosestEntrance;

        BOOL bSpawnedHouse;

        SHouse(const D3DXVECTOR3& _pos, LPDIRECT3DTEXTURE9 _pTex, CSound* _pSound, SMall* _pCloseMall);
        ~SHouse();

        BOOL OnFrameMove(float elapsedTime);
        VOID RenderThruSprite(LPD3DXSPRITE pSprite);

        static LPDIRECT3DTEXTURE9 pPersonTex;
        static CSound* pPersonSound;
        static VOID SetPersonInfo(LPDIRECT3DTEXTURE9 _pTex, CSound *_pSound);

        VOID AttachMall(SMall* _pCloseMall);
        VOID CreatePerson();
    };   

    //---------------------------------------------------------------------------
    // Game Players!
    //---------------------------------------------------------------------------
    struct SPlayer {
        static UINT UniqueColorIndex; // initialized as 0.
        DWORD color;
        float money, mallCost, otherMallCost;

        int nMallsPurchased;
        int nBombs;
                                                        // if i ever go crazy and go over 10
        SPlayer() : money(g_nInitialMallCost), 
                    color(g_PlayerColors[UniqueColorIndex%10]), 
                    mallCost(g_nInitialMallCost), 
                    otherMallCost(0.0f),
                    nMallsPurchased(0),
                    nBombs(3)
        { UniqueColorIndex++; }

        ~SPlayer() {}

        VOID ComputerAI();
    };

    struct SBomb {
        CSound* pSound;
        D3DXMATRIX mWorld;
        RECT rect;
        LPDIRECT3DTEXTURE9 pTex;
        bool bActive;
        float fTime;
        float fSpeed;
    };

    // --------------------------------
    // Sound stuff
    // --------------------------------
    CSoundManager* m_pSoundManager;
    vector<CSound*> m_vpSound;    

    //---------------------------------------------------------------------------
    // D3DX stuff
    //---------------------------------------------------------------------------
    LPD3DXSPRITE m_pSprite;
    LPDIRECT3DTEXTURE9 m_pMallTex, m_pHouseTex, m_pPersonTex, m_pBombTex;

    //---------------------------------------------------------------------------
    // Game stuff
    //---------------------------------------------------------------------------
    //float inflation;
    vector<SMall*> m_vpMalls;
    vector<SHouse*> m_vpHouses;
    
    vector<SPlayer*> m_vpComputerPlayers;

    SPlayer m_Player;
    SBomb m_CultureBomb;
    SMall* m_pCursorMall;
    SHouse* m_pReferenceHouse;    

private:
    VOID InitSounds(HWND hWnd);    

    VOID CreateComputerPlayer();

    VOID ComputerRequestMall(const D3DXVECTOR3& vec, SPlayer* pplayer);

    VOID SpawnHouse(SMall* pmall, float spawnRadius, float objectlength); //! Spawns a house as close to pos as it can find or gives up eventually.    
    bool BuildMall(const D3DXVECTOR3& vec, SPlayer* pplayer, SMall** ppmall = NULL); //! Returns true on success

    VOID AttachClosestHouses(SMall* pmall); //! Checks houses if it's closest to this mall.
    SMall* GetClosestMall(const D3DXVECTOR3& pos); //! Returns the mall closest to this point.

    BOOL IsCollision(const D3DXVECTOR3& newpos, float length);

public:
    CGameEntities(IDirect3DDevice9 *pd3dDevice);
    ~CGameEntities(void);
    
    VOID InitGame(HWND hWnd, UINT nCompPlayers);    

    VOID RequestMallAtMouse();    
    VOID DropCultureBombAtMouse();

    float GetPlayerMoney() { return m_Player.money; }
    float GetPlayerMallCost() { 
        if(m_Player.otherMallCost > 0) return m_Player.otherMallCost; 
        else                           return m_Player.mallCost;
    }

    int GetNumHouses() { return (int)m_vpHouses.size(); }
    int GetNumComputerPlayers() { return (int)m_vpComputerPlayers.size(); }
    float GetComputerPlayerMoney(int index) { return m_vpComputerPlayers[index]->money; }
    float GetComputerPlayerCost(int index) { return m_vpComputerPlayers[index]->mallCost; }
    DWORD GetComputerPlayerColor(int index) { return m_vpComputerPlayers[index]->color; }

    VOID SetDrawMouseLocation(const D3DXVECTOR3& mouseLoc);    

    VOID RestartEntities(int nOpponents);

    bool OnFrameMove(float elapsedTime);
    HRESULT OnFrameRender(IDirect3DDevice9 *pd3dDevice);

    HRESULT OnLostDevice() { return m_pSprite->OnLostDevice(); }
    HRESULT OnResetDevice() { return m_pSprite->OnResetDevice(); }
};

BOOL IsBoxCollision(const D3DXVECTOR3& pos, float plength, const D3DXVECTOR3& dest, float dlength);
BOOL IsWithinDistance(const D3DXVECTOR3& pos, const D3DXVECTOR3& dest, float length);
BOOL IsWithinBGEllipse(const D3DXVECTOR3& pos, float length);

