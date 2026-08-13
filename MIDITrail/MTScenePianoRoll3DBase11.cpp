//******************************************************************************
//
// MIDITrail / MTScenePianoRoll3DBase11
//
// PianoRoll 3D/2D scene base class.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2012-2026 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTScenePianoRoll3DBase11.h"
#include "SMMsgParser.h"

using namespace DirectX::SimpleMath;


//******************************************************************************
// コンストラクタ
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
// デストラクタ
//******************************************************************************
MTScenePianoRoll3DBase11::~MTScenePianoRoll3DBase11()
{
	Release();
}

//******************************************************************************
// 生成
//******************************************************************************
int MTScenePianoRoll3DBase11::Create(
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
	if (m_pGrid) _RegisterComponent(m_pGrid);
	_RegisterComponent(&m_TimeIndicator);
	_RegisterComponent(&m_Dashboard);
	_RegisterComponent(&m_NotePitchBend);
	_RegisterModeComponents();

EXIT:;
	return result;
}

//******************************************************************************
// 解放
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

	MTSceneBase11::Release();
}

//******************************************************************************
// 描画
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
		result = m_Dashboard.Draw(pContext,
		                          rect.right - rect.left,
		                          rect.bottom - rect.top);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// シーン固有コンポーネント描画
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
// シーケンサメッセージ受信（共通: PitchBend + Dashboard）
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
// 再生開始
//******************************************************************************
int MTScenePianoRoll3DBase11::OnPlayStart()
{
	_Reset();
	return 0;
}

//******************************************************************************
// 再生終了
//******************************************************************************
int MTScenePianoRoll3DBase11::OnPlayEnd()
{
	m_isMonitoringActive = false;
	return 0;
}

//******************************************************************************
// 表示トグル
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
		m_Dashboard.SetEnable(isEnable);
		break;
	case MTEffectFileName:
		m_Dashboard.SetEnableFileName(isEnable);
		break;
	case MTEffectPitchBend:
		m_NotePitchBend.SetEnable(isEnable);
		break;
	default:
		break;
	}
}

//******************************************************************************
// 再生速度設定
//******************************************************************************
void MTScenePianoRoll3DBase11::SetPlaySpeedRatio(unsigned long ratio)
{
	m_Dashboard.SetPlaySpeedRatio(ratio);
}

//******************************************************************************
// ウィンドウリサイズ通知
//******************************************************************************
void MTScenePianoRoll3DBase11::OnWindowResize()
{
	m_BackgroundImage.OnWindowResize();
	m_Dashboard.OnWindowResize();
}

//******************************************************************************
// デフォルト視点計算
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
// 視点補正量
//******************************************************************************
float MTScenePianoRoll3DBase11::_GetViewpointCompensation() const
{
	return m_TimeIndicator.GetPos();
}

//******************************************************************************
// リセット
//******************************************************************************
void MTScenePianoRoll3DBase11::_Reset()
{
	MTSceneBase11::_Reset();
}
