//******************************************************************************
//
// MIDITrail / MTPianoKeyboardCtrlLive
//
// ���C�u���j�^�p�s�A�m�L�[�{�[�h����N���X
//
// Copyright (C) 2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTPianoKeyboard.h"
#include "MTPianoKeyboardCtrlLive.h"

using namespace YNBaseLib;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTPianoKeyboardCtrlLive::MTPianoKeyboardCtrlLive(void)
{
	unsigned char chNo = 0;
	
	//�L�[�{�[�h�I�u�W�F�N�g�z�񏉊���
	for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
		m_pPianoKeyboard[chNo] = NULL;
	}
	
	//�m�[�g���z�񏉊���
	_ClearNoteStatus();
	
	m_isEnable = true;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTPianoKeyboardCtrlLive::~MTPianoKeyboardCtrlLive(void)
{
	Release();
}

//******************************************************************************
// ��������
//******************************************************************************
int MTPianoKeyboardCtrlLive::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName,
		MTNotePitchBend* pNotePitchBend
	)
{
	int result = 0;
	
	Release();
	
	//�m�[�g�f�U�C���I�u�W�F�N�g������
	result = m_NoteDesign.Initialize(pSceneName, NULL);
	if (result != 0) goto EXIT;
	
	//�L�[�{�[�h�f�U�C��������
	result = m_KeyboardDesign.Initialize(pSceneName, NULL);
	if (result != 0) goto EXIT;
	
	//�m�[�g���z�񏉊���
	_ClearNoteStatus();
	
	//�L�[�{�[�h����
	result = _CreateKeyboards(pD3DDevice, pSceneName);
	if (result != 0) goto EXIT;
	
	//�s�b�`�x���h���
	m_pNotePitchBend = pNotePitchBend;
	
EXIT:;
	return result;
}

//******************************************************************************
// �m�[�g���z�񏉊���
//******************************************************************************
void MTPianoKeyboardCtrlLive::_ClearNoteStatus()
{
	unsigned long chNo = 0;
	unsigned long noteNo = 0;
	
	//�m�[�g��ԃ��X�g������
	for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
		for (noteNo = 0; noteNo < SM_MAX_NOTE_NUM; noteNo++) {
			m_NoteStatus[chNo][noteNo].isActive = false;
			m_NoteStatus[chNo][noteNo].startTime = 0;
			m_NoteStatus[chNo][noteNo].endTime = 0;
			m_NoteStatus[chNo][noteNo].keyDownRate = 0.0f;
		}
	}
	
	return;
}

//******************************************************************************
// �L�[�{�[�h�`��I�u�W�F�N�g����
//******************************************************************************
int MTPianoKeyboardCtrlLive::_CreateKeyboards(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName
	)
{
	int result = 0;
	unsigned char chNo = 0;
	LPDIRECT3DTEXTURE9 pTexture = NULL;
	
	for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
		try {
			m_pPianoKeyboard[chNo] = new MTPianoKeyboard;
		}
		catch (std::bad_alloc) {
			result = YN_SET_ERR("Could not allocate memory.", 0, 0);
			goto EXIT;
		}
		
		result = m_pPianoKeyboard[chNo]->Create(pD3DDevice, pSceneName, NULL, pTexture);
		if (result != 0) goto EXIT;
		
		//�擪�I�u�W�F�N�g�ō쐬�����e�N�X�`�����ė��p����
		pTexture = m_pPianoKeyboard[chNo]->GetTexture();
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// �ړ�
//******************************************************************************
int MTPianoKeyboardCtrlLive::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice,
		float rollAngle
	)
{
	int result = 0;
	unsigned char portNo = 0;
	unsigned char chNo = 0;
	D3DXVECTOR3 moveVector;
	
	//���ݔ������m�[�g�̒��_�X�V
	result = _TransformActiveNotes(pD3DDevice);
	if (result != 0) goto EXIT;
	
	//�e�L�[�{�[�h�̈ړ�
	for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
		
		//�ړ��x�N�g���F�L�[�{�[�h����W
		moveVector = m_KeyboardDesign.GetKeyboardBasePos(portNo, chNo);
		
		//�ړ��x�N�g���F�s�b�`�x���h�V�t�g�𔽉f
		moveVector.x += _GetPichBendShiftPosX(portNo, chNo);
		
		//�ړ��x�N�g���F�Đ��ʂɒǏ]����
		//moveVector.y += m_NoteDesign.GetPlayPosX(m_CurTickTime);
		
		//�L�[�{�[�h�ړ�
		result = m_pPianoKeyboard[chNo]->Transform(pD3DDevice, moveVector, rollAngle);
		if (result != 0) goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// �m�[�g�̒��_����
//******************************************************************************
int MTPianoKeyboardCtrlLive::_TransformActiveNotes(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	
	//�m�[�g�̏�ԍX�V
	result = _UpdateStatusOfActiveNotes(pD3DDevice);
	if (result != 0) goto EXIT;
	
	//�m�[�g�̒��_�X�V
	result = _UpdateVertexOfActiveNotes(pD3DDevice);
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// �m�[�g�̏�ԍX�V
//******************************************************************************
int MTPianoKeyboardCtrlLive::_UpdateStatusOfActiveNotes(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	unsigned long chNo = 0;
	unsigned long noteNo = 0;
	unsigned long keyUpDuration = 0;
	unsigned long curTime = 0;
	
	curTime = timeGetTime();
	
	//�L�[�㏸���~����(msec)
	keyUpDuration = m_KeyboardDesign.GetKeyUpDuration();
	
	//�m�[�g�����X�V����
	for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
		for (noteNo = 0; noteNo < SM_MAX_NOTE_NUM; noteNo++) {
			//�m�[�gOFF��̏�Ԃ��X�V
			if ((m_NoteStatus[chNo][noteNo].isActive) && (m_NoteStatus[chNo][noteNo].endTime != 0)) {
				//�m�[�gOFF����L�[�A�b�v�܂Ŋ��������ꍇ�̓L�[�����N���A����
				if ((curTime - m_NoteStatus[chNo][noteNo].endTime) > keyUpDuration) {
					m_NoteStatus[chNo][noteNo].isActive = false;
					m_NoteStatus[chNo][noteNo].startTime = 0;
					m_NoteStatus[chNo][noteNo].endTime = 0;
					m_NoteStatus[chNo][noteNo].keyDownRate = 0.0f;
					result = m_pPianoKeyboard[chNo]->ResetKey((unsigned char)noteNo);
					if (result != 0) goto EXIT;
				}
				//�L�[���������X�V
				else {
					m_NoteStatus[chNo][noteNo].keyDownRate
						= 1.0f - ((float)(curTime - m_NoteStatus[chNo][noteNo].endTime) / (float)keyUpDuration);
				}
			}
		}
	}
		
EXIT:;
	return result;
}

//******************************************************************************
// �m�[�g�̒��_�X�V
//******************************************************************************
int MTPianoKeyboardCtrlLive::_UpdateVertexOfActiveNotes(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	unsigned long chNo = 0;
	unsigned long noteNo = 0;
	unsigned long elapsedTime = 0;
	unsigned long curTime = 0;
	
	curTime = timeGetTime();
		
	//�m�[�g�̒��_���X�V
	for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
		for (noteNo = 0; noteNo < SM_MAX_NOTE_NUM; noteNo++) {
			if (m_NoteStatus[chNo][noteNo].isActive) {
				//�����J�n����̌o�ߎ���
				elapsedTime = curTime - m_NoteStatus[chNo][noteNo].startTime;
			
				//�����ΏۃL�[����]
				result = m_pPianoKeyboard[chNo]->PushKey(
									(unsigned char)noteNo,
									m_NoteStatus[chNo][noteNo].keyDownRate,
									elapsedTime
								);
				if (result != 0) goto EXIT;
			}
		}
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// �`��
//******************************************************************************
int MTPianoKeyboardCtrlLive::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
   )
{
	int result = 0;
	unsigned char chNo = 0;
	unsigned long count = 0;
	unsigned long dispNum = 0;
	
	if (!m_isEnable) goto EXIT;
	
	//�L�[�{�[�h�ő�\����
	dispNum = SM_MAX_CH_NUM;
	if (m_KeyboardDesign.GetKeyboardMaxDispNum() < dispNum) {
		dispNum = m_KeyboardDesign.GetKeyboardMaxDispNum();
	}
	
	//�L�[�{�[�h�̕`��
	for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
		//�L�[�{�[�h�\�����̐������m�F
		count++;
		if (dispNum < count) break;
		
		//�L�[�{�[�h�`��
		result = m_pPianoKeyboard[chNo]->Draw(pD3DDevice);
		if (result != 0) goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// �s�b�`�x���h���f�F�L�[�{�[�h�V�t�g
//******************************************************************************
float MTPianoKeyboardCtrlLive::_GetPichBendShiftPosX(
		unsigned char portNo,
		unsigned char chNo
	)
{
	float shift = 0.0f;
	short pitchBendValue = 0;
	unsigned char pitchBendSensitivity = SM_DEFAULT_PITCHBEND_SENSITIVITY;
	
	//�`�����l���̃s�b�`�x���h���
	pitchBendValue =       m_pNotePitchBend->GetValue(portNo, chNo);
	pitchBendSensitivity = m_pNotePitchBend->GetSensitivity(portNo, chNo);
	
	//�s�b�`�x���h�ɂ��L�[�{�[�h�V�t�g��
	shift = m_KeyboardDesign.GetPitchBendShift(pitchBendValue, pitchBendSensitivity);
	
	return shift;
}

//******************************************************************************
// ���
//******************************************************************************
void MTPianoKeyboardCtrlLive::Release()
{
	unsigned char chNo = 0;
	
	for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
		if (m_pPianoKeyboard[chNo] != NULL) {
			m_pPianoKeyboard[chNo]->Release();
			delete m_pPianoKeyboard[chNo];
			m_pPianoKeyboard[chNo] = NULL;
		}
	}
	
	return;
}

//******************************************************************************
// ���Z�b�g
//******************************************************************************
void MTPianoKeyboardCtrlLive::Reset()
{
	int result = 0;
	unsigned long chNo = 0;
	unsigned long noteNo = 0;
	
	for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
		for (noteNo = 0; noteNo < SM_MAX_NOTE_NUM; noteNo++) {
			m_NoteStatus[chNo][noteNo].isActive = false;
			m_NoteStatus[chNo][noteNo].startTime = 0;
			m_NoteStatus[chNo][noteNo].endTime = 0;
			m_NoteStatus[chNo][noteNo].keyDownRate = 0.0f;
			result = m_pPianoKeyboard[chNo]->ResetKey((unsigned char)noteNo);
			if (result != 0) goto EXIT;
		}
	}

EXIT:;
	return;
}

//******************************************************************************
// �\���ݒ�
//******************************************************************************
void MTPianoKeyboardCtrlLive::SetEnable(
		bool isEnable
	)
{
	m_isEnable = isEnable;
}

//******************************************************************************
// �m�[�gON�o�^
//******************************************************************************
void MTPianoKeyboardCtrlLive::SetNoteOn(
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		unsigned char velocity
	)
{
	unsigned long curTime = 0;
	
	curTime = timeGetTime();
	
	//�m�[�g���o�^
	m_NoteStatus[chNo][noteNo].isActive = true;
	m_NoteStatus[chNo][noteNo].startTime = curTime;
	m_NoteStatus[chNo][noteNo].endTime = 0;
	m_NoteStatus[chNo][noteNo].keyDownRate = 1.0f;
	
	return;
}

//******************************************************************************
// �m�[�gOFF�o�^
//******************************************************************************
void MTPianoKeyboardCtrlLive::SetNoteOff(
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo
	)
{
	unsigned long curTime = 0;
	
	curTime = timeGetTime();
	
	//�m�[�g���X�V
	m_NoteStatus[chNo][noteNo].endTime = curTime;
	
	return;
}

//******************************************************************************
// �S�m�[�gOFF
//******************************************************************************
void MTPianoKeyboardCtrlLive::AllNoteOff()
{
	unsigned long chNo = 0;
	unsigned long noteNo = 0;
	unsigned long curTime = 0;
	
	curTime = timeGetTime();
	
	//�m�[�gOFF���ݒ肳��Ă��Ȃ��m�[�g���ɏI��������ݒ�
	for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
		for (noteNo = 0; noteNo < SM_MAX_NOTE_NUM; noteNo++) {
			if ((m_NoteStatus[chNo][noteNo].isActive)
				&& (m_NoteStatus[chNo][noteNo].endTime == 0)) {
					m_NoteStatus[chNo][noteNo].endTime = curTime;
			}
		}
	}
	
	return;
}

//******************************************************************************
// �S�m�[�gOFF�i�`�����l���w��j
//******************************************************************************
void MTPianoKeyboardCtrlLive::AllNoteOffOnCh(
		unsigned char portNo,
		unsigned char chNo
	)
{
	unsigned long noteNo = 0;
	unsigned long curTime = 0;
	
	curTime = timeGetTime();
	
	//�w��`�����l���Ńm�[�gOFF���ݒ肳��Ă��Ȃ��m�[�g���ɏI��������ݒ�
	for (noteNo = 0; noteNo < SM_MAX_NOTE_NUM; noteNo++) {
		if ((m_NoteStatus[chNo][noteNo].isActive)
			&& (m_NoteStatus[chNo][noteNo].endTime == 0)) {
				m_NoteStatus[chNo][noteNo].endTime = curTime;
		}
	}
	
	return;
}


