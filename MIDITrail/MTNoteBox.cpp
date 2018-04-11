//******************************************************************************
//
// MIDITrail / MTNoteBox
//
// �m�[�g�{�b�N�X�`��N���X
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTNoteBox.h"

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
MTNoteBox::MTNoteBox(void)
{
	m_CurTickTime = 0;
	m_CurNoteIndex = 0;
	m_ActiveNoteNum = 0;
	m_pNoteStatus = NULL;
	m_isSkipping = false;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTNoteBox::~MTNoteBox(void)
{
	Release();
}

//******************************************************************************
// ��������
//******************************************************************************
int MTNoteBox::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData,
		MTNotePitchBend* pNotePitchBend
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

	//�g���b�N�擾
	result = pSeqData->GetMergedTrack(&track);
	if (result != 0) goto EXIT;

	//�m�[�g���X�g�擾
	result = track.GetNoteList(&m_NoteList);
	if (result != 0) goto EXIT;

	//�S�m�[�g�{�b�N�X����
	result = _CreateAllNoteBox(pD3DDevice);
	if (result != 0) goto EXIT;

	//�������m�[�g�{�b�N�X�����i�o�b�t�@�m�ہj
	result = _CreateActiveNoteBox(pD3DDevice);
	if (result != 0) goto EXIT;

	//�m�[�g���z�񐶐�
	result = _CreateNoteStatus();
	if (result != 0) goto EXIT;

	//�s�b�`�x���h���
	m_pNotePitchBend = pNotePitchBend;

EXIT:;
	return result;
}

//******************************************************************************
// �S�m�[�g�{�b�N�X����
//******************************************************************************
int MTNoteBox::_CreateAllNoteBox(
		LPDIRECT3DDEVICE9 pD3DDevice
   )
{
	int result = 0;
	unsigned long vertexNum = 0;
	unsigned long indexNum = 0;
	MTNOTEBOX_VERTEX* pVertex = NULL;
	unsigned long* pIndex = NULL;
	unsigned long i = 0;
	D3DMATERIAL9 material;
	SMNote note;

	//�v���~�e�B�u������
	result = m_PrimitiveAllNotes.Initialize(
					sizeof(MTNOTEBOX_VERTEX),	//���_�T�C�Y
					_GetFVFFormat(),			//���_FVF�t�H�[�}�b�g
					D3DPT_TRIANGLELIST			//�v���~�e�B�u���
				);
	if (result != 0) goto EXIT;

	//���_�o�b�t�@����
	vertexNum = NOTE_VERTEX_NUM * m_NoteList.GetSize();
	result = m_PrimitiveAllNotes.CreateVertexBuffer(pD3DDevice, vertexNum);
	if (result != 0) goto EXIT;

	//�C���f�b�N�X�o�b�t�@����
	indexNum = NOTE_INDEX_NUM * m_NoteList.GetSize();
	result = m_PrimitiveAllNotes.CreateIndexBuffer(pD3DDevice, indexNum);
	if (result != 0) goto EXIT;

	//�o�b�t�@�̃��b�N
	result = m_PrimitiveAllNotes.LockVertex((void**)&pVertex);
	if (result != 0) goto EXIT;
	result = m_PrimitiveAllNotes.LockIndex(&pIndex);
	if (result != 0) goto EXIT;

	//�o�b�t�@�ɒ��_�ƃC���f�b�N�X����������
	for (i = 0; i < m_NoteList.GetSize(); i++) {
		result = m_NoteList.GetNote(i, &note);
		if (result != 0) goto EXIT;

		result = _CreateVertexOfNote(
						note,							//�m�[�g���
						&(pVertex[NOTE_VERTEX_NUM * i]),//���_�o�b�t�@�������݈ʒu
						NOTE_VERTEX_NUM * i,			//���_�o�b�t�@�C���f�b�N�X�I�t�Z�b�g
						&(pIndex[NOTE_INDEX_NUM * i])	//�C���f�b�N�X�o�b�t�@�������݈ʒu
					);
		if (result != 0) goto EXIT;
	}

	//�o�b�t�@�̃��b�N����
	result = m_PrimitiveAllNotes.UnlockVertex();
	if (result != 0) goto EXIT;
	result = m_PrimitiveAllNotes.UnlockIndex();
	if (result != 0) goto EXIT;

	//�}�e���A���쐬
	_MakeMaterial(&material);
	m_PrimitiveAllNotes.SetMaterial(material);

EXIT:;
	return result;
}

//******************************************************************************
// �������m�[�g�{�b�N�X�����i�o�b�t�@�m�ہj
//******************************************************************************
int MTNoteBox::_CreateActiveNoteBox(
		LPDIRECT3DDEVICE9 pD3DDevice
   )
{
	int result = 0;
	SMTrack track;
	unsigned long vertexNum = 0;
	unsigned long indexNum = 0;
	MTNOTEBOX_VERTEX* pVertex = NULL;
	unsigned long* pIndex = NULL;
	unsigned long i = 0;
	D3DMATERIAL9 material;
	SMNote note;

	ZeroMemory(&note, sizeof(SMNote));

	m_CurTickTime = 0;
	m_CurNoteIndex = 0;

	//�v���~�e�B�u������
	result = m_PrimitiveActiveNotes.Initialize(
					sizeof(MTNOTEBOX_VERTEX),	//���_�T�C�Y
					_GetFVFFormat(),			//���_FVF�t�H�[�}�b�g
					D3DPT_TRIANGLELIST			//�v���~�e�B�u���
				);
	if (result != 0) goto EXIT;

	//���_�o�b�t�@����
	vertexNum = NOTE_VERTEX_NUM * MTNOTEBOX_MAX_ACTIVENOTE_NUM;
	result = m_PrimitiveActiveNotes.CreateVertexBuffer(pD3DDevice, vertexNum);
	if (result != 0) goto EXIT;

	//�C���f�b�N�X�o�b�t�@����
	indexNum = NOTE_INDEX_NUM * MTNOTEBOX_MAX_ACTIVENOTE_NUM;
	result = m_PrimitiveActiveNotes.CreateIndexBuffer(pD3DDevice, indexNum);
	if (result != 0) goto EXIT;

	//�o�b�t�@�̃��b�N
	result = m_PrimitiveActiveNotes.LockVertex((void**)&pVertex);
	if (result != 0) goto EXIT;
	result = m_PrimitiveActiveNotes.LockIndex(&pIndex);
	if (result != 0) goto EXIT;

	//�o�b�t�@�ɒ��_�ƃC���f�b�N�X����������
	for (i = 0; i < MTNOTEBOX_MAX_ACTIVENOTE_NUM; i++) {
		result = _CreateVertexOfNote(
						note,							//�m�[�g���
						&(pVertex[NOTE_VERTEX_NUM * i]),//���_�o�b�t�@�������݈ʒu
						NOTE_VERTEX_NUM * i,			//���_�o�b�t�@�C���f�b�N�X�I�t�Z�b�g
						&(pIndex[NOTE_INDEX_NUM * i])	//�C���f�b�N�X�o�b�t�@�������݈ʒu
					);
		if (result != 0) goto EXIT;
	}

	//�o�b�t�@�̃��b�N����
	result = m_PrimitiveActiveNotes.UnlockVertex();
	if (result != 0) goto EXIT;
	result = m_PrimitiveActiveNotes.UnlockIndex();
	if (result != 0) goto EXIT;

	//�}�e���A���쐬
	_MakeMaterialForActiveNote(&material);
	m_PrimitiveActiveNotes.SetMaterial(material);

EXIT:;
	return result;
}

//******************************************************************************
// �m�[�g���z�񐶐�
//******************************************************************************
int MTNoteBox::_CreateNoteStatus()
{
	int result = 0;
	unsigned long i = 0;

	//�m�[�g���z�񐶐�
	try {
		m_pNoteStatus = new NoteStatus[MTNOTEBOX_MAX_ACTIVENOTE_NUM];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

	//�m�[�g��ԃ��X�g������
	for (i = 0; i < MTNOTEBOX_MAX_ACTIVENOTE_NUM; i++) {
		m_pNoteStatus[i].isActive = false;
		m_pNoteStatus[i].isHide = false;
		m_pNoteStatus[i].index = 0;
		m_pNoteStatus[i].startTime = 0;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �ړ�
//******************************************************************************
int MTNoteBox::Transform(
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
// �������m�[�g�̒��_����
//******************************************************************************
int MTNoteBox::_TransformActiveNotes(
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
int MTNoteBox::_UpdateStatusOfActiveNotes(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	unsigned long i = 0;
	unsigned long curTime = 0;
	bool isFound = false;
	SMNote note;

	curTime = timeGetTime();

	//�����I���m�[�g�̏���j������
	for (i = 0; i < MTNOTEBOX_MAX_ACTIVENOTE_NUM; i++) {
		if (m_pNoteStatus[i].isActive) {
			result = m_NoteList.GetNote(m_pNoteStatus[i].index, &note);
			if (result != 0) goto EXIT;

			if (note.endTime < m_CurTickTime) {
				if (m_pNoteStatus[i].isHide) {
					result = _ShowNoteBox(m_pNoteStatus[i].index);
					if (result != 0) goto EXIT;
				}
				m_pNoteStatus[i].isActive = false;
				m_pNoteStatus[i].isHide = false;
				m_pNoteStatus[i].index = 0;
				m_pNoteStatus[i].startTime = 0;
			}
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
				if ((m_pNoteStatus[i].isActive)
				 && (m_pNoteStatus[i].index == m_CurNoteIndex)) {
					isFound = true;
					break;
				}
			}
			//�󂢂Ă���Ƃ���ɒǉ�����
			if (!isFound) {
				for (i = 0; i < MTNOTEBOX_MAX_ACTIVENOTE_NUM; i++) {
					if (!(m_pNoteStatus[i].isActive)) {
						m_pNoteStatus[i].isActive = true;
						m_pNoteStatus[i].isHide = false;
						m_pNoteStatus[i].index = m_CurNoteIndex;
						m_pNoteStatus[i].startTime = curTime;
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
// �������m�[�g�̒��_�X�V
//******************************************************************************
int MTNoteBox::_UpdateVertexOfActiveNotes(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	unsigned long i = 0;
	unsigned long activeNoteNum = 0;
	unsigned long curTime = 0;
	unsigned long elapsedTime = 0;
	MTNOTEBOX_VERTEX* pVertex = NULL;
	unsigned long* pIndex = NULL;
	SMNote note;

	curTime = timeGetTime();

	//�o�b�t�@�̃��b�N
	result = m_PrimitiveActiveNotes.LockVertex((void**)&pVertex);
	if (result != 0) goto EXIT;
	result = m_PrimitiveActiveNotes.LockIndex(&pIndex);
	if (result != 0) goto EXIT;

	//�������m�[�g�ɂ��Ē��_���X�V
	for (i = 0; i < MTNOTEBOX_MAX_ACTIVENOTE_NUM; i++) {
		if (m_pNoteStatus[i].isActive) {
			//�m�[�g���擾
			result = m_NoteList.GetNote(m_pNoteStatus[i].index, &note);
			if (result != 0) goto EXIT;

			//�����J�n����̌o�ߎ���
			elapsedTime = curTime - m_pNoteStatus[i].startTime;

			//���_�X�V
			result = _CreateVertexOfNote(
							note,										//�m�[�g���
							&(pVertex[NOTE_VERTEX_NUM * activeNoteNum]),//���_�o�b�t�@�������݈ʒu
							NOTE_VERTEX_NUM * activeNoteNum,			//���_�o�b�t�@�C���f�b�N�X�I�t�Z�b�g
							&(pIndex[NOTE_INDEX_NUM * activeNoteNum]),	//�C���f�b�N�X�o�b�t�@�������݈ʒu
							elapsedTime,								//�����o�ߎ���
							true										//�s�b�`�x���h�K�p
						);
			if (result != 0) goto EXIT;

			//�������m�[�g���s�b�`�x���h�ňړ�����ꍇ
			//�����I���܂ŃI���W�i���̃m�[�g���\���ɂ���
			if (!(m_pNoteStatus[i].isHide)) {
				if ((m_pNotePitchBend->GetValue(note.portNo, note.chNo) != 0)
				 && (m_pNotePitchBend->GetSensitivity(note.portNo, note.chNo) != 0)) {
					result = _HideNoteBox(m_pNoteStatus[i].index);
					if (result != 0) goto EXIT;
					m_pNoteStatus[i].isHide = true;
				}
			}

			activeNoteNum++;
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
// �`��
//******************************************************************************
int MTNoteBox::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
   )
{
	int result = 0;

	//�S�m�[�g�̕`��
	result = m_PrimitiveAllNotes.Draw(pD3DDevice);
	if (result != 0) goto EXIT;

	//�������m�[�g�̕`��
	if (m_ActiveNoteNum > 0) {
		result = m_PrimitiveActiveNotes.Draw(
						pD3DDevice,							//�f�o�C�X
						NULL,								//�e�N�X�`���F�Ȃ�
						(NOTE_INDEX_NUM/3)*m_ActiveNoteNum	//�v���~�e�B�u��
					);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// ���
//******************************************************************************
void MTNoteBox::Release()
{
	m_PrimitiveAllNotes.Release();
	m_PrimitiveActiveNotes.Release();
	m_NoteList.Clear();

	if(m_pNoteStatus != NULL) {
		delete [] m_pNoteStatus;
		m_pNoteStatus = NULL;
	}
}

//******************************************************************************
// �m�[�g�{�b�N�X�̒��_����
//******************************************************************************
int MTNoteBox::_CreateVertexOfNote(
		SMNote note,
		MTNOTEBOX_VERTEX* pVertex,
		unsigned long vertexOffset,
		unsigned long* pIndex,
		unsigned long elapsedTime,
		bool isEnablePitchBend
	)
{
	int result = 0;
	unsigned long i;
	D3DXVECTOR3 vectorStartLU;
	D3DXVECTOR3 vectorStartRU;
	D3DXVECTOR3 vectorStartLD;
	D3DXVECTOR3 vectorStartRD;
	D3DXVECTOR3 vectorEndLU;
	D3DXVECTOR3 vectorEndRU;
	D3DXVECTOR3 vectorEndLD;
	D3DXVECTOR3 vectorEndRD;
	D3DXCOLOR color;
	short pitchBendValue = 0;
	unsigned char pitchBendSensitivity = SM_DEFAULT_PITCHBEND_SENSITIVITY;

	if (isEnablePitchBend) {
		pitchBendValue =       m_pNotePitchBend->GetValue(note.portNo, note.chNo);
		pitchBendSensitivity = m_pNotePitchBend->GetSensitivity(note.portNo, note.chNo);
	}

	//     +   1+----+3   +
	//    /|   / �� /    /|         y x
	//   + | 0+----+2   + |�E       |/
	// ��| +   7+----+5 | +      z--+0
	//   |/    / �� /   |/
	//   +   6+----+4   + �� 4 �����_(0,0,0)
	//

	//�m�[�g�{�b�N�X���_���W�擾
	m_NoteDesign.GetNoteBoxVirtexPos(
			note.startTime,
			note.portNo,
			note.chNo,
			note.noteNo,
			&vectorStartLU,
			&vectorStartRU,
			&vectorStartLD,
			&vectorStartRD,
			pitchBendValue,
			pitchBendSensitivity
		);
	m_NoteDesign.GetNoteBoxVirtexPos(
			note.endTime,
			note.portNo,
			note.chNo,
			note.noteNo,
			&vectorEndLU,
			&vectorEndRU,
			&vectorEndLD,
			&vectorEndRD,
			pitchBendValue,
			pitchBendSensitivity
		);

	//���_���W�E�E�E�@�����قȂ�̂Œ��_��8�ɏW��ł��Ȃ�
	//��̖�
	pVertex[0].p = vectorStartLU;
	pVertex[1].p = vectorEndLU;
	pVertex[2].p = vectorStartRU;
	pVertex[3].p = vectorEndRU;
	//���̖�
	pVertex[4].p = vectorStartRD;
	pVertex[5].p = vectorEndRD;
	pVertex[6].p = vectorStartLD;
	pVertex[7].p = vectorEndLD;
	//�E�̖�
	pVertex[8].p  = pVertex[2].p;
	pVertex[9].p  = pVertex[3].p;
	pVertex[10].p = pVertex[4].p;
	pVertex[11].p = pVertex[5].p;
	//���̖�
	pVertex[12].p = pVertex[6].p;
	pVertex[13].p = pVertex[7].p;
	pVertex[14].p = pVertex[0].p;
	pVertex[15].p = pVertex[1].p;
	//�O�̖�
	pVertex[16].p = pVertex[0].p;
	pVertex[17].p = pVertex[2].p;
	pVertex[18].p = pVertex[6].p;
	pVertex[19].p = pVertex[4].p;
	//��̖�
	pVertex[20].p = pVertex[3].p;
	pVertex[21].p = pVertex[1].p;
	pVertex[22].p = pVertex[5].p;
	pVertex[23].p = pVertex[7].p;

	//�@��
	//��̖�
	pVertex[0].n = D3DXVECTOR3( 0.0f, 1.0f, 0.0f);
	pVertex[1].n = D3DXVECTOR3( 0.0f, 1.0f, 0.0f);
	pVertex[2].n = D3DXVECTOR3( 0.0f, 1.0f, 0.0f);
	pVertex[3].n = D3DXVECTOR3( 0.0f, 1.0f, 0.0f);
	//���̖�
	pVertex[4].n = D3DXVECTOR3( 0.0f,-1.0f, 0.0f);
	pVertex[5].n = D3DXVECTOR3( 0.0f,-1.0f, 0.0f);
	pVertex[6].n = D3DXVECTOR3( 0.0f,-1.0f, 0.0f);
	pVertex[7].n = D3DXVECTOR3( 0.0f,-1.0f, 0.0f);
	//�E�̖�
	pVertex[8].n  = D3DXVECTOR3( 0.0f, 0.0f,-1.0f);
	pVertex[9].n  = D3DXVECTOR3( 0.0f, 0.0f,-1.0f);
	pVertex[10].n = D3DXVECTOR3( 0.0f, 0.0f,-1.0f);
	pVertex[11].n = D3DXVECTOR3( 0.0f, 0.0f,-1.0f);
	//���̖�
	pVertex[12].n = D3DXVECTOR3( 0.0f, 0.0f, 1.0f);
	pVertex[13].n = D3DXVECTOR3( 0.0f, 0.0f, 1.0f);
	pVertex[14].n = D3DXVECTOR3( 0.0f, 0.0f, 1.0f);
	pVertex[15].n = D3DXVECTOR3( 0.0f, 0.0f, 1.0f);
	//�O�̖�
	pVertex[16].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
	pVertex[17].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
	pVertex[18].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
	pVertex[19].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
	//��̖�
	pVertex[20].n = D3DXVECTOR3( 1.0f, 0.0f, 0.0f);
	pVertex[21].n = D3DXVECTOR3( 1.0f, 0.0f, 0.0f);
	pVertex[22].n = D3DXVECTOR3( 1.0f, 0.0f, 0.0f);
	pVertex[23].n = D3DXVECTOR3( 1.0f, 0.0f, 0.0f);

	//�e���_�̃f�B�t���[�Y�F
	if (elapsedTime == 0xFFFFFFFF) {
		color = m_NoteDesign.GetNoteBoxColor(note.portNo, note.chNo, note.noteNo);
	}
	else {
		//�������͔����J�n����̌o�ߎ��Ԃɂ���ĐF���ω�����
		color = m_NoteDesign.GetActiveNoteBoxColor(note.portNo, note.chNo, note.noteNo, elapsedTime);
	}

	//���_�̐F�ݒ芮��
	for (i = 0; i < NOTE_VERTEX_NUM; i++) {
		pVertex[i].c = color;
	}

	//�C���f�b�N�X�FDrawIndexdPrimitive�Ăяo����1��ōςނ悤��TRIANGLELIST�Ƃ���
	for (i = 0; i < NOTE_INDEX_NUM; i++) {
		pIndex[i] = vertexOffset + _GetVertexIndexOfNote(i);
	}

	return result;
}

//******************************************************************************
// �m�[�g���_�C���f�b�N�X�擾
//******************************************************************************
unsigned long MTNoteBox::_GetVertexIndexOfNote(
		unsigned long index
	)
{
	unsigned long vertexIndex = 0;

	unsigned long vertexIndexes[NOTE_INDEX_NUM] = {
		//TRIANGLE-1   TRIANGLE-2
		 0,  1,  2,     2,  1,  3,	//��
		 4,  5,  6,     6,  5,  7,	//��
		 8,  9, 10,    10,  9, 11,	//�E
		12, 13, 14,    14, 13, 15,	//��
		16, 17, 18,    18, 17, 19,	//�O
		20, 21, 22,    22, 21, 23,	//��
	};

	if (index < NOTE_INDEX_NUM) {
		vertexIndex = vertexIndexes[index];
	}

	return vertexIndex;
}

//******************************************************************************
// �}�e���A���쐬
//******************************************************************************
void MTNoteBox::_MakeMaterial(
		D3DMATERIAL9* pMaterial
	)
{
	ZeroMemory(pMaterial, sizeof(D3DMATERIAL9));
	
	//�g�U��
	pMaterial->Diffuse.r = 1.0f;
	pMaterial->Diffuse.g = 1.0f;
	pMaterial->Diffuse.b = 1.0f;
	pMaterial->Diffuse.a = 1.0f;
	//�����F�e�̐F
	pMaterial->Ambient.r = 0.5f;
	pMaterial->Ambient.g = 0.5f;
	pMaterial->Ambient.b = 0.5f;
	pMaterial->Ambient.a = 1.0f;
	//���ʔ��ˌ�
	pMaterial->Specular.r = 0.2f;
	pMaterial->Specular.g = 0.2f;
	pMaterial->Specular.b = 0.2f;
	pMaterial->Specular.a = 1.0f;
	//���ʔ��ˌ��̑N���x
	pMaterial->Power = 40.0f;
	//�����F
	pMaterial->Emissive.r = 0.0f;
	pMaterial->Emissive.g = 0.0f;
	pMaterial->Emissive.b = 0.0f;
	pMaterial->Emissive.a = 0.0f;
}

//******************************************************************************
// �}�e���A���쐬�F�������m�[�g�p
//******************************************************************************
void MTNoteBox::_MakeMaterialForActiveNote(
		D3DMATERIAL9* pMaterial
	)
{
	ZeroMemory(pMaterial, sizeof(D3DMATERIAL9));
	
	//�g�U��
	pMaterial->Diffuse.r = 1.0f;
	pMaterial->Diffuse.g = 1.0f;
	pMaterial->Diffuse.b = 1.0f;
	pMaterial->Diffuse.a = 1.0f;
	//�����F�e�̐F
	pMaterial->Ambient.r = 0.5f;
	pMaterial->Ambient.g = 0.5f;
	pMaterial->Ambient.b = 0.5f;
	pMaterial->Ambient.a = 1.0f;
	//���ʔ��ˌ�
	pMaterial->Specular.r = 0.2f;
	pMaterial->Specular.g = 0.2f;
	pMaterial->Specular.b = 0.2f;
	pMaterial->Specular.a = 1.0f;
	//���ʔ��ˌ��̑N���x
	pMaterial->Power = 40.0f;
	//�����F
	pMaterial->Emissive = m_NoteDesign.GetActiveNoteEmissive();
}

//******************************************************************************
// �J�����g�`�b�N�^�C���ݒ�
//******************************************************************************
void MTNoteBox::SetCurTickTime(
		unsigned long curTickTime
	)
{
	m_CurTickTime = curTickTime;
}

//******************************************************************************
// ���Z�b�g
//******************************************************************************
void MTNoteBox::Reset()
{
	int result = 0;
	unsigned long i = 0;

	m_CurTickTime = 0;
	m_CurNoteIndex = 0;
	m_ActiveNoteNum = 0;

	for (i = 0; i < MTNOTEBOX_MAX_ACTIVENOTE_NUM; i++) {

		//��\���ɂ��Ă���m�[�g�𕜌�����
		if (m_pNoteStatus[i].isHide) {
			result = _ShowNoteBox(m_pNoteStatus[i].index);
			//if (result != 0) goto EXIT;
		}

		m_pNoteStatus[i].isActive = false;
		m_pNoteStatus[i].isHide = false;
		m_pNoteStatus[i].index = 0;
		m_pNoteStatus[i].startTime = 0;
	}

	return;
}

//******************************************************************************
// �m�[�g�{�b�N�X��\��
//******************************************************************************
int MTNoteBox::_HideNoteBox(
		unsigned long index
	)
{
	int result = 0;
	unsigned long offset = 0;
	unsigned long size = 0;
	unsigned long i = 0;
	unsigned long* pIndex = NULL;

	offset = sizeof(unsigned long) * NOTE_INDEX_NUM * index;
	size = sizeof(unsigned long) * NOTE_INDEX_NUM;

	result = m_PrimitiveAllNotes.LockIndex(&pIndex, offset, size);
	if (result != 0) goto EXIT;

	for (i = 0; i < NOTE_INDEX_NUM; i++) {
		pIndex[i] = NOTE_VERTEX_NUM * index;
	}

	result = m_PrimitiveAllNotes.UnlockIndex();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �m�[�g�{�b�N�X�\��
//******************************************************************************
int MTNoteBox::_ShowNoteBox(
		unsigned long index
	)
{
	int result = 0;
	unsigned long offset = 0;
	unsigned long size = 0;
	unsigned long i = 0;
	unsigned long* pIndex = NULL;

	offset = sizeof(unsigned long) * NOTE_INDEX_NUM * index;
	size = sizeof(unsigned long) * NOTE_INDEX_NUM;

	result = m_PrimitiveAllNotes.LockIndex(&pIndex, offset, size);
	if (result != 0) goto EXIT;

	for (i = 0; i < NOTE_INDEX_NUM; i++) {
		pIndex[i] = NOTE_VERTEX_NUM * index + _GetVertexIndexOfNote(i);
	}

	result = m_PrimitiveAllNotes.UnlockIndex();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �X�L�b�v��Ԑݒ�
//******************************************************************************
void MTNoteBox::SetSkipStatus(
		bool isSkipping
	)
{
	m_isSkipping = isSkipping;
}

