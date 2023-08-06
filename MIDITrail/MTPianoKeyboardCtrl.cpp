//******************************************************************************
//
// MIDITrail / MTPianoKeyboardCtrl
//
// �s�A�m�L�[�{�[�h����N���X
//
// Copyright (C) 2010-2013 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTPianoKeyboard.h"
#include "MTPianoKeyboardCtrl.h"

using namespace YNBaseLib;


//******************************************************************************
// �p�����[�^��`
//******************************************************************************
#define MTPIANOKEYBOARD_MAX_ACTIVENOTE_NUM (256)


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTPianoKeyboardCtrl::MTPianoKeyboardCtrl(void)
{
	unsigned char chNo = 0;

	for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
		m_pPianoKeyboard[chNo] = NULL;
	}
	m_PlayTimeMSec = 0;
	m_CurTickTime = 0;
	m_CurNoteIndex = 0;
	m_pNoteStatus = NULL;
	m_isEnable = true;
	m_isSkipping = false;
	m_isSingleKeyboard = false;
	ZeroMemory(m_KeyDownRate, sizeof(float) * SM_MAX_CH_NUM * SM_MAX_NOTE_NUM);
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTPianoKeyboardCtrl::~MTPianoKeyboardCtrl(void)
{
	Release();
}

//******************************************************************************
// ��������
//******************************************************************************
int MTPianoKeyboardCtrl::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData,
		MTNotePitchBend* pNotePitchBend,
		bool isSingleKeyboard
	)
{
	int result = 0;
	SMTrack track;

	Release();

	if (pSeqData == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//�m�[�g�f�U�C���I�u�W�F�N�g������
	result = m_NoteDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	//�L�[�{�[�h�f�U�C��������
	result = m_KeyboardDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	//�g���b�N�擾
	result = pSeqData->GetMergedTrack(&track);
	if (result != 0) goto EXIT;

	//�m�[�g���X�g�擾�FstartTime, endTime �̓��A���^�C��(msec)
	result = track.GetNoteListWithRealTime(&m_NoteListRT, pSeqData->GetTimeDivision());
	if (result != 0) goto EXIT;

	//�m�[�g���z�񐶐�
	result = _CreateNoteStatus();
	if (result != 0) goto EXIT;

	//�L�[�{�[�h����
	result = _CreateKeyboards(pD3DDevice, pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	//�s�b�`�x���h���
	m_pNotePitchBend = pNotePitchBend;

	//�V���O���L�[�{�[�h�t���O
	m_isSingleKeyboard = isSingleKeyboard;

EXIT:;
	return result;
}

//******************************************************************************
// �m�[�g���z�񐶐�
//******************************************************************************
int MTPianoKeyboardCtrl::_CreateNoteStatus()
{
	int result = 0;
	unsigned long i = 0;

	//�m�[�g���z�񐶐�
	try {
		m_pNoteStatus = new NoteStatus[MTPIANOKEYBOARD_MAX_ACTIVENOTE_NUM];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

	//�m�[�g��ԃ��X�g������
	for (i = 0; i < MTPIANOKEYBOARD_MAX_ACTIVENOTE_NUM; i++) {
		m_pNoteStatus[i].isActive = false;
		m_pNoteStatus[i].keyStatus = BeforeNoteON;
		m_pNoteStatus[i].index = 0;
		m_pNoteStatus[i].keyDownRate = 0.0f;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �L�[�{�[�h�`��I�u�W�F�N�g����
//******************************************************************************
int MTPianoKeyboardCtrl::_CreateKeyboards(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
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

		result = m_pPianoKeyboard[chNo]->Create(pD3DDevice, pSceneName, pSeqData, pTexture);
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
int MTPianoKeyboardCtrl::Transform(
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
		moveVector.y += m_NoteDesign.GetPlayPosX(m_CurTickTime);

		//�L�[�{�[�h�ړ�
		result = m_pPianoKeyboard[chNo]->Transform(pD3DDevice, moveVector, rollAngle);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �������m�[�g�̒��_����
//******************************************************************************
int MTPianoKeyboardCtrl::_TransformActiveNotes(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;

	//�X�L�b�v���Ȃ牽�����Ȃ�
	if (m_isSkipping) goto EXIT;

	//�������m�[�g�̏�ԍX�V
	result = _UpdateStatusOfActiveNotes(pD3DDevice);
	if (result != 0) goto EXIT;

	//�������m�[�g�̒��_�X�V
	result = _UpdateVertexOfActiveNotes(pD3DDevice);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �������m�[�g�̏�ԍX�V
//******************************************************************************
int MTPianoKeyboardCtrl::_UpdateStatusOfActiveNotes(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	unsigned long i = 0;
	unsigned long keyDownDuration = 0;
	unsigned long keyUpDuration = 0;
	bool isFound = false;
	bool isRegist = false;
	SMNote note;

	//�L�[�㏸���~����(msec)
	keyDownDuration = m_KeyboardDesign.GetKeyDownDuration();
	keyUpDuration   = m_KeyboardDesign.GetKeyUpDuration();

	//�m�[�g�����X�V����
	for (i = 0; i < MTPIANOKEYBOARD_MAX_ACTIVENOTE_NUM; i++) {
		if (m_pNoteStatus[i].isActive) {
			//�m�[�g���擾
			result = m_NoteListRT.GetNote(m_pNoteStatus[i].index, &note);
			if (result != 0) goto EXIT;

			//�������m�[�g��ԍX�V
			result = _UpdateNoteStatus(
							m_PlayTimeMSec,
							keyDownDuration,
							keyUpDuration,
							note,
							&(m_pNoteStatus[i])
						);
			if (result != 0) goto EXIT;
		}
	}

	//�O�񌟍��I���ʒu���甭���J�n�m�[�g������
	while (m_CurNoteIndex < m_NoteListRT.GetSize()) {
		//�m�[�g���擾
		result = m_NoteListRT.GetNote(m_CurNoteIndex, &note);
		if (result != 0) goto EXIT;

		//���t���Ԃ��L�[�����J�n���ԁi�����J�n���O�j�ɂ��ǂ���Ă��Ȃ���Ό����I��
		if (note.startTime > keyDownDuration) {
			if (m_PlayTimeMSec < (note.startTime - keyDownDuration)) break;
		}

		//�m�[�g���o�^����
		isRegist = false;
		if (note.startTime < keyDownDuration) {
			isRegist = true;
		}
		else if (((note.startTime - keyDownDuration) <= m_PlayTimeMSec)
		      && (m_PlayTimeMSec <= (note.endTime + keyUpDuration))) {
			isRegist = true;
		}

		//�m�[�g���o�^
		//  �L�[���~���^�㏸���̏����o�^�ΏۂƂ��Ă��邽��
		//  ����m�[�g�ŕ����G���g�������ꍇ�����邱�Ƃɒ��ӂ���
		if (isRegist) {
			//���łɓ���C���f�b�N�X�œo�^�ς݂̏ꍇ�͉������Ȃ�
			isFound = false;
			for (i = 0; i < MTPIANOKEYBOARD_MAX_ACTIVENOTE_NUM; i++) {
				if ((m_pNoteStatus[i].isActive)
				 && (m_pNoteStatus[i].index == m_CurNoteIndex)) {
					isFound = true;
					break;
				}
			}
			//�󂢂Ă���Ƃ���ɒǉ�����
			if (!isFound) {
				for (i = 0; i < MTPIANOKEYBOARD_MAX_ACTIVENOTE_NUM; i++) {
					if (!(m_pNoteStatus[i].isActive)) {
						m_pNoteStatus[i].isActive = true;
						m_pNoteStatus[i].keyStatus = BeforeNoteON;
						m_pNoteStatus[i].index = m_CurNoteIndex;
						m_pNoteStatus[i].keyDownRate = 0.0f;
						break;
					}
				}
			}
			//�������m�[�g��ԍX�V
			result = _UpdateNoteStatus(
							m_PlayTimeMSec,
							keyDownDuration,
							keyUpDuration,
							note,
							&(m_pNoteStatus[i])
						);
			if (result != 0) goto EXIT;
		}
		m_CurNoteIndex++;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �������m�[�g��ԍX�V
//******************************************************************************
int MTPianoKeyboardCtrl::_UpdateNoteStatus(
		unsigned long playTimeMSec,
		unsigned long keyDownDuration,
		unsigned long keyUpDuration,
		SMNote note,
		NoteStatus* pNoteStatus
	)
{
	int result= 0;
	unsigned char targetChNo = 0;

	//�m�[�gON�O�i�L�[���~���j
	if (playTimeMSec < note.startTime) {
		pNoteStatus->keyStatus = BeforeNoteON;
		if (keyDownDuration == 0) {
			pNoteStatus->keyDownRate = 0.0f;
		}
		else {
			pNoteStatus->keyDownRate = 1.0f - ((float)(note.startTime - playTimeMSec) / (float)keyDownDuration);
		}
	}
	//�m�[�gON����OFF�܂�
	else if ((note.startTime <= playTimeMSec) && (playTimeMSec <= note.endTime)) {
		pNoteStatus->keyStatus = NoteON;
		pNoteStatus->keyDownRate = 1.0f;
	}
	//�m�[�gOFF��i�L�[�㏸���j
	else if ((note.endTime < playTimeMSec) && (playTimeMSec <= (note.endTime + keyUpDuration))) {
		pNoteStatus->keyStatus = AfterNoteOFF;
		if (keyUpDuration == 0) {
			pNoteStatus->keyDownRate = 0.0f;
		}
		else {
			pNoteStatus->keyDownRate = 1.0f - ((float)(playTimeMSec - note.endTime) / (float)keyUpDuration);
		}
	}
	//�m�[�gOFF��i�L�[���A�ς݁j
	else {
		//�m�[�g����j��
		//TODO: �����|�[�g�Ή�
		if (note.portNo == 0) {
			//�V���O���L�[�{�[�h�ł͕����`�����l���̃L�[��Ԃ�擪�`�����l���ɏW�񂷂�
			targetChNo = note.chNo;
			if (m_isSingleKeyboard) {
				targetChNo = 0;
			}
			result = m_pPianoKeyboard[targetChNo]->ResetKey(note.noteNo);
			if (result != 0) goto EXIT;
		}
		pNoteStatus->isActive = false;
		pNoteStatus->keyStatus = BeforeNoteON;
		pNoteStatus->index = 0;
		pNoteStatus->keyDownRate = 0.0f;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �������m�[�g�̒��_�X�V
//******************************************************************************
int MTPianoKeyboardCtrl::_UpdateVertexOfActiveNotes(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	unsigned long i = 0;
	unsigned long elapsedTime = 0;
	SMNote note;
	unsigned char targetChNo = 0;
	D3DXCOLOR noteColor;

	ZeroMemory(m_KeyDownRate, sizeof(float) * SM_MAX_CH_NUM * SM_MAX_NOTE_NUM);

	//�������m�[�g�ɂ��Ē��_���X�V
	for (i = 0; i < MTPIANOKEYBOARD_MAX_ACTIVENOTE_NUM; i++) {
		//�������łȂ���΃X�L�b�v
		if (!(m_pNoteStatus[i].isActive)) continue;

		//�m�[�g���擾
		result = m_NoteListRT.GetNote(m_pNoteStatus[i].index, &note);
		if (result != 0) goto EXIT;

		//�����J�n����̌o�ߎ���
		elapsedTime = 0;
		if (m_pNoteStatus[i].keyStatus == NoteON) {
			elapsedTime = m_PlayTimeMSec - note.startTime;
		}

		//�L�[�̏�ԍX�V
		//  TODO: �����|�[�g�Ή�
		if (note.portNo == 0) {
			//�V���O���L�[�{�[�h�ł͕����`�����l���̃L�[��Ԃ�擪�`�����l���ɏW�񂷂�
			targetChNo = note.chNo;
			if (m_isSingleKeyboard) {
				targetChNo = 0;
			}
			
			//�m�[�g�̐F
			noteColor = m_NoteDesign.GetNoteBoxColor(note.portNo, note.chNo, note.noteNo);
			
			//�����ΏۃL�[����]
			//  ���łɓ���m�[�g�ɑ΂��Ē��_���X�V���Ă���ꍇ
			//  ���������O���������ꍇ�Ɍ��蒸�_���X�V����
			if (m_KeyDownRate[targetChNo][note.noteNo] < m_pNoteStatus[i].keyDownRate) {
				result = m_pPianoKeyboard[targetChNo]->PushKey(
														note.noteNo,
														m_pNoteStatus[i].keyDownRate,
														elapsedTime,
														&noteColor
													);
				if (result != 0) goto EXIT;
				m_KeyDownRate[targetChNo][note.noteNo] = m_pNoteStatus[i].keyDownRate;
			}
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// �`��
//******************************************************************************
int MTPianoKeyboardCtrl::Draw(
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
float MTPianoKeyboardCtrl::_GetPichBendShiftPosX(
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
void MTPianoKeyboardCtrl::Release()
{
	unsigned char chNo = 0;

	for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
		if (m_pPianoKeyboard[chNo] != NULL) {
			m_pPianoKeyboard[chNo]->Release();
			delete m_pPianoKeyboard[chNo];
			m_pPianoKeyboard[chNo] = NULL;
		}
	}

// >>> modify 20120728 yossiepon begin
	//20120728 yossiepon: delete �� delete[] �ɏC��
	delete[] m_pNoteStatus;
	m_pNoteStatus = NULL;
// <<< modify 20120728 yossiepon end
}

//******************************************************************************
// �J�����g�`�b�N�^�C���ݒ�
//******************************************************************************
void MTPianoKeyboardCtrl::SetCurTickTime(
		unsigned long curTickTime
	)
{
	m_CurTickTime = curTickTime;
}

//******************************************************************************
// ���t���Ԑݒ�
//******************************************************************************
void MTPianoKeyboardCtrl::SetPlayTimeMSec(
		unsigned long playTimeMsec
	)
{
	m_PlayTimeMSec = playTimeMsec;
}

//******************************************************************************
// ���Z�b�g
//******************************************************************************
void MTPianoKeyboardCtrl::Reset()
{
	int result = 0;
	unsigned long i = 0;
	SMNote note;
	unsigned char targetChNo = 0;

	m_PlayTimeMSec = 0;
	m_CurTickTime = 0;
	m_CurNoteIndex = 0;

	for (i = 0; i < MTPIANOKEYBOARD_MAX_ACTIVENOTE_NUM; i++) {
		if (m_pNoteStatus[i].isActive) {
			result = m_NoteListRT.GetNote(m_pNoteStatus[i].index, &note);
			//if (result != 0) goto EXIT;

			//TODO: �����|�[�g�Ή�
			if (note.portNo == 0) {
				//�V���O���L�[�{�[�h�ł͕����`�����l���̃L�[��Ԃ�擪�`�����l���ɏW�񂷂�
				targetChNo = note.chNo;
				if (m_isSingleKeyboard) {
					targetChNo = 0;
				}
				result = m_pPianoKeyboard[targetChNo]->ResetKey(note.noteNo);
				//if (result != 0) goto EXIT;
			}
		}
		m_pNoteStatus[i].isActive = false;
		m_pNoteStatus[i].keyStatus = BeforeNoteON;
		m_pNoteStatus[i].index = 0;
		m_pNoteStatus[i].keyDownRate = 0.0f;
	}

	return;
}

//******************************************************************************
// �\���ݒ�
//******************************************************************************
void MTPianoKeyboardCtrl::SetEnable(
		bool isEnable
	)
{
	m_isEnable = isEnable;
}

//******************************************************************************
// �X�L�b�v��Ԑݒ�
//******************************************************************************
void MTPianoKeyboardCtrl::SetSkipStatus(
		bool isSkipping
	)
{
	m_isSkipping = isSkipping;
}


