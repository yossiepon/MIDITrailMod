//******************************************************************************
//
// MIDITrail / MTScenePianoRollRingBase11
//
// PianoRoll Ring scene base class.
//
// Copyright (C) 2019-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2019-2026 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTScenePianoRollRingBase11.h"
#include "MTNoteDesignRing11.h"
#include "SMMsgParser.h"

using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor
//******************************************************************************
MTScenePianoRollRingBase11::MTScenePianoRollRingBase11()
{
	m_hWnd = NULL;
	m_pGridRing = NULL;

	m_Traits.viewpointCompensation = true;
}

//******************************************************************************
// Destructor
//******************************************************************************
MTScenePianoRollRingBase11::~MTScenePianoRollRingBase11()
{
	Release();
}

//******************************************************************************
// Create
//******************************************************************************
int MTScenePianoRollRingBase11::Create(
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

	result = m_Camera.Initialize(hWnd, GetName(), pSeqData);
	if (result != 0) goto EXIT;

	m_Camera.SetProgressDirection(MTFirstPersonCam::DirX);

	{
		MTViewParamMap defaultView;
		_ComputeDefaultViewParam(&defaultView);
		m_Camera.SetViewParam(&defaultView);
	}

	result = m_Stars.Create(pDevice, pContext, GetName());
	if (result != 0) goto EXIT;

	result = m_BackgroundImage.Create(pDevice, pContext, hWnd);
	if (result != 0) goto EXIT;

	result = m_NotePitchBend.Initialize();
	if (result != 0) goto EXIT;

	result = _CreateModeComponents(pDevice, pContext, pSeqData);
	if (result != 0) goto EXIT;

	_RegisterComponent(&m_Stars);
	_RegisterComponent(&m_BackgroundImage);
	if (m_pGridRing) _RegisterComponent(m_pGridRing);
	_RegisterComponent(&m_TimeIndicator);
	_RegisterComponent(&m_PictBoard);
	_RegisterComponent(&m_Dashboard);
	_RegisterComponent(&m_NotePitchBend);
	_RegisterModeComponents();

EXIT:;
	return result;
}

//******************************************************************************
// Release
//******************************************************************************
void MTScenePianoRollRingBase11::Release()
{
	delete m_pGridRing;
	m_pGridRing = NULL;
	m_TimeIndicator.Release();
	m_PictBoard.Release();
	m_Ripple.Release();
	m_Dashboard.Release();
	m_Stars.Release();
	m_BackgroundImage.Release();

	MTSceneBase11::Release();
}

//******************************************************************************
// Draw
//******************************************************************************
int MTScenePianoRollRingBase11::Draw(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		float rollAngle,
		const Vector3& camPos
	)
{
	return _DrawSceneComponents(pContext, viewProj, rollAngle, camPos);
}

//******************************************************************************
// Draw scene-specific components
// Applies the DX9 Mod pattern's dynamic draw-order switching
//******************************************************************************
int MTScenePianoRollRingBase11::_DrawSceneComponents(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		float rollAngle,
		const Vector3& camPos
	)
{
	int result = 0;
	Vector4 lightDir(1.0f, -1.0f, 2.0f, 0.0f);

	result = m_BackgroundImage.Draw(pContext);
	if (result != 0) goto EXIT;

	if (m_pGridRing != NULL) {
		result = m_pGridRing->Draw(pContext, viewProj, lightDir);
		if (result != 0) goto EXIT;
	}

	result = _DrawNotes(pContext, viewProj, lightDir);
	if (result != 0) goto EXIT;

	if (m_TimeIndicator.GetPos() > camPos.x) {
		result = m_TimeIndicator.Draw(pContext, viewProj, lightDir);
		if (result != 0) goto EXIT;
		result = _DrawLyrics(pContext, viewProj, lightDir, camPos);
		if (result != 0) goto EXIT;
		result = m_Ripple.Draw(pContext, viewProj, lightDir, camPos);
		if (result != 0) goto EXIT;
		result = m_PictBoard.Draw(pContext, viewProj, lightDir);
		if (result != 0) goto EXIT;
	}
	else {
		result = m_PictBoard.Draw(pContext, viewProj, lightDir);
		if (result != 0) goto EXIT;
		result = m_Ripple.Draw(pContext, viewProj, lightDir, camPos);
		if (result != 0) goto EXIT;
		result = _DrawLyrics(pContext, viewProj, lightDir, camPos);
		if (result != 0) goto EXIT;
		result = m_TimeIndicator.Draw(pContext, viewProj, lightDir);
		if (result != 0) goto EXIT;
	}

	result = m_Stars.Draw(pContext, viewProj, rollAngle);
	if (result != 0) goto EXIT;

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
// Sequencer message reception (common: PitchBend + Dashboard)
//******************************************************************************
int MTScenePianoRollRingBase11::_OnRecvSequencerMsg(
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

	return result;
}

//******************************************************************************
// Playback start / end
//******************************************************************************
int MTScenePianoRollRingBase11::OnPlayStart()
{
	_Reset();
	return 0;
}

int MTScenePianoRollRingBase11::OnPlayEnd()
{
	m_isMonitoringActive = false;
	return 0;
}

//******************************************************************************
// Display toggle
//******************************************************************************
void MTScenePianoRollRingBase11::SetEffect(MTEffectType type, bool isEnable)
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
			break;
		default:
			break;
	}
}

//******************************************************************************
// Set playback speed
//******************************************************************************
void MTScenePianoRollRingBase11::SetPlaySpeedRatio(unsigned long ratio)
{
	m_Dashboard.SetPlaySpeedRatio(ratio);
}

//******************************************************************************
// Window resize notification
//******************************************************************************
void MTScenePianoRollRingBase11::OnWindowResize()
{
	m_BackgroundImage.OnWindowResize();
	m_Dashboard.OnWindowResize();
}

//******************************************************************************
// Compute default viewpoint
//******************************************************************************
void MTScenePianoRollRingBase11::_ComputeDefaultViewParam(MTViewParamMap* pParamMap)
{
	pParamMap->clear();
	(*pParamMap)["X"] = -13.0f;
	(*pParamMap)["Y"] = 0.0f;
	(*pParamMap)["Z"] = 0.0f;
	(*pParamMap)["Phi"] = 0.0f;
	(*pParamMap)["Theta"] = 90.0f;
	(*pParamMap)["ManualRollAngle"] = 0.0f;
	(*pParamMap)["AutoRollVelocity"] = 0.0f;
}

//******************************************************************************
// Viewpoint compensation amount
//******************************************************************************
float MTScenePianoRollRingBase11::_GetViewpointCompensation() const
{
	return m_TimeIndicator.GetPos();
}

//******************************************************************************
// Reset
//******************************************************************************
void MTScenePianoRollRingBase11::_Reset()
{
	MTSceneBase11::_Reset();
}
