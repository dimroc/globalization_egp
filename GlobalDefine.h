#pragma once

//////////////////////////// GAME STUFF /////////////////////////////

static const float g_fMoneyPerHousehold =   0.01f; // ! cash money
static const UINT  g_nCompPlayers       =   1;

static const int   g_nMaxHousePop       =   1;
static const float g_nMaxMallRadius     =   2.5f;
static const float g_fTakeOverRadius    =   5.0f;   // these 2 are actually multipliers to mall->length.
static const float g_fBombRadius        =   250.f;

static const int   g_nVisitsTill2ndHouse=   0;

static const float g_nInitialMallCost   =   15.0f;
static const float g_fPerHouseCost      =   30.0f;

static const float g_PersonScale        =   0.002f;
static const float g_HouseScale         =   0.01f;
static const float g_MallScale          =   0.03f;

//////////////////////////// TEXTURE STUFF /////////////////////////////

static LPCWSTR g_wstrBackgroundTex      =   L"Media/bg_white_glob.bmp";
static LPCWSTR g_wstrSplashTex          =   L"Media/glob_splash.png";
static LPCWSTR g_wstrLogoTex            =   L"Media/glob_logo.png";
static LPCWSTR g_wstrVisionTex          =   L"Media/impaired_vision.bmp";

static LPCWSTR g_wstrMallTex            =   L"Media/mall.png";
static LPCWSTR g_wstrHouseTex           =   L"Media/house.png";
static LPCWSTR g_wstrBombTex            =   L"Media/bomb.bmp";
static LPCWSTR g_wstrPersonTex          =   L"Media/stick_figure.bmp";
static LPCWSTR g_wstrNumberTex          =   L"Media/numbers.png";
static LPCWSTR g_wstrCashTex            =   L"Media/cash.png";
static LPCWSTR g_wstrCostTex            =   L"Media/cost.png";
static LPCWSTR g_wstrLevelTex           =   L"Media/level.png";

static const int g_nPersonTexHeight     =   256;

//////////////////////////// BACKGROUND & HUD STUFF /////////////////////////////

static const float g_bgWidth            =   120.0f; // 105
static const float g_bgHeight           =   80.0f; // 70
static const float g_bgZFar             =   10.0f;
static const float g_bgTexTile          =   1.0f;

static const float g_hudCashX           =   750.0f;

//////////////////////////// MOVEMENT CONSTANTS //////////////////////////

static const float g_PersonSpeed        =   2.0f;

static const float g_MovementThreshX    =   70.0f; // no movement window
static const float g_MovementThreshY    =   70.0f;

static const float g_MovementSpeed      =   30.0f;
static const float g_ZoomSpeed          =   40.0f; //! Camera Zoom speed

static const float g_fAnimSpeed         =   0.3f; // sprite animation speed threshold

static const float g_zDist              =   g_bgZFar - 0.1f;

static const float g_fMaxZ              =   -5.0f;
static const float g_fMinZ              =   -g_bgWidth;

static const float g_fMaxX              =   50.0f;
static const float g_fMaxY              =   50.0f;

/////////////////////////// THEME SONG ////////////////////////////////////

static LPCWSTR g_cstrThemeSong          =   L"Media/themesong.mp3";

/////////////////////////// SOUND FX ////////////////////////////////////

static UINT g_nSOUNDFX                  =   4;


static UINT g_PersonSoundIndex          =   0;
static UINT g_MallSoundIndex            =   1;
static UINT g_HouseSoundIndex           =   2;
static UINT g_nConstructionSoundIndex   =   3;


static LPWSTR g_wstrSoundFX[4]         =   { 
                                                L"Media/star.wav",
                                                L"Media/richbiatch.wav",
                                                L"Media/cdplayerson.wav",
                                                L"Media/mallconstruction.wav",
                                            };
static LPWSTR g_wstrCultureBombSound     =      L"Media/richbiatch.wav";


struct BoxVertex {
    static const UINT FVF = D3DFVF_XYZ | D3DFVF_TEX1;

    D3DXVECTOR3 pos;    
    float u,v;

    BoxVertex() {}
    BoxVertex(D3DXVECTOR3 vec, float _u, float _v) :
        pos(vec), u(_u), v(_v)
    {} 
};

/////////////////////////// Misc  ////////////////////////////////////

static const DWORD g_PlayerColors[10]       =       {
                                                        0xFFFFFFFF,
                                                        0xFFB77400,
                                                        0xFFB00000,
                                                        0xFF0000B0,
                                                        0xFFB09D00,
                                                        0xFF00B000,
                                                        0xFFFDA8A8,                                                                                                     
                                                        0xFFAA00AA,
                                                        0xFF909090,
                                                        0xFF0000B0                                                        
                                                    };

static const float EPSILON              =   0.0000001f;

static const int     MIN_INT    =                             (int)(-2147483647 - 1);
static const int     MAX_INT    =                             2147483647;

// static const UINT    MIN_UINT   =                             0; // duh
static const UINT    MAX_UINT   =                             4294967295;

static const float   MIN_FLOAT  =                             1.175494351e-38F;
static const float   MAX_FLOAT  =                             3.402823466e+38F;

