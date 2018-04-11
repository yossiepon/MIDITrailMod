//******************************************************************************
//
// MIDITrail / MTPianoKeyboardCtrlMod
//
// �s�A�m�L�[�{�[�h����Mod�N���X
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTPianoKeyboardCtrlMod.h"
#include "MTPianoKeyboardMod.h"
#include "MTNoteRippleMod.h"
#include "MTNoteLyrics.h"

using namespace YNBaseLib;


//******************************************************************************
// �p�����[�^��`
//******************************************************************************
#define MTPIANOKEYBOARD_MAX_ACTIVENOTE_NUM (256)


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTPianoKeyboardCtrlMod::MTPianoKeyboardCtrlMod(void)
{
	m_MaxPortIndex = 0;
	ZeroMemory(m_KeyDownRateMod, sizeof(float) * SM_MAX_CH_NUM* SM_MAX_CH_NUM * SM_MAX_NOTE_NUM);
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTPianoKeyboardCtrlMod::~MTPianoKeyboardCtrlMod(void)
{
	Release();
}

//******************************************************************************
// ��������
//******************************************************************************
int MTPianoKeyboardCtrlMod::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData,
		MTNotePitchBend* pNotePitchBend,
		bool isSingleKeyboard
	)
{
	int result = 0;
	unsigned long index = 0;
	unsigned long portIndex = 0;
	unsigned char portNo = 0;
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
	result = m_KeyboardDesignMod.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	//�V�[�P���X�f�[�^�F�|�[�g���X�g�擾
	result = pSeqData->GetPortList(&m_PortList);
	if (result != 0) goto EXIT;

	//�|�[�g�ԍ��ɏ����̃C���f�b�N�X��U��
	//�|�[�g 0�� 3�� 5�� �ɏo�͂���ꍇ�̃C���f�b�N�X�͂��ꂼ�� 0, 1, 2
	for (index = 0; index < SM_MAX_PORT_NUM; index++) {
		m_PortIndex[index] = -1;
	}
	for (index = 0; index < m_PortList.GetSize(); index++) {
		m_PortList.GetPort(index, &portNo);
		m_PortIndex[portNo] = portIndex;
		portIndex++;
		if(portIndex == m_KeyboardDesignMod.GetKeyboardMaxDispNum()){
			break;
		}
	}
	m_MaxPortIndex = (unsigned char)portIndex;

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
	//���t���O���󂯎���Ă��g�p���Ȃ��B�|�[�g�ʃV���O���L�[�{�[�h�ŏ�ɓ��삷��
	m_isSingleKeyboard = isSingleKeyboard;

EXIT:;
	return result;
}

//******************************************************************************
// �L�[�{�[�h�`��I�u�W�F�N�g����
//******************************************************************************
int MTPianoKeyboardCtrlMod::_CreateKeyboards(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;
	unsigned char portIndex = 0;
	LPDIRECT3DTEXTURE9 pTexture = NULL;

	for (portIndex = 0; portIndex < m_MaxPortIndex; portIndex++) {
		try {
			m_pPianoKeyboard[portIndex] = new MTPianoKeyboardMod;
		}
		catch (std::bad_alloc) {
			result = YN_SET_ERR("Could not allocate memory.", 0, 0);
			goto EXIT;
		}

		result = m_pPianoKeyboard[portIndex]->Create(pD3DDevice, pSceneName, pSeqData, pTexture);
		if (result != 0) goto EXIT;

		//�擪�I�u�W�F�N�g�ō쐬�����e�N�X�`�����ė��p����
		pTexture = m_pPianoKeyboard[portIndex]->GetTexture();
	}

EXIT:;
	return result;
}

//******************************************************************************
// �ړ�
//******************************************************************************
int MTPianoKeyboardCtrlMod::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice,
		float rollAngle
	)
{
	int result = 0;
	unsigned char portNo = 0;
	unsigned char chNo = 0;
	D3DXVECTOR3 vectorLU;
	D3DXVECTOR3 vectorRU;
	D3DXVECTOR3 vectorLD;
	D3DXVECTOR3 vectorRD;
	D3DXVECTOR3 moveVector1;
	D3DXVECTOR3 moveVector2;

	//���ݔ������m�[�g�̒��_�X�V
	result = _TransformActiveNotes(pD3DDevice);
	if (result != 0) goto EXIT;

	//�Đ��ʒ��_���W�擾
	m_NoteDesign.GetPlaybackSectionVirtexPos(
			0,
			&vectorLU,
			&vectorRU,
			&vectorLD,
			&vectorRD
		);

	float boardHeight = vectorLU.y - vectorLD.y;
	float keyboardWidth = m_KeyboardDesignMod.GetPortOriginX(0) * -2.0f;

	//�ړ��x�N�g���F�Đ��ʂɒǏ]����
	moveVector2 = m_NoteDesign.GetWorldMoveVector();
	moveVector2.x += m_NoteDesign.GetPlayPosX(m_CurTickTime);

	unsigned char lastPortNo = 0;

	m_PortList.GetPort(m_PortList.GetSize()-1, &lastPortNo);

	for(portNo = 0; portNo <= lastPortNo; portNo ++) {

		int portIndex = m_PortIndex[portNo];

		if(portIndex == -1) {
			continue;
		}

		//�s�b�`�x���h�V�t�g�̍ő�ʂ����߂�
		float maxAbsPitchBendShift = 0.0f;
		float curMaxPitchBendShift = 0.0f;

		for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {

			float pitchBendShift = _GetPichBendShiftPosX(portNo, chNo);
			if(maxAbsPitchBendShift < fabs(pitchBendShift)) {

				curMaxPitchBendShift = pitchBendShift;
				maxAbsPitchBendShift = fabs(pitchBendShift);
			}
		}

		//�ړ��x�N�g���F�L�[�{�[�h����W
		moveVector1 = m_KeyboardDesignMod.GetKeyboardBasePos(portIndex, 0, keyboardWidth / boardHeight);

		//�ړ��x�N�g���F�s�b�`�x���h�V�t�g�𔽉f
		moveVector1.x += curMaxPitchBendShift;

		if(rollAngle < 0.0f) {
			rollAngle += 360.0f;
		}

		float portWidth =  m_KeyboardDesignMod.GetChStep() * 16.0f;

		if((rollAngle > 120.0f) && (rollAngle < 300.0f)) {

			//���Ղ�1/2�̕�������������
			moveVector1.x += m_KeyboardDesignMod.GetWhiteKeyStep() / 2.0f;

			//�|�[�g���_Y
			moveVector1.y -= portWidth * (m_PortList.GetSize() - portIndex - 1) * (keyboardWidth / boardHeight);

			//���Ղ̌��_��Ch15��
			moveVector1.y -= m_KeyboardDesignMod.GetChStep()  * (keyboardWidth / boardHeight) * 15.0f;

			//���Ղ�1/4�̍�����������
			moveVector1.y -= m_KeyboardDesignMod.GetWhiteKeyHeight() / 4.0f;

			//���Ղ̒����{���b�v���}�[�W���{�̎��}�[�W��������O��
			moveVector1.z -= m_KeyboardDesignMod.GetWhiteKeyLen() + 0.002f * (MTNOTELYRICS_MAX_LYRICS_NUM + MTNOTERIPPLE_MAX_RIPPLE_NUM) * (keyboardWidth / boardHeight);

		} else {

			//���Ղ�1/2�̕�������������
			moveVector1.x += m_KeyboardDesignMod.GetWhiteKeyStep() / 2.0f;

			//�|�[�g���_Y
			moveVector1.y += portWidth * (m_PortList.GetSize() - portIndex - 1) * (keyboardWidth / boardHeight);

			//���Ղ�1/4�̍�����������
			moveVector1.y -= m_KeyboardDesignMod.GetWhiteKeyHeight() / 4.0f;

			//���b�v���}�[�W���{�̎��}�[�W����������
			moveVector1.z += 0.002f * (MTNOTELYRICS_MAX_LYRICS_NUM + MTNOTERIPPLE_MAX_RIPPLE_NUM) * (keyboardWidth / boardHeight);
		}

		//�L�[�{�[�h�ړ�
		result = m_pPianoKeyboard[portIndex]->Transform(pD3DDevice, moveVector1, moveVector2, boardHeight / keyboardWidth, vectorLU.z, rollAngle);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �������m�[�g��ԍX�V
//******************************************************************************
int MTPianoKeyboardCtrlMod::_UpdateNoteStatus(
		unsigned long playTimeMSec,
		unsigned long keyDownDuration,
		unsigned long keyUpDuration,
		SMNote note,
		NoteStatus* pNoteStatus
	)
{
	int result= 0;

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
		//�����`�����l���̃L�[��Ԃ��|�[�g�ʂɏW�񂷂�
		result = m_pPianoKeyboard[m_PortIndex[note.portNo]]->ResetKey(note.noteNo);
		if (result != 0) goto EXIT;

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
int MTPianoKeyboardCtrlMod::_UpdateVertexOfActiveNotes(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	unsigned long i = 0;
	unsigned long elapsedTime = 0;
	SMNote note;
	D3DXCOLOR noteColor;

	ZeroMemory(m_KeyDownRateMod, sizeof(float) * SM_MAX_CH_NUM* SM_MAX_CH_NUM * SM_MAX_NOTE_NUM);

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

		//�m�[�g�̐F
		noteColor = m_NoteDesign.GetNoteBoxColor(note.portNo, note.chNo, note.noteNo);
		
		//�����ΏۃL�[����]
		//  ���łɓ���m�[�g�ɑ΂��Ē��_���X�V���Ă���ꍇ
		//  ���������O���������ꍇ�Ɍ��蒸�_���X�V����
		if (m_KeyDownRateMod[note.portNo][note.chNo][note.noteNo] < m_pNoteStatus[i].keyDownRate) {
			//�����`�����l���̃L�[��Ԃ��|�[�g�ʂɏW�񂷂�
			result = m_pPianoKeyboard[m_PortIndex[note.portNo]]->PushKey(
																	note.chNo,
																	note.noteNo,
																	m_pNoteStatus[i].keyDownRate,
																	elapsedTime,
																	&noteColor
																);
			if (result != 0) goto EXIT;
			m_KeyDownRateMod[note.portNo][note.chNo][note.noteNo] = m_pNoteStatus[i].keyDownRate;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// �`��
//******************************************************************************
int MTPianoKeyboardCtrlMod::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
   )
{
	int result = 0;
	unsigned char portIndex = 0;
	unsigned long count = 0;
	unsigned long dispNum = 0;

	if (!m_isEnable) goto EXIT;

	//�L�[�{�[�h�̕`��
	for (portIndex = 0; portIndex < m_MaxPortIndex; portIndex++) {

		result = m_pPianoKeyboard[portIndex]->Draw(pD3DDevice);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// ���Z�b�g
//******************************************************************************
void MTPianoKeyboardCtrlMod::Reset()
{
	int result = 0;
	unsigned long i = 0;
	SMNote note;

	m_PlayTimeMSec = 0;
	m_CurTickTime = 0;
	m_CurNoteIndex = 0;

	for (i = 0; i < MTPIANOKEYBOARD_MAX_ACTIVENOTE_NUM; i++) {
		if (m_pNoteStatus[i].isActive) {
			result = m_NoteListRT.GetNote(m_pNoteStatus[i].index, &note);
			//if (result != 0) goto EXIT;

			//�����`�����l���̃L�[��Ԃ��|�[�g�ʂɏW�񂷂�
			result = m_pPianoKeyboard[m_PortIndex[note.portNo]]->ResetKey(note.noteNo);
			//if (result != 0) goto EXIT;
		}
		m_pNoteStatus[i].isActive = false;
		m_pNoteStatus[i].keyStatus = BeforeNoteON;
		m_pNoteStatus[i].index = 0;
		m_pNoteStatus[i].keyDownRate = 0.0f;
	}

	return;
}
