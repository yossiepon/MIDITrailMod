//******************************************************************************
//
// MIDITrail / MTScenePianoRollRain11
//
// DX11 PianoRoll Rain scene.
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMIDILib.h"
#include "MTScenePianoRollRain11.h"

using namespace YNBaseLib;
using namespace SMIDILib;
using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTScenePianoRollRain11::MTScenePianoRollRain11(bool isLive, bool is2D)
{
	m_Is2D = is2D;
	m_hWnd = NULL;
	m_IsLive = isLive;
}

MTScenePianoRollRain11::~MTScenePianoRollRain11()
{
	Release();
}

//******************************************************************************
// Name
//******************************************************************************
const TCHAR* MTScenePianoRollRain11::GetName() const
{
	if (m_Is2D) {
		return _T("PianoRollRain2D");
	}
	return _T("PianoRollRain");
}

//******************************************************************************
// Create
//******************************************************************************
int MTScenePianoRollRain11::Create(
		HWND hWnd,
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		SMIDILib::SMSeqData* pSeqData
	)
{
	int result = 0;

	m_pDevice = pDevice;
	m_pContext = pContext;
	m_hWnd = hWnd;

	_LoadConf();

	// Camera
	result = m_Camera.Initialize(hWnd, GetName(), pSeqData);
	if (result != 0) goto EXIT;

	m_Camera.SetProgressDirection(MTFirstPersonCam::DirNone);

	{
		MTViewParamMap defaultView;
		_ComputeDefaultViewParam(&defaultView);
		m_Camera.SetViewParam(&defaultView);
	}

	// Stars
	result = m_Stars.Create(pDevice, pContext, GetName());
	if (result != 0) goto EXIT;

	// Background image
	result = m_BackgroundImage.Create(pDevice, pContext, hWnd);
	if (result != 0) goto EXIT;

	if (pSeqData == NULL) goto EXIT;

	// PitchBend
	result = m_NotePitchBend.Initialize();
	if (result != 0) goto EXIT;

	if (m_Is2D) {
		m_NotePitchBend.SetEnable(false);
	}

	// NoteTracker (for keyboard per-key index)
	result = m_NoteTracker.Create(pSeqData);
	if (result != 0) goto EXIT;

	// Keyboard (non-Mod, single keyboard for 2D)
	result = m_KeyboardCtrl.Create(pDevice, pContext, GetName(), pSeqData,
	                               &m_NoteTracker, &m_NotePitchBend, m_Is2D);
	if (result != 0) goto EXIT;

	// Rain: keyboard stays at fixed position (camera tracks via DirY)
	m_KeyboardCtrl.SetPlaybackPosTracking(false);

	// Note rain
	result = m_NoteRain.Create(pDevice, pContext, GetName(), pSeqData, &m_NotePitchBend);
	if (result != 0) goto EXIT;

	// Dashboard
	result = m_Dashboard.Create(pDevice, pContext, GetName(), pSeqData, hWnd);
	if (result != 0) goto EXIT;
	m_NoteTracker.AddListener(&m_Dashboard, NoteEventType::Note);

	// Register components for auto Update/Reset
	_RegisterComponent(&m_Stars);
	_RegisterComponent(&m_BackgroundImage);
	_RegisterComponent(&m_NotePitchBend);
	_RegisterComponent(&m_NoteTracker);
	_RegisterComponent(&m_KeyboardCtrl);
	_RegisterComponent(&m_NoteRain);
	_RegisterComponent(&m_Dashboard);

EXIT:;
	return result;
}

//******************************************************************************
// Release
//******************************************************************************
void MTScenePianoRollRain11::Release()
{
	m_KeyboardCtrl.Release();
	m_NoteRain.Release();
	m_Dashboard.Release();
	m_Stars.Release();
	m_BackgroundImage.Release();
}

//******************************************************************************
// Draw
//******************************************************************************
int MTScenePianoRollRain11::Draw(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		float rollAngle,
		const Vector3& camPos
	)
{
	int result = 0;

	result = _DrawSceneComponents(pContext, viewProj, rollAngle, camPos);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

int MTScenePianoRollRain11::_DrawSceneComponents(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		float rollAngle,
		const Vector3& camPos
	)
{
	int result = 0;

	Vector4 lightDir(1.0f, -2.0f, 0.5f, 0.0f);

	// Background image
	result = m_BackgroundImage.Draw(pContext);
	if (result != 0) goto EXIT;

	// Keyboard
	result = m_KeyboardCtrl.Draw(pContext, viewProj, lightDir);
	if (result != 0) goto EXIT;

	// Note rain
	result = m_NoteRain.Draw(pContext, viewProj, lightDir);
	if (result != 0) goto EXIT;

	// Stars
	result = m_Stars.Draw(pContext, viewProj, rollAngle);
	if (result != 0) goto EXIT;

	// Dashboard (last — uses transformed vertices)
	{
		RECT rect;
		GetClientRect(m_hWnd, &rect);
		result = m_Dashboard.Draw(pContext,
		                          rect.right - rect.left,
		                          rect.bottom - rect.top);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// OnPlayStart / OnPlayEnd
//******************************************************************************
int MTScenePianoRollRain11::OnPlayStart()
{
	_Reset();
	return 0;
}

int MTScenePianoRollRain11::OnPlayEnd()
{
	return 0;
}

//******************************************************************************
// Sequencer message
//******************************************************************************
int MTScenePianoRollRain11::_OnRecvSequencerMsg(
		unsigned long param1,
		unsigned long param2
	)
{
	int result = 0;
	SMMsgParser parser;

	parser.Parse(param1, param2);

	if (parser.GetMsg() == SMMsgParser::MsgPlayStatus) {
	}
	else if (parser.GetMsg() == SMMsgParser::MsgPlayTime) {
		m_Dashboard.SetTotalPlayTimeSec(parser.GetPlayTimeSec());
	}
	else if (parser.GetMsg() == SMMsgParser::MsgTempo) {
		m_Dashboard.SetTempoBPM(parser.GetTempoBPM());
	}
	else if (parser.GetMsg() == SMMsgParser::MsgBar) {
		m_Dashboard.SetBarNo(parser.GetBarNo());
	}
	else if (parser.GetMsg() == SMMsgParser::MsgBeat) {
		m_Dashboard.SetBeat(parser.GetBeatNumerator(), parser.GetBeatDenominator());
	}
	else if (parser.GetMsg() == SMMsgParser::MsgPitchBend) {
		m_NotePitchBend.SetPitchBend(
			parser.GetPortNo(), parser.GetChNo(),
			parser.GetPitchBendValue(), parser.GetPitchBendSensitivity());
	}
	else if (parser.GetMsg() == SMMsgParser::MsgSkipStart) {
		if (parser.GetSkipStartDirection() == SMMsgParser::SkipBack) {
			m_NotePitchBend.Reset();
		}
		m_KeyboardCtrl.Reset();
		m_KeyboardCtrl.SetSkipStatus(true);
		m_NoteRain.Reset();
		m_NoteRain.SetSkipStatus(true);
		m_IsSkipping = true;
	}
	else if (parser.GetMsg() == SMMsgParser::MsgSkipEnd) {
		// NoteTracker リスナーがカウントを管理するため SetNotesCount は不要
		m_KeyboardCtrl.SetSkipStatus(false);
		m_NoteRain.SetSkipStatus(false);
		m_IsSkipping = false;
		m_NoteTracker.Seek(m_PlayTimeMSec);
	}

	return result;
}

//******************************************************************************
// SetEffect
//******************************************************************************
void MTScenePianoRollRain11::SetEffect(
		MTEffectType type,
		bool isEnable
	)
{
	switch (type) {
		case MTEffectPianoKeyboard:
			m_KeyboardCtrl.SetEnable(isEnable);
			break;
		case MTEffectPitchBend:
			if (!m_Is2D) {
				m_NotePitchBend.SetEnable(isEnable);
			}
			break;
		case MTEffectStars:
			m_Stars.SetEnable(isEnable);
			break;
		case MTEffectCounter:
			m_Dashboard.SetEnable(isEnable);
			break;
		case MTEffectBackgroundImage:
			m_BackgroundImage.SetEnable(isEnable);
			break;
		default:
			break;
	}
}

//******************************************************************************
// PlaySpeedRatio / NoteCount
//******************************************************************************
void MTScenePianoRollRain11::SetPlaySpeedRatio(unsigned long ratio)
{
	m_Dashboard.SetPlaySpeedRatio(ratio);
}

unsigned long MTScenePianoRollRain11::GetNoteCount() const
{
	return m_NoteRain.GetNoteCount();
}

//******************************************************************************
// Default viewpoint
//******************************************************************************
void MTScenePianoRollRain11::_ComputeDefaultViewParam(
		MTViewParamMap* pParamMap
	)
{
	pParamMap->clear();
	(*pParamMap)["X"] = 0.0f;
	(*pParamMap)["Y"] = 0.0f;
	(*pParamMap)["Z"] = -(1.5f * 16.0f / 2.0f) - 10.0f;
	(*pParamMap)["Phi"] = 90.0f;
	(*pParamMap)["Theta"] = 90.0f;
	(*pParamMap)["ManualRollAngle"] = 0.0f;
	(*pParamMap)["AutoRollVelocity"] = 0.0f;
}

//******************************************************************************
// Viewpoint compensation
//******************************************************************************
float MTScenePianoRollRain11::_GetViewpointCompensation() const
{
	return 0.0f;
}

//******************************************************************************
// Reset
//******************************************************************************
void MTScenePianoRollRain11::_Reset()
{
	MTSceneBase11::_Reset();
}
