//******************************************************************************
//
// MIDITrail / IMTScene11
//
// DX11 scene interface.
// Replaces the DX9-coupled MTScene with a DX11-native pure virtual interface.
// Designed for future DLL plugin extensibility (COM-style ABI).
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <d3d11.h>
#include <directxtk/SimpleMath.h>
#include <tchar.h>
#include "MTViewParamMap.h"

namespace SMIDILib { class SMSeqData; }

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
	virtual bool IsLive() const = 0;

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

	// Transform updates component state (positions, animations) for the
	// current playback position. No device access needed.
	virtual void Transform(
					unsigned long curTickTime,
					unsigned long playTimeMSec
				) = 0;

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
	virtual void OnPlayStart() = 0;
	virtual void OnPlayEnd() = 0;

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
	virtual void Rewind() = 0;
	virtual void SetPlaySpeedRatio(unsigned long ratio) = 0;

	//----------------------------------------------------------------------
	// Key-value parameters
	//----------------------------------------------------------------------
	virtual void SetParam(const char* pKey, const char* pValue) = 0;
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
	// Information
	//----------------------------------------------------------------------
	virtual unsigned long GetNoteCount() const = 0;
};
