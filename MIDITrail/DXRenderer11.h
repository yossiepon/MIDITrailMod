//******************************************************************************
//
// MIDITrail / DXRenderer11
//
// Direct3D 11 renderer (migration target; replaces the D3D9 DXRenderer)
//
//******************************************************************************

// MEMO:
// DX11 device + swap chain + render target/depth + ImGui(dx11).
// M1: clears the screen and draws the ImGui UI. Scene geometry is ported in
// later milestones (M2+).

#pragma once

#include <d3d11.h>
#include <dxgi.h>
#include "DXScene.h"
#include "DXPrimitive11.h"
#include "MTLogo11.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"


class MTKeyboard11;
class MTFirstPersonCam;
class DXNoteBox11;
class MTNoteRipple11;
class MTNoteLyrics11;
class MTNoteBoxLive11;
class MTNoteRainLive11;
class MTGridBox11;
class MTDashboard11;
class MTConfigManager11;
class MTTimeIndicator11;
class MTPictBoard11;
class DXNoteRain11;
class MTKeyboardRain11;
class DXNoteBoxRing11;
class MTGridRing11;
class MTTimeIndicatorRing11;
class MTPictBoardRing11;
class MTBackgroundImage11;
class MTStars11;

//******************************************************************************
// Parameters
//******************************************************************************
#define DXRENDERER11_ERR_DEVICE_LOST  (100)


//******************************************************************************
// Direct3D 11 renderer class
//******************************************************************************
class DXRenderer11
{
public:

	DXRenderer11();
	virtual ~DXRenderer11();

	//Initialize / terminate
	int Initialize(HWND hWnd, unsigned long multiSampleType = 0, bool isFullScreen = false,
			unsigned long superSample = 1);
	void Terminate();

	//ImGui context
	ImGuiContext* CreateImContext();

	//Device / context accessors
	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetContext();

	//MSAA support query (for the graphics config dialog)
	bool IsMultiSampleSupported(unsigned int sampleCount);

	//Render one frame (M1: clear + ImGui UI; scene draw added in M2+)
	int RenderScene(DXScene* pScene);

	//M2: a DX11 keyboard to draw (NULL = none)
	void SetKeyboard11(MTKeyboard11* pKbd) { m_pKbd11 = pKbd; }

	//M2.5: the real first-person camera (NULL = use the keyboard's static hint)
	void SetCamera11(MTFirstPersonCam* pCam) { m_pCam11 = pCam; }

	//M3: the instanced note field to draw (NULL = none)
	void SetNoteBox11(DXNoteBox11* pNoteBox) { m_pNoteBox11 = pNoteBox; }

	//M4.7: the instanced falling-note field (Rain scene) to draw (NULL = none)
	void SetNoteRain11(DXNoteRain11* pNoteRain) { m_pNoteRain11 = pNoteRain; }

	//M4.9: the instanced Ring-scene note field to draw (NULL = none)
	void SetNoteBoxRing11(DXNoteBoxRing11* pRing) { m_pNoteBoxRing11 = pRing; }

	//M4.15: background image (drawn behind everything; NULL = none)
	void SetBackgroundImage11(MTBackgroundImage11* p) { m_pBackgroundImage11 = p; }

	//M4.16: starfield (drawn behind the scene, follows the camera; NULL = none)
	void SetStars11(MTStars11* p) { m_pStars11 = p; }

	//M5: background clear color ([Color] BackGroundRGB). D3DCOLOR 0xAARRGGBB.
	void SetBackgroundColor(D3DCOLOR c) { m_BgColor = c; }

	//M6 (video export): render the scene off-screen at an arbitrary resolution
	//with the CURRENT (fixed) camera viewpoint - no input polling, no ImGui, no
	//Present. The caller drives the playback tick on the components per frame.
	//  BeginOffscreen   - (re)create the off-screen RGBA target + depth + 2 staging
	//  RenderOffscreenFrame - draw one frame and queue a GPU copy to staging[stageSlot]
	//                         (transparent = clear alpha 0, skip bg image). The copy is
	//                         GPU-async; read it back a frame later so the GPU can render
	//                         the next frame while the CPU maps this one (no stall).
	//  ReadOffscreenBGRA - map staging[stageSlot] into a CPU buffer (BGRA, top-down)
	//  EndOffscreen     - release the off-screen resources, restore the backbuffer viewport
	int BeginOffscreen(int width, int height, bool equirect360 = false);
	int RenderOffscreenFrame(bool transparent, int stageSlot);
	int ReadOffscreenBGRA(int stageSlot, unsigned char* pDst, int dstRowBytes);
	void EndOffscreen();
	int GetOffscreenWidth()  { return (int)m_OffW; }
	int GetOffscreenHeight() { return (int)m_OffH; }

	//M4.13: Ring-scene decorations (NULL = none)
	void SetGridRing11(MTGridRing11* p) { m_pGridRing11 = p; }
	void SetTimeIndicatorRing11(MTTimeIndicatorRing11* p) { m_pTimeIndicatorRing11 = p; }
	void SetPictBoardRing11(MTPictBoardRing11* p) { m_pPictBoardRing11 = p; }

	//M4.7b: the Rain-scene keyboard to draw (NULL = none)
	void SetKeyboardRain11(MTKeyboardRain11* pKbd) { m_pKbdRain11 = pKbd; }

	//M3: the note ripple effect to draw (NULL = none)
	void SetNoteRipple11(MTNoteRipple11* pRipple) { m_pNoteRipple11 = pRipple; }
	void SetNoteLyrics11(MTNoteLyrics11* pLyrics) { m_pNoteLyrics11 = pLyrics; }
	void SetNoteBoxLive11(MTNoteBoxLive11* pLive) { m_pNoteBoxLive11 = pLive; }
	void SetNoteRainLive11(MTNoteRainLive11* pLive) { m_pNoteRainLive11 = pLive; }

	//M3: the grid box to draw (NULL = none)
	void SetGridBox11(MTGridBox11* pGrid) { m_pGridBox11 = pGrid; }

	//M4: the dashboard (on-screen info text) to draw (NULL = none)
	void SetDashboard11(MTDashboard11* pDash) { m_pDashboard11 = pDash; }
	void SetConfigManager11(MTConfigManager11* pCfg) { m_pConfigMgr11 = pCfg; }

	//M4.4: the time indicator (playback section) to draw (NULL = none)
	void SetTimeIndicator11(MTTimeIndicator11* pTI) { m_pTimeIndicator11 = pTI; }

	//M4.5: the picture board to draw (NULL = none)
	void SetPictBoard11(MTPictBoard11* pPB) { m_pPictBoard11 = pPB; }

	//Loading screen (called periodically during a long load)
	int DrawLoadingScreen(const char* message, float progress);

	//Handle a window resize (recreate backbuffer-sized targets)
	int Resize(unsigned int width, unsigned int height);

private:

	HWND m_hWnd;
	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pContext;
	IDXGISwapChain* m_pSwapChain;
	ID3D11RenderTargetView* m_pRTV;
	ID3D11Texture2D* m_pDepthTex;
	ID3D11DepthStencilView* m_pDSV;
	ImGuiContext* m_ImContext;
	unsigned int m_Width;
	unsigned int m_Height;
	unsigned int m_SampleCount;   // MSAA sample count (1 = off)

	// ced 20260628: SSAA (supersampling). When m_SuperSample > 1 the 3D scene is
	// rendered to an offscreen target at (m_Width*ss x m_Height*ss) then downscaled
	// (linear) to the backbuffer. Works on any GPU and stacks on top of MSAA.
	unsigned int m_SuperSample;            // 1 = off, 2/3/4 = NxN supersampling
	ID3D11Texture2D*          m_pSSColor;  // SS-size scene color (single sample, has SRV)
	ID3D11RenderTargetView*   m_pSSRTV;
	ID3D11ShaderResourceView* m_pSSSRV;
	ID3D11Texture2D*          m_pSSDepth;  // SS-size depth
	ID3D11DepthStencilView*   m_pSSDSV;
	// downscale blit (fullscreen triangle that samples m_pSSSRV with a linear sampler)
	ID3D11VertexShader*       m_pBlitVS;
	ID3D11PixelShader*        m_pBlitPS;
	ID3D11Buffer*             m_pBlitCB;   // { float2 texelSize; float factor; float pad; }
	ID3D11SamplerState*       m_pBlitSampler;
	ID3D11DepthStencilState*  m_pBlitNoDepth;
	ID3D11RasterizerState*    m_pBlitRaster;
	int  _InitBlit();
	void _ReleaseBlit();
	int  _CreateSSTargets(unsigned int width, unsigned int height);
	void _ReleaseSSTargets();
	void _BlitSSToBackbuffer();

	// TEMP (M2 verification): a test quad drawn via DXPrimitive11
	DXPrimitive11 m_TestQuad;
	bool m_TestQuadReady;
	int _InitTestQuad();

	// M4.3: startup / title logo (drawn when no song is loaded)
	MTLogo11 m_Logo11;

	// M2: keyboard to draw (not owned)
	MTKeyboard11* m_pKbd11;

	// M2.5: real first-person camera (not owned; NULL = static hint)
	MTFirstPersonCam* m_pCam11;

	// M3: instanced note field (not owned; NULL = none)
	DXNoteBox11* m_pNoteBox11;

	// M4.7: instanced falling-note field, Rain scene (not owned; NULL = none)
	DXNoteRain11* m_pNoteRain11;

	// M4.7b: Rain-scene keyboard (not owned; NULL = none)
	MTKeyboardRain11* m_pKbdRain11;

	// M4.9: Ring-scene note field (not owned; NULL = none)
	DXNoteBoxRing11* m_pNoteBoxRing11;

	// M4.15: background image (not owned; NULL = none)
	MTBackgroundImage11* m_pBackgroundImage11;

	// M5: background clear color ([Color] BackGroundRGB); 0 = black
	D3DCOLOR m_BgColor;

	// M6: off-screen video-export target (NULL = inactive)
	ID3D11Texture2D*        m_pOffTex;
	ID3D11RenderTargetView* m_pOffRTV;
	ID3D11Texture2D*        m_pOffDepth;
	ID3D11DepthStencilView* m_pOffDSV;
	ID3D11Texture2D*        m_pOffStaging[2];   // double-buffered for async readback
	// MSAA render target for export (resolved into m_pOffTex); NULL when 1x
	ID3D11Texture2D*        m_pOffTexMS;
	ID3D11RenderTargetView* m_pOffRTVMS;
	ID3D11Texture2D*        m_pOffDepthMS;
	ID3D11DepthStencilView* m_pOffDSVMS;
	unsigned int            m_OffSampleCount;   // export MSAA sample count (1 = off)
	unsigned int            m_OffW;
	unsigned int            m_OffH;

	// 360 (equirectangular) export: render the scene into a cubemap from the camera
	// position, then a fullscreen pass remaps it to a 2:1 equirectangular frame
	// (m_pOffTex) oriented so the camera forward is the panorama center.
	bool                    m_Off360;
	unsigned int            m_CubeFaceRes;
	ID3D11Texture2D*        m_pCubeTex;
	ID3D11RenderTargetView* m_pCubeRTV[6];
	ID3D11Texture2D*        m_pCubeDepth;
	ID3D11DepthStencilView* m_pCubeDSV;
	ID3D11ShaderResourceView* m_pCubeSRV;
	// per-face MSAA render target (resolved into each cube face for antialiasing)
	ID3D11Texture2D*        m_pCubeFaceMS;
	ID3D11RenderTargetView* m_pCubeFaceMSRTV;
	ID3D11Texture2D*        m_pCubeFaceMSDepth;
	ID3D11DepthStencilView* m_pCubeFaceMSDSV;
	unsigned int            m_CubeSampleCount;
	ID3D11VertexShader*     m_pEqVS;
	ID3D11PixelShader*      m_pEqPS;
	ID3D11Buffer*           m_pEqCB;
	ID3D11SamplerState*     m_pEqSampler;
	ID3D11DepthStencilState* m_pEqNoDepth;
	ID3D11RasterizerState*  m_pEqRaster;

	int  _InitEquirect();
	void _ReleaseEquirect();
	int  _RenderFrame360(bool transparent, int stageSlot);
	void _DrawSceneComponents(const DirectX::XMMATRIX& viewProj, const DirectX::XMFLOAT3& camPos,
			float rollAngle, bool transparent);

	// M4.16: starfield (not owned; NULL = none)
	MTStars11* m_pStars11;

	// M4.13: Ring-scene decorations (not owned; NULL = none)
	MTGridRing11* m_pGridRing11;
	MTTimeIndicatorRing11* m_pTimeIndicatorRing11;
	MTPictBoardRing11* m_pPictBoardRing11;

	// M3: note ripple effect (not owned; NULL = none)
	MTNoteRipple11* m_pNoteRipple11;
	MTNoteLyrics11* m_pNoteLyrics11;
	MTNoteBoxLive11* m_pNoteBoxLive11;
	MTNoteRainLive11* m_pNoteRainLive11;

	// M3: grid box (not owned; NULL = none)
	MTGridBox11* m_pGridBox11;

	// M4: dashboard overlay (not owned; NULL = none)
	MTDashboard11* m_pDashboard11;
	MTConfigManager11* m_pConfigMgr11;

	// M4.4: time indicator / playback section (not owned; NULL = none)
	MTTimeIndicator11* m_pTimeIndicator11;

	// M4.5: picture board (not owned; NULL = none)
	MTPictBoard11* m_pPictBoard11;

	int _CreateTargets(unsigned int width, unsigned int height);
	void _ReleaseTargets();
	void _BeginImGuiFrame();

	//disable copy
	void operator=(const DXRenderer11&);
	DXRenderer11(const DXRenderer11&);
};
