//******************************************************************************
//
// MIDITrail / MTScenePianoRollRingLiveLive
//
// ���C�u���j�^�p�s�A�m���[�������O�V�[���`��N���X
//
// Copyright (C) 2019-2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "stdafx.h"
#include <windows.h>
#include <mmsystem.h>
#include "YNBaseLib.h"
#include "DXColorUtil.h"
#include "MTConfFile.h"
#include "MTScenePianoRollRingLive.h"

using namespace YNBaseLib;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTScenePianoRollRingLive::MTScenePianoRollRingLive()
{
	m_IsEnableLight = false;
	m_IsMouseCamMode = false;
	m_IsAutoRollMode = false;
	m_IsSkipping = false;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTScenePianoRollRingLive::~MTScenePianoRollRingLive()
{
	Release();
}

//******************************************************************************
// ���̎擾
//******************************************************************************
const TCHAR* MTScenePianoRollRingLive::GetName()
{
	return _T("PianoRollRingLive");
}

//******************************************************************************
// �V�[������
//******************************************************************************
int MTScenePianoRollRingLive::Create(
		HWND hWnd,
		LPDIRECT3DDEVICE9 pD3DDevice,
		SMSeqData* pSeqData
	)
{
	int result = 0;
	
	//���C�u���j�^�̂��� pSeqData �ɂ� NULL ���w�肳���
	
	Release();
	
	if (pD3DDevice == NULL) {
		result =  YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	
	//�ݒ�t�@�C���ǂݍ���
	result = _LoadConf();
	if (result != 0) goto EXIT;
	
	//�m�[�g�f�U�C���I�u�W�F�N�g������
	result = m_NoteDesign.Initialize(GetName(), pSeqData);
	if (result != 0) goto EXIT;
	
	//----------------------------------
	// �J����
	//----------------------------------
	//�J����������
	result = m_FirstPersonCam.Initialize(hWnd, GetName(), pSeqData);
	if (result != 0) goto EXIT;
	
	//�f�t�H���g���_���擾
	GetDefaultViewParam(&m_ViewParamMap);
	
	//���_��ݒ�
	SetViewParam(&m_ViewParamMap);
	
	//----------------------------------
	// ���C�g
	//----------------------------------
	//���C�g����
	m_IsEnableLight = false;

	//���C�g������
	result = m_DirLight.Initialize();
	if (result != 0) goto EXIT;
	
	//���C�g����
	//  ���_�������Ƃ��Ă��̕������x�N�g���ŕ\������
	m_DirLight.SetDirection(D3DXVECTOR3(-1.0f, 0.0f, 0.0f));
	
	//���C�g�̃f�o�C�X�o�^
	result = m_DirLight.SetDevice(pD3DDevice, m_IsEnableLight);
	if (result != 0) goto EXIT;
	
	//----------------------------------
	// �`��I�u�W�F�N�g
	//----------------------------------
	//�s�b�`�x���h��񏉊���
	result = m_NotePitchBend.Initialize();
	if (result != 0) goto EXIT;
	
	//�m�[�g�{�b�N�X����
	result = m_NoteBoxLive.Create(pD3DDevice, GetName(), &m_NotePitchBend);
	if (result != 0) goto EXIT;
	
	//�m�[�g�g�䐶��
	result = m_NoteRipple.Create(pD3DDevice, GetName(), pSeqData, &m_NotePitchBend);
	if (result != 0) goto EXIT;
	
	//�O���b�h�����O����
	result = m_GridRingLive.Create(pD3DDevice, GetName());
	if (result != 0) goto EXIT;
	
	//�s�N�`���{�[�h����
	result = m_PictBoard.Create(pD3DDevice, GetName(), pSeqData, true);
	if (result != 0) goto EXIT;
	
	//�_�b�V���{�[�h����
	result = m_DashboardLive.Create(pD3DDevice, GetName(), hWnd);
	if (result != 0) goto EXIT;
	
	//������
	result = m_Stars.Create(pD3DDevice, GetName(), &m_DirLight);
	if (result != 0) goto EXIT;
	
	//�^�C���C���W�P�[�^����
	result = m_TimeIndicator.Create(pD3DDevice, GetName(), pSeqData);
	if (result != 0) goto EXIT;
	
	//���b�V�����䐶��
	result = m_MeshCtrl.Create(pD3DDevice, GetName());
	if (result != 0) goto EXIT;
	
	//�w�i�摜����
	result = m_BackgroundImage.Create(pD3DDevice, hWnd);
	if (result != 0) goto EXIT;
	
	//----------------------------------
	// �����_�����O�X�e�[�g
	//----------------------------------
	//��ʕ`�惂�[�h
	pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	
	//Z�[�x��r�FON
	pD3DDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	
	//�f�B�U�����O:ON ���i���`��
	pD3DDevice->SetRenderState(D3DRS_DITHERENABLE, TRUE);
	
	//�}���`�T���v�����O�A���`�G�C���A�X�F�L��
	pD3DDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
	
	//�����_�����O�X�e�[�g�ݒ�F�ʏ�̃A���t�@����
	pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	
EXIT:;
	return result;
}

//******************************************************************************
// �ϊ�����
//******************************************************************************
int MTScenePianoRollRingLive::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	float rollAngle = 0.0f;
	D3DXVECTOR3 camVector;
	
	if (pD3DDevice == NULL) {
		result =  YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	
	//�J�����X�V
	result = m_FirstPersonCam.Transform(pD3DDevice);
	if (result != 0) goto EXIT;
	
	//�J�������W�擾
	m_FirstPersonCam.GetPosition(&camVector);
	
	//��]�p�x�擾
	rollAngle = m_FirstPersonCam.GetManualRollAngle();
	
	//�m�[�g�{�b�N�X�X�V
	result = m_NoteBoxLive.Transform(pD3DDevice, rollAngle);
	if (result != 0) goto EXIT;
	
	//�O���b�h�����O�X�V
	result = m_GridRingLive.Transform(pD3DDevice, rollAngle);
	if (result != 0) goto EXIT;
	
	//�s�N�`���{�[�h�X�V
	result = m_PictBoard.Transform(pD3DDevice, camVector, rollAngle);
	if (result != 0) goto EXIT;
	
	//�_�b�V���{�[�h�X�V
	result = m_DashboardLive.Transform(pD3DDevice, camVector);
	if (result != 0) goto EXIT;
	
	//���X�V
	result = m_Stars.Transform(pD3DDevice, camVector);
	if (result != 0) goto EXIT;
	
	//���b�V���X�V
	result = m_MeshCtrl.Transform(pD3DDevice, m_TimeIndicator.GetMoveVector());
	if (result != 0) goto EXIT;

	//�^�C���C���W�P�[�^�X�V
	result = m_TimeIndicator.Transform(pD3DDevice, camVector, rollAngle);
	if (result != 0) goto EXIT;
	
	//�m�[�g�g��X�V
	result = m_NoteRipple.Transform(pD3DDevice, camVector, rollAngle);
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// �`��
//******************************************************************************
int MTScenePianoRollRingLive::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	
	if (pD3DDevice == NULL) {
		result =  YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	
	//�X�V
	result = Transform(pD3DDevice);
	if (result != 0) goto EXIT;
	
	//�w�i�摜�`��
	result = m_BackgroundImage.Draw(pD3DDevice);
	if (result != 0) goto EXIT;
	
	//�O���b�h�����O�`��
	result = m_GridRingLive.Draw(pD3DDevice);
	if (result != 0) goto EXIT;
	
	//�m�[�g�{�b�N�X�`��
	result = m_NoteBoxLive.Draw(pD3DDevice);
	if (result != 0) goto EXIT;
	
	//�s�N�`���{�[�h�`��
	result = m_PictBoard.Draw(pD3DDevice);
	if (result != 0) goto EXIT;
	
	//���`��
	result = m_Stars.Draw(pD3DDevice);
	if (result != 0) goto EXIT;
	
	//���b�V���`��
	result = m_MeshCtrl.Draw(pD3DDevice);
	if (result != 0) goto EXIT;

	//�^�C���C���W�P�[�^�`��
	result = m_TimeIndicator.Draw(pD3DDevice);
	if (result != 0) goto EXIT;
	
	//�m�[�g�g��`��
	result = m_NoteRipple.Draw(pD3DDevice);
	if (result != 0) goto EXIT;
	
	//�_�b�V���{�[�h�`��F���ˉe�̂��߈�ԍŌ�ɕ`�悷��
	result = m_DashboardLive.Draw(pD3DDevice);
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// �j��
//******************************************************************************
void MTScenePianoRollRingLive::Release()
{
	m_NoteBoxLive.Release();
	m_GridRingLive.Release();
	m_PictBoard.Release();
	m_DashboardLive.Release();
	m_Stars.Release();
	m_TimeIndicator.Release();
	m_NoteRipple.Release();
	m_MeshCtrl.Release();
	m_BackgroundImage.Release();
}

//******************************************************************************
// �E�B���h�E�N���b�N�C�x���g��M
//******************************************************************************
int MTScenePianoRollRingLive::OnWindowClicked(
		UINT button,
		WPARAM wParam,
		LPARAM lParam
	)
{
	int result = 0;
	
	//���{�^��
	if (button == WM_LBUTTONDOWN) {
		//������������ ON/OFF
		m_IsMouseCamMode = m_IsMouseCamMode ? false : true;
		m_FirstPersonCam.SetMouseCamMode(m_IsMouseCamMode);
	}
	//�E�{�^��
	else if (button == WM_RBUTTONDOWN) {
		//�������Ȃ�
	}
	//���{�^��
	else if (button == WM_MBUTTONDOWN) {
		//������]���[�h ON/OFF
		m_IsAutoRollMode = m_IsAutoRollMode ? false : true;
		m_FirstPersonCam.SetAutoRollMode(m_IsAutoRollMode);
		if (m_IsAutoRollMode) {
			m_FirstPersonCam.SwitchAutoRllDirecton();
		}
	}
	
	return result;
}

//******************************************************************************
// ���t�J�n�C�x���g��M
//******************************************************************************
int MTScenePianoRollRingLive::OnPlayStart(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	
	_Reset();
	
	m_PictBoard.OnPlayStart();
	
	m_DashboardLive.SetMonitoringStatus(true);
	result = m_DashboardLive.SetMIDIINDeviceName(pD3DDevice, GetParam(_T("MIDI_IN_DEVICE_NAME")));
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// ���t�I���C�x���g��M
//******************************************************************************
int MTScenePianoRollRingLive::OnPlayEnd(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	
	m_NoteBoxLive.AllNoteOff();
	m_PictBoard.OnPlayEnd();
	
	m_DashboardLive.SetMonitoringStatus(false);
	
//EXIT:;
	return result;
}

//******************************************************************************
// �V�[�P���T���b�Z�[�W��M
//******************************************************************************
int MTScenePianoRollRingLive::OnRecvSequencerMsg(
		unsigned long param1,
		unsigned long param2
	)
{
	int result = 0;
	SMMsgParser parser;
	
	parser.Parse(param1, param2);
	
	//���t��Ԓʒm
	if (parser.GetMsg() == SMMsgParser::MsgPlayStatus) {
		if (parser.GetPlayStatus() == SMMsgParser::StatusStop) {
			//��~�i�I���j
		}
		else if (parser.GetPlayStatus() == SMMsgParser::StatusPlay) {
			//���t
		}
		else if (parser.GetPlayStatus() == SMMsgParser::StatusPause) {
			//�ꎞ��~
		}
	}
	//�m�[�gOFF�ʒm
	else if (parser.GetMsg() == SMMsgParser::MsgNoteOff) {
		m_NoteBoxLive.SetNoteOff(parser.GetPortNo(), parser.GetChNo(), parser.GetNoteNo());
		m_NoteRipple.SetNoteOff(parser.GetPortNo(), parser.GetChNo(), parser.GetNoteNo());
	}
	//�m�[�gON�ʒm
	else if (parser.GetMsg() == SMMsgParser::MsgNoteOn) {
		m_DashboardLive.SetNoteOn();
		m_NoteBoxLive.SetNoteOn(parser.GetPortNo(), parser.GetChNo(), parser.GetNoteNo(), parser.GetVelocity());
		m_NoteRipple.SetNoteOn(parser.GetPortNo(), parser.GetChNo(), parser.GetNoteNo(), parser.GetVelocity());
	}
	//�s�b�`�x���h�ʒm
	else if (parser.GetMsg() == SMMsgParser::MsgPitchBend) {
		m_NotePitchBend.SetPitchBend(
							parser.GetPortNo(),
							parser.GetChNo(),
							parser.GetPitchBendValue(),
							parser.GetPitchBendSensitivity()
						);
	}
	//�I�[���m�[�gOFF�ʒm
	else if (parser.GetMsg() == SMMsgParser::MsgAllNoteOff) {
		m_NoteBoxLive.AllNoteOffOnCh(parser.GetPortNo(), parser.GetChNo());
	}
	
	//EXIT:;
	return result;
}

//******************************************************************************
// �����߂�
//******************************************************************************
int MTScenePianoRollRingLive::Rewind()
{
	int result = 0;
	
	_Reset();
	
	//���_��ݒ�
	SetViewParam(&m_ViewParamMap);
	
	return result;
}

//******************************************************************************
// ���Z�b�g
//******************************************************************************
void MTScenePianoRollRingLive::_Reset()
{
	m_DashboardLive.Reset();
	m_FirstPersonCam.Reset();
	m_TimeIndicator.Reset();
	m_PictBoard.Reset();
	m_NoteBoxLive.Reset();
	m_NoteRipple.Reset();
	m_NotePitchBend.Reset();
}

//******************************************************************************
// �f�t�H���g���_�擾
//******************************************************************************
void MTScenePianoRollRingLive::GetDefaultViewParam(
		MTViewParamMap* pParamMap
	)
{
	D3DXVECTOR3 viewPointVector;
	D3DXVECTOR3 baseVector;
	D3DXVECTOR3 moveVctor;
	float phi = 0.0f;
	float theta= 0.0f;
	
	//�f�t�H���g�̃J�������W
	baseVector = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	
	//���E���W�z�u�ړ��x�N�g���擾
	moveVctor = m_NoteDesign.GetWorldMoveVector();
	
	//���_���쐬
	viewPointVector.x =  baseVector.x + moveVctor.x + 13.0f;
	viewPointVector.y =  baseVector.y + moveVctor.y;
	viewPointVector.z =  baseVector.z + moveVctor.z;
	phi      =  180.0f;	//-X������
	theta    =  90.0f;	//+X������
	
	pParamMap->clear();
	pParamMap->insert(MTViewParamMapPair("X", viewPointVector.x));
	pParamMap->insert(MTViewParamMapPair("Y", viewPointVector.y));
	pParamMap->insert(MTViewParamMapPair("Z", viewPointVector.z));
	pParamMap->insert(MTViewParamMapPair("Phi", phi));
	pParamMap->insert(MTViewParamMapPair("Theta", theta));
	pParamMap->insert(MTViewParamMapPair("ManualRollAngle", 0.0f));
	pParamMap->insert(MTViewParamMapPair("AutoRollVelocity", 0.0f));
	
	return;
}

//******************************************************************************
// ���_�擾
//******************************************************************************
void MTScenePianoRollRingLive::GetViewParam(
		MTViewParamMap* pParamMap
	)
{
	D3DXVECTOR3 viewPointVector;
	float phi = 0.0f;
	float theta = 0.0f;
	float manualRollAngle = 0.0f;
	float autoRollVelocity = 0.0f;
	
	//�J�����̈ʒu�ƕ������擾
	m_FirstPersonCam.GetPosition(&viewPointVector);
	m_FirstPersonCam.GetDirection(&phi, &theta);
	
	//�Đ��ʂɑ΂��鎋�_�ł��邽��X�������͍Đ��ʒu���l������
	viewPointVector.x -= m_TimeIndicator.GetPos();
	
	//��]�p�x���擾
	manualRollAngle = m_FirstPersonCam.GetManualRollAngle();
	if (m_IsAutoRollMode) {
		autoRollVelocity = m_FirstPersonCam.GetAutoRollVelocity();
	}
	
	pParamMap->clear();
	pParamMap->insert(MTViewParamMapPair("X", viewPointVector.x));
	pParamMap->insert(MTViewParamMapPair("Y", viewPointVector.y));
	pParamMap->insert(MTViewParamMapPair("Z", viewPointVector.z));
	pParamMap->insert(MTViewParamMapPair("Phi", phi));
	pParamMap->insert(MTViewParamMapPair("Theta", theta));
	pParamMap->insert(MTViewParamMapPair("ManualRollAngle", manualRollAngle));
	pParamMap->insert(MTViewParamMapPair("AutoRollVelocity", autoRollVelocity));
	
	return;
}

//******************************************************************************
// ���_�o�^
//******************************************************************************
void MTScenePianoRollRingLive::SetViewParam(
		MTViewParamMap* pParamMap
	)
{
	D3DXVECTOR3 viewPointVector = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	float phi = 0.0f;
	float theta = 0.0f;
	float manualRollAngle = 0.0f;
	float autoRollVelocity = 0.0f;
	MTViewParamMap::iterator itr;
	
	itr = pParamMap->find("X");
	if (itr != pParamMap->end()) {
		viewPointVector.x = itr->second;
	}
	itr = pParamMap->find("Y");
	if (itr != pParamMap->end()) {
		viewPointVector.y = itr->second;
	}
	itr = pParamMap->find("Z");
	if (itr != pParamMap->end()) {
		viewPointVector.z = itr->second;
	}
	itr = pParamMap->find("Phi");
	if (itr != pParamMap->end()) {
		phi = itr->second;
	}
	itr = pParamMap->find("Theta");
	if (itr != pParamMap->end()) {
		theta = itr->second;
	}
	itr = pParamMap->find("ManualRollAngle");
	if (itr != pParamMap->end()) {
		manualRollAngle = itr->second;
	}
	itr = pParamMap->find("AutoRollVelocity");
	if (itr != pParamMap->end()) {
		autoRollVelocity = itr->second;
	}
	
	//�Đ��ʂɑ΂��鎋�_�ł��邽��X�������͍Đ��ʒu���l������
	viewPointVector.x += m_TimeIndicator.GetPos();
	
	//�J�����̈ʒu�ƕ�����ݒ�
	m_FirstPersonCam.SetPosition(viewPointVector);
	m_FirstPersonCam.SetDirection(phi, theta);
	
	//�蓮��]�p�x��ݒ�
	m_FirstPersonCam.SetManualRollAngle(manualRollAngle);
	
	//������]���x��ݒ�
	m_IsAutoRollMode = false;
	if (autoRollVelocity != 0.0f) {
		m_IsAutoRollMode = true;
		m_FirstPersonCam.SetAutoRollVelocity(autoRollVelocity);
	}
	m_FirstPersonCam.SetAutoRollMode(m_IsAutoRollMode);
	
	//�p�����[�^�̕ۑ�
	if (pParamMap != (&m_ViewParamMap)) {
		m_ViewParamMap.clear();
		for (itr = pParamMap->begin(); itr != pParamMap->end(); itr++) {
			m_ViewParamMap.insert(MTViewParamMapPair(itr->first, itr->second));
		}
	}
	
	return;
}

//******************************************************************************
// �ÓI���_�ړ�
//******************************************************************************
void MTScenePianoRollRingLive::MoveToStaticViewpoint(
		unsigned long viewpointNo
	)
{
	MTScene::MTViewParamMap::iterator itr;
	MTViewParamMap paramMap;

	if (viewpointNo == 1) {
		GetDefaultViewParam(&paramMap);
		SetViewParam(&paramMap);
	}
	else if (viewpointNo == 2) {
		SetViewParam(&m_Viewpoint2);
	}
	else if (viewpointNo == 3) {
		SetViewParam(&m_Viewpoint3);
	}
	else {
		GetDefaultViewParam(&paramMap);
		SetViewParam(&paramMap);
	}

	return;
}

//******************************************************************************
// ���_���Z�b�g
//******************************************************************************
void MTScenePianoRollRingLive::ResetViewpoint()
{
	MTViewParamMap paramMap;
	
	//�f�t�H���g���_���擾
	GetDefaultViewParam(&paramMap);
	
	//���_�o�^
	SetViewParam(&paramMap);
}

//******************************************************************************
// �\�����ʐݒ�
//******************************************************************************
void MTScenePianoRollRingLive::SetEffect(
		MTScene::EffectType type,
		bool isEnable
	)
{
	switch (type) {
		case EffectPianoKeyboard:
			m_PictBoard.SetEnable(isEnable);
			break;
		case EffectRipple:
			m_NoteRipple.SetEnable(isEnable);
			break;
		case EffectPitchBend:
			m_NotePitchBend.SetEnable(isEnable);
			break;
		case EffectStars:
			m_Stars.SetEnable(isEnable);
			break;
		case EffectCounter:
			m_DashboardLive.SetEnable(isEnable);
			break;
		case EffectBackgroundImage:
			m_BackgroundImage.SetEnable(isEnable);
			break;
		case EffectGridLine:
			m_GridRingLive.SetEnable(isEnable);
			break;
		case EffectTimeIndicator:
			m_TimeIndicator.SetEnable(isEnable);
			break;
		default:
			break;
	}
	
	return;
}

//******************************************************************************
// �ݒ�t�@�C���ǂݍ���
//******************************************************************************
int MTScenePianoRollRingLive::_LoadConf()
{
	int result = 0;
	TCHAR hexColor[16] = {_T('\0')};
	MTConfFile confFile;

	result = confFile.Initialize(GetName());
	if (result != 0) goto EXIT;

	result = confFile.SetCurSection(_T("Color"));
	if (result != 0) goto EXIT;

	result = confFile.GetStr(_T("BackGroundRGB"), hexColor, 16, _T("000000"));
	if (result != 0) goto EXIT;

	SetBGColor(DXColorUtil::MakeColorFromHexRGBA(hexColor));

	result = _LoadConfViewpoint(&confFile, 2, &m_Viewpoint2);
	if (result != 0) goto EXIT;

	result = _LoadConfViewpoint(&confFile, 3, &m_Viewpoint3);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �ݒ�t�@�C���ǂݍ��݁F���_
//******************************************************************************
int MTScenePianoRollRingLive::_LoadConfViewpoint(
		MTConfFile* pConfFile,
		unsigned long viewpointNo,
		MTViewParamMap* pParamMap
	)
{
	int result = 0;
	int eresult = 0;
	TCHAR sectionStr[32] = {0};
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float phi = 0.0f;
	float theta = 0.0f;
	float manualRollAngle = 0.0f;
	float autoRollVelocity = 0.0f;

	//�Z�N�V�������쐬
	eresult = _stprintf_s(sectionStr, 32, _T("Viewpoint-%d"), viewpointNo);
	if (eresult < 0) {
		result = YN_SET_ERR("Program error.", viewpointNo, 0);
		goto EXIT;
	}

	//�Z�N�V�����ݒ�
	result = pConfFile->SetCurSection(sectionStr);
	if (result != 0) goto EXIT;

	//�p�����[�^�擾
	result = pConfFile->GetFloat(_T("X"), &x, 0.0f);
	if (result != 0) goto EXIT;
	result = pConfFile->GetFloat(_T("Y"), &y, 0.0f);
	if (result != 0) goto EXIT;
	result = pConfFile->GetFloat(_T("Z"), &z, 0.0f);
	if (result != 0) goto EXIT;
	result = pConfFile->GetFloat(_T("Phi"), &phi, 0.0f);
	if (result != 0) goto EXIT;
	result = pConfFile->GetFloat(_T("Theta"), &theta, 0.0f);
	if (result != 0) goto EXIT;
	result = pConfFile->GetFloat(_T("ManualRollAngle"), &manualRollAngle, 0.0f);
	if (result != 0) goto EXIT;
	result = pConfFile->GetFloat(_T("AutoRollVelocity"), &autoRollVelocity, 0.0f);
	if (result != 0) goto EXIT;

	//�}�b�v�o�^
	pParamMap->clear();
	pParamMap->insert(MTViewParamMapPair("X", x));
	pParamMap->insert(MTViewParamMapPair("Y", y));
	pParamMap->insert(MTViewParamMapPair("Z", z));
	pParamMap->insert(MTViewParamMapPair("Phi", phi));
	pParamMap->insert(MTViewParamMapPair("Theta", theta));
	pParamMap->insert(MTViewParamMapPair("ManualRollAngle", manualRollAngle));
	pParamMap->insert(MTViewParamMapPair("AutoRollVelocity", autoRollVelocity));

EXIT:;
	return result;
}

