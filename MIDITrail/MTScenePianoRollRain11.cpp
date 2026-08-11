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
#include "MTPianoKeyboardCtrlRain11.h"
#include "MTPianoKeyboardCtrlRainLive11.h"
#include "SMMsgParser.h"

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
	m_pNoteRainLive = NULL;
	m_pKeyboardCtrl = NULL;
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
	if (m_IsLive) {
		return m_Is2D ? _T("PianoRollRain2DLive") : _T("PianoRollRainLive");
	}
	return m_Is2D ? _T("PianoRollRain2D") : _T("PianoRollRain");
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

	// PitchBend（Live/Playback 共通で必要）
	result = m_NotePitchBend.Initialize();
	if (result != 0) goto EXIT;

	if (m_Is2D) {
		m_NotePitchBend.SetEnable(false);
	}

	if (m_IsLive) {
		// === Live モード ===

		// Live Notes
		try { m_pNoteRainLive = new MTNoteAABBLive11(); }
		catch (std::bad_alloc) { result = YN_SET_ERR("Could not allocate memory.", 0, 0); goto EXIT; }
		result = m_pNoteRainLive->Create(pDevice, pContext, GetName(), &m_NotePitchBend,
		                                 MTAABBLiveMode::Rain);
		if (result != 0) goto EXIT;
		m_pNoteRainLive->SetLightEnable(false);

		// NoteTrackerLive
		result = m_NoteTrackerLive.Create();
		if (result != 0) goto EXIT;

		// Keyboard (Live)
		try { m_pKeyboardCtrl = new MTPianoKeyboardCtrlRainLive11(); }
		catch (std::bad_alloc) { result = YN_SET_ERR("Could not allocate memory.", 0, 0); goto EXIT; }
		result = ((MTPianoKeyboardCtrlRainLive11*)m_pKeyboardCtrl)->Create(
			pDevice, pContext, GetName(), &m_NotePitchBend, m_Is2D);
		if (result != 0) goto EXIT;
		m_NoteTrackerLive.AddListener((MTPianoKeyboardCtrlRainLive11*)m_pKeyboardCtrl, NoteEventType::Note);

		// Dashboard (Live monitor mode)
		result = m_Dashboard.Create(pDevice, pContext, GetName(), NULL, hWnd);
		if (result != 0) goto EXIT;
		m_Dashboard.SetMonitorMode(true, _T(""));
		m_NoteTrackerLive.AddListener(&m_Dashboard, NoteEventType::Note);

		// コンポーネント登録
		_RegisterComponent(&m_Stars);
		_RegisterComponent(&m_BackgroundImage);
		_RegisterComponent(&m_Dashboard);
		_RegisterComponent(&m_NotePitchBend);
		_RegisterComponent(&m_NoteTrackerLive);
		_RegisterComponent(m_pNoteRainLive);
		_RegisterComponent(m_pKeyboardCtrl);
	}
	else {
		// === Playback モード ===

		if (pSeqData == NULL) goto EXIT;

		// NoteTracker
		result = m_NoteTracker.Create(pSeqData);
		if (result != 0) goto EXIT;

		// Keyboard (Playback)
		try { m_pKeyboardCtrl = new MTPianoKeyboardCtrlRain11(); }
		catch (std::bad_alloc) { result = YN_SET_ERR("Could not allocate memory.", 0, 0); goto EXIT; }
		result = ((MTPianoKeyboardCtrlRain11*)m_pKeyboardCtrl)->Create(
			pDevice, pContext, GetName(), pSeqData, &m_NoteTracker, &m_NotePitchBend, m_Is2D);
		if (result != 0) goto EXIT;
		((MTPianoKeyboardCtrlRain11*)m_pKeyboardCtrl)->SetPlaybackPosTracking(false);

		// Note rain
		result = m_NoteRain.Create(pDevice, pContext, GetName(), pSeqData, nullptr, &m_NotePitchBend,
		                           MTAABBMode::Rain);
		if (result != 0) goto EXIT;

		// Dashboard
		result = m_Dashboard.Create(pDevice, pContext, GetName(), pSeqData, hWnd);
		if (result != 0) goto EXIT;
		m_NoteTracker.AddListener(&m_Dashboard, NoteEventType::Note);

		// コンポーネント登録
		_RegisterComponent(&m_Stars);
		_RegisterComponent(&m_BackgroundImage);
		_RegisterComponent(&m_NotePitchBend);
		_RegisterComponent(&m_NoteTracker);
		_RegisterComponent(m_pKeyboardCtrl);
		_RegisterComponent(&m_NoteRain);
		_RegisterComponent(&m_Dashboard);
	}

EXIT:;
	return result;
}

//******************************************************************************
// Release
//******************************************************************************
void MTScenePianoRollRain11::Release()
{
	delete m_pKeyboardCtrl;
	m_pKeyboardCtrl = NULL;
	m_NoteRain.Release();
	delete m_pNoteRainLive;
	m_pNoteRainLive = NULL;
	m_Dashboard.Release();
	m_Stars.Release();
	m_BackgroundImage.Release();

	MTSceneBase11::Release();
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
	if (m_pKeyboardCtrl != NULL) {
		result = m_pKeyboardCtrl->Draw(pContext, viewProj, lightDir);
		if (result != 0) goto EXIT;
	}

	// Note rain
	if (m_pNoteRainLive != NULL) {
		result = m_pNoteRainLive->Draw(pContext, viewProj, lightDir);
	}
	else {
		result = m_NoteRain.Draw(pContext, viewProj, lightDir);
	}
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
	if (m_IsLive) {
		m_Dashboard.SetMonitoringStatus(true);
		m_Dashboard.SetMIDIINDeviceName(GetParam("MIDI_IN_DEVICE_NAME"));
	}
	return 0;
}

int MTScenePianoRollRain11::OnPlayEnd()
{
	if (m_pNoteRainLive != NULL) {
		m_pNoteRainLive->AllNoteOff();
		m_NoteTrackerLive.AllNoteOff();
		m_Dashboard.SetMonitoringStatus(false);
	}
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
		m_NoteRain.Reset();
		m_NoteRain.SetSkipStatus(true);
		if (m_pKeyboardCtrl) m_pKeyboardCtrl->Reset();
		if (m_pKeyboardCtrl) m_pKeyboardCtrl->SetSkipStatus(true);
		m_IsSkipping = true;
	}
	else if (parser.GetMsg() == SMMsgParser::MsgSkipEnd) {
		m_NoteRain.SetSkipStatus(false);
		if (m_pKeyboardCtrl) m_pKeyboardCtrl->SetSkipStatus(false);
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
			if (m_pKeyboardCtrl) m_pKeyboardCtrl->SetEnable(isEnable);
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
// Live note events
//******************************************************************************
void MTScenePianoRollRain11::SetNoteOnLive(
		unsigned char portNo, unsigned char chNo,
		unsigned char noteNo, unsigned char velocity)
{
	if (m_pNoteRainLive != NULL) {
		m_pNoteRainLive->SetNoteOn(portNo, chNo, noteNo, velocity);
		m_NoteTrackerLive.SetNoteOn(portNo, chNo, noteNo, velocity);
	}
}

void MTScenePianoRollRain11::SetNoteOffLive(
		unsigned char portNo, unsigned char chNo,
		unsigned char noteNo)
{
	if (m_pNoteRainLive != NULL) {
		m_pNoteRainLive->SetNoteOff(portNo, chNo, noteNo);
		m_NoteTrackerLive.SetNoteOff(portNo, chNo, noteNo);
	}
}

void MTScenePianoRollRain11::AllNoteOffLive()
{
	if (m_pNoteRainLive != NULL) {
		m_pNoteRainLive->AllNoteOff();
		m_NoteTrackerLive.AllNoteOff();
	}
}

void MTScenePianoRollRain11::AllNoteOffOnChLive(
		unsigned char portNo, unsigned char chNo)
{
	if (m_pNoteRainLive != NULL) {
		m_pNoteRainLive->AllNoteOffOnCh(portNo, chNo);
		m_NoteTrackerLive.AllNoteOffOnCh(portNo, chNo);
	}
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
