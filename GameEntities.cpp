#include "dxstdafx.h"
#include ".\gameentities.h"

#include "globaldefine.h"

CGameEntities::CGameEntities(IDirect3DDevice9 *pd3dDevice) :
    m_pCursorMall(NULL)
{
    HRESULT hr;
    V( D3DXCreateTextureFromFile( pd3dDevice, g_wstrMallTex, &m_pMallTex) );
    V( D3DXCreateTextureFromFile( pd3dDevice, g_wstrHouseTex, &m_pHouseTex) );
    V( D3DXCreateTextureFromFile( pd3dDevice, g_wstrPersonTex, &m_pPersonTex) );
    V( D3DXCreateTextureFromFile( pd3dDevice, g_wstrBombTex, &m_pBombTex) );

    memset(&m_CultureBomb, 0, sizeof(SBomb));
    m_CultureBomb.pTex = m_pBombTex;
    m_CultureBomb.fSpeed = 0.08f;

    m_CultureBomb.rect.top = 0;
    m_CultureBomb.rect.bottom = 256;
    m_CultureBomb.rect.left = 0;
    m_CultureBomb.rect.right = 128;

    V( D3DXCreateSprite( pd3dDevice, &m_pSprite ) );
}

CGameEntities::~CGameEntities(void)
{
    // Delete Entities!
    for(size_t i=0;i<m_vpMalls.size();i++) {
        SAFE_DELETE(m_vpMalls[i]);
    }
    for(size_t i=0;i<m_vpHouses.size();i++) {
        SAFE_DELETE(m_vpHouses[i]);
    }
    for(size_t i=0;i<m_vpComputerPlayers.size();i++) {
        SAFE_DELETE(m_vpComputerPlayers[i]);
    }
    
    SAFE_DELETE(m_pCursorMall);
    SAFE_DELETE(m_pReferenceHouse);

    // Delete Texs
    SAFE_RELEASE(m_pMallTex);
    SAFE_RELEASE(m_pHouseTex);
    SAFE_RELEASE(m_pPersonTex);
    SAFE_RELEASE(m_pBombTex);

    SAFE_RELEASE(m_pSprite);  

    // Delete Sounds
    for(size_t i=0; i<m_vpSound.size(); i++) {
        SAFE_DELETE(m_vpSound[i]);
    }
    SAFE_DELETE( m_CultureBomb.pSound );
    SAFE_DELETE( m_pSoundManager );
}

VOID CGameEntities::InitSounds(HWND hWnd)
{
    HRESULT hr;
    //! Initialize Sound Manager
    m_pSoundManager = new CSoundManager();
    if( NULL == m_pSoundManager )
    {
        DXTRACE_ERR_MSGBOX( TEXT("Initialize"), E_OUTOFMEMORY );
        EndDialog( hWnd, IDABORT );
        return;
    }
    V( m_pSoundManager->Initialize( hWnd, DSSCL_PRIORITY ) );
    V( m_pSoundManager->SetPrimaryBufferFormat( 1, 22050, 8 ) );    // For 3D Sound, wav must be PCM mono.

    // Create sounds for game entities to use.
    m_vpSound.resize(g_nSOUNDFX, NULL);
    for(size_t i=0; i<g_nSOUNDFX; i++) {
        if(FAILED( m_pSoundManager->Create(&m_vpSound[i], g_wstrSoundFX[i], DSBCAPS_CTRL3D, DS3DALG_NO_VIRTUALIZATION , 2) ) )
        {
            DXTRACE_ERR_MSGBOX( TEXT("InitSounds"), hr );
            MessageBox( hWnd, L"Error creating sound buffer.", 
                                L"DirectSound Sample", MB_OK | MB_ICONERROR );
            EndDialog( hWnd, IDABORT );
            return;
        }
    }

    // Create culture bomb sound
    if(FAILED( m_pSoundManager->Create(&m_CultureBomb.pSound, g_wstrCultureBombSound, DSBCAPS_CTRL3D, DS3DALG_NO_VIRTUALIZATION , 2) ) )
    {
        DXTRACE_ERR_MSGBOX( TEXT("InitSounds"), hr );
        MessageBox( hWnd, L"Error creating sound buffer.", 
                            L"DirectSound Sample", MB_OK | MB_ICONERROR );
        EndDialog( hWnd, IDABORT );
        return;
    }    
}

VOID CGameEntities::InitGame(HWND hWnd, UINT nCompPlayers)
{
    srand((UINT)time(NULL));
    InitSounds(hWnd);    
    
    // Initialize SPerson information with SHouse class so it can spawn people.
    SHouse::SetPersonInfo(m_pPersonTex, m_vpSound[g_PersonSoundIndex]);    

    // Init Cursor Icon Mall for construction
    m_pCursorMall = new SMall(D3DXVECTOR3(0.0f,0.0f,0.0f), m_pMallTex, m_vpSound[g_nConstructionSoundIndex], NULL);

    // House used just to reference length
    m_pReferenceHouse = new SHouse(D3DXVECTOR3(0.0f,0.0f,0.0f), m_pHouseTex, m_vpSound[g_HouseSoundIndex], NULL);

    // Create Mall for Player
    BuildMall(D3DXVECTOR3(0.0f, 0.0f, g_zDist), &m_Player);

    m_Player.mallCost -= g_nInitialMallCost;
    m_Player.money = g_nInitialMallCost / 2;
    m_Player.nMallsPurchased--;
    m_Player.nBombs = max(4 - nCompPlayers, 2);

    for(UINT i=0; i<nCompPlayers; i++) 
    {   // Create Computer Players
        CreateComputerPlayer();
    }
}

VOID CGameEntities::CreateComputerPlayer()
{
    SPlayer* pplayer = new SPlayer();

    SMall* pmall;
    // Create Player
    bool bDone = false;
    while(!bDone) {
        float x = (float)(rand()%(int)(g_bgWidth-50));
        if(rand()%2)
            x = -x;
        float y = (float)(rand()%(int)(g_bgHeight-50));
        if(rand()%2)
            y = -y;

        D3DXVECTOR3 vec(x, y, g_zDist);
        bDone = BuildMall(vec, pplayer, &pmall);        
    }
    pplayer->mallCost -= g_nInitialMallCost;
    pplayer->money = g_nInitialMallCost / 2;
    pplayer->nMallsPurchased--;
    m_vpComputerPlayers.push_back(pplayer);    
}

VOID CGameEntities::ComputerRequestMall(const D3DXVECTOR3& vec, SPlayer* pplayer)
{
    for(size_t i=0; i<m_vpMalls.size(); i++)
    {
        if(IsBoxCollision(m_vpMalls[i]->pos, m_vpMalls[i]->length, vec, m_pCursorMall->length)
            && m_vpMalls[i]->pOwner != pplayer)
        {
            float otherMallCost = pplayer->mallCost + m_vpMalls[i]->nHousesUsing * g_fPerHouseCost;
            if(pplayer->money > otherMallCost)
            {                
                float minlength = MAX_FLOAT;
                for(size_t j=0; j<m_vpMalls.size(); j++)
                {   // find player's closest mall to takeover target
                    if(j == i || m_vpMalls[j]->pOwner != pplayer) continue;

                    D3DXVECTOR3 vec = m_vpMalls[i]->center - m_vpMalls[j]->center;
                    float length = D3DXVec3Length(&vec);

                    if(length < minlength)
                        minlength = length;
                }
                // can only buy if ur mall is within takeover distance
                if(minlength < m_pCursorMall->length * g_fTakeOverRadius)
                {
                    //m_vpMalls[i]->pOwner->money += otherMallCost;
                    pplayer->money -= otherMallCost;
                    pplayer->nMallsPurchased++;

                    pplayer->mallCost += g_nInitialMallCost * pplayer->nMallsPurchased;

                    m_vpMalls[i]->pOwner = pplayer;
                    m_vpMalls[i]->color = pplayer->color;

                    AttachClosestHouses(m_vpMalls[i]);

                    m_vpMalls[i]->pSound->Play();			        
                }
                else 
                {   // not within range to take over. should probably print something out.

                }                
            }
            return ;
        }
    }
    BuildMall(vec, pplayer);    
}

VOID CGameEntities::RequestMallAtMouse()
{
    if(m_Player.otherMallCost > 0.0f && m_Player.money >= m_Player.otherMallCost)
    {   // players cursor is on another persons mall!
        for(size_t i=0; i<m_vpMalls.size(); i++)
        {            
            if(IsBoxCollision(m_vpMalls[i]->pos, m_vpMalls[i]->length, m_pCursorMall->pos, m_pCursorMall->length)
                && m_vpMalls[i]->pOwner != &m_Player)
            {
                float minlength = MAX_FLOAT;
                for(size_t j=0; j<m_vpMalls.size(); j++)
                {   // find player's closest mall to takeover target
                    if(j == i || m_vpMalls[j]->pOwner != &m_Player) continue;

                    D3DXVECTOR3 vec = m_vpMalls[i]->center - m_vpMalls[j]->center;
                    float length = D3DXVec3Length(&vec);

                    if(length < minlength)
                        minlength = length;
                }

                // can only buy if ur mall is within takeover distance
                if(minlength < m_pCursorMall->length * g_fTakeOverRadius)
                {
                    //m_vpMalls[i]->pOwner->money += m_Player.otherMallCost;
                    m_Player.money -= m_Player.otherMallCost;
                    m_Player.nMallsPurchased++;

                    m_Player.mallCost += g_nInitialMallCost * m_Player.nMallsPurchased;
                    m_Player.otherMallCost = 0.0f;

                    m_vpMalls[i]->pOwner = &m_Player;
                    m_vpMalls[i]->color = m_Player.color;

                    AttachClosestHouses(m_vpMalls[i]);
                    m_vpMalls[i]->bFull = false;

                    m_vpMalls[i]->pSound->Play();				    
                }
                else
                {   // not in take over distance. should probably say something

                }
                break;
            }
        }
    }        
    else if(m_Player.money >= m_Player.mallCost)
    {
        float halflength = m_pCursorMall->length / 2.0f;
        D3DXVECTOR3 vec(m_pCursorMall->pos.x - halflength, m_pCursorMall->pos.y + halflength, m_pCursorMall->pos.z);

        BuildMall(vec, &m_Player);
    }
}

bool CGameEntities::BuildMall(const D3DXVECTOR3& vec, SPlayer* pplayer, SMall** ppmall)
{
    if(IsWithinBGEllipse(vec, m_pCursorMall->length) && !IsCollision(vec, m_pCursorMall->length) ) {

        SMall* pmall = new SMall(vec, m_pCursorMall->pTex, m_vpSound[g_MallSoundIndex], pplayer);
        m_vpMalls.push_back(pmall);
        AttachClosestHouses(pmall);

        pplayer->money -= pplayer->mallCost;
        pplayer->nMallsPurchased++;

        if(pplayer == &m_Player)
            pplayer->mallCost += g_nInitialMallCost * pplayer->nMallsPurchased * 0.65f; // cheat to make it easier for player
        else
            pplayer->mallCost += g_nInitialMallCost * pplayer->nMallsPurchased; // cheat to make it easier for player

        m_pCursorMall->pSound->Play();

        // pmall->length is always same!
        //static float spawnRadius = pmall->length*2;
        static float spawnRadius = m_pReferenceHouse->length*2;

        if(ppmall)
            (*ppmall) = pmall;
        SpawnHouse(pmall, spawnRadius, m_pReferenceHouse->length);
        SpawnHouse(pmall, spawnRadius, m_pReferenceHouse->length);
        return true;
    }
    if(ppmall)
        (*ppmall) = NULL;
    return false;
}

VOID CGameEntities::DropCultureBombAtMouse()
{
    if(m_Player.nBombs <= 0)
        return; // No Bombs    
    float halflength = m_pCursorMall->length / 2.0f;
    D3DXVECTOR3 vec(m_pCursorMall->pos.x - halflength, m_pCursorMall->pos.y + halflength, m_pCursorMall->pos.z);
    if( IsWithinBGEllipse(vec, m_pCursorMall->length) ) // within map
    {   // drop culture bomb and take over malls within bomb radius
        m_Player.nBombs--;
        // update world matrix
        D3DXMatrixIdentity(&m_CultureBomb.mWorld);
        D3DXMatrixTranslation(&m_CultureBomb.mWorld, vec.x, vec.y, vec.z);
        D3DXMATRIX scale;
        D3DXMatrixScaling(&scale, .1f, -.075f, 1.f);  
        m_CultureBomb.mWorld = scale * m_CultureBomb.mWorld;
        //scale?

        for(size_t i=0; i<m_vpMalls.size(); i++)
        {   // convert all enemy malls within radius to player            
            D3DXVECTOR3 dif = vec - m_vpMalls[i]->center;
            dif.z = 0.f;
            float fDist = D3DXVec3LengthSq(&dif);

            if(fDist < g_fBombRadius)
            {
                m_Player.mallCost += g_nInitialMallCost * m_Player.nMallsPurchased;
                m_Player.otherMallCost = 0.0f;

                m_vpMalls[i]->pOwner = &m_Player;
                m_vpMalls[i]->color = m_Player.color;

                AttachClosestHouses(m_vpMalls[i]);
                m_vpMalls[i]->bFull = false;

                m_CultureBomb.pSound->Play();
                m_CultureBomb.bActive = true;
            }
        }
    }
}

VOID CGameEntities::SetDrawMouseLocation(const D3DXVECTOR3& mouseLoc) 
{
    m_pCursorMall->pos = mouseLoc;
    m_pCursorMall->UpdateWorldMatrix(); 

	static float offset = m_pCursorMall->length / 2.0f;

	// Move from middle of mall to topleft
	D3DXVECTOR3 vec;
	vec.x = m_pCursorMall->pos.x - offset;
	vec.y = m_pCursorMall->pos.y + offset;
	vec.z = m_pCursorMall->pos.z;

    // Check for collision with any opponent malls and update cost accordingly
    for(size_t i=0; i<m_vpMalls.size(); i++)
    {
        if(IsBoxCollision(m_vpMalls[i]->pos, m_vpMalls[i]->length, vec, m_pCursorMall->length)
            && m_vpMalls[i]->pOwner != &m_Player)
        {
            m_Player.otherMallCost = m_Player.mallCost + m_vpMalls[i]->nHousesUsing * g_fPerHouseCost;
            return ;
        }
    }
    m_Player.otherMallCost = 0.0f;
}

VOID CGameEntities::RestartEntities(int nOpponents)
{
    for(size_t i=0;i<m_vpMalls.size();i++) {
        SAFE_DELETE(m_vpMalls[i]);
    }
    m_vpMalls.clear();

    for(size_t i=0;i<m_vpHouses.size();i++) {
        SAFE_DELETE(m_vpHouses[i]);
    }
    m_vpHouses.clear();

    for(size_t i=0;i<m_vpComputerPlayers.size();i++) {
        SAFE_DELETE(m_vpComputerPlayers[i]);
    }
    m_vpComputerPlayers.clear();

    // Create Mall for Player
    m_Player.mallCost = g_nInitialMallCost;
    m_Player.nMallsPurchased = 0;
    BuildMall(D3DXVECTOR3(0.0f, 0.0f, g_zDist), &m_Player);
    m_Player.nMallsPurchased--;    
    m_Player.mallCost -= g_nInitialMallCost;
    m_Player.otherMallCost = 0.0f;
    m_Player.money = g_nInitialMallCost / 2;    

    for(int i=0; i<nOpponents; i++) 
    {   // Create Computer Players
        CreateComputerPlayer();
    }        
}

bool CGameEntities::OnFrameMove(float elapsedTime)
{
    srand((UINT)time(NULL));
    bool bLevelDone = true;
    for(size_t i=0;i<m_vpMalls.size();i++) {
        m_vpMalls[i]->OnFrameMove(elapsedTime);
        if(m_vpMalls[i]->pOwner != &m_Player)
            bLevelDone = false;
    }

    for(size_t i=0;i<m_vpHouses.size();i++) {
        BOOL bMovingOut = m_vpHouses[i]->OnFrameMove(elapsedTime);
        if(bMovingOut) 
        {   // Make new house close to the mall 
            static float spawnRadius = m_vpHouses[i]->pClosestMall->length*g_nMaxMallRadius;            

            SpawnHouse(m_vpHouses[i]->pClosestMall, spawnRadius, 
                                                    m_vpHouses[i]->length);
        }
    }


    // Compute player AIs
    for(size_t i=0;i<m_vpComputerPlayers.size();i++)
    {
        SPlayer* pplayer = m_vpComputerPlayers[i];
        if(pplayer->money >= pplayer->mallCost)
        {   // go thru random opponents malls
            bool bDone = false;
            int j = rand()%(int)(m_vpMalls.size());
            while(!bDone)
            {   // only consider attack if dont own already
                if(m_vpMalls[j]->pOwner != pplayer)
                {
                    bDone = true; // Build attempt is done. might not succeed tho.
                    static float spawnradius = m_vpMalls[j]->length * g_nMaxMallRadius * 2;
                    D3DXVECTOR3 newpos = m_vpMalls[j]->pos;
                    if(rand()%2)    newpos.x += ( (rand()%100 + 1) / 100.0f ) * spawnradius;
                    else            newpos.x -= ( (rand()%100 + 1) / 100.0f ) * spawnradius;

                    if(rand()%2)    newpos.y += ( (rand()%100 + 1) / 100.0f ) * spawnradius;
                    else            newpos.y -= ( (rand()%100 + 1) / 100.0f ) * spawnradius;

                    ComputerRequestMall(newpos, pplayer);
                }
                else {                    
                    while(true) {
                        int t = rand()%(int)(m_vpMalls.size());
                        if(t!=j) {
                            j = t;
                            break;                            
                        }
                    }
                }
            }
        }
    } 
    if(m_CultureBomb.bActive)
    {   // animate sprite by moving rect
        m_CultureBomb.fTime += elapsedTime;
        if(m_CultureBomb.fTime > m_CultureBomb.fSpeed)
        {
            m_CultureBomb.rect.left += 102;
            m_CultureBomb.rect.right += 102;

            if(m_CultureBomb.rect.left > 102*4)
            {
                m_CultureBomb.rect.left = 0;
                m_CultureBomb.rect.right = 102;
                m_CultureBomb.bActive = false;
            }
            m_CultureBomb.fTime = 0.f;
        }
    }

    return bLevelDone;
}

HRESULT CGameEntities::OnFrameRender(IDirect3DDevice9 *pd3dDevice)
{
    HRESULT hr;

    static DWORD flags = D3DXSPRITE_SORT_TEXTURE |
                         D3DXSPRITE_ALPHABLEND |
                         D3DXSPRITE_OBJECTSPACE;                        
    //D3DXSPRITE_SORT_DEPTH_FRONTTOBACK |

    V( m_pSprite->Begin(flags) );    
    for(size_t i=0;i<m_vpMalls.size();i++) {
        m_vpMalls[i]->RenderThruSprite(m_pSprite);
    }
    for(size_t i=0;i<m_vpHouses.size();i++) {
        m_vpHouses[i]->RenderThruSprite(m_pSprite);
    }
    V( m_pSprite->End() );    

    if(m_Player.money >= m_Player.mallCost) 
    {   // draw construction mall under mouse icon    
        V( m_pSprite->Begin(flags) );   // reason its outside loop is because i want to draw it on top of others.
        static D3DXVECTOR3 center(128/2.0f, 128/2.0f, 0.0f);
        V( m_pSprite->SetTransform(&m_pCursorMall->mWorld) );        
        V( m_pSprite->Draw(m_pCursorMall->pTex, NULL, &center, NULL, 0x66FFFFFF) );                
        V( m_pSprite->End() );
    }

    // Draw culture bomb animtion if any
    if(m_CultureBomb.bActive)
    {
        static D3DXVECTOR3 center(128/2.0f, 256/2.0f, 0.0f);
        V( m_pSprite->Begin(flags) );
        V( m_pSprite->SetTransform( &m_CultureBomb.mWorld ) );
        V( pd3dDevice->SetSamplerState(0, D3DSAMP_MAXMIPLEVEL, 0) );
        V( m_pSprite->Draw(m_CultureBomb.pTex, &m_CultureBomb.rect, &center, NULL, m_Player.color) );
        V( m_pSprite->End() );
    }

    return S_OK;
}

VOID CGameEntities::SpawnHouse(SMall* pmall, float spawnRadius, float objectlength)
{   
    if(pmall->bFull)
        return ;
    int nTries = 30;    
    for(int i=0; i<nTries; i++)
    {
        D3DXVECTOR3 newpos = pmall->pos;
        if(rand()%2)    newpos.x += ( (rand()%100 + 1) / 100.0f ) * spawnRadius ;
        else            newpos.x -= ( (rand()%100 + 1) / 100.0f ) * spawnRadius ;

        if(rand()%2)    newpos.y += ( (rand()%100 + 1) / 100.0f ) * spawnRadius ;
        else            newpos.y -= ( (rand()%100 + 1) / 100.0f ) * spawnRadius ;

        BOOL bCollide = IsCollision(newpos, objectlength);
        if(!bCollide && IsWithinBGEllipse(newpos, objectlength)) 
        {   // Have to find closest mall! Might not be the same as parent household one.
            // pmall = GetClosestMall(newpos);

            SHouse* phome = new SHouse(newpos, m_pHouseTex, m_vpSound[g_HouseSoundIndex], pmall);
            m_vpHouses.push_back(phome);
            return ;
        }
        else if(bCollide)
        {   // Change mall association of the house it collided with if house.   
            /*
            for(size_t i=0; i<m_vpHouses.size(); i++)
            {
                if(IsBoxCollision(newpos, m_vpHouses[i]->length, m_vpHouses[i]->pos, m_vpHouses[i]->length))
                {                    
                    m_vpHouses[i]->AttachMall(pmall);
                    return ;                    
                }
            }
            */            
        }
    }
    pmall->bFull = true;
}

//! Checks houses if it's closest to this mall.
VOID CGameEntities::AttachClosestHouses(SMall* pmall)
{    
    for(size_t i=0;i<m_vpHouses.size();i++)
    {
        SHouse* phouse = m_vpHouses[i];
        D3DXVECTOR3 dif1 = phouse->pClosestMall->center - phouse->center;
        D3DXVECTOR3 dif2 = pmall->center - phouse->center;

        float oldlength = D3DXVec3LengthSq(&dif1);
        float newlength = D3DXVec3LengthSq(&dif2);

        if(newlength < oldlength)
        {   // attach new mall to this house
            phouse->AttachMall(pmall);
        }
    }
}

//! Returns the mall closest to this point.
CGameEntities::SMall* CGameEntities::GetClosestMall(const D3DXVECTOR3& pos)
{
    float minlength = MAX_FLOAT;
    size_t bestindex = 0;
    for(size_t i=0;i<m_vpMalls.size();i++)
    {
        D3DXVECTOR3 dif = m_vpMalls[i]->pos - pos;
        float length = D3DXVec3LengthSq(&dif);
        if(length < minlength) {
            minlength = length;
            bestindex = i;            
        }
    }
    return m_vpMalls[bestindex];
}

BOOL CGameEntities::IsCollision(const D3DXVECTOR3& newpos, float length)
{
    for(size_t i=0; i<m_vpMalls.size(); i++) {
        if(IsBoxCollision(newpos, length, m_vpMalls[i]->pos, m_vpMalls[i]->length))
            return true;
    }

    for(size_t i=0; i<m_vpHouses.size(); i++) {
        if(IsBoxCollision(newpos, length, m_vpHouses[i]->pos, m_vpHouses[i]->length))
            return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------
// IEntity
//--------------------------------------------------------------------------------------

CGameEntities::IEntity::IEntity(const D3DXVECTOR3& _pos, LPDIRECT3DTEXTURE9 _pTex, CSound* _pSound) :
    pos(_pos), pTex(_pTex), pSound(_pSound), 
    timeElapsed(0.0f), nFrame(0), fScale(g_PersonScale), speed(0.0f), color(0xFFFFFFFF)
{
    HRESULT hr;
    // get length
    D3DSURFACE_DESC desc;
    V( pTex->GetLevelDesc(0, &desc) );
    length = desc.Height * fScale;

    ZeroMemory(&srcRect, sizeof(srcRect));   

    UpdateWorldMatrix(); // updates world matrix with position and scale.
}

CGameEntities::IEntity::~IEntity()
{
}

VOID CGameEntities::IEntity::UpdateWorldMatrix(bool bFlipHorizontal)
{
    D3DXMATRIX mtxScale, mtxTranslate;

    if(bFlipHorizontal)
        D3DXMatrixScaling(&mtxScale, -fScale, -fScale, 1);
    else
        D3DXMatrixScaling(&mtxScale, fScale, -fScale, 1);
    
    D3DXMatrixTranslation(&mtxTranslate, pos.x, pos.y, pos.z);
    mWorld = mtxScale * mtxTranslate;
}

//--------------------------------------------------------------------------------------
// SPerson
//--------------------------------------------------------------------------------------

CGameEntities::SPerson::SPerson(LPDIRECT3DTEXTURE9 _pTex, CSound* _pSound, SHouse* _home) :
    IEntity(_home->pos, _pTex, _pSound), curDir(0.0f, 0.0f, 0.0f), fMoney(g_fMoneyPerHousehold), pHome(_home)
{
    // add some random speed element to each shopper
    pos.y += 0.01f; // dont start in house area otherwise it triggers a respawn.
    speed = g_PersonSpeed * (rand() % 9 + 16) / 20;

    // set direction
    pDestMall = pHome->pClosestMall;
    curDir = pDestMall->vEntrances[pHome->nClosestEntrance] - pos;
    D3DXVec3Normalize(&curDir, &curDir);
}

VOID CGameEntities::SPerson::OnFrameMove(float elapsedTime)
{   // walks person back and forth from home to mall
    pos.x += speed * elapsedTime * curDir.x;
    pos.y += speed * elapsedTime * curDir.y;
    
    if(fMoney > 0.0f && IsWithinDistance(pos, pDestMall->pos, pDestMall->length))
    {   // take the shoppers money and send home        
        // pDestMall->pOwner->money += fMoney;
        // NOTE: money is now gained thru ticks * elapsedTime elsewhere.
        fMoney = 0.0f;
        
        curDir = pHome->center - pos;
        D3DXVec3Normalize(&curDir, &curDir);

        //pDestMall->pSound->Play();
    }
    else if(fMoney < EPSILON && IsWithinDistance(pos, pHome->pos, pHome->length))
    {   // reimburse with money and send back to mall!
        fMoney = g_fMoneyPerHousehold;

        pDestMall = pHome->pClosestMall;        
        curDir = pDestMall->vEntrances[pHome->nClosestEntrance] - pos;
        D3DXVec3Normalize(&curDir, &curDir);

        pHome->nVisits++;
        //pHome->pSound->Play();

        if(pHome->vpPeople.size() < g_nMaxHousePop)
            pHome->CreatePerson();
    }    

    // Do walking animation
    timeElapsed += elapsedTime;
    if(timeElapsed > g_fAnimSpeed) {        
        nFrame = (nFrame == 2) ? 1:2;

        srcRect.top = 0;
        srcRect.bottom = g_nPersonTexHeight;

        srcRect.left = nFrame * 170;
        srcRect.right = (nFrame + 1) * 170;
        timeElapsed = 0;
    }
    UpdateWorldMatrix();
}

VOID CGameEntities::SPerson::RenderThruSprite(LPD3DXSPRITE pSprite)
{
    HRESULT hr;
    V( pSprite->SetTransform(&mWorld) );
    V( pSprite->Draw(pTex, &srcRect, NULL, NULL, color) );
}

//--------------------------------------------------------------------------------------
// SHouse
//--------------------------------------------------------------------------------------

CGameEntities::SHouse::SHouse(const D3DXVECTOR3& _pos, LPDIRECT3DTEXTURE9 _pTex, CSound* _pSound, SMall* _pCloseMall) :
    IEntity(_pos, _pTex, _pSound), pClosestMall(NULL), bSpawnedHouse(false), nVisits(0)
{
    fScale = g_HouseScale;
    UpdateWorldMatrix(); // updates world matrix with position and scale.

    HRESULT hr;
    // get length
    D3DSURFACE_DESC desc;
    V( pTex->GetLevelDesc(0, &desc) );
    length = desc.Height * fScale;    

    float halflength = length/2.0f;
    center.x = pos.x + halflength;
    center.y = pos.y - halflength;
    center.z = pos.z;

    if(_pCloseMall) {
        AttachMall(_pCloseMall);
        CreatePerson();
    }
}

CGameEntities::SHouse::~SHouse()
{
    for(size_t i=0;i<vpPeople.size();i++) {
        SAFE_DELETE(vpPeople[i]);
    }
}

VOID CGameEntities::SHouse::AttachMall(SMall* _pCloseMall)
{
    if(pClosestMall)    pClosestMall->nHousesUsing--;

    pClosestMall = _pCloseMall;
    color = pClosestMall->color;
    pClosestMall->nHousesUsing++;

    // Find closest entrance index
    float length = MAX_FLOAT;    
    for(int i=0; i<4; i++)
    {
        D3DXVECTOR3 v = pClosestMall->vEntrances[i] - pos;
        float newlength = D3DXVec3LengthSq(&v);
        if(newlength < length) {
            nClosestEntrance = i;
            length = newlength;
        }
    }
}

VOID CGameEntities::SHouse::CreatePerson()
{
    vpPeople.push_back(new CGameEntities::SPerson(pPersonTex, pPersonSound, this));
}

BOOL CGameEntities::SHouse::OnFrameMove(float elapsedTime)
{
    color = pClosestMall->color;
    for(size_t i = 0;i<vpPeople.size(); i++)
        vpPeople[i]->OnFrameMove(elapsedTime);

    if(nVisits > g_nMaxHousePop) {
        nVisits = -g_nVisitsTill2ndHouse;
        return true;
    }
    return false;
}

VOID CGameEntities::SHouse::RenderThruSprite(LPD3DXSPRITE pSprite)
{
    HRESULT hr;
    V( pSprite->SetTransform(&mWorld) );        
    V( pSprite->Draw(pTex, NULL, NULL, NULL, color) );

    for(size_t i = 0;i<vpPeople.size(); i++)
        vpPeople[i]->RenderThruSprite(pSprite);
}

VOID CGameEntities::SHouse::SetPersonInfo(LPDIRECT3DTEXTURE9 _pTex, CSound* _pSound)
{
    pPersonTex = _pTex;
    pPersonSound = _pSound;
}

LPDIRECT3DTEXTURE9 CGameEntities::SHouse::pPersonTex = NULL;
CSound* CGameEntities::SHouse::pPersonSound = NULL;

//--------------------------------------------------------------------------------------
// SMall
//--------------------------------------------------------------------------------------

CGameEntities::SMall::SMall(const D3DXVECTOR3& _pos, LPDIRECT3DTEXTURE9 _pTex, CSound* _pSound, SPlayer* _player) :
    IEntity(_pos, _pTex, _pSound), nHousesUsing(0), pOwner(_player), bFull(false)
{
    fScale = g_MallScale;
    if(pOwner)    color = pOwner->color;
    UpdateWorldMatrix(); // updates world matrix with position and scale.

    HRESULT hr;
    // get length
    D3DSURFACE_DESC desc;
    V( pTex->GetLevelDesc(0, &desc) );
    length = desc.Height * fScale;

    center.x = pos.x + length / 2.0f;
    center.y = pos.y - length / 2.0f;
    center.z = pos.z;    

    // Generate the locations of the 4 entrances.
    vEntrances[0] = pos;    // North entrance
    vEntrances[0].x += length/2.0f;

    vEntrances[1] = pos;    // East entrance
    vEntrances[1].x += length;
    vEntrances[1].y -= length/2.0f;

    vEntrances[2] = pos;    // South entrance
    vEntrances[2].x += length/2.0f;
    vEntrances[2].y -= length;

    vEntrances[3] = pos;    // West entrance
    vEntrances[3].y -= length/2.0f;
}

VOID CGameEntities::SMall::OnFrameMove(float elapsedTime)
{   // Give money to owner
    pOwner->money += nHousesUsing * g_fMoneyPerHousehold * 0.6f; // give player advantage
}

VOID CGameEntities::SMall::RenderThruSprite(LPD3DXSPRITE pSprite)
{
    HRESULT hr;
    V( pSprite->SetTransform(&mWorld) );        
    V( pSprite->Draw(pTex, NULL, NULL, NULL, color) );
}

//--------------------------------------------------------------------------------------
// SPlayer!
//--------------------------------------------------------------------------------------

UINT CGameEntities::SPlayer::UniqueColorIndex   =   0;

VOID CGameEntities::SPlayer::ComputerAI()
{
}

//--------------------------------------------------------------------------------------
// Global Misc.
//--------------------------------------------------------------------------------------

BOOL IsBoxCollision(const D3DXVECTOR3& pos, float plength, const D3DXVECTOR3& dest, float dlength)
{
    if( (pos.x + plength > dest.x) && (pos.x < dest.x + dlength) && (pos.y - plength < dest.y) && (pos.y > dest.y - dlength) )
        return true;
    return false;
}

BOOL IsWithinDistance(const D3DXVECTOR3& pos, const D3DXVECTOR3& dest, float length)
{
    return (pos.x >= dest.x && pos.y <= dest.y && pos.x <= dest.x + length && pos.y >= dest.y - length);    
}

BOOL IsWithinBGEllipse(const D3DXVECTOR3& _pos, float length)
{
    D3DXVECTOR3 pos(_pos.x + length/2, _pos.y - length/2, _pos.z);
    static float InverseA = 1/(( (g_bgWidth - 5)*(g_bgWidth - 5) ) / 4);
    static float InverseB = 1/(( (g_bgHeight - 5)*(g_bgHeight - 5) ) / 4);

    float x2 = pos.x*pos.x;
    float y2 = pos.y*pos.y;

    float term1 = x2 * InverseA;
    float term2 = y2 * InverseB;

    return (term1 + term2 - 1 < 0);
}

