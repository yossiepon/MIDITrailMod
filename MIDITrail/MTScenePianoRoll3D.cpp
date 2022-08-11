//******************************************************************************
//
// MIDITrail / MTScenePianoRoll3D
//
// �s�A�m���[��3D�V�[���`��N���X
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "stdafx.h"
#include <windows.h>
#include <mmsystem.h>
#include "Commdlg.h"
#include "YNBaseLib.h"
#include "DXColorUtil.h"
#include "MTConfFile.h"
#include "MTScenePianoRoll3D.h"

using namespace YNBaseLib;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTScenePianoRoll3D::MTScenePianoRoll3D()
{
	m_IsEnableLight = TRUE;
	m_IsMouseCamMode = false;
	m_IsAutoRollMode = false;
	m_IsSkipping = false;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTScenePianoRoll3D::~MTScenePianoRoll3D()
{
	Release();
}

//******************************************************************************
// ���̎擾
//******************************************************************************
const TCHAR* MTScenePianoRoll3D::GetName()
{
	return _T("PianoRoll3D");
}

//******************************************************************************
// �V�[������
//******************************************************************************
int MTScenePianoRoll3D::Create(
		HWND hWnd,
		LPDIRECT3DDEVICE9 pD3DDevice,
		SMSeqData* pSeqData
	)
{
	int result = 0;

	Release();

	if (pD3DDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
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

	//���C�g�F
	_SetLightColor(&m_DirLight);

	//���C�g����
	m_DirLight.SetDirection(D3DXVECTOR3(1.0f, -1.0f, 2.0f));

	//���C�g�̃f�o�C�X�o�^
	result = m_DirLight.SetDevice(pD3DDevice, 0, m_IsEnableLight);
	if (result != 0) goto EXIT;

	//----------------------------------
	// ���C�g2
	//----------------------------------
	//���C�g������
	result = m_DirLight2.Initialize();
	if (result != 0) goto EXIT;

	//���C�g�F
	_SetLightColor2(&m_DirLight2);

	//���C�g����
	m_DirLight2.SetDirection(D3DXVECTOR3(-1.0f, 1.0f, -2.0f));

	//���C�g�̃f�o�C�X�o�^
	result = m_DirLight2.SetDevice(pD3DDevice, 1, m_IsEnableLight);
	if (result != 0) goto EXIT;

	//----------------------------------
	// �`��I�u�W�F�N�g
	//----------------------------------
	//�s�b�`�x���h��񏉊���
	result = m_NotePitchBend.Initialize();
	if (result != 0) goto EXIT;

	//�m�[�g�{�b�N�X����
	result = m_NoteBox.Create(pD3DDevice, GetName(), pSeqData, &m_NotePitchBend);
	if (result != 0) goto EXIT;

	//�m�[�g�g�䐶��
	result = m_NoteRipple.Create(pD3DDevice, GetName(), pSeqData, &m_NotePitchBend);
	if (result != 0) goto EXIT;

	//�O���b�h�{�b�N�X����
	result = m_GridBox.Create(pD3DDevice, GetName(), pSeqData);
	if (result != 0) goto EXIT;
	
	//�s�N�`���{�[�h����
	result = m_PictBoard.Create(pD3DDevice, GetName(), pSeqData);
	if (result != 0) goto EXIT;

	//�_�b�V���{�[�h����
	result = m_Dashboard.Create(pD3DDevice, GetName(), pSeqData, hWnd);
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
int MTScenePianoRoll3D::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	float rollAngle = 0.0f;
	D3DXVECTOR3 camVector;

	if (pD3DDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
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
	result = m_NoteBox.Transform(pD3DDevice, rollAngle);
	if (result != 0) goto EXIT;

	//�O���b�h�{�b�N�X�X�V
	result = m_GridBox.Transform(pD3DDevice, rollAngle);
	if (result != 0) goto EXIT;

	//�s�N�`���{�[�h�X�V
	result = m_PictBoard.Transform(pD3DDevice, camVector, rollAngle);
	if (result != 0) goto EXIT;

	//�_�b�V���{�[�h�X�V
	result = m_Dashboard.Transform(pD3DDevice, camVector);
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
int MTScenePianoRoll3D::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;

	if (pD3DDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//�X�V
	result = Transform(pD3DDevice);
	if (result != 0) goto EXIT;

	//�w�i�摜�`��
	result = m_BackgroundImage.Draw(pD3DDevice);
	if (result != 0) goto EXIT;

	//�O���b�h�{�b�N�X�`��
	result = m_GridBox.Draw(pD3DDevice);
	if (result != 0) goto EXIT;

	//�m�[�g�{�b�N�X�`��
	result = m_NoteBox.Draw(pD3DDevice);
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

	//���C�g���ꎞ�I�ɖ����ɂ���
	//  �m�[�g�g��ƃ_�b�V���{�[�h�̕`��F�̓��C�g�̕����Ɉˑ������Ȃ�����
	result = m_DirLight.SetDevice(pD3DDevice, 0, FALSE);
	if (result != 0) goto EXIT;
	result = m_DirLight2.SetDevice(pD3DDevice, 1, FALSE);
	if (result != 0) goto EXIT;

	//�m�[�g�g��`��
	result = m_NoteRipple.Draw(pD3DDevice);
	if (result != 0) goto EXIT;

	//�_�b�V���{�[�h�`��F���W�ϊ��ςݒ��_��p���邽�߈�ԍŌ�ɕ`�悷��
	result = m_Dashboard.Draw(pD3DDevice);
	if (result != 0) goto EXIT;

	//���C�g��߂�
	result = m_DirLight.SetDevice(pD3DDevice, 0, m_IsEnableLight);
	if (result != 0) goto EXIT;
	result = m_DirLight2.SetDevice(pD3DDevice, 1, m_IsEnableLight);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �j��
//******************************************************************************
void MTScenePianoRoll3D::Release()
{
	m_NoteBox.Release();
	m_GridBox.Release();
	m_PictBoard.Release();
	m_Dashboard.Release();
	m_Stars.Release();
	m_TimeIndicator.Release();
	m_NoteRipple.Release();
	m_MeshCtrl.Release();
	m_BackgroundImage.Release();
}

//******************************************************************************
// �E�B���h�E�N���b�N�C�x���g��M
//******************************************************************************
int MTScenePianoRoll3D::OnWindowClicked(
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
int MTScenePianoRoll3D::OnPlayStart(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;

	_Reset();

	m_PictBoard.OnPlayStart();

	return result;
}

//******************************************************************************
// ���t�I���C�x���g��M
//******************************************************************************
int MTScenePianoRoll3D::OnPlayEnd(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;

	m_PictBoard.OnPlayEnd();

	return result;
}

//******************************************************************************
// �V�[�P���T���b�Z�[�W��M
//******************************************************************************
int MTScenePianoRoll3D::OnRecvSequencerMsg(
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
	//���t�`�b�N�^�C���ʒm
	else if (parser.GetMsg() == SMMsgParser::MsgPlayTime) {
		m_Dashboard.SetPlayTimeSec(parser.GetPlayTimeSec());
		m_FirstPersonCam.SetCurTickTime(parser.GetPlayTickTime());
		m_TimeIndicator.SetCurTickTime(parser.GetPlayTickTime());
		m_NoteRipple.SetCurTickTime(parser.GetPlayTickTime());
		m_PictBoard.SetCurTickTime(parser.GetPlayTickTime());
		m_NoteBox.SetCurTickTime(parser.GetPlayTickTime());
	}
	//�e���|�ύX�ʒm
	else if (parser.GetMsg() == SMMsgParser::MsgTempo) {
		m_Dashboard.SetTempoBPM(parser.GetTempoBPM());
	}
	//���ߔԍ��ʒm
	else if (parser.GetMsg() == SMMsgParser::MsgBar) {
		m_Dashboard.SetBarNo(parser.GetBarNo());
	}
	//���q�L���ύX�ʒm
	else if (parser.GetMsg() == SMMsgParser::MsgBeat) {
		m_Dashboard.SetBeat(parser.GetBeatNumerator(), parser.GetBeatDenominator());
	}
	//�m�[�gOFF�ʒm
	else if (parser.GetMsg() == SMMsgParser::MsgNoteOff) {
		m_NoteRipple.SetNoteOff(parser.GetPortNo(), parser.GetChNo(), parser.GetNoteNo());
	}
	//�m�[�gON�ʒm
	else if (parser.GetMsg() == SMMsgParser::MsgNoteOn) {
		m_Dashboard.SetNoteOn();
		m_NoteRipple.SetNoteOn(parser.GetPortNo(), parser.GetChNo(), parser.GetNoteNo(), parser.GetVelocity());
	}
	//�s�b�`�x���h�ʒm
	else if (parser.GetMsg() == SMMsgParser::MsgPitchBend) {
		m_NotePitchBend.SetPitchBend(parser.GetPortNo(), parser.GetChNo(), parser.GetPitchBendValue(), parser.GetPitchBendSensitivity());
	}
	//�X�L�b�v�J�n�ʒm
	else if (parser.GetMsg() == SMMsgParser::MsgSkipStart) {
		if (parser.GetSkipStartDirection() == SMMsgParser::SkipBack) {
			m_NotePitchBend.Reset();
		}
		m_NoteBox.Reset();
		m_NoteBox.SetSkipStatus(true);
		m_NoteRipple.Reset();
		m_NoteRipple.SetSkipStatus(true);
		m_IsSkipping = true;
	}
	//�X�L�b�v�I���ʒm
	else if (parser.GetMsg() == SMMsgParser::MsgSkipEnd) {
		m_Dashboard.SetNotesCount(parser.GetSkipEndNotesCount());
		m_NoteBox.SetSkipStatus(false);
		m_NoteRipple.SetSkipStatus(false);
		m_IsSkipping = false;
	}

//EXIT:;
	return result;
}

//******************************************************************************
// �����߂�
//******************************************************************************
int MTScenePianoRoll3D::Rewind()
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
void MTScenePianoRoll3D::_Reset()
{
	m_Dashboard.Reset();
	m_FirstPersonCam.Reset();
	m_TimeIndicator.Reset();
	m_PictBoard.Reset();
	m_NoteBox.Reset();
	m_NoteRipple.Reset();
	m_NotePitchBend.Reset();
}

//******************************************************************************
// �f�t�H���g���_�擾
//******************************************************************************
void MTScenePianoRoll3D::GetDefaultViewParam(
		MTViewParamMap* pParamMap
	)
{
	D3DXVECTOR3 viewPointVector;
	D3DXVECTOR3 e4Vector;
	D3DXVECTOR3 moveVctor;
	float phi = 0.0f;
	float theta = 0.0f;

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
void MTScenePianoRoll3D::GetViewParam(
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
void MTScenePianoRoll3D::SetViewParam(
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
void MTScenePianoRoll3D::MoveToStaticViewpoint(
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
void MTScenePianoRoll3D::ResetViewpoint()
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
void MTScenePianoRoll3D::SetEffect(
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
			m_Dashboard.SetEnable(isEnable);
			break;
		case EffectBackgroundImage:
			m_BackgroundImage.SetEnable(isEnable);
			break;
		case EffectGridLine:
			m_GridBox.SetEnable(isEnable);
			break;
		case EffectTimeIndicator:
			m_TimeIndicator.SetEnable(isEnable);
			break;
		case EffectFileName:
			m_Dashboard.SetEnableFileName(isEnable);
			break;
		default:
			break;
	}

	return;
}

//******************************************************************************
// ���t���x�ݒ�
//******************************************************************************
void MTScenePianoRoll3D::SetPlaySpeedRatio(
		unsigned long ratio
	)
{
	m_Dashboard.SetPlaySpeedRatio(ratio);
}

//******************************************************************************
// ���C�g�F�ݒ�
//******************************************************************************
void MTScenePianoRoll3D::_SetLightColor(
		DXDirLight* pLight
	)
{
	D3DXCOLOR diffuse;
	D3DXCOLOR specular;
	D3DXCOLOR ambient;

	//�g�U��
	diffuse.r = 1.2f;
	diffuse.g = 1.2f;
	diffuse.b = 1.2f;
	diffuse.a = 1.0f;
	//���ʔ��ˌ�
	specular.r = 0.0f;
	specular.g = 0.0f;
	specular.b = 0.0f;
	specular.a = 0.0f;
	//����
	ambient.r = 0.2f;
	ambient.g = 0.2f;
	ambient.b = 0.2f;
	ambient.a = 1.0f;

	pLight->SetColor(diffuse, specular, ambient);

	return;
}

//******************************************************************************
// ���C�g2�F�ݒ�
//******************************************************************************
void MTScenePianoRoll3D::_SetLightColor2(
		DXDirLight* pLight
	)
{
	D3DXCOLOR diffuse;
	D3DXCOLOR specular;
	D3DXCOLOR ambient;

	//�g�U��
	diffuse.r = 1.2f;
	diffuse.g = 1.2f;
	diffuse.b = 1.2f;
	diffuse.a = 1.0f;
	//���ʔ��ˌ�
	specular.r = 0.0f;
	specular.g = 0.0f;
	specular.b = 0.0f;
	specular.a = 0.0f;
	//����
	ambient.r = 0.0f;
	ambient.g = 0.0f;
	ambient.b = 0.0f;
	ambient.a = 0.0f;

	pLight->SetColor(diffuse, specular, ambient);

	return;
}

//******************************************************************************
// �ݒ�t�@�C���ǂݍ���
//******************************************************************************
int MTScenePianoRoll3D::_LoadConf()
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
int MTScenePianoRoll3D::_LoadConfViewpoint(
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

