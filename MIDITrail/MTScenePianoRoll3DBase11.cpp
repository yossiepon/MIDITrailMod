//******************************************************************************
//
// MIDITrail / MTScenePianoRoll3DBase11
//
// PianoRoll 3D/2D scene base class.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2012-2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTScenePianoRoll3DBase11.h"
#include "SMMsgParser.h"
#include "RDDiagManager.h"

using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor
//******************************************************************************
MTScenePianoRoll3DBase11::MTScenePianoRoll3DBase11(bool is2D)
{
	m_Is2D = is2D;
	m_hWnd = NULL;
	m_pGrid = NULL;
	m_pKeyboardCtrl = NULL;

	m_Traits.cameraDir = MTCameraDirX;
	m_Traits.cameraTracksPlayback = true;
	m_Traits.lightEnabled = !is2D;
	m_Traits.lightCount = 2;
	m_Traits.viewpointCompensation = true;
}

//******************************************************************************
// Destructor
//******************************************************************************
MTScenePianoRoll3DBase11::~MTScenePianoRoll3DBase11()
{
	Release();
}

//******************************************************************************
// Create
//******************************************************************************
int MTScenePianoRoll3DBase11::Create(
		HWND hWnd,
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		SMIDILib::SMSeqData* pSeqData,
		const MTLoadProgressContext* pProgress
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

	result = _CreateModeComponents(pDevice, pContext, pSeqData, pProgress);
	if (result != 0) goto EXIT;

	_RegisterComponent(&m_Stars);
	_RegisterComponent(&m_BackgroundImage);
	if (m_pGrid) _RegisterComponent(m_pGrid);
	_RegisterComponent(&m_TimeIndicator);
	_RegisterComponent(&m_NotePitchBend);
	_RegisterModeComponents();

EXIT:;
	return result;
}

//******************************************************************************
// Release
//******************************************************************************
void MTScenePianoRoll3DBase11::Release()
{
	delete m_pKeyboardCtrl;
	m_pKeyboardCtrl = NULL;
	m_Ripple.Release();
	m_Stars.Release();
	delete m_pGrid;
	m_pGrid = NULL;
	m_TimeIndicator.Release();
	m_BackgroundImage.Release();
	m_Dashboard.Release();
	m_DiagOverlay.Release();

	MTSceneBase11::Release();
}

//******************************************************************************
// Draw
//******************************************************************************
int MTScenePianoRoll3DBase11::Draw(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		float rollAngle,
		const Vector3& camPos
	)
{
	int result = 0;

	result = m_BackgroundImage.Draw(pContext);
	if (result != 0) goto EXIT;

	result = _DrawSceneComponents(pContext, viewProj, rollAngle, camPos);
	if (result != 0) goto EXIT;

	{
		RECT rect;
		GetClientRect(m_hWnd, &rect);
		unsigned int w = rect.right - rect.left;
		unsigned int h = rect.bottom - rect.top;
		result = _DrawDashboard(pContext, w, h);
		if (result != 0) goto EXIT;
		result = m_DiagOverlay.Draw(pContext, w, h);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// Draw scene-specific components
//******************************************************************************
int MTScenePianoRoll3DBase11::_DrawSceneComponents(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		float rollAngle,
		const Vector3& camPos
	)
{
	int result = 0;
	Vector4 lightDir(1.0f, -1.0f, 2.0f, 0.0f);

	if (m_pGrid != NULL) {
		result = m_pGrid->Draw(pContext, viewProj, lightDir);
		if (result != 0) goto EXIT;
	}

	result = _DrawNotes(pContext, viewProj, lightDir);
	if (result != 0) goto EXIT;

	if (m_TimeIndicator.GetPos() > camPos.x) {
		result = m_TimeIndicator.Draw(pContext, viewProj, lightDir, rollAngle);
		if (result != 0) goto EXIT;
		result = _DrawLyrics(pContext, viewProj, lightDir, camPos);
		if (result != 0) goto EXIT;
		result = m_Ripple.Draw(pContext, viewProj, lightDir, camPos);
		if (result != 0) goto EXIT;
		if (m_pKeyboardCtrl != NULL) {
			result = m_pKeyboardCtrl->Draw(pContext, viewProj, lightDir);
			if (result != 0) goto EXIT;
		}
	}
	else {
		if (m_pKeyboardCtrl != NULL) {
			result = m_pKeyboardCtrl->Draw(pContext, viewProj, lightDir);
			if (result != 0) goto EXIT;
		}
		result = m_Ripple.Draw(pContext, viewProj, lightDir, camPos);
		if (result != 0) goto EXIT;
		result = _DrawLyrics(pContext, viewProj, lightDir, camPos);
		if (result != 0) goto EXIT;
		result = m_TimeIndicator.Draw(pContext, viewProj, lightDir, rollAngle);
		if (result != 0) goto EXIT;
	}

	result = m_Stars.Draw(pContext, viewProj, rollAngle);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Sequencer message reception (common: PitchBend + Dashboard)
//******************************************************************************
int MTScenePianoRoll3DBase11::_OnRecvSequencerMsg(
		unsigned long param1,
		unsigned long param2
	)
{
	int result = 0;
	SMMsgParser parser;

	parser.Parse(param1, param2);

	if (parser.GetMsg() == SMMsgParser::MsgTempo) {
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
// Playback start
//******************************************************************************
int MTScenePianoRoll3DBase11::OnPlayStart()
{
	_Reset();
	return 0;
}

//******************************************************************************
// Playback end
//******************************************************************************
int MTScenePianoRoll3DBase11::OnPlayEnd()
{
	m_isMonitoringActive = false;
	RDDiagManager::SetInt(RDMetricId::AppNoteTracking, 0);
	return 0;
}

//******************************************************************************
// Display toggle
//******************************************************************************
void MTScenePianoRoll3DBase11::SetEffect(MTEffectType type, bool isEnable)
{
	switch (type) {
	case MTEffectPianoKeyboard:
		if (m_pKeyboardCtrl) m_pKeyboardCtrl->SetEnable(isEnable);
		break;
	case MTEffectRipple:
		m_Ripple.SetEnable(isEnable);
		break;
	case MTEffectLyrics:
		break;
	case MTEffectStars:
		m_Stars.SetEnable(isEnable);
		break;
	case MTEffectGridBox:
		if (m_pGrid) m_pGrid->SetEnable(isEnable);
		break;
	case MTEffectTimeIndicator:
		m_TimeIndicator.SetEnable(isEnable);
		break;
	case MTEffectBackgroundImage:
		m_BackgroundImage.SetEnable(isEnable);
		break;
	case MTEffectCounter:
		_SetDashboardEnable(isEnable);
		break;
	case MTEffectFileName:
		m_Dashboard.SetEnableFileName(isEnable);
		break;
	case MTEffectPitchBend:
		m_NotePitchBend.SetEnable(isEnable);
		break;
	case MTEffectDiagOverlay:
		m_DiagOverlay.SetEnable(isEnable);
		break;
	default:
		break;
	}
}

//******************************************************************************
// Set playback speed
//******************************************************************************
void MTScenePianoRoll3DBase11::SetPlaySpeedRatio(unsigned long ratio)
{
	m_Dashboard.SetPlaySpeedRatio(ratio);
}

//******************************************************************************
// Window resize notification
//******************************************************************************
void MTScenePianoRoll3DBase11::OnWindowResize()
{
	m_BackgroundImage.OnWindowResize();
	_OnDashboardWindowResize();
}

//******************************************************************************
// Compute default viewpoint
//******************************************************************************
void MTScenePianoRoll3DBase11::_ComputeDefaultViewParam(MTViewParamMap* pParamMap)
{
	float noteStep = 0.1f;
	float defaultY = noteStep * 64.0f;
	float defaultZ = -18.0f;

	(*pParamMap)["X"] = 0.0f;
	(*pParamMap)["Y"] = defaultY;
	(*pParamMap)["Z"] = defaultZ;
	(*pParamMap)["Phi"] = 90.0f;
	(*pParamMap)["Theta"] = 90.0f;
	(*pParamMap)["ManualRollAngle"] = 0.0f;
	(*pParamMap)["AutoRollVelocity"] = 0.0f;
}

//******************************************************************************
// Viewpoint compensation amount
//******************************************************************************
float MTScenePianoRoll3DBase11::_GetViewpointCompensation() const
{
	return m_TimeIndicator.GetPos();
}

//******************************************************************************
// Reset
//******************************************************************************
void MTScenePianoRoll3DBase11::_Reset()
{
	MTSceneBase11::_Reset();
}

//******************************************************************************
// Dashboard virtual methods (default: Playback dashboard)
//******************************************************************************
int MTScenePianoRoll3DBase11::_DrawDashboard(
		ID3D11DeviceContext* pContext,
		unsigned int screenWidth,
		unsigned int screenHeight
	)
{
	return m_Dashboard.Draw(pContext, screenWidth, screenHeight);
}

void MTScenePianoRoll3DBase11::_OnDashboardWindowResize()
{
	m_Dashboard.OnWindowResize();
	m_DiagOverlay.OnWindowResize();
}

void MTScenePianoRoll3DBase11::_SetDashboardEnable(bool isEnable)
{
	m_Dashboard.SetEnable(isEnable);
}
