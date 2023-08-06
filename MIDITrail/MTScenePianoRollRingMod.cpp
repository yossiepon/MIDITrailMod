//******************************************************************************
//
// MIDITrail / MTScenePianoRollRingMod
//
// �s�A�m���[�������O�V�[���`��Mod�N���X
//
// Copyright (C) 2019 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "stdafx.h"
#include <windows.h>
#include <mmsystem.h>
#include "Commdlg.h"
#include "YNBaseLib.h"
#include "DXColorUtil.h"
#include "MTConfFile.h"
#include "MTScenePianoRollRingMod.h"

using namespace YNBaseLib;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTScenePianoRollRingMod::MTScenePianoRollRingMod()
{
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTScenePianoRollRingMod::~MTScenePianoRollRingMod()
{
	Release();
}

//******************************************************************************
// ���̎擾
//******************************************************************************
const TCHAR* MTScenePianoRollRingMod::GetName()
{
	return _T("PianoRollRing");
}

//******************************************************************************
// �V�[������
//******************************************************************************
int MTScenePianoRollRingMod::Create(
		HWND hWnd,
		LPDIRECT3DDEVICE9 pD3DDevice,
		SMSeqData* pSeqData
	)
{
	int result = 0;

	Release();

	//���N���X�̃V�[�������������Ăяo��
	result = MTScenePianoRollRing::Create(hWnd, pD3DDevice, pSeqData);
	if (result != 0) goto EXIT;

	//----------------------------------
	// �`��I�u�W�F�N�g
	//----------------------------------
	//�m�[�g�g�䐶��
	result = m_NoteRippleMod.Create(pD3DDevice, GetName(), pSeqData, &m_NotePitchBend);
	if (result != 0) goto EXIT;

	//�O���b�h�����O����
	result = m_GridRingMod.Create(pD3DDevice, GetName(), pSeqData);
	if (result != 0) goto EXIT;

	//�^�C���C���W�P�[�^����
	result = m_TimeIndicatorMod.Create(pD3DDevice, GetName(), pSeqData);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �ϊ�����
//******************************************************************************
int MTScenePianoRollRingMod::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	float rollAngle = 0.0f;
	D3DXVECTOR3 camVector;

	//���N���X�̕ϊ��������Ăяo��
	result = MTScenePianoRollRing::Transform(pD3DDevice);
	if (result != 0) goto EXIT;

	//�J�������W�擾
	m_FirstPersonCam.GetPosition(&camVector);

	//��]�p�x�擾
	rollAngle = m_FirstPersonCam.GetManualRollAngle();

	//�O���b�h�����O�X�V
	result = m_GridRingMod.Transform(pD3DDevice, rollAngle);
	if (result != 0) goto EXIT;

	//�^�C���C���W�P�[�^�X�V
	result = m_TimeIndicatorMod.Transform(pD3DDevice, camVector, rollAngle);
	if (result != 0) goto EXIT;

	//�m�[�g�g��X�V
	result = m_NoteRippleMod.Transform(pD3DDevice, camVector, rollAngle);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �`��
//******************************************************************************
int MTScenePianoRollRingMod::Draw(
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

	//�O���b�h�����O�`��
	result = m_GridRingMod.Draw(pD3DDevice);
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
	result = m_TimeIndicatorMod.Draw(pD3DDevice);
	if (result != 0) goto EXIT;

	//�m�[�g�g��`��
	result = m_NoteRippleMod.Draw(pD3DDevice);
	if (result != 0) goto EXIT;

	//�_�b�V���{�[�h�`��F���W�ϊ��ςݒ��_��p���邽�߈�ԍŌ�ɕ`�悷��
	result = m_Dashboard.Draw(pD3DDevice);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �j��
//******************************************************************************
void MTScenePianoRollRingMod::Release()
{
	m_GridRingMod.Release();
	m_TimeIndicatorMod.Release();
	m_NoteRippleMod.Release();

	MTScenePianoRollRing::Release();
}

//******************************************************************************
// �V�[�P���T���b�Z�[�W��M
//******************************************************************************
int MTScenePianoRollRingMod::OnRecvSequencerMsg(
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
		m_TimeIndicatorMod.SetCurTickTime(parser.GetPlayTickTime());
		m_NoteRippleMod.SetPlayTimeMSec(parser.GetPlayTimeMSec());
		m_NoteRippleMod.SetCurTickTime(parser.GetPlayTickTime());
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
		//m_NoteRippleMod.SetNoteOff(parser.GetPortNo(), parser.GetChNo(), parser.GetNoteNo());
	}
	//�m�[�gON�ʒm
	else if (parser.GetMsg() == SMMsgParser::MsgNoteOn) {
		m_Dashboard.SetNoteOn();
		//m_NoteRippleMod.SetNoteOn(parser.GetPortNo(), parser.GetChNo(), parser.GetNoteNo(), parser.GetVelocity());
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
		m_NoteRippleMod.Reset();
		m_NoteRippleMod.SetSkipStatus(true);
		m_IsSkipping = true;
	}
	//�X�L�b�v�I���ʒm
	else if (parser.GetMsg() == SMMsgParser::MsgSkipEnd) {
		m_Dashboard.SetNotesCount(parser.GetSkipEndNotesCount());
		m_NoteBox.SetSkipStatus(false);
		m_NoteRippleMod.SetSkipStatus(false);
		m_IsSkipping = false;
	}

	//EXIT:;
	return result;
}

//******************************************************************************
// ���Z�b�g
//******************************************************************************
void MTScenePianoRollRingMod::_Reset()
{
	MTScenePianoRollRing::_Reset();

	m_TimeIndicatorMod.Reset();
	m_NoteRippleMod.Reset();
}

//******************************************************************************
// �\�����ʐݒ�
//******************************************************************************
void MTScenePianoRollRingMod::SetEffect(
		MTScene::EffectType type,
		bool isEnable
	)
{
	switch (type) {
		case EffectRipple:
			m_NoteRippleMod.SetEnable(isEnable);
			break;
		case EffectTimeIndicator:
			m_TimeIndicatorMod.SetEnable(isEnable);
			break;
		case EffectGridBox:
			m_GridRingMod.SetEnable(isEnable);
			break;
		default:
			MTScenePianoRollRing::SetEffect(type, isEnable);
			break;
	}

	return;
}
