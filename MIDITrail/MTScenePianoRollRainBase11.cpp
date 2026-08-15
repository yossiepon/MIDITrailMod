//******************************************************************************
//
// MIDITrail / MTScenePianoRollRainBase11
//
// PianoRoll Rain scene base class.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTScenePianoRollRainBase11.h"
#include "SMMsgParser.h"

using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor
//******************************************************************************
MTScenePianoRollRainBase11::MTScenePianoRollRainBase11(bool is2D)
{
	m_Is2D = is2D;
	m_hWnd = NULL;
	m_pKeyboardCtrl = NULL;
}

//******************************************************************************
// Destructor
//******************************************************************************
MTScenePianoRollRainBase11::~MTScenePianoRollRainBase11()
{
	Release();
}

//******************************************************************************
// Create
//******************************************************************************
int MTScenePianoRollRainBase11::Create(
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

	m_Camera.SetProgressDirection(MTFirstPersonCam::DirNone);

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
	_RegisterComponent(&m_NotePitchBend);
	_RegisterModeComponents();

EXIT:;
	return result;
}

//******************************************************************************
// Release
//******************************************************************************
void MTScenePianoRollRainBase11::Release()
{
	delete m_pKeyboardCtrl;
	m_pKeyboardCtrl = NULL;
	m_Stars.Release();
	m_BackgroundImage.Release();
	m_Dashboard.Release();

	MTSceneBase11::Release();
}

//******************************************************************************
// Draw
//******************************************************************************
int MTScenePianoRollRainBase11::Draw(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		float rollAngle,
		const Vector3& camPos
	)
{
	return _DrawSceneComponents(pContext, viewProj, rollAngle, camPos);
}

int MTScenePianoRollRainBase11::_DrawSceneComponents(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		float rollAngle,
		const Vector3& camPos
	)
{
	int result = 0;
	Vector4 lightDir(1.0f, -2.0f, 0.5f, 0.0f);

	result = m_BackgroundImage.Draw(pContext);
	if (result != 0) goto EXIT;

	if (m_pKeyboardCtrl != NULL) {
		result = m_pKeyboardCtrl->Draw(pContext, viewProj, lightDir);
		if (result != 0) goto EXIT;
	}

	result = _DrawNotes(pContext, viewProj, lightDir);
	if (result != 0) goto EXIT;

	result = m_Stars.Draw(pContext, viewProj, rollAngle);
	if (result != 0) goto EXIT;

	{
		RECT rect;
		GetClientRect(m_hWnd, &rect);
		result = _DrawDashboard(pContext,
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
int MTScenePianoRollRainBase11::_OnRecvSequencerMsg(
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
int MTScenePianoRollRainBase11::OnPlayStart()
{
	_Reset();
	return 0;
}

int MTScenePianoRollRainBase11::OnPlayEnd()
{
	m_isMonitoringActive = false;
	return 0;
}

//******************************************************************************
// Display toggle
//******************************************************************************
void MTScenePianoRollRainBase11::SetEffect(MTEffectType type, bool isEnable)
{
	switch (type) {
		case MTEffectPianoKeyboard:
			if (m_pKeyboardCtrl) m_pKeyboardCtrl->SetEnable(isEnable);
			break;
		case MTEffectPitchBend:
			m_NotePitchBend.SetEnable(isEnable);
			break;
		case MTEffectStars:
			m_Stars.SetEnable(isEnable);
			break;
		case MTEffectCounter:
			_SetDashboardEnable(isEnable);
			break;
		case MTEffectBackgroundImage:
			m_BackgroundImage.SetEnable(isEnable);
			break;
		default:
			break;
	}
}

//******************************************************************************
// Set playback speed
//******************************************************************************
void MTScenePianoRollRainBase11::SetPlaySpeedRatio(unsigned long ratio)
{
	m_Dashboard.SetPlaySpeedRatio(ratio);
}

//******************************************************************************
// Window resize notification
//******************************************************************************
void MTScenePianoRollRainBase11::OnWindowResize()
{
	m_BackgroundImage.OnWindowResize();
	_OnDashboardWindowResize();
}

//******************************************************************************
// Compute default viewpoint
//******************************************************************************
void MTScenePianoRollRainBase11::_ComputeDefaultViewParam(MTViewParamMap* pParamMap)
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
// Viewpoint compensation amount
//******************************************************************************
float MTScenePianoRollRainBase11::_GetViewpointCompensation() const
{
	return 0.0f;
}

//******************************************************************************
// Reset
//******************************************************************************
void MTScenePianoRollRainBase11::_Reset()
{
	MTSceneBase11::_Reset();
}

//******************************************************************************
// Dashboard virtual methods (default: Playback dashboard)
//******************************************************************************
int MTScenePianoRollRainBase11::_DrawDashboard(
		ID3D11DeviceContext* pContext,
		unsigned int screenWidth,
		unsigned int screenHeight
	)
{
	return m_Dashboard.Draw(pContext, screenWidth, screenHeight);
}

void MTScenePianoRollRainBase11::_OnDashboardWindowResize()
{
	m_Dashboard.OnWindowResize();
}

void MTScenePianoRollRainBase11::_SetDashboardEnable(bool isEnable)
{
	m_Dashboard.SetEnable(isEnable);
}
