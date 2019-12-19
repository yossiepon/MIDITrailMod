//******************************************************************************
//
// MIDITrail / MTNoteBoxLive
//
// ���C�u���j�^�p�m�[�g�{�b�N�X�`��N���X
//
// Copyright (C) 2012-2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTNoteBoxLive.h"

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
MTNoteBoxLive::MTNoteBoxLive(void)
{
	m_pNoteDesign = NULL;
	m_NoteNum = 0;
	m_pNoteStatus = NULL;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTNoteBoxLive::~MTNoteBoxLive(void)
{
	Release();
}

//******************************************************************************
// ��������
//******************************************************************************
int MTNoteBoxLive::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName,
		MTNotePitchBend* pNotePitchBend
	)
{
	int result = 0;
	
	Release();

	//�m�[�g�f�U�C���I�u�W�F�N�g����
	result = _CreateNoteDesign();
	if (result != 0) goto EXIT;

	//�m�[�g�f�U�C���I�u�W�F�N�g������
	result = m_pNoteDesign->Initialize(pSceneName, NULL);
	if (result != 0) goto EXIT;
	
	//���C�u���j�^�\������
	m_LiveMonitorDisplayDuration = m_pNoteDesign->GetLiveMonitorDisplayDuration();
	
	//�m�[�g���z�񐶐�
	result = _CreateNoteStatus();
	if (result != 0) goto EXIT;
	
	//�m�[�g�{�b�N�X�����i�o�b�t�@�m�ہj
	result = _CreateNoteBox(pD3DDevice);
	if (result != 0) goto EXIT;
	
	//�s�b�`�x���h���
	m_pNotePitchBend = pNotePitchBend;
	
EXIT:;
	return result;
}

//******************************************************************************
// �m�[�g�f�U�C������
//******************************************************************************
int MTNoteBoxLive::_CreateNoteDesign()
{
	int result = 0;

	try {
		m_pNoteDesign = new MTNoteDesign();
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �m�[�g���z�񐶐�
//******************************************************************************
int MTNoteBoxLive::_CreateNoteStatus()
{
	int result = 0;
	unsigned long i = 0;
	
	//�m�[�g���z�񐶐�
	try {
		m_pNoteStatus = new NoteStatus[MTNOTEBOX_MAX_LIVENOTE_NUM];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}
	
	//�m�[�g��ԃ��X�g������
	for (i = 0; i < MTNOTEBOX_MAX_LIVENOTE_NUM; i++) {
		m_pNoteStatus[i].isActive = false;
		m_pNoteStatus[i].portNo = 0;
		m_pNoteStatus[i].chNo = 0;
		m_pNoteStatus[i].noteNo = 0;
		m_pNoteStatus[i].startTime = 0;
		m_pNoteStatus[i].endTime = 0;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// �������m�[�g�{�b�N�X�����i�o�b�t�@�m�ہj
//******************************************************************************
int MTNoteBoxLive::_CreateNoteBox(
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
	NoteStatus note;
	
	memset(&note, 0, sizeof(NoteStatus));
	
	//�v���~�e�B�u������
	result = m_PrimitiveNotes.Initialize(
					sizeof(MTNOTEBOX_VERTEX),	//���_�T�C�Y
					_GetFVFFormat(),			//���_FVF�t�H�[�}�b�g
					D3DPT_TRIANGLELIST			//�v���~�e�B�u���
				);
	if (result != 0) goto EXIT;
	
	//���_�o�b�t�@����
	vertexNum = NOTE_VERTEX_NUM * MTNOTEBOX_MAX_LIVENOTE_NUM;
	result = m_PrimitiveNotes.CreateVertexBuffer(pD3DDevice, vertexNum);
	if (result != 0) goto EXIT;
	
	//�C���f�b�N�X�o�b�t�@����
	indexNum = NOTE_INDEX_NUM * MTNOTEBOX_MAX_LIVENOTE_NUM;
	result = m_PrimitiveNotes.CreateIndexBuffer(pD3DDevice, indexNum);
	if (result != 0) goto EXIT;
	
	//�o�b�t�@�̃��b�N
	result = m_PrimitiveNotes.LockVertex((void**)&pVertex);
	if (result != 0) goto EXIT;
	result = m_PrimitiveNotes.LockIndex(&pIndex);
	if (result != 0) goto EXIT;
	
	//�o�b�t�@�ɒ��_�ƃC���f�b�N�X����������
	for (i = 0; i < MTNOTEBOX_MAX_LIVENOTE_NUM; i++) {
		result = _CreateVertexOfNote(
						m_pNoteStatus[i],				//�m�[�g���
						&(pVertex[NOTE_VERTEX_NUM * i]),//���_�o�b�t�@�������݈ʒu
						NOTE_VERTEX_NUM * i,			//���_�o�b�t�@�C���f�b�N�X�I�t�Z�b�g
						&(pIndex[NOTE_INDEX_NUM * i]),	//�C���f�b�N�X�o�b�t�@�������݈ʒu
						0								//���ݎ���
					);
		if (result != 0) goto EXIT;
	}
	
	//�o�b�t�@�̃��b�N����
	result = m_PrimitiveNotes.UnlockVertex();
	if (result != 0) goto EXIT;
	result = m_PrimitiveNotes.UnlockIndex();
	if (result != 0) goto EXIT;
	
	//�}�e���A���쐬
	_MakeMaterial(&material);
	m_PrimitiveNotes.SetMaterial(material);
	
EXIT:;
	return result;
}

//******************************************************************************
// �ړ�
//******************************************************************************
int MTNoteBoxLive::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice,
		float rollAngle
	)
{
	int result = 0;
	D3DXVECTOR3 moveVector;
	D3DXMATRIX rotateMatrix;
	D3DXMATRIX moveMatrix;
	D3DXMATRIX worldMatrix;
	
	//�m�[�g�̒��_����
	result = _TransformNotes(pD3DDevice);
	if (result != 0) goto EXIT;
	
	//�s�񏉊���
	D3DXMatrixIdentity(&rotateMatrix);
	D3DXMatrixIdentity(&moveMatrix);
	D3DXMatrixIdentity(&worldMatrix);
	
	//��]�s��
	D3DXMatrixRotationX(&rotateMatrix, D3DXToRadian(rollAngle));
	
	//�ړ��s��
	moveVector = m_pNoteDesign->GetWorldMoveVector();
	D3DXMatrixTranslation(&moveMatrix, moveVector.x, moveVector.y, moveVector.z);
	
	//�s��̍���
	D3DXMatrixMultiply(&worldMatrix, &rotateMatrix, &moveMatrix);
	
	//�ϊ��s��ݒ�
	m_PrimitiveNotes.Transform(worldMatrix);
	
EXIT:;
	return result;
}

//******************************************************************************
// �m�[�g�̒��_����
//******************************************************************************
int MTNoteBoxLive::_TransformNotes(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	
	//�m�[�g�̏�ԍX�V
	result = _UpdateStatusOfNotes(pD3DDevice);
	if (result != 0) goto EXIT;
	
	//�m�[�g�̒��_�X�V
	result = _UpdateVertexOfNotes(pD3DDevice);
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// �m�[�g�̏�ԍX�V
//******************************************************************************
int MTNoteBoxLive::_UpdateStatusOfNotes(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	unsigned long i = 0;
	unsigned long curTime = 0;
	
	curTime = timeGetTime();
	
	//�Â��m�[�g����j������
	for (i = 0; i < MTNOTEBOX_MAX_LIVENOTE_NUM; i++) {
		if (m_pNoteStatus[i].isActive) {
			if ((m_pNoteStatus[i].endTime != 0)
				&& ((curTime - m_pNoteStatus[i].endTime) > m_LiveMonitorDisplayDuration)) {
				m_pNoteStatus[i].isActive = false;
				m_pNoteStatus[i].portNo = 0;
				m_pNoteStatus[i].chNo = 0;
				m_pNoteStatus[i].noteNo = 0;
				m_pNoteStatus[i].startTime = 0;
				m_pNoteStatus[i].endTime = 0;
			}
		}
	}
	
//EXIT:;
	return result;
}

//******************************************************************************
// �m�[�g�̒��_�X�V
//******************************************************************************
int MTNoteBoxLive::_UpdateVertexOfNotes(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	unsigned long i = 0;
	unsigned long noteNum = 0;
	unsigned long curTime = 0;
	MTNOTEBOX_VERTEX* pVertex = NULL;
	unsigned long* pIndex = NULL;
	
	curTime = timeGetTime();
	
	//�o�b�t�@�̃��b�N
	result = m_PrimitiveNotes.LockVertex((void**)&pVertex);
	if (result != 0) goto EXIT;
	result = m_PrimitiveNotes.LockIndex(&pIndex);
	if (result != 0) goto EXIT;
	
	//�������m�[�g�ɂ��Ē��_���X�V
	for (i = 0; i < MTNOTEBOX_MAX_LIVENOTE_NUM; i++) {
		if (m_pNoteStatus[i].isActive) {
			//���_�X�V
			result = _CreateVertexOfNote(
							m_pNoteStatus[i],						//�m�[�g���
							&(pVertex[NOTE_VERTEX_NUM * noteNum]),	//���_�o�b�t�@�������݈ʒu
							NOTE_VERTEX_NUM * noteNum,				//���_�o�b�t�@�C���f�b�N�X�I�t�Z�b�g
							&(pIndex[NOTE_INDEX_NUM * noteNum]),	//�C���f�b�N�X�o�b�t�@�������݈ʒu
							curTime,								//���݂̎���
							true									//�s�b�`�x���h�K�p
						);
			if (result != 0) goto EXIT;
			
			noteNum++;
		}
	}
	m_NoteNum = noteNum;
	
	//�o�b�t�@�̃��b�N����
	result = m_PrimitiveNotes.UnlockVertex();
	if (result != 0) goto EXIT;
	result = m_PrimitiveNotes.UnlockIndex();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �`��
//******************************************************************************
int MTNoteBoxLive::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
   )
{
	int result = 0;
	
	//�������m�[�g�̕`��
	if (m_NoteNum > 0) {
		result = m_PrimitiveNotes.Draw(
						pD3DDevice,						//�f�o�C�X
						NULL,							//�e�N�X�`���F�Ȃ�
						(NOTE_INDEX_NUM/3)*m_NoteNum	//�v���~�e�B�u��
					);
		if (result != 0) goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// ���
//******************************************************************************
void MTNoteBoxLive::Release()
{
	delete m_pNoteDesign;
	m_pNoteDesign = NULL;

	m_PrimitiveNotes.Release();
	
	delete [] m_pNoteStatus;
	m_pNoteStatus = NULL;
}

//******************************************************************************
// �m�[�g�{�b�N�X�̒��_����
//******************************************************************************
int MTNoteBoxLive::_CreateVertexOfNote(
		NoteStatus note,
		MTNOTEBOX_VERTEX* pVertex,
		unsigned long vertexOffset,
		unsigned long* pIndex,
		unsigned long curTime,
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
	unsigned long elapsedTime = 0;
	
	if ((isEnablePitchBend) && (note.endTime == 0)) {
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
	elapsedTime = curTime - note.startTime;
	if (elapsedTime > m_LiveMonitorDisplayDuration) {
		elapsedTime = m_LiveMonitorDisplayDuration;
	}
	m_pNoteDesign->GetNoteBoxVirtexPosLive(
			elapsedTime,
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
	
	elapsedTime = 0;
	if (note.endTime != 0) {
		elapsedTime = curTime - note.endTime;
	}
	m_pNoteDesign->GetNoteBoxVirtexPosLive(
			elapsedTime,
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
	if (note.endTime != 0) {
		color = m_pNoteDesign->GetNoteBoxColor(note.portNo, note.chNo, note.noteNo);
	}
	else {
		//�������͔����J�n����̌o�ߎ��Ԃɂ���ĐF���ω�����
		elapsedTime = curTime - note.startTime;
		color = m_pNoteDesign->GetActiveNoteBoxColor(note.portNo, note.chNo, note.noteNo, elapsedTime);
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
unsigned long MTNoteBoxLive::_GetVertexIndexOfNote(
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
void MTNoteBoxLive::_MakeMaterial(
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
// ���Z�b�g
//******************************************************************************
void MTNoteBoxLive::Reset()
{
	unsigned long i = 0;
	
	m_NoteNum = 0;
	
	for (i = 0; i < MTNOTEBOX_MAX_LIVENOTE_NUM; i++) {
		m_pNoteStatus[i].isActive = false;
		m_pNoteStatus[i].portNo = 0;
		m_pNoteStatus[i].chNo = 0;
		m_pNoteStatus[i].noteNo = 0;
		m_pNoteStatus[i].startTime = 0;
		m_pNoteStatus[i].endTime = 0;
	}

	return;
}

//******************************************************************************
// �m�[�gON�o�^
//******************************************************************************
void MTNoteBoxLive::SetNoteOn(
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		unsigned char velocity
	)
{
	unsigned long i = 0;
	unsigned long cleardIndex = 0;
	bool isFind = false;
	unsigned long curTime = 0;
	
	curTime = timeGetTime();
	
	//�󂫃X�y�[�X�Ƀm�[�g����o�^
	//�󂫂�������Ȃ���Γo�^��������߂�
	for (i = 0; i < MTNOTEBOX_MAX_LIVENOTE_NUM; i++) {
		if (m_pNoteStatus[i].isActive) {
			if ((m_pNoteStatus[i].endTime != 0)
				&& ((curTime - m_pNoteStatus[i].endTime) > m_LiveMonitorDisplayDuration)) {
				m_pNoteStatus[i].isActive = false;
				cleardIndex = i;
				isFind = true;
				break;
			}
		}
		else {
			m_pNoteStatus[i].isActive = false;
			cleardIndex = i;
			isFind = true;
			break;
		}
	}
	
	//�󂫃X�y�[�X��������Ȃ��ꍇ�͍ł��Â��m�[�g�����N���A����
	if (!isFind) {
		_ClearOldestNoteStatus(&cleardIndex);
	}
	
	//�m�[�g���o�^
	m_pNoteStatus[cleardIndex].isActive = true;
	m_pNoteStatus[cleardIndex].portNo = portNo;
	m_pNoteStatus[cleardIndex].chNo = chNo;
	m_pNoteStatus[cleardIndex].noteNo = noteNo;
	m_pNoteStatus[cleardIndex].startTime = timeGetTime();
	m_pNoteStatus[cleardIndex].endTime = 0;
	
	return;
}

//******************************************************************************
// �m�[�gOFF�o�^
//******************************************************************************
void MTNoteBoxLive::SetNoteOff(
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo
	)
{
	unsigned long i = 0;
	
	//�Y���̃m�[�g���ɏI��������ݒ�
	for (i = 0; i < MTNOTEBOX_MAX_LIVENOTE_NUM; i++) {
		if ((m_pNoteStatus[i].isActive)
			&& (m_pNoteStatus[i].portNo == portNo)
			&& (m_pNoteStatus[i].chNo == chNo)
			&& (m_pNoteStatus[i].noteNo == noteNo)
			&& (m_pNoteStatus[i].endTime == 0)) {
			m_pNoteStatus[i].endTime = timeGetTime();
			break;
		}
	}
	
	return;
}

//******************************************************************************
// �S�m�[�gOFF
//******************************************************************************
void MTNoteBoxLive::AllNoteOff()
{
	unsigned long i = 0;
	
	//�m�[�gOFF���ݒ肳��Ă��Ȃ��m�[�g���ɏI��������ݒ�
	for (i = 0; i < MTNOTEBOX_MAX_LIVENOTE_NUM; i++) {
		if ((m_pNoteStatus[i].isActive) && (m_pNoteStatus[i].endTime == 0)) {
			m_pNoteStatus[i].endTime = timeGetTime();
		}
	}
	
	return;
}

//******************************************************************************
// �S�m�[�gOFF�i�`�����l���w��j
//******************************************************************************
void MTNoteBoxLive::AllNoteOffOnCh(
		unsigned char portNo,
		unsigned char chNo
	)
{
	unsigned long i = 0;
	
	//�w��`�����l���Ńm�[�gOFF���ݒ肳��Ă��Ȃ��m�[�g���ɏI��������ݒ�
	for (i = 0; i < MTNOTEBOX_MAX_LIVENOTE_NUM; i++) {
		if ((m_pNoteStatus[i].isActive) && (m_pNoteStatus[i].endTime == 0)
			&& (m_pNoteStatus[i].portNo == portNo) && (m_pNoteStatus[i].chNo == chNo)) {
			m_pNoteStatus[i].endTime = timeGetTime();
		}
	}
	
	return;
}

//******************************************************************************
// �ł��Â��m�[�g���̃N���A
//******************************************************************************
void MTNoteBoxLive::_ClearOldestNoteStatus(
		unsigned long* pCleardIndex
	)
{
	unsigned long i = 0;
	unsigned long oldestIndex = 0;
	bool isFind = false;
	
	//�m�[�gON�������ł��Â��m�[�g�����N���A����
	for (i = 0; i < MTNOTEBOX_MAX_LIVENOTE_NUM; i++) {
		if (m_pNoteStatus[i].isActive) {
			if (!isFind) {
				//�L���ȃm�[�g�����ꂽ�ꍇ�F����
				oldestIndex = i;
				isFind = true;
			}
			else {
				//�L���ȃm�[�g�����ꂽ�ꍇ�F2��ڈȍ~
				if (m_pNoteStatus[i].startTime < m_pNoteStatus[oldestIndex].startTime) {
					oldestIndex = i;
				}
			}
		}
	}
	
	//�m�[�g���N���A
	//  �m�[�g������o�^����Ă��Ȃ��ꍇ�͔z��̐擪���N���A�ΏۂƂȂ�
	m_pNoteStatus[oldestIndex].isActive = false;
	*pCleardIndex = oldestIndex;
	
	return;
}

