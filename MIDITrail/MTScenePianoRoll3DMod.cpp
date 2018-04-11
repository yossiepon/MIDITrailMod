//******************************************************************************
//
// MIDITrail / MTScenePianoRoll3DMod
//
// �s�A�m���[��3D�V�[���`��Mod�N���X
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "stdafx.h"
#include "YNBaseLib.h"
#include "MTScenePianoRoll3DMod.h"

using namespace YNBaseLib;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTScenePianoRoll3DMod::MTScenePianoRoll3DMod()
{
	m_IsSingleKeyboard = false;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTScenePianoRoll3DMod::~MTScenePianoRoll3DMod()
{
	Release();
}

//******************************************************************************
// �V�[������
//******************************************************************************
int MTScenePianoRoll3DMod::Create(
		HWND hWnd,
		LPDIRECT3DDEVICE9 pD3DDevice,
		SMSeqData* pSeqData
	)
{
	int result = 0;

	Release();

	//���N���X�̃V�[�������������Ăяo��
	result = MTScenePianoRoll3D::Create(hWnd, pD3DDevice, pSeqData);
	if (result != 0) goto EXIT;

	//----------------------------------
	// ���C�g2
	//----------------------------------
	//���C�g2������
	result = m_DirLightBack.Initialize();
	if (result != 0) goto EXIT;

	//���C�g2�F
	_SetLightColor(&m_DirLightBack);

	//���C�g2����
	m_DirLightBack.SetDirection(D3DXVECTOR3(-1.0f, 1.0f, -2.0f));

	//���C�g�̃f�o�C�X�o�^
	result = m_DirLightBack.SetDevice(pD3DDevice, 1, m_IsEnableLight);
	if (result != 0) goto EXIT;

	//----------------------------------
	// �`��I�u�W�F�N�g
	//----------------------------------

	//�O���b�h�{�b�N�X����
	result = m_GridBoxMod.Create(pD3DDevice, GetName(), pSeqData);
	if (result != 0) goto EXIT;
	
	//�m�[�g�{�b�N�X����
	result = m_NoteBoxMod.Create(pD3DDevice, GetName(), pSeqData, &m_NotePitchBend);
	if (result != 0) goto EXIT;

	//�m�[�g�g�䐶��
	result = m_NoteRippleMod.Create(pD3DDevice, GetName(), pSeqData, &m_NotePitchBend);
	if (result != 0) goto EXIT;

	//�m�[�g�̎�����
	result = m_NoteLyrics.Create(pD3DDevice, GetName(), pSeqData, &m_NotePitchBend);
	if (result != 0) goto EXIT;

	//�s�N�`���{�[�h����
	m_PictBoard.SetEnable(false);

	//�s�A�m�L�[�{�[�h����
	result = m_PianoKeyboardCtrlMod.Create(pD3DDevice, GetName(), pSeqData, &m_NotePitchBend, m_IsSingleKeyboard);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �ϊ�����
//******************************************************************************
int MTScenePianoRoll3DMod::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	float rollAngle = 0.0f;
	D3DXVECTOR3 camVector;

	//���N���X�̕ϊ��������Ăяo��
	result = MTScenePianoRoll3D::Transform(pD3DDevice);
	if (result != 0) goto EXIT;

	//�J�������W�擾
	m_FirstPersonCam.GetPosition(&camVector);

	//��]�p�x�擾
	rollAngle = m_FirstPersonCam.GetManualRollAngle();

	//�O���b�h�{�b�N�X�X�V
	result = m_GridBoxMod.Transform(pD3DDevice, rollAngle);
	if (result != 0) goto EXIT;

	//�m�[�g�{�b�N�X�X�V
	result = m_NoteBoxMod.Transform(pD3DDevice, rollAngle);
	if (result != 0) goto EXIT;

	//�m�[�g�g��X�V
	result = m_NoteRippleMod.Transform(pD3DDevice, camVector, rollAngle);
	if (result != 0) goto EXIT;

	//�m�[�g�̎��X�V
	result = m_NoteLyrics.Transform(pD3DDevice, camVector, rollAngle);
	if (result != 0) goto EXIT;

	//�s�A�m�L�[�{�[�h�X�V
	result = m_PianoKeyboardCtrlMod.Transform(pD3DDevice, rollAngle);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �`��
//******************************************************************************
int MTScenePianoRoll3DMod::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	D3DXVECTOR3 camVector;

	if (pD3DDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//�X�V
	result = Transform(pD3DDevice);
	if (result != 0) goto EXIT;

	//�J�������W�擾
	m_FirstPersonCam.GetPosition(&camVector);

	//�w�i�摜�`��
	result = m_BackgroundImage.Draw(pD3DDevice);
	if (result != 0) goto EXIT;

	//���`��
	result = m_Stars.Draw(pD3DDevice);
	if (result != 0) goto EXIT;

	//�O���b�h�{�b�N�X�`��
	result = m_GridBoxMod.Draw(pD3DDevice);
	if (result != 0) goto EXIT;

	//�m�[�g�{�b�N�X�`��
	result = m_NoteBoxMod.Draw(pD3DDevice);
	if (result != 0) goto EXIT;

	// �J�����ʒu�����t�ʒu����O���ł����
	if(m_TimeIndicator.GetPos() > camVector.x) {

		//���b�V�����^�C���C���W�P�[�^���̎����g�䁄�L�[�{�[�h�̏��ŉ�����`��

		//���b�V���`��
		result = m_MeshCtrl.Draw(pD3DDevice);
		if (result != 0) goto EXIT;

		//�^�C���C���W�P�[�^�`��
		result = m_TimeIndicator.Draw(pD3DDevice);
		if (result != 0) goto EXIT;

		//�m�[�g�̎��`��
		result = m_NoteLyrics.Draw(pD3DDevice);
		if (result != 0) goto EXIT;

		//�m�[�g�g��`��
		result = m_NoteRippleMod.Draw(pD3DDevice);
		if (result != 0) goto EXIT;

		//�s�A�m�L�[�{�[�h�`��
		result = m_PianoKeyboardCtrlMod.Draw(pD3DDevice);
		if (result != 0) goto EXIT;

	} else {

		//�L�[�{�[�h���g�䁄�̎����^�C���C���W�P�[�^�����b�V���̏��ŉ�����`��

		//�s�A�m�L�[�{�[�h�`��
		result = m_PianoKeyboardCtrlMod.Draw(pD3DDevice);
		if (result != 0) goto EXIT;

		//�m�[�g�g��`��
		result = m_NoteRippleMod.Draw(pD3DDevice);
		if (result != 0) goto EXIT;

		//�m�[�g�̎��`��
		result = m_NoteLyrics.Draw(pD3DDevice);
		if (result != 0) goto EXIT;

		//�^�C���C���W�P�[�^�`��
		result = m_TimeIndicator.Draw(pD3DDevice);
		if (result != 0) goto EXIT;

		//���b�V���`��
		result = m_MeshCtrl.Draw(pD3DDevice);
		if (result != 0) goto EXIT;

	}

	//�_�b�V���{�[�h�`��F���W�ϊ��ςݒ��_��p���邽�߈�ԍŌ�ɕ`�悷��
	result = m_Dashboard.Draw(pD3DDevice);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �j��
//******************************************************************************
void MTScenePianoRoll3DMod::Release()
{
	m_NoteBoxMod.Release();
	m_NoteRippleMod.Release();
	m_NoteLyrics.Release();
	m_PianoKeyboardCtrlMod.Release();

	MTScenePianoRoll3D::Release();
}

//******************************************************************************
// �V�[�P���T���b�Z�[�W��M
//******************************************************************************
int MTScenePianoRoll3DMod::OnRecvSequencerMsg(
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
		m_Dashboard.SetPlayTimeMSec(parser.GetPlayTimeMSec());
		m_FirstPersonCam.SetCurTickTime(parser.GetPlayTickTime());
		m_TimeIndicator.SetCurTickTime(parser.GetPlayTickTime());
		m_NoteRippleMod.SetPlayTimeMSec(parser.GetPlayTimeMSec());
		m_NoteRippleMod.SetCurTickTime(parser.GetPlayTickTime());
		m_PictBoard.SetCurTickTime(parser.GetPlayTickTime());
		m_NoteBoxMod.SetPlayTimeMSec(parser.GetPlayTimeMSec());
		m_NoteBoxMod.SetCurTickTime(parser.GetPlayTickTime());
		m_NoteLyrics.SetPlayTimeMSec(parser.GetPlayTimeMSec());
		m_NoteLyrics.SetCurTickTime(parser.GetPlayTickTime());
		m_PianoKeyboardCtrlMod.SetPlayTimeMSec(parser.GetPlayTimeMSec());
		m_PianoKeyboardCtrlMod.SetCurTickTime(parser.GetPlayTickTime());
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
		// NOP
	}
	//�m�[�gON�ʒm
	else if (parser.GetMsg() == SMMsgParser::MsgNoteOn) {
		m_Dashboard.SetNoteOn();
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
		m_NoteBoxMod.Reset();
		m_NoteBoxMod.SetSkipStatus(true);
		m_NoteRippleMod.Reset();
		m_NoteRippleMod.SetSkipStatus(true);
		m_NoteLyrics.Reset();
		m_NoteLyrics.SetSkipStatus(true);
		m_PianoKeyboardCtrlMod.Reset();
		m_PianoKeyboardCtrlMod.SetSkipStatus(true);
		m_IsSkipping = true;
	}
	//�X�L�b�v�I���ʒm
	else if (parser.GetMsg() == SMMsgParser::MsgSkipEnd) {
		m_Dashboard.SetNotesCount(parser.GetSkipEndNotesCount());
		m_NoteBoxMod.SetSkipStatus(false);
		m_NoteRippleMod.SetSkipStatus(false);
		m_NoteLyrics.SetSkipStatus(false);
		m_PianoKeyboardCtrlMod.SetSkipStatus(false);
		m_IsSkipping = false;
	}

//EXIT:;
	return result;
}

//******************************************************************************
// ���Z�b�g
//******************************************************************************
void MTScenePianoRoll3DMod::_Reset()
{
	MTScenePianoRoll3D::_Reset();

	m_NoteBoxMod.Reset();
	m_NoteRippleMod.Reset();
	m_NoteLyrics.Reset();
	m_PianoKeyboardCtrlMod.Reset();
}

//******************************************************************************
// �\�����ʐݒ�
//******************************************************************************
void MTScenePianoRoll3DMod::SetEffect(
		MTScene::EffectType type,
		bool isEnable
	)
{
	switch (type) {
		case EffectPianoKeyboard:
			m_PianoKeyboardCtrlMod.SetEnable(isEnable);
			break;
		case EffectRipple:
			m_NoteRippleMod.SetEnable(isEnable);
			m_NoteLyrics.SetEnable(isEnable);
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
		case EffectFileName:
			m_Dashboard.SetEnableFileName(isEnable);
			break;
		case EffectBackgroundImage:
			m_BackgroundImage.SetEnable(isEnable);
			break;
		default:
			break;
	}

	return;
}
