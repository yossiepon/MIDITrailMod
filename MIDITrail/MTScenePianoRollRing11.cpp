//******************************************************************************
//
// MIDITrail / MTScenePianoRollRing11
//
// DX11 PianoRoll Ring scene.
//
// Copyright (C) 2019-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMIDILib.h"
#include "MTScenePianoRollRing11.h"
#include "MTGridRing11.h"
#include "MTGridRingLive11.h"
#include "SMMsgParser.h"

using namespace YNBaseLib;
using namespace SMIDILib;
using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTScenePianoRollRing11::MTScenePianoRollRing11(bool isLive)
{
	m_hWnd = NULL;
	m_IsLive = isLive;
	m_pNoteBoxLive = NULL;
	m_pGridRing = NULL;

	m_Traits.viewpointCompensation = true;
}

MTScenePianoRollRing11::~MTScenePianoRollRing11()
{
	Release();
}

//******************************************************************************
// Name
//******************************************************************************
const TCHAR* MTScenePianoRollRing11::GetName() const
{
	return m_IsLive ? _T("PianoRollRingLive") : _T("PianoRollRing");
}

//******************************************************************************
// Create
//******************************************************************************
int MTScenePianoRollRing11::Create(
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

	m_Camera.SetProgressDirection(MTFirstPersonCam::DirX);

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

	if (m_IsLive) {
		// === Live モード ===

		// Live Notes
		try { m_pNoteBoxLive = new MTNoteCylindricalLive11(); }
		catch (std::bad_alloc) { result = YN_SET_ERR("Could not allocate memory.", 0, 0); goto EXIT; }
		result = m_pNoteBoxLive->Create(pDevice, pContext, GetName(), &m_NotePitchBend);
		if (result != 0) goto EXIT;
		m_pNoteBoxLive->SetLightEnable(false);

		// NoteTrackerLive
		result = m_NoteTrackerLive.Create();
		if (result != 0) goto EXIT;

		// Grid Ring (Live)
		try { m_pGridRing = new MTGridRingLive11(); }
		catch (std::bad_alloc) { result = YN_SET_ERR("Could not allocate memory.", 0, 0); goto EXIT; }
		result = ((MTGridRingLive11*)m_pGridRing)->Create(pDevice, pContext, GetName());
		if (result != 0) goto EXIT;

		// TimeIndicator Ring (pSeqData=NULL OK)
		result = m_TimeIndicator.Create(pDevice, pContext, GetName(), NULL);
		if (result != 0) goto EXIT;

		// PictBoard Ring (pSeqData=NULL OK)
		result = m_PictBoard.Create(pDevice, pContext, GetName(), NULL);
		if (result != 0) goto EXIT;

		// Ripple (inject NoteDesignRingLive11 for linear decay + Ring coordinates)
		result = m_NoteDesignRingLive.Initialize(GetName());
		if (result != 0) goto EXIT;
		result = m_Ripple.Create(pDevice, pContext, GetName(), NULL, &m_NotePitchBend, &m_NoteDesignRingLive);
		if (result != 0) goto EXIT;
		m_NoteTrackerLive.AddListener(&m_Ripple, NoteEventType::Note);

		// Dashboard (Live monitor mode)
		result = m_Dashboard.Create(pDevice, pContext, GetName(), NULL, hWnd);
		if (result != 0) goto EXIT;
		m_Dashboard.SetMonitorMode(true, _T(""));
		m_NoteTrackerLive.AddListener(&m_Dashboard, NoteEventType::Note);

		// コンポーネント登録
		_RegisterComponent(&m_Stars);
		_RegisterComponent(&m_BackgroundImage);
		_RegisterComponent(m_pGridRing);
		_RegisterComponent(&m_TimeIndicator);
		_RegisterComponent(&m_PictBoard);
		_RegisterComponent(&m_Dashboard);
		_RegisterComponent(&m_NotePitchBend);
		_RegisterComponent(&m_NoteTrackerLive);
		_RegisterComponent(&m_Ripple);
		_RegisterComponent(m_pNoteBoxLive);
	}
	else {
		// === Playback モード ===

		if (pSeqData == NULL) goto EXIT;

		// Ring NoteDesign (shared by NoteBox, Ripple, Lyrics)
		result = m_NoteDesignRing.Initialize(GetName(), pSeqData);
		if (result != 0) goto EXIT;

		// Grid Ring (Playback)
		try { m_pGridRing = new MTGridRing11(); }
		catch (std::bad_alloc) { result = YN_SET_ERR("Could not allocate memory.", 0, 0); goto EXIT; }
		result = ((MTGridRing11*)m_pGridRing)->Create(pDevice, pContext, GetName(), pSeqData);
		if (result != 0) goto EXIT;

		// Time indicator ring
		result = m_TimeIndicator.Create(pDevice, pContext, GetName(), pSeqData);
		if (result != 0) goto EXIT;

		// PictBoard ring
		result = m_PictBoard.Create(pDevice, pContext, GetName(), pSeqData);
		if (result != 0) goto EXIT;

		// Dashboard
		result = m_Dashboard.Create(pDevice, pContext, GetName(), pSeqData, hWnd);
		if (result != 0) goto EXIT;

		// NoteTracker
		result = m_NoteTracker.Create(pSeqData);
		if (result != 0) goto EXIT;

		// NoteBox (GPU-instanced Ring renderer, shares scene's NoteDesignRing11)
		result = m_NoteBox.Create(pDevice, pContext, GetName(), pSeqData,
		                          &m_NoteTracker, &m_NotePitchBend, &m_NoteDesignRing);
		if (result != 0) goto EXIT;

		// Ripple (with Ring NoteDesign injection)
		result = m_Ripple.Create(pDevice, pContext, GetName(), pSeqData,
		                         &m_NotePitchBend, &m_NoteDesignRing);
		if (result != 0) goto EXIT;
		m_NoteTracker.AddListener(&m_Ripple, NoteEventType::Note);

		// Lyrics (with Ring NoteDesign injection)
		result = m_Lyrics.Create(pDevice, pContext, GetName(), pSeqData,
		                         &m_NotePitchBend, &m_NoteDesignRing);
		if (result != 0) goto EXIT;
		m_NoteTracker.AddListener(&m_Lyrics, NoteEventType::Lyric);
		m_NoteTracker.AddListener(&m_Dashboard, NoteEventType::Note);

		// コンポーネント登録
		_RegisterComponent(&m_Stars);
		_RegisterComponent(&m_BackgroundImage);
		_RegisterComponent(m_pGridRing);
		_RegisterComponent(&m_TimeIndicator);
		_RegisterComponent(&m_PictBoard);
		_RegisterComponent(&m_Dashboard);
		_RegisterComponent(&m_NotePitchBend);
		_RegisterComponent(&m_NoteTracker);
		_RegisterComponent(&m_NoteBox);
		_RegisterComponent(&m_Ripple);
		_RegisterComponent(&m_Lyrics);
	}

EXIT:;
	return result;
}

//******************************************************************************
// Release
//******************************************************************************
void MTScenePianoRollRing11::Release()
{
	delete m_pGridRing;
	m_pGridRing = NULL;
	m_TimeIndicator.Release();
	m_PictBoard.Release();
	m_NoteBox.Release();
	delete m_pNoteBoxLive;
	m_pNoteBoxLive = NULL;
	m_Ripple.Release();
	m_Lyrics.Release();
	m_Dashboard.Release();
	m_Stars.Release();
	m_BackgroundImage.Release();

	MTSceneBase11::Release();
}

//******************************************************************************
// Draw
//******************************************************************************
int MTScenePianoRollRing11::Draw(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		float rollAngle,
		const Vector3& camPos
	)
{
	return _DrawSceneComponents(pContext, viewProj, rollAngle, camPos);
}

int MTScenePianoRollRing11::_DrawSceneComponents(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		float rollAngle,
		const Vector3& camPos
	)
{
	int result = 0;
	Vector4 lightDir(1.0f, -1.0f, 2.0f, 0.0f);

	// Background image
	result = m_BackgroundImage.Draw(pContext);
	if (result != 0) goto EXIT;

	// Grid ring
	if (m_pGridRing != NULL) {
		result = m_pGridRing->Draw(pContext, viewProj, lightDir);
		if (result != 0) goto EXIT;
	}

	// NoteBox
	if (m_pNoteBoxLive != NULL) {
		result = m_pNoteBoxLive->Draw(pContext, viewProj, lightDir);
	}
	else {
		result = m_NoteBox.Draw(pContext, viewProj, lightDir);
	}
	if (result != 0) goto EXIT;

	// PictBoard ring
	result = m_PictBoard.Draw(pContext, viewProj, lightDir);
	if (result != 0) goto EXIT;

	// Stars
	result = m_Stars.Draw(pContext, viewProj, rollAngle);
	if (result != 0) goto EXIT;

	// Time indicator ring
	result = m_TimeIndicator.Draw(pContext, viewProj, lightDir);
	if (result != 0) goto EXIT;

	// Ripple
	result = m_Ripple.Draw(pContext, viewProj, lightDir, camPos);
	if (result != 0) goto EXIT;

	// Lyrics
	result = m_Lyrics.Draw(pContext, viewProj, lightDir, camPos);
	if (result != 0) goto EXIT;

	// Dashboard (last)
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
int MTScenePianoRollRing11::OnPlayStart()
{
	_Reset();
	if (m_IsLive) {
		m_Dashboard.SetMonitoringStatus(true);
	}
	return 0;
}

int MTScenePianoRollRing11::OnPlayEnd()
{
	if (m_pNoteBoxLive != NULL) {
		m_pNoteBoxLive->AllNoteOff();
		m_NoteTrackerLive.AllNoteOff();
		m_Dashboard.SetMonitoringStatus(false);
	}
	return 0;
}

//******************************************************************************
// Sequencer message
//******************************************************************************
int MTScenePianoRollRing11::_OnRecvSequencerMsg(
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
		m_NoteBox.Reset();
		m_NoteBox.SetSkipStatus(true);
		m_Ripple.SetSkipStatus(true);
		m_Lyrics.SetSkipStatus(true);
		m_IsSkipping = true;
	}
	else if (parser.GetMsg() == SMMsgParser::MsgSkipEnd) {
		// NoteTracker リスナーがカウントを管理するため SetNotesCount は不要
		m_NoteBox.SetSkipStatus(false);
		m_Ripple.SetSkipStatus(false);
		m_Lyrics.SetSkipStatus(false);
		m_IsSkipping = false;
		m_NoteTracker.Seek(m_PlayTimeMSec);
	}

	return result;
}

//******************************************************************************
// SetEffect
//******************************************************************************
void MTScenePianoRollRing11::SetEffect(
		MTEffectType type,
		bool isEnable
	)
{
	switch (type) {
		case MTEffectPianoKeyboard:
			m_PictBoard.SetEnable(isEnable);
			break;
		case MTEffectRipple:
			m_Ripple.SetEnable(isEnable);
			break;
		case MTEffectPitchBend:
			m_NotePitchBend.SetEnable(isEnable);
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
		case MTEffectGridBox:
			if (m_pGridRing) m_pGridRing->SetEnable(isEnable);
			break;
		case MTEffectTimeIndicator:
			m_TimeIndicator.SetEnable(isEnable);
			break;
		case MTEffectFileName:
			m_Dashboard.SetEnableFileName(isEnable);
			break;
		case MTEffectLyrics:
			m_Lyrics.SetEnable(isEnable);
			break;
		default:
			break;
	}
}

//******************************************************************************
// PlaySpeedRatio / NoteCount
//******************************************************************************
void MTScenePianoRollRing11::SetPlaySpeedRatio(unsigned long ratio)
{
	m_Dashboard.SetPlaySpeedRatio(ratio);
}

unsigned long MTScenePianoRollRing11::GetNoteCount() const
{
	return m_NoteBox.GetNoteCount();
}

//******************************************************************************
// Live note events
//******************************************************************************
void MTScenePianoRollRing11::SetNoteOnLive(
		unsigned char portNo, unsigned char chNo,
		unsigned char noteNo, unsigned char velocity)
{
	if (m_pNoteBoxLive != NULL) {
		m_pNoteBoxLive->SetNoteOn(portNo, chNo, noteNo, velocity);
		m_NoteTrackerLive.SetNoteOn(portNo, chNo, noteNo, velocity);
	}
}

void MTScenePianoRollRing11::SetNoteOffLive(
		unsigned char portNo, unsigned char chNo,
		unsigned char noteNo)
{
	if (m_pNoteBoxLive != NULL) {
		m_pNoteBoxLive->SetNoteOff(portNo, chNo, noteNo);
		m_NoteTrackerLive.SetNoteOff(portNo, chNo, noteNo);
	}
}

void MTScenePianoRollRing11::AllNoteOffLive()
{
	if (m_pNoteBoxLive != NULL) {
		m_pNoteBoxLive->AllNoteOff();
		m_NoteTrackerLive.AllNoteOff();
	}
}

void MTScenePianoRollRing11::AllNoteOffOnChLive(
		unsigned char portNo, unsigned char chNo)
{
	if (m_pNoteBoxLive != NULL) {
		m_pNoteBoxLive->AllNoteOffOnCh(portNo, chNo);
		m_NoteTrackerLive.AllNoteOffOnCh(portNo, chNo);
	}
}

//******************************************************************************
// Default viewpoint
//******************************************************************************
void MTScenePianoRollRing11::_ComputeDefaultViewParam(
		MTViewParamMap* pParamMap
	)
{
	Vector3 moveVec = m_NoteDesignRing.GetWorldMoveVector();

	pParamMap->clear();
	(*pParamMap)["X"] = moveVec.x - 13.0f;
	(*pParamMap)["Y"] = moveVec.y;
	(*pParamMap)["Z"] = moveVec.z;
	(*pParamMap)["Phi"] = 0.0f;
	(*pParamMap)["Theta"] = 90.0f;
	(*pParamMap)["ManualRollAngle"] = 0.0f;
	(*pParamMap)["AutoRollVelocity"] = 0.0f;
}

//******************************************************************************
// Viewpoint compensation
//******************************************************************************
float MTScenePianoRollRing11::_GetViewpointCompensation() const
{
	return m_TimeIndicator.GetPos();
}

//******************************************************************************
// Reset
//******************************************************************************
void MTScenePianoRollRing11::_Reset()
{
	MTSceneBase11::_Reset();
}
