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
	m_MaxKeyboardIndex = 0;
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
	unsigned long keyboardIndex = 0;
	unsigned char portNo = 0;
	SMTrack track;

	Release();

	if (pSeqData == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//�m�[�g�f�U�C���I�u�W�F�N�g������
	result = m_NoteDesignMod.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	//�L�[�{�[�h�f�U�C��������
	result = m_KeyboardDesignMod.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	//�V�[�P���X�f�[�^�F�|�[�g���X�g�擾
	result = pSeqData->GetPortList(&m_PortList);
	if (result != 0) goto EXIT;

	for (index = 0; index < SM_MAX_PORT_NUM; index++) {
		m_KeyboardIndex[index] = -1;
	}

	//�V���O���L�[�{�[�h�łȂ��ꍇ
	if (!isSingleKeyboard) {
		//�|�[�g�ԍ��ɏ����̃L�[�{�[�h�C���f�b�N�X��U��
		//�|�[�g 0�� 3�� 5�� �ɏo�͂���ꍇ�̃C���f�b�N�X�͂��ꂼ�� 0, 1, 2
		for (index = 0; index < m_PortList.GetSize(); index++) {
			m_PortList.GetPort(index, &portNo);
			m_KeyboardIndex[portNo] = keyboardIndex;
			keyboardIndex++;
			if(keyboardIndex == m_KeyboardDesignMod.GetKeyboardMaxDispNum()){
				break;
			}
		}
		m_MaxKeyboardIndex = (unsigned char)keyboardIndex;
	}
	//�V���O���L�[�{�[�h�̏ꍇ
	else {
		//�L�[�{�[�h�f�U�C�����V���O�����[�h�ɐݒ�
		m_KeyboardDesignMod.SetKeyboardSingle();
		//�|�[�g�ƃL�[�{�[�h�̑Ή���1:1�ɌŒ�
		m_KeyboardIndex[0] = 0;
		m_MaxKeyboardIndex = 1;
	}

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
// �L�[�{�[�h�`��I�u�W�F�N�g����
//******************************************************************************
int MTPianoKeyboardCtrlMod::_CreateKeyboards(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;
	unsigned char index = 0;
	LPDIRECT3DTEXTURE9 pTexture = NULL;

	for (index = 0; index < m_MaxKeyboardIndex; index++) {
		try {
			m_pPianoKeyboard[index] = new MTPianoKeyboardMod;
		}
		catch (std::bad_alloc) {
			result = YN_SET_ERR("Could not allocate memory.", 0, 0);
			goto EXIT;
		}

		result = m_pPianoKeyboard[index]->Create(pD3DDevice, pSceneName, pSeqData, pTexture);
		if (result != 0) goto EXIT;

		//�擪�I�u�W�F�N�g�ō쐬�����e�N�X�`�����ė��p����
		pTexture = m_pPianoKeyboard[index]->GetTexture();
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
	int index;
	D3DXVECTOR3 portWindowLU;
	D3DXVECTOR3 portWindowRU;
	D3DXVECTOR3 portWindowLD;
	D3DXVECTOR3 portWindowRD;
	D3DXVECTOR3 transformVector;
	D3DXVECTOR3 playbackPosVector;

	//�A�N�e�B�u�|�[�g�t���O�N���A
	for (index = 0; index < SM_MAX_PORT_NUM; index++) {
		m_isActivePort[index] = false;
	}

	//���ݔ������m�[�g�̒��_�X�V
	result = _TransformActiveNotes(pD3DDevice);
	if (result != 0) goto EXIT;

	//�Đ��ʒ��_���W�擾
	m_NoteDesignMod.GetPlaybackSectionVirtexPos(
			0,
			&portWindowLU,
			&portWindowRU,
			&portWindowLD,
			&portWindowRD
		);

	float boardHeight = portWindowLU.y - portWindowLD.y;
	float keyboardWidth = m_KeyboardDesignMod.GetPortOriginX() * -2.0f;

	float resizeSacle = boardHeight / keyboardWidth;

	float rippleSpacing = m_NoteDesignMod.GetRippleSpacing();
	float rippleMargin = rippleSpacing * (MTNOTELYRICS_MAX_LYRICS_NUM + MTNOTERIPPLE_MAX_RIPPLE_NUM); // * antiResizeScale;

	//�ړ��x�N�g���F�Đ��ʂɒǏ]����
	playbackPosVector = m_NoteDesignMod.GetWorldMoveVector();
	playbackPosVector.x += m_NoteDesignMod.GetPlayPosX(m_CurTickTime);

	unsigned char lastPortNo = 0;

	if (!m_isSingleKeyboard) {
		//�V���O���L�[�{�[�h�łȂ��ꍇ�A�ŏI�|�[�g�ԍ����擾
		m_PortList.GetPort(m_PortList.GetSize()-1, &lastPortNo);
	}
	else {
		//�V���O���L�[�{�[�h�̏ꍇ�A�ŏI�|�[�g�ԍ���0�Œ�
		lastPortNo = 0;
	}

	for(portNo = 0; portNo <= lastPortNo; portNo ++) {

		//�|�[�g�ԍ�����L�[�{�[�h�C���f�b�N�X���擾
		//�V���O���L�[�{�[�h�̏ꍇ�A�C���f�b�N�X��0�Œ�
		int keyboardIndex = !m_isSingleKeyboard ? m_KeyboardIndex[portNo] : 0;

		if(keyboardIndex == -1) {
			continue;
		}

		//�ړ��x�N�g���F�L�[�{�[�h����W
		transformVector = m_KeyboardDesignMod.GetKeyboardBasePos(keyboardIndex, rippleMargin, boardHeight, rollAngle);

		//�ړ��x�N�g���F�s�b�`�x���h�V�t�g�𔽉f
		transformVector.x += GetMaxPitchBendShift(portNo);

		//�L�[�{�[�h�ړ�
		result = m_pPianoKeyboard[keyboardIndex]->Transform(pD3DDevice, transformVector, playbackPosVector, resizeSacle, portWindowLU.z, rollAngle);
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
		//�������̃L�[�����Z�b�g����
		result = m_pPianoKeyboard[_GetKeyboardIndexFromNote(note)]->ResetKey(note.noteNo);
		if (result != 0) goto EXIT;

		pNoteStatus->isActive = false;
		pNoteStatus->keyStatus = BeforeNoteON;
		pNoteStatus->index = 0;
		pNoteStatus->keyDownRate = 0.0f;
	}

	//��ԍX�V��A�������ł����
	if (pNoteStatus->isActive) {
		//�A�N�e�B�u�|�[�g�t���O�𗧂Ă�
		m_isActivePort[note.portNo] = true;
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
	unsigned char notePortNo;

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
		noteColor = m_NoteDesignMod.GetNoteBoxColor(note.portNo, note.chNo, note.noteNo);

		//�V���O���L�[�{�[�h�łȂ��ꍇ
		if (!m_isSingleKeyboard) {
			//�m�[�g�̃|�[�g�ԍ����擾
			notePortNo = note.portNo;
		}
		//�V���O���L�[�{�[�h�̏ꍇ
		else {
			//�m�[�g�̃|�[�g�ԍ���0�Œ��
			notePortNo = 0;
		}

		//�����ΏۃL�[����]
		//  ���łɓ���m�[�g�ɑ΂��Ē��_���X�V���Ă���ꍇ
		//  ���������O���������ꍇ�Ɍ��蒸�_���X�V����
		if (m_KeyDownRateMod[notePortNo][note.chNo][note.noteNo] < m_pNoteStatus[i].keyDownRate) {
			//�����`�����l���̃L�[��Ԃ��|�[�g�ʂɏW�񂷂�
			result = m_pPianoKeyboard[_GetKeyboardIndexFromNote(note)]->PushKey(
																		note.chNo,
																		note.noteNo,
																		m_pNoteStatus[i].keyDownRate,
																		elapsedTime,
																		&noteColor
																	);
			if (result != 0) goto EXIT;
			m_KeyDownRateMod[notePortNo][note.chNo][note.noteNo] = m_pNoteStatus[i].keyDownRate;
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
	unsigned char index = 0;
	unsigned long count = 0;
	unsigned long dispNum = 0;

	if (!m_isEnable) goto EXIT;

	//�����_�����O�X�e�[�g�ݒ�FZ�o�b�t�@�ւ̏������݃I�t
//	pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	//�L�[�{�[�h�̕`��
	for (index = 0; index < m_MaxKeyboardIndex; index++) {

		result = m_pPianoKeyboard[index]->Draw(pD3DDevice);
		if (result != 0) goto EXIT;
	}

	//�����_�����O�X�e�[�g�ݒ�FZ�o�b�t�@�ւ̏������݃I��
//	pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

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

			//�������̃L�[�����Z�b�g����
			result = m_pPianoKeyboard[_GetKeyboardIndexFromNote(note)]->ResetKey(note.noteNo);
			//if (result != 0) goto EXIT;
		}
		m_pNoteStatus[i].isActive = false;
		m_pNoteStatus[i].keyStatus = BeforeNoteON;
		m_pNoteStatus[i].index = 0;
		m_pNoteStatus[i].keyDownRate = 0.0f;
	}

	return;
}

int MTPianoKeyboardCtrlMod::_GetKeyboardIndexFromNote(const SMNote &note)
{
	//�V���O���L�[�{�[�h�łȂ��ꍇ
	if (!m_isSingleKeyboard) {
		//�m�[�g�̃s�A�m�ԍ����擾
		return m_KeyboardIndex[note.portNo];
	}
	//�V���O���L�[�{�[�h�̏ꍇ
	else {
		//�m�[�g�̃s�A�m�ԍ���0�Œ��
		return 0;
	}
}

//******************************************************************************
// �s�b�`�x���h�V�t�g�̍ő�ʂ����߂�
//******************************************************************************
float MTPianoKeyboardCtrlMod::GetMaxPitchBendShift(unsigned char portNo) {

	float max = 0.0f;
	float cur = 0.0f;

	//�V���O���L�[�{�[�h�łȂ��ꍇ�A�w��̃|�[�g�ԍ����狁�߂�
	//�V���O���L�[�{�[�h�̏ꍇ�A�V�[�P���X�Ɋ܂܂��|�[�g�ԍ����ׂĂ��狁�߂�
	int portListSize = !m_isSingleKeyboard ? 1 : m_PortList.GetSize();

	for (int i = 0; i < portListSize; i++) {

		if (m_isSingleKeyboard) {
			m_PortList.GetPort(i, &portNo);
		}

		if (!m_isActivePort[portNo]) {
			continue;
		}

		for (unsigned char chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {

			float pitchBendShift = _GetPichBendShiftPosX(portNo, chNo);
			if(max < fabs(pitchBendShift)) {

				cur = pitchBendShift;
				max = fabs(pitchBendShift);
			}
		}
	}

	return cur;
}