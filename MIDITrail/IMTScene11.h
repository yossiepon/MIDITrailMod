//******************************************************************************
//
// MIDITrail / IMTScene11
//
// DX11 scene interface.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <d3d11.h>
#include <directxtk/SimpleMath.h>
#include <tchar.h>
#include "MTViewParamMap.h"

namespace SMIDILib { class SMSeqData; }
class MTFirstPersonCam;

//******************************************************************************
// Effect type enum (scene-level visibility toggles)
//******************************************************************************
enum MTEffectType {
	MTEffectPianoKeyboard,
	MTEffectRipple,
	MTEffectPitchBend,
	MTEffectStars,
	MTEffectCounter,
	MTEffectBackgroundImage,
	MTEffectFileName,
	MTEffectTimeIndicator,
	MTEffectGridBox,
	MTEffectLyrics,
	MTEffectSingleKeyboard,
};

//******************************************************************************
// DX11 scene interface
//******************************************************************************
class IMTScene11 {
public:

	virtual ~IMTScene11() = default;

	//----------------------------------------------------------------------
	// Identification
	//----------------------------------------------------------------------
	virtual const TCHAR* GetName() const = 0;

	//----------------------------------------------------------------------
	// Lifecycle
	//----------------------------------------------------------------------

	// pSeqData is NULL for live monitoring scenes.
	virtual int  Create(
					HWND hWnd,
					ID3D11Device* pDevice,
					ID3D11DeviceContext* pContext,
					SMIDILib::SMSeqData* pSeqData
				) = 0;

	virtual void Release() = 0;

	//----------------------------------------------------------------------
	// Per-frame update and draw
	//----------------------------------------------------------------------

	// Update component state (positions, animations) for the current
	// playback position. No device access needed.
	virtual int  Update() = 0;

	// Draw renders all scene components. The caller (DXRenderer11) provides
	// camera-derived parameters. Scene owns the draw order and lighting.
	virtual int  Draw(
					ID3D11DeviceContext* pContext,
					const DirectX::SimpleMath::Matrix& viewProj,
					float rollAngle,
					const DirectX::SimpleMath::Vector3& camPos
				) = 0;

	//----------------------------------------------------------------------
	// Playback events
	//----------------------------------------------------------------------
	virtual int  OnPlayStart() = 0;
	virtual int  OnPlayEnd() = 0;

	//----------------------------------------------------------------------
	// Sequencer message dispatch
	//----------------------------------------------------------------------
	virtual int  OnRecvSequencerMsg(
					unsigned long param1,
					unsigned long param2
				) = 0;

	//----------------------------------------------------------------------
	// Effect (visibility) toggles
	//----------------------------------------------------------------------
	virtual void SetEffect(MTEffectType type, bool isEnable) = 0;

	//----------------------------------------------------------------------
	// Viewpoint management
	//----------------------------------------------------------------------
	virtual void GetDefaultViewParam(MTViewParamMap* pParamMap) = 0;
	virtual void GetViewParam(MTViewParamMap* pParamMap) = 0;
	virtual void SetViewParam(MTViewParamMap* pParamMap) = 0;
	virtual void MoveToStaticViewpoint(unsigned long viewpointNo) = 0;
	virtual void ResetViewpoint() = 0;

	//----------------------------------------------------------------------
	// Input
	//----------------------------------------------------------------------
	virtual int  OnWindowClicked(
					unsigned int button,
					WPARAM wParam,
					LPARAM lParam
				) = 0;

	//----------------------------------------------------------------------
	// Playback control
	//----------------------------------------------------------------------
	virtual int  Rewind() = 0;
	virtual void SetPlaySpeedRatio(unsigned long ratio) = 0;

	//----------------------------------------------------------------------
	// Key-value parameters
	//----------------------------------------------------------------------
	virtual int  SetParam(const char* pKey, const char* pValue) = 0;
	virtual const char* GetParam(const char* pKey) = 0;

	//----------------------------------------------------------------------
	// Live MIDI input (no-op for playback scenes)
	//----------------------------------------------------------------------
	virtual void SetNoteOnLive(
					unsigned char portNo,
					unsigned char chNo,
					unsigned char noteNo,
					unsigned char velocity
				) = 0;
	virtual void SetNoteOffLive(
					unsigned char portNo,
					unsigned char chNo,
					unsigned char noteNo
				) = 0;
	virtual void AllNoteOffLive() = 0;
	virtual void AllNoteOffOnChLive(
					unsigned char portNo,
					unsigned char chNo
				) = 0;

	//----------------------------------------------------------------------
	// Window resize notification
	//----------------------------------------------------------------------
	virtual void OnWindowResize() {}

	//----------------------------------------------------------------------
	// Information
	//----------------------------------------------------------------------
	virtual unsigned long GetNoteCount() const = 0;

	//----------------------------------------------------------------------
	// Camera access (used by DXRenderer11 for view/projection)
	//----------------------------------------------------------------------
	virtual MTFirstPersonCam* GetCamera() = 0;

	//----------------------------------------------------------------------
	// Background color (RGBA float[4], used by DXRenderer11 for Clear)
	//----------------------------------------------------------------------
	virtual const float* GetBGColor() const = 0;
};
