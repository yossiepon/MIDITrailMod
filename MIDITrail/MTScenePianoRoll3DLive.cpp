//******************************************************************************
//
// MIDITrail / MTScenePianoRoll3DLiveLive
//
// ���C�u���j�^�p�s�A�m���[��3D�V�[���`��N���X
//
// Copyright (C) 2012-2014 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "stdafx.h"
#include <windows.h>
#include <mmsystem.h>
#include "YNBaseLib.h"
#include "DXColorUtil.h"
#include "MTConfFile.h"
#include "MTScenePianoRoll3DLive.h"

using namespace YNBaseLib;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTScenePianoRoll3DLive::MTScenePianoRoll3DLive()
{
	m_IsEnableLight = true;
	m_IsMouseCamMode = false;
	m_IsAutoRollMode = false;
	m_IsSkipping = false;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTScenePianoRoll3DLive::~MTScenePianoRoll3DLive()
{
	Release();
}

//******************************************************************************
// ���̎擾
//******************************************************************************
const TCHAR* MTScenePianoRoll3DLive::GetName()
{
	return _T("PianoRoll3DLive");
}

//******************************************************************************
// �V�[������
//******************************************************************************
int MTScenePianoRoll3DLive::Create(
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
	//���C�g������
	result = m_DirLight.Initialize();
	if (result != 0) goto EXIT;
	
	//���C�g����
	//  ���_�������Ƃ��Ă��̕������x�N�g���ŕ\������
	m_DirLight.SetDirection(D3DXVECTOR3(1.0f, -1.0f, 2.0f));
	
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
	
	//�O���b�h�{�b�N�X����
	result = m_GridBoxLive.Create(pD3DDevice, GetName());
	if (result != 0) goto EXIT;
	
	//�s�N�`���{�[�h����
	result = m_PictBoard.Create(pD3DDevice, GetName(), pSeqData);
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
int MTScenePianoRoll3DLive::Transform(
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
	
	//�O���b�h�{�b�N�X�X�V
	result = m_GridBoxLive.Transform(pD3DDevice, rollAngle);
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
int MTScenePianoRoll3DLive::Draw(
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
	
	//�O���b�h�{�b�N�X�`��
	result = m_GridBoxLive.Draw(pD3DDevice);
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
void MTScenePianoRoll3DLive::Release()
{
	m_NoteBoxLive.Release();
	m_GridBoxLive.Release();
	m_PictBoard.Release();
	m_DashboardLive.Release();
	m_Stars.Release();
	m_TimeIndicator.Release();
	m_NoteRipple.Release();
	m_MeshCtrl.Release();
}

//******************************************************************************
// �E�B���h�E�N���b�N�C�x���g��M
//******************************************************************************
int MTScenePianoRoll3DLive::OnWindowClicked(
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
int MTScenePianoRoll3DLive::OnPlayStart(
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
int MTScenePianoRoll3DLive::OnPlayEnd(
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
int MTScenePianoRoll3DLive::OnRecvSequencerMsg(
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
int MTScenePianoRoll3DLive::Rewind()
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
void MTScenePianoRoll3DLive::_Reset()
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
void MTScenePianoRoll3DLive::GetDefaultViewParam(
		MTViewParamMap* pParamMap
	)
{
	D3DXVECTOR3 viewPointVector;
	D3DXVECTOR3 e4Vector;
	D3DXVECTOR3 moveVctor;
	float phi, theta = 0.0f;
	
	//�f�t�H���g�̃J����Y���W�i�����j��E4�̈ʒu�Ƃ���
	e4Vector = m_NoteDesign.GetNoteBoxCenterPosX(
					0,		//���ݎ���
					0,		//�|�[�g�ԍ�
					0,		//�`�����l���ԍ�
					64		//�m�[�g�ԍ��FE4
				);
	
	//���E���W�z�u�ړ��x�N�g���擾
	moveVctor = m_NoteDesign.GetWorldMoveVector();
	
	//���_���쐬
	viewPointVector.x =  e4Vector.x + moveVctor.x;
	viewPointVector.y =  e4Vector.y + moveVctor.y;
	viewPointVector.z =  e4Vector.z + moveVctor.z - 18.0f;
	phi      =  90.0f;	//+Z������
	theta    =  90.0f;	//+Z������
	
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
void MTScenePianoRoll3DLive::GetViewParam(
		MTViewParamMap* pParamMap
	)
{
	D3DXVECTOR3 viewPointVector;
	float phi, theta = 0.0f;
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
void MTScenePianoRoll3DLive::SetViewParam(
		MTViewParamMap* pParamMap
	)
{
	D3DXVECTOR3 viewPointVector = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	float phi, theta = 0.0f;
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
// ���_���Z�b�g
//******************************************************************************
void MTScenePianoRoll3DLive::ResetViewpoint()
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
void MTScenePianoRoll3DLive::SetEffect(
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
		default:
			break;
	}
	
	return;
}

//******************************************************************************
// �ݒ�t�@�C���ǂݍ���
//******************************************************************************
int MTScenePianoRoll3DLive::_LoadConf()
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

EXIT:;
	return result;
}

