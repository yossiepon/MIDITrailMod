//******************************************************************************
//
// MIDITrail / MTScenePianoRoll3DLive11
//
// PianoRoll 3D/2D scene (Live).
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2026 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTScenePianoRoll3DLive11.h"
#include "MTGridBoxLive11.h"
#include "MTPianoKeyboardCtrlRollLive11.h"

using namespace DirectX::SimpleMath;


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTScenePianoRoll3DLive11::MTScenePianoRoll3DLive11(bool is2D)
	: MTScenePianoRoll3DBase11(is2D)
{
	m_IsLive = true;
	m_pNoteBoxLive = NULL;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTScenePianoRoll3DLive11::~MTScenePianoRoll3DLive11()
{
	Release();
}

//******************************************************************************
// シーン名取得
//******************************************************************************
const TCHAR* MTScenePianoRoll3DLive11::GetName() const
{
	return m_Is2D ? _T("PianoRoll2DLive") : _T("PianoRoll3DLive");
}

//******************************************************************************
// モード固有コンポーネント生成
//******************************************************************************
int MTScenePianoRoll3DLive11::_CreateModeComponents(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		SMIDILib::SMSeqData* pSeqData
	)
{
	int result = 0;

	// Live Notes
	try { m_pNoteBoxLive = new MTNoteAABBLive11(); }
	catch (std::bad_alloc) { result = YN_SET_ERR("Could not allocate memory.", 0, 0); goto EXIT; }
	result = m_pNoteBoxLive->Create(pDevice, pContext, GetName(), &m_NotePitchBend,
	                                m_Is2D ? MTAABBLiveMode::Roll2D : MTAABBLiveMode::Roll3D);
	if (result != 0) goto EXIT;
	m_pNoteBoxLive->SetLightEnable(!m_Is2D);
	if (!m_Is2D) {
		m_pNoteBoxLive->SetBilateralLighting(true, 1.2f);
		m_pNoteBoxLive->SetMaterialAmbient(0.2f, 0.2f, 0.2f);
	}

	// NoteTrackerLive
	result = m_NoteTrackerLive.Create();
	if (result != 0) goto EXIT;

	// Grid (Live)
	try { m_pGrid = new MTGridBoxLive11(); }
	catch (std::bad_alloc) { result = YN_SET_ERR("Could not allocate memory.", 0, 0); goto EXIT; }
	result = ((MTGridBoxLive11*)m_pGrid)->Create(pDevice, pContext, GetName());
	if (result != 0) goto EXIT;

	// TimeIndicator (pSeqData=NULL OK)
	result = m_TimeIndicator.Create(pDevice, pContext, GetName(), NULL);
	if (result != 0) goto EXIT;

	// Ripple (NoteDesignLive11 injection)
	result = m_NoteDesignLive.Initialize(GetName());
	if (result != 0) goto EXIT;
	result = m_Ripple.Create(pDevice, pContext, GetName(), NULL, &m_NotePitchBend, &m_NoteDesignLive);
	if (result != 0) goto EXIT;
	m_NoteTrackerLive.AddListener(&m_Ripple, NoteEventType::Note);

	// Dashboard (Live monitor mode)
	result = m_Dashboard.Create(pDevice, pContext, GetName(), NULL, m_hWnd);
	if (result != 0) goto EXIT;
	m_Dashboard.SetMonitorMode(true, _T(""));
	m_NoteTrackerLive.AddListener(&m_Dashboard, NoteEventType::Note);

	// Keyboard (Live)
	try { m_pKeyboardCtrl = new MTPianoKeyboardCtrlRollLive11(); }
	catch (std::bad_alloc) { result = YN_SET_ERR("Could not allocate memory.", 0, 0); goto EXIT; }
	result = ((MTPianoKeyboardCtrlRollLive11*)m_pKeyboardCtrl)->Create(
		pDevice, pContext, GetName(), &m_NotePitchBend, true);
	if (result != 0) goto EXIT;
	m_NoteTrackerLive.AddListener((MTPianoKeyboardCtrlRollLive11*)m_pKeyboardCtrl, NoteEventType::Note);

EXIT:;
	return result;
}

//******************************************************************************
// モード固有コンポーネント登録
//******************************************************************************
void MTScenePianoRoll3DLive11::_RegisterModeComponents()
{
	_RegisterComponent(&m_NoteTrackerLive);
	_RegisterComponent(&m_Ripple);
	_RegisterComponent(m_pNoteBoxLive);
	_RegisterComponent(m_pKeyboardCtrl);
}

//******************************************************************************
// 解放
//******************************************************************************
void MTScenePianoRoll3DLive11::Release()
{
	delete m_pNoteBoxLive;
	m_pNoteBoxLive = NULL;

	MTScenePianoRoll3DBase11::Release();
}

//******************************************************************************
// ノート描画
//******************************************************************************
int MTScenePianoRoll3DLive11::_DrawNotes(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		const Vector4& lightDir
	)
{
	if (m_pNoteBoxLive != NULL) {
		return m_pNoteBoxLive->Draw(pContext, viewProj, lightDir);
	}
	return 0;
}

//******************************************************************************
// 再生開始
//******************************************************************************
int MTScenePianoRoll3DLive11::OnPlayStart()
{
	MTScenePianoRoll3DBase11::OnPlayStart();
	m_isMonitoringActive = true;
	m_Dashboard.SetMonitoringStatus(true);
	m_Dashboard.SetMIDIINDeviceName(GetParam("MIDI_IN_DEVICE_NAME"));
	return 0;
}

//******************************************************************************
// 再生終了
//******************************************************************************
int MTScenePianoRoll3DLive11::OnPlayEnd()
{
	MTScenePianoRoll3DBase11::OnPlayEnd();
	if (m_pNoteBoxLive != NULL) {
		m_pNoteBoxLive->AllNoteOff();
		m_NoteTrackerLive.AllNoteOff();
		m_Dashboard.SetMonitoringStatus(false);
	}
	return 0;
}

//******************************************************************************
// Live note events
//******************************************************************************
void MTScenePianoRoll3DLive11::SetNoteOnLive(
		unsigned char portNo, unsigned char chNo,
		unsigned char noteNo, unsigned char velocity)
{
	if (!m_isMonitoringActive) return;
	if (m_pNoteBoxLive != NULL) {
		m_pNoteBoxLive->SetNoteOn(portNo, chNo, noteNo, velocity);
		m_NoteTrackerLive.SetNoteOn(portNo, chNo, noteNo, velocity);
	}
}

void MTScenePianoRoll3DLive11::SetNoteOffLive(
		unsigned char portNo, unsigned char chNo,
		unsigned char noteNo)
{
	if (m_pNoteBoxLive != NULL) {
		m_pNoteBoxLive->SetNoteOff(portNo, chNo, noteNo);
		m_NoteTrackerLive.SetNoteOff(portNo, chNo, noteNo);
	}
}

void MTScenePianoRoll3DLive11::AllNoteOffLive()
{
	if (m_pNoteBoxLive != NULL) {
		m_pNoteBoxLive->AllNoteOff();
		m_NoteTrackerLive.AllNoteOff();
	}
}

void MTScenePianoRoll3DLive11::AllNoteOffOnChLive(
		unsigned char portNo, unsigned char chNo)
{
	if (m_pNoteBoxLive != NULL) {
		m_pNoteBoxLive->AllNoteOffOnCh(portNo, chNo);
		m_NoteTrackerLive.AllNoteOffOnCh(portNo, chNo);
	}
}
