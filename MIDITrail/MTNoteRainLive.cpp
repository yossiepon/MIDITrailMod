//******************************************************************************
//
// MIDITrail / MTNoteRainLive
//
// ���C�u���j�^�p�m�[�g���C���`��N���X
//
// Copyright (C) 2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// Windows�ł̃\�[�X���ڐA���Ă��邽�߁A���W�͍���n(DirectX)�ŏ������Ă���B
// ����n(DirectX)=>�E��n(OpenGL)�ւ̕ϊ��� LH2RH �}�N���Ŏ�������B

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTNoteRainLive.h"

using namespace YNBaseLib;


//******************************************************************************
// �p�����[�^��`
//******************************************************************************
//1�m�[�g������̒��_�� = 1�����`4���_ * 1��
#define NOTE_VERTEX_NUM  (4 * 1)

//1�m�[�g������̃C���f�b�N�X�� = 1�O�p�`3���_ * 2�� * 1��
#define NOTE_INDEX_NUM   (3 * 2 * 1)

//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTNoteRainLive::MTNoteRainLive(void)
{
	m_NoteNum = 0;
	m_pNoteStatus = NULL;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTNoteRainLive::~MTNoteRainLive(void)
{
	Release();
}

//******************************************************************************
// ��������
//******************************************************************************
int MTNoteRainLive::Create(
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
	
	//�L�[�{�[�h�f�U�C���I�u�W�F�N�g������
	result = m_KeyboardDesign.Initialize(pSceneName, NULL);
	if (result != 0) goto EXIT;
	
	//���C�u���j�^�\������
	m_LiveMonitorDisplayDuration = m_NoteDesign.GetLiveMonitorDisplayDuration();
	
	//�m�[�g���z�񐶐�
	result = _CreateNoteStatus();
	if (result != 0) goto EXIT;
	
	//�m�[�g�{�b�N�X�����i�o�b�t�@�m�ہj
	result = _CreateNoteRain(pD3DDevice);
	if (result != 0) goto EXIT;
	
	//�s�b�`�x���h���
	m_pNotePitchBend = pNotePitchBend;
	
EXIT:;
	return result;
}

//******************************************************************************
// �m�[�g���z�񐶐�
//******************************************************************************
int MTNoteRainLive::_CreateNoteStatus()
{
	int result = 0;
	unsigned long i = 0;
	
	//�m�[�g���z�񐶐�
	try {
		m_pNoteStatus = new NoteStatus[MTNOTERAIN_MAX_LIVENOTE_NUM];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}
	
	//�m�[�g��ԃ��X�g������
	for (i = 0; i < MTNOTERAIN_MAX_LIVENOTE_NUM; i++) {
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
// �������m�[�g���C�������i�o�b�t�@�m�ہj
//******************************************************************************
int MTNoteRainLive::_CreateNoteRain(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	unsigned long vertexNum = 0;
	unsigned long indexNum = 0;
	MTNOTERAIN_VERTEX* pVertex = NULL;
	unsigned long* pIndex = NULL;
	unsigned long i = 0;
	D3DMATERIAL9 material;
	NoteStatus note;
	
	memset(&note, 0, sizeof(NoteStatus));
	
	//�v���~�e�B�u������
	result = m_PrimitiveNotes.Initialize(
					sizeof(MTNOTERAIN_VERTEX),	//���_�T�C�Y
					_GetFVFFormat(),			//���_FVF�t�H�[�}�b�g
					D3DPT_TRIANGLELIST			//�v���~�e�B�u���
				);
	if (result != 0) goto EXIT;
	
	//���_�o�b�t�@����
	vertexNum = NOTE_VERTEX_NUM * MTNOTERAIN_MAX_LIVENOTE_NUM;
	result = m_PrimitiveNotes.CreateVertexBuffer(pD3DDevice, vertexNum);
	if (result != 0) goto EXIT;
	
	//�C���f�b�N�X�o�b�t�@����
	indexNum = NOTE_INDEX_NUM * MTNOTERAIN_MAX_LIVENOTE_NUM;
	result = m_PrimitiveNotes.CreateIndexBuffer(pD3DDevice, indexNum);
	if (result != 0) goto EXIT;
	
	//�o�b�t�@�̃��b�N
	result = m_PrimitiveNotes.LockVertex((void**)&pVertex);
	if (result != 0) goto EXIT;
	result = m_PrimitiveNotes.LockIndex(&pIndex);
	if (result != 0) goto EXIT;
	
	//�o�b�t�@�ɒ��_�ƃC���f�b�N�X����������
	for (i = 0; i < MTNOTERAIN_MAX_LIVENOTE_NUM; i++) {
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
int MTNoteRainLive::Transform(
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
	D3DXMatrixRotationY(&rotateMatrix, D3DXToRadian(rollAngle));
	
	//�ړ��s��
	//  �m�[�g���ړ�������ꍇ
	moveVector = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	//  �m�[�g���ړ��������ɃJ�����ƃL�[�{�[�h���ړ�������ꍇ
	//moveVector = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	D3DXMatrixTranslation(&moveMatrix, moveVector.x, moveVector.y, moveVector.z);
	
	//�s��̍����F�ړ�����]
	D3DXMatrixMultiply(&worldMatrix, &moveMatrix, &rotateMatrix);
	
	//�ϊ��s��ݒ�
	m_PrimitiveNotes.Transform(worldMatrix);
	
EXIT:;
	return result;
}

//******************************************************************************
// �m�[�g�̒��_����
//******************************************************************************
int MTNoteRainLive::_TransformNotes(
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
int MTNoteRainLive::_UpdateStatusOfNotes(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	unsigned long i = 0;
	unsigned long curTime = 0;
	
	curTime = timeGetTime();
	
	//�Â��m�[�g����j������
	for (i = 0; i < MTNOTERAIN_MAX_LIVENOTE_NUM; i++) {
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
int MTNoteRainLive::_UpdateVertexOfNotes(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	unsigned long i = 0;
	unsigned long noteNum = 0;
	unsigned long curTime = 0;
	MTNOTERAIN_VERTEX* pVertex = NULL;
	unsigned long* pIndex = NULL;
	
	curTime = timeGetTime();
	
	//�o�b�t�@�̃��b�N
	result = m_PrimitiveNotes.LockVertex((void**)&pVertex);
	if (result != 0) goto EXIT;
	result = m_PrimitiveNotes.LockIndex(&pIndex);
	if (result != 0) goto EXIT;
	
	//�������m�[�g�ɂ��Ē��_���X�V
	for (i = 0; i < MTNOTERAIN_MAX_LIVENOTE_NUM; i++) {
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
int MTNoteRainLive::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	DWORD value = 0;
	
	//�����_�����O�X�e�[�g���J�����O�Ȃ��ɂ���
	pD3DDevice->GetRenderState(D3DRS_CULLMODE, &value);
	pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	
	//�������m�[�g�̕`��
	if (m_NoteNum > 0) {
		result = m_PrimitiveNotes.Draw(
						pD3DDevice,						//�f�o�C�X
						NULL,							//�e�N�X�`���F�Ȃ�
						(NOTE_INDEX_NUM/3)*m_NoteNum	//�v���~�e�B�u��
					);
		if (result != 0) goto EXIT;
	}
	
	//�����_�����O�X�e�[�g��߂�
	pD3DDevice->SetRenderState(D3DRS_CULLMODE, value);
	
EXIT:;
	return result;
}

//******************************************************************************
// ���
//******************************************************************************
void MTNoteRainLive::Release()
{
	m_PrimitiveNotes.Release();
	
	delete [] m_pNoteStatus;
	m_pNoteStatus = NULL;
}

//******************************************************************************
// �m�[�g���C���̒��_����
//******************************************************************************
int MTNoteRainLive::_CreateVertexOfNote(
		NoteStatus note,
		MTNOTERAIN_VERTEX* pVertex,
		unsigned long vertexOffset,
		unsigned long* pIndex,
		unsigned long curTime,
		bool isEnablePitchBend
	)
{
	int result = 0;
	D3DXVECTOR3 startVector;
	D3DXVECTOR3 endVector;
	D3DXVECTOR3 moveVector;
	D3DXCOLOR color;
	float rainWidth = m_KeyboardDesign.GetBlackKeyWidth();
	float pitchBendShift = 0.0f;
	short pitchBendValue = 0;
	unsigned char pitchBendSensitivity = SM_DEFAULT_PITCHBEND_SENSITIVITY;
	unsigned long elapsedTime = 0;
	
	if ((isEnablePitchBend) && (note.endTime == 0)) {
		pitchBendValue =       m_pNotePitchBend->GetValue(note.portNo, note.chNo);
		pitchBendSensitivity = m_pNotePitchBend->GetSensitivity(note.portNo, note.chNo);
		pitchBendShift = m_KeyboardDesign.GetPitchBendShift(pitchBendValue, pitchBendSensitivity);
	}
	
	//�m�[�gON���W
	elapsedTime = curTime - note.startTime;
	if (elapsedTime > m_LiveMonitorDisplayDuration) {
		elapsedTime = m_LiveMonitorDisplayDuration;
	}
	startVector.x = pitchBendShift;
	startVector.y = m_NoteDesign.GetLivePosX(elapsedTime);
	startVector.z = 0.0f;
	
	//�m�[�gOFF���W
	elapsedTime = 0;
	if (note.endTime != 0) {
		elapsedTime = curTime - note.endTime;
	}
	endVector.x = pitchBendShift;
	endVector.y = m_NoteDesign.GetLivePosX(elapsedTime);
	endVector.z = 0.0f;
	
	//�ړ��x�N�g��
	moveVector    = m_KeyboardDesign.GetKeyboardBasePos(note.portNo, note.chNo);
	moveVector.x += m_KeyboardDesign.GetKeyCenterPosX(note.noteNo);
	moveVector.y += m_KeyboardDesign.GetWhiteKeyHeight() / 2.0f;
	moveVector.z += m_KeyboardDesign.GetNoteDropPosZ(note.noteNo);
	
	//���W�X�V
	startVector = startVector + moveVector;
	endVector   = endVector   + moveVector;
	
	//���_
	pVertex[0].p = D3DXVECTOR3(startVector.x - rainWidth/2.0f, startVector.y, startVector.z);
	pVertex[1].p = D3DXVECTOR3(startVector.x + rainWidth/2.0f, startVector.y, startVector.z);
	pVertex[2].p = D3DXVECTOR3(endVector.x   + rainWidth/2.0f, endVector.y,   endVector.z);
	pVertex[3].p = D3DXVECTOR3(endVector.x   - rainWidth/2.0f, endVector.y,   endVector.z);
	
	//�@��
	//���ۂ̖ʂ̕����ɍ��킹��(0,0,-1)�Ƃ���ƃ��C�g��K�p�����Ƃ��ɈÂ��Ȃ�
	//���Ղ̏�ʂɍ��킹�邱�ƂŖ��邭����
	pVertex[0].n = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	pVertex[1].n = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	pVertex[2].n = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	pVertex[3].n = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	
	//�F
	color = m_NoteDesign.GetNoteBoxColor(note.portNo, note.chNo, note.noteNo);
	pVertex[0].c = D3DXCOLOR(color.r, color.g, color.b, 1.0f);
	pVertex[1].c = D3DXCOLOR(color.r, color.g, color.b, 1.0f);
	pVertex[2].c = D3DXCOLOR(color.r, color.g, color.b, 0.5f); //�m�[�gOFF�ɋ߂Â��قǔ������ɂ���
	pVertex[3].c = D3DXCOLOR(color.r, color.g, color.b, 0.5f); //�m�[�gOFF�ɋ߂Â��قǔ������ɂ���
	
	//�C���f�b�N�X
	pIndex[0] = vertexOffset + 0;
	pIndex[1] = vertexOffset + 2;
	pIndex[2] = vertexOffset + 1;
	pIndex[3] = vertexOffset + 0;
	pIndex[4] = vertexOffset + 3;
	pIndex[5] = vertexOffset + 2;
	
	return result;
}

//******************************************************************************
// �}�e���A���쐬
//******************************************************************************
void MTNoteRainLive::_MakeMaterial(
		D3DMATERIAL9* pMaterial
	)
{
	memset(pMaterial, 0, sizeof(D3DMATERIAL9));
	
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
void MTNoteRainLive::Reset()
{
	unsigned long i = 0;
	
	m_NoteNum = 0;
	
	for (i = 0; i < MTNOTERAIN_MAX_LIVENOTE_NUM; i++) {
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
void MTNoteRainLive::SetNoteOn(
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
	for (i = 0; i < MTNOTERAIN_MAX_LIVENOTE_NUM; i++) {
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
void MTNoteRainLive::SetNoteOff(
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo
	)
{
	unsigned long i = 0;
	
	//�Y���̃m�[�g���ɏI��������ݒ�
	for (i = 0; i < MTNOTERAIN_MAX_LIVENOTE_NUM; i++) {
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
void MTNoteRainLive::AllNoteOff()
{
	unsigned long i = 0;
	
	//�m�[�gOFF���ݒ肳��Ă��Ȃ��m�[�g���ɏI��������ݒ�
	for (i = 0; i < MTNOTERAIN_MAX_LIVENOTE_NUM; i++) {
		if ((m_pNoteStatus[i].isActive) && (m_pNoteStatus[i].endTime == 0)) {
			m_pNoteStatus[i].endTime = timeGetTime();
		}
	}
	
	return;
}

//******************************************************************************
// �S�m�[�gOFF�i�`�����l���w��j
//******************************************************************************
void MTNoteRainLive::AllNoteOffOnCh(
		unsigned char portNo,
		unsigned char chNo
	)
{
	unsigned long i = 0;
	
	//�w��`�����l���Ńm�[�gOFF���ݒ肳��Ă��Ȃ��m�[�g���ɏI��������ݒ�
	for (i = 0; i < MTNOTERAIN_MAX_LIVENOTE_NUM; i++) {
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
void MTNoteRainLive::_ClearOldestNoteStatus(
		unsigned long* pCleardIndex
	)
{
	unsigned long i = 0;
	unsigned long oldestIndex = 0;
	bool isFind = false;
	
	//�m�[�gON�������ł��Â��m�[�g�����N���A����
	for (i = 0; i < MTNOTERAIN_MAX_LIVENOTE_NUM; i++) {
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

