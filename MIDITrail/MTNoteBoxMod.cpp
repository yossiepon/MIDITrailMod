//******************************************************************************
//
// MIDITrail / MTNoteBoxMod
//
// �m�[�g�{�b�N�X�`��Mod�N���X
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTNoteBoxMod.h"

using namespace YNBaseLib;


//******************************************************************************
// �p�����[�^��`
//******************************************************************************
//1�m�[�g������̒��_�� = 1�����`4���_ * 6�� 
#define NOTE_VERTEX_NUM  (4 * 6)

//1�m�[�g������̃C���f�b�N�X�� = 1�O�p�`3���_ * 2�� * 6��
#define NOTE_INDEX_NUM   (3 * 2 * 6)

//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTNoteBoxMod::MTNoteBoxMod(void) : MTNoteBox()
{
	m_pNoteStatusMod = NULL;
	ZeroMemory(m_KeyDownRate, sizeof(float) * MT_NOTEBOX_MAX_PORT_NUM * SM_MAX_CH_NUM * SM_MAX_NOTE_NUM);
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTNoteBoxMod::~MTNoteBoxMod(void)
{
	Release();
}

//******************************************************************************
// ��������
//******************************************************************************
int MTNoteBoxMod::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData,
		MTNotePitchBend* pNotePitchBend
	)
{
	int result = 0;
	SMTrack track;

	Release();

	// ���N���X�̐����������Ăяo��
	result = MTNoteBox::Create(pD3DDevice, pSceneName, pSeqData, pNotePitchBend);
	if (result != 0) goto EXIT;

	//�m�[�g�f�U�C��Mod�I�u�W�F�N�g������
	result = m_NoteDesignMod.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	//�g���b�N�擾
	result = pSeqData->GetMergedTrack(&track);
	if (result != 0) goto EXIT;

	//�m�[�g���X�g�擾�FstartTime, endTime �̓��A���^�C��(msec)
	result = track.GetNoteListWithRealTime(&m_NoteListRT, pSeqData->GetTimeDivision());
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �m�[�g���z�񐶐�
//******************************************************************************
int MTNoteBoxMod::_CreateNoteStatus()
{
	int result = 0;
	unsigned long i = 0;

	//�m�[�g���z�񐶐�
	try {
		m_pNoteStatusMod = new NoteStatusMod[MTNOTEBOX_MAX_ACTIVENOTE_NUM];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

	//�m�[�g��ԃ��X�g������
	for (i = 0; i < MTNOTEBOX_MAX_ACTIVENOTE_NUM; i++) {
		m_pNoteStatusMod[i].isActive = false;
		m_pNoteStatusMod[i].isHide = false;
		m_pNoteStatusMod[i].index = 0;
		m_pNoteStatusMod[i].keyStatus = BeforeNoteON;
		m_pNoteStatusMod[i].keyDownRate = 0.0f;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �ړ�
//******************************************************************************
int MTNoteBoxMod::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice,
		float rollAngle
	)
{
	int result = 0;
	D3DXVECTOR3 moveVector;
	D3DXMATRIX rotateMatrix;
	D3DXMATRIX moveMatrix;
	D3DXMATRIX worldMatrix;

	//���ݔ������m�[�g�̒��_����
	result = _TransformActiveNotes(pD3DDevice);
	if (result != 0) goto EXIT;

	//�s�񏉊���
	D3DXMatrixIdentity(&rotateMatrix);
	D3DXMatrixIdentity(&moveMatrix);
	D3DXMatrixIdentity(&worldMatrix);

	//��]�s��
	//TODO: ini �Ő؂�ւ�����悤�ɂ���
	//D3DXMatrixRotationX(&rotateMatrix, D3DXToRadian(rollAngle + 180.0f));
	D3DXMatrixRotationX(&rotateMatrix, D3DXToRadian(rollAngle));

	//�ړ��s��
	moveVector = m_NoteDesign.GetWorldMoveVector();
	D3DXMatrixTranslation(&moveMatrix, moveVector.x, moveVector.y, moveVector.z);

	//�s��̍���
	D3DXMatrixMultiply(&worldMatrix, &rotateMatrix, &moveMatrix);

	//�ϊ��s��ݒ�
	m_PrimitiveAllNotes.Transform(worldMatrix);
	m_PrimitiveActiveNotes.Transform(worldMatrix);

EXIT:;
	return result;
}

//******************************************************************************
// �������m�[�g�̏�ԍX�V
//******************************************************************************
int MTNoteBoxMod::_UpdateStatusOfActiveNotes(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	unsigned long i = 0;
	bool isFound = false;
	SMNote note;

	//�g��f�B�P�C�E�����[�X����(msec)
	unsigned long decayDuration = m_NoteDesignMod.GetRippleDecayDuration();
	unsigned long releaseDuration   = m_NoteDesignMod.GetRippleReleaseDuration();

	//�m�[�g�����X�V����
	for (i = 0; i < MTNOTEBOX_MAX_ACTIVENOTE_NUM; i++) {
		if (m_pNoteStatusMod[i].isActive) {
			//�m�[�g���擾
			result = m_NoteListRT.GetNote(m_pNoteStatusMod[i].index, &note);
			if (result != 0) goto EXIT;

			//�������m�[�g��ԍX�V
			result = _UpdateNoteStatus(
							m_PlayTimeMSec,
							decayDuration,
							releaseDuration,
							note,
							&(m_pNoteStatusMod[i])
						);
			if (result != 0) goto EXIT;
		}
	}

	//�O�񌟍��I���ʒu���甭���J�n�m�[�g������
	while (m_CurNoteIndex < m_NoteList.GetSize()) {
		//�m�[�g���擾
		result = m_NoteList.GetNote(m_CurNoteIndex, &note);
		if (result != 0) goto EXIT;

		//���݃`�b�N�^�C���������J�n�`�b�N�^�C���ɂ��ǂ���Ă��Ȃ���Ό����I��
		if (m_CurTickTime < note.startTime) break;

		//�������m�[�g��o�^
		if ((note.startTime <= m_CurTickTime) && (m_CurTickTime <= note.endTime)) {
			//���łɓo�^�ς݂Ȃ牽�����Ȃ�
			isFound = false;
			for (i = 0; i < MTNOTEBOX_MAX_ACTIVENOTE_NUM; i++) {
				if ((m_pNoteStatusMod[i].isActive)
				 && (m_pNoteStatusMod[i].index == m_CurNoteIndex)) {
					isFound = true;
					break;
				}
			}
			//�󂢂Ă���Ƃ���ɒǉ�����
			if (!isFound) {
				for (i = 0; i < MTNOTEBOX_MAX_ACTIVENOTE_NUM; i++) {
					if (!(m_pNoteStatusMod[i].isActive)) {
						m_pNoteStatusMod[i].isActive = true;
						m_pNoteStatusMod[i].isHide = false;
						m_pNoteStatusMod[i].index = m_CurNoteIndex;
						m_pNoteStatusMod[i].keyStatus = BeforeNoteON;
						m_pNoteStatusMod[i].keyDownRate = 0.0f;
						break;
					}
				}
			}
		}
		m_CurNoteIndex++;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �������m�[�g��ԍX�V
//******************************************************************************
int MTNoteBoxMod::_UpdateNoteStatus(
		unsigned long playTimeMSec,
		unsigned long decayDuration,
		unsigned long releaseDuration,
		SMNote note,
		NoteStatusMod* pNoteStatus
	)
{
	int result= 0;

	//�����I���m�[�g
	if(playTimeMSec > note.endTime) {
		if(pNoteStatus->isHide) {
			result = _ShowNoteBox(pNoteStatus->index);
			if (result != 0) goto EXIT;
		}
		//�m�[�g����j��
		pNoteStatus->isActive = false;
		pNoteStatus->isHide = false;
		pNoteStatus->keyStatus = BeforeNoteON;
		pNoteStatus->index = 0;
		pNoteStatus->keyDownRate = 0.0f;

		goto EXIT;
	}

	unsigned long noteLen = note.endTime - note.startTime;

	float decayRatio = 0.3f;
	float sustainRatio = 0.4f;
	float releaseRatio = 0.3f;

	//�g��f�B�P�C���Ԃ���������蒷���ꍇ�A�f�B�P�C���������Ԃ܂łɏC������
	if(noteLen < decayDuration) {

		//decayDuration = noteLen;
		//releaseDuration = 0;

		//decayRatio = 0.3f;
		//sustainRatio = 0.0f;
		//releaseRatio = 0.0f;
	}
	//�g��f�B�P�C�{�����[�X���Ԃ���������蒷���ꍇ�A�����[�X�J�n���Ԃ��f�B�P�C���Ԍo�ߒ���ɏC������
	else if(noteLen < (decayDuration + releaseDuration)) {

		releaseDuration = noteLen - decayDuration;
		decayRatio = 0.5f;
		sustainRatio = 0.0f;
		releaseRatio = 0.5f;
	}
	//���������i�g��f�B�P�C�{�����[�X���ԁj�~�Q�ȓ��̏ꍇ�ASustainRatio��0.0�`0.4�͈̔͂ŕω�������
	else if(noteLen < (decayDuration + releaseDuration) * 2) {

		sustainRatio = 0.4f * (float)(noteLen - (decayDuration + releaseDuration)) / (float)noteLen;
		decayRatio = (1.0f - sustainRatio) / 2.0f;
		releaseRatio = decayRatio;
	}

	//�m�[�gON��i�������j
	if (playTimeMSec < (note.startTime + decayDuration)) {
		pNoteStatus->keyStatus = BeforeNoteON;
		if (decayDuration == 0) {
			pNoteStatus->keyDownRate = 0.0f;
		}
		else {
			pNoteStatus->keyDownRate = decayRatio * (float)(playTimeMSec - note.startTime) / (float)decayDuration;
		}
	}
	//�m�[�gON�����ォ�烊���[�X�O�܂�
	else if (((note.startTime + decayDuration) <= playTimeMSec)
			&& (playTimeMSec <= (note.endTime - releaseDuration))) {
		pNoteStatus->keyStatus = NoteON;

		unsigned long denominator = noteLen - (decayDuration + releaseDuration);
		if(denominator > 0) {
			pNoteStatus->keyDownRate = decayRatio + sustainRatio
					* (float)(playTimeMSec - (note.startTime + decayDuration)) / (float)denominator;
		} else {
			pNoteStatus->keyDownRate = decayRatio + sustainRatio;
		}
	}
	//�m�[�gOFF�O�i�����[�X���j
	else if (((note.endTime - releaseDuration) < playTimeMSec) && (playTimeMSec <= note.endTime)) {
		pNoteStatus->keyStatus = AfterNoteOFF;
		if (releaseDuration == 0) {
			pNoteStatus->keyDownRate = 1.0f;
		}
		else {
			pNoteStatus->keyDownRate = decayRatio + sustainRatio + releaseRatio
					* (float)(playTimeMSec - (note.endTime - releaseDuration)) / (float)releaseDuration;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// �������m�[�g�̒��_�X�V
//******************************************************************************
int MTNoteBoxMod::_UpdateVertexOfActiveNotes(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	unsigned long i = 0;
	unsigned long activeNoteNum = 0;
	MTNOTEBOX_VERTEX* pVertex = NULL;
	unsigned long* pIndex = NULL;
	SMNote note;

	//�o�b�t�@�̃��b�N
	result = m_PrimitiveActiveNotes.LockVertex((void**)&pVertex);
	if (result != 0) goto EXIT;
	result = m_PrimitiveActiveNotes.LockIndex(&pIndex);
	if (result != 0) goto EXIT;

	ZeroMemory(m_KeyDownRate, sizeof(float) * MT_NOTEBOX_MAX_PORT_NUM * SM_MAX_CH_NUM * SM_MAX_NOTE_NUM);

	//�������m�[�g�ɂ��Ē��_���X�V
	for (i = 0; i < MTNOTEBOX_MAX_ACTIVENOTE_NUM; i++) {
		if (m_pNoteStatusMod[i].isActive) {
			//�m�[�g���擾
			result = m_NoteList.GetNote(m_pNoteStatusMod[i].index, &note);
			if (result != 0) goto EXIT;

			//���_�X�V
			result = _CreateVertexOfNote(
							note,										//�m�[�g���
							&(pVertex[NOTE_VERTEX_NUM * activeNoteNum]),//���_�o�b�t�@�������݈ʒu
							NOTE_VERTEX_NUM * activeNoteNum,			//���_�o�b�t�@�C���f�b�N�X�I�t�Z�b�g
							&(pIndex[NOTE_INDEX_NUM * activeNoteNum]),	//�C���f�b�N�X�o�b�t�@�������݈ʒu
							m_pNoteStatusMod[i].keyDownRate,				//�m�[�g���
							true										//�s�b�`�x���h�K�p
						);
			if (result != 0) goto EXIT;

			//�������m�[�g���s�b�`�x���h�ňړ�����ꍇ
			//�����I���܂ŃI���W�i���̃m�[�g���\���ɂ���
			if (!(m_pNoteStatusMod[i].isHide)) {
				if ((m_pNotePitchBend->GetValue(note.portNo, note.chNo) != 0)
				 && (m_pNotePitchBend->GetSensitivity(note.portNo, note.chNo) != 0)) {
					result = _HideNoteBox(m_pNoteStatusMod[i].index);
					if (result != 0) goto EXIT;
					m_pNoteStatusMod[i].isHide = true;
				}
			}

			activeNoteNum++;
			m_KeyDownRate[note.portNo][note.chNo][note.noteNo] = m_pNoteStatusMod[i].keyDownRate;
		}
	}
	m_ActiveNoteNum = activeNoteNum;

	//�o�b�t�@�̃��b�N����
	result = m_PrimitiveActiveNotes.UnlockVertex();
	if (result != 0) goto EXIT;
	result = m_PrimitiveActiveNotes.UnlockIndex();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ���
//******************************************************************************
void MTNoteBoxMod::Release()
{
	if(m_pNoteStatusMod != NULL) {
		delete [] m_pNoteStatusMod;
		m_pNoteStatusMod = NULL;
	}
}

//******************************************************************************
// �m�[�g�{�b�N�X�̒��_����
//******************************************************************************
int MTNoteBoxMod::_CreateVertexOfNote(
		SMNote note,
		MTNOTEBOX_VERTEX* pVertex,
		unsigned long vertexOffset,
		unsigned long* pIndex,
		float keyDownRate,
		bool isEnablePitchBend
	)
{
	unsigned long i;
	D3DXCOLOR color;

	// ���N���X�̒��_���������̌Ăяo��
	MTNoteBox::_CreateVertexOfNote(note, pVertex, vertexOffset, pIndex, -1, isEnablePitchBend);

	//���N���X�̐ݒ�F���㏑������

	//�e���_�̃f�B�t���[�Y�F
	if (keyDownRate == 0.0f) {
		color = m_NoteDesign.GetNoteBoxColor(note.portNo, note.chNo, note.noteNo);
	}
	else {
		//�������͔����J�n����̌o�ߎ��Ԃɂ���ĐF���ω�����
		color = m_NoteDesignMod.GetActiveNoteBoxColor(note.portNo, note.chNo, note.noteNo, keyDownRate);
	}

	//���_�̐F�ݒ芮��
	for (i = 0; i < NOTE_VERTEX_NUM; i++) {
		pVertex[i].c = color;
	}

	return 0;
}

//******************************************************************************
// ���t���Ԑݒ�
//******************************************************************************
void MTNoteBoxMod::SetPlayTimeMSec(
		unsigned long playTimeMsec
	)
{
	m_PlayTimeMSec = playTimeMsec;
}

//******************************************************************************
// ���Z�b�g
//******************************************************************************
void MTNoteBoxMod::Reset()
{
	int result = 0;
	unsigned long i = 0;

	m_CurTickTime = 0;
	m_CurNoteIndex = 0;
	m_ActiveNoteNum = 0;

	for (i = 0; i < MTNOTEBOX_MAX_ACTIVENOTE_NUM; i++) {

		//��\���ɂ��Ă���m�[�g�𕜌�����
		if (m_pNoteStatusMod[i].isHide) {
			result = _ShowNoteBox(m_pNoteStatusMod[i].index);
			//if (result != 0) goto EXIT;
		}

		m_pNoteStatusMod[i].isActive = false;
		m_pNoteStatusMod[i].isHide = false;
		m_pNoteStatusMod[i].index = 0;
		m_pNoteStatusMod[i].keyStatus = BeforeNoteON;
		m_pNoteStatusMod[i].keyDownRate = 0.0f;
	}

	return;
}