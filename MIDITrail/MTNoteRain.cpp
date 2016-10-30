//******************************************************************************
//
// MIDITrail / MTNoteRain
//
// �m�[�g���C���`��N���X
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTNoteRain.h"

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
MTNoteRain::MTNoteRain(void)
{
	m_CurTickTime = 0;
	m_CurNoteIndex = 0;
	m_pNoteStatus = NULL;
	m_CurPos = 0.0f;
	m_isSkipping = false;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTNoteRain::~MTNoteRain(void)
{
	Release();
}

//******************************************************************************
// ��������
//******************************************************************************
int MTNoteRain::Create(
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

	//�L�[�{�[�h�f�U�C���I�u�W�F�N�g������
	result = m_KeyboardDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	//�g���b�N�擾
	result = pSeqData->GetMergedTrack(&track);
	if (result != 0) goto EXIT;

	//�m�[�g���X�g�擾
	result = track.GetNoteList(&m_NoteList);
	if (result != 0) goto EXIT;

	//�S�m�[�g���C������
	result = _CreateAllNoteRain(pD3DDevice);
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
// �S�m�[�g���C������
//******************************************************************************
int MTNoteRain::_CreateAllNoteRain(
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
	SMNote note;

	//�v���~�e�B�u������
	result = m_PrimitiveAllNotes.Initialize(
					sizeof(MTNOTERAIN_VERTEX),	//���_�T�C�Y
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
// �m�[�g�{�b�N�X�̒��_����
//******************************************************************************
int MTNoteRain::_CreateVertexOfNote(
		SMNote note,
		MTNOTERAIN_VERTEX* pVertex,
		unsigned long vertexOffset,
		unsigned long* pIndex
	)
{
	int result = 0;
	D3DXVECTOR3 startVector;
	D3DXVECTOR3 endVector;
	D3DXVECTOR3 moveVector;
	D3DXCOLOR color;
	float rainWidth    = m_KeyboardDesign.GetBlackKeyWidth();

	//�m�[�gON���W
	startVector.x = 0.0f;
	startVector.y = m_NoteDesign.GetPlayPosX(note.startTime);
	startVector.z = 0.0f;

	//�m�[�gOFF���W
	endVector.x = 0.0f;
	endVector.y = m_NoteDesign.GetPlayPosX(note.endTime);
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
// �m�[�g���z�񐶐�
//******************************************************************************
int MTNoteRain::_CreateNoteStatus()
{
	int result = 0;
	unsigned long i = 0;

	//�m�[�g���z�񐶐�
	try {
		m_pNoteStatus = new NoteStatus[MTNOTERAIN_MAX_ACTIVENOTE_NUM];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

	//�m�[�g��ԃ��X�g������
	for (i = 0; i < MTNOTERAIN_MAX_ACTIVENOTE_NUM; i++) {
		m_pNoteStatus[i].isActive = false;
		m_pNoteStatus[i].index = 0;
		m_pNoteStatus[i].startTime = 0;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �}�e���A���쐬
//******************************************************************************
void MTNoteRain::_MakeMaterial(
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
// �ړ�
//******************************************************************************
int MTNoteRain::Transform(
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
	D3DXMatrixRotationY(&rotateMatrix, D3DXToRadian(rollAngle));

	//���t�ʒu
	m_CurPos = m_NoteDesign.GetPlayPosX(m_CurTickTime);

	//�ړ��s��
	//  �m�[�g���ړ�������ꍇ
	moveVector = D3DXVECTOR3(0.0f, -m_CurPos, 0.0f);
	//  �m�[�g���ړ��������ɃJ�����ƃL�[�{�[�h���ړ�������ꍇ
	//moveVector = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	D3DXMatrixTranslation(&moveMatrix, moveVector.x, moveVector.y, moveVector.z);

	//�s��̍����F�ړ�����]
	D3DXMatrixMultiply(&worldMatrix, &moveMatrix, &rotateMatrix);

	//�ϊ��s��ݒ�
	m_PrimitiveAllNotes.Transform(worldMatrix);

EXIT:;
	return result;
}

//******************************************************************************
// �������m�[�g�̒��_����
//******************************************************************************
int MTNoteRain::_TransformActiveNotes(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;

	//�X�L�b�v���Ȃ牽�����Ȃ�
	if (m_isSkipping) goto EXIT;

	//�������m�[�g�̏�ԍX�V
	result = _UpdateStatusOfActiveNotes(pD3DDevice);
	if (result != 0) goto EXIT;

	//�������m�[�g�̍X�V
	result = _UpdateActiveNotes(pD3DDevice);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �������m�[�g�̏�ԍX�V
//******************************************************************************
int MTNoteRain::_UpdateStatusOfActiveNotes(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	unsigned long i = 0;
	unsigned long curTime = 0;
	bool isFound = false;
	SMNote note;

	//TODO: �����m�[�g�̊Ǘ������C�u������������

	curTime = timeGetTime();

	//�����I���m�[�g�̏���j������
	for (i = 0; i < MTNOTERAIN_MAX_ACTIVENOTE_NUM; i++) {
		if (m_pNoteStatus[i].isActive) {
			result = m_NoteList.GetNote(m_pNoteStatus[i].index, &note);
			if (result != 0) goto EXIT;

			if (note.endTime < m_CurTickTime) {
				result = _UpdateVertexOfNote(m_pNoteStatus[i].index);
				if (result != 0) goto EXIT;
				m_pNoteStatus[i].isActive = false;
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
			for (i = 0; i < MTNOTERAIN_MAX_ACTIVENOTE_NUM; i++) {
				if ((m_pNoteStatus[i].isActive)
				 && (m_pNoteStatus[i].index == m_CurNoteIndex)) {
					isFound = true;
					break;
				}
			}
			//�󂢂Ă���Ƃ���ɒǉ�����
			if (!isFound) {
				for (i = 0; i < MTNOTERAIN_MAX_ACTIVENOTE_NUM; i++) {
					if (!(m_pNoteStatus[i].isActive)) {
						m_pNoteStatus[i].isActive = true;
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
// �������m�[�g�̍X�V
//******************************************************************************
int MTNoteRain::_UpdateActiveNotes(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	SMNote note;
	unsigned long i = 0;
	unsigned long curTime = 0;
	unsigned long elapsedTime = 0;
	bool isEnablePichBendShift = true;

	curTime = timeGetTime();

	//�������m�[�g�ɂ��Ē��_���X�V
	for (i = 0; i < MTNOTERAIN_MAX_ACTIVENOTE_NUM; i++) {
		if (m_pNoteStatus[i].isActive) {
			//�m�[�g���擾
			result = m_NoteList.GetNote(m_pNoteStatus[i].index, &note);
			if (result != 0) goto EXIT;

			//�����J�n����̌o�ߎ���
			elapsedTime = curTime - m_pNoteStatus[i].startTime;

			//�������m�[�g�̒��_���X�V����
			result = _UpdateVertexOfNote(m_pNoteStatus[i].index, isEnablePichBendShift);
			if (result != 0) goto EXIT;
		}
	}

EXIT:;
	return result;
}
//******************************************************************************
// �������m�[�g�̍X�V
//******************************************************************************
int MTNoteRain::_UpdateVertexOfNote(
		unsigned long index,
		bool isEnablePitchBendShift
	)
{
	int result = 0;
	unsigned long offset = 0;
	unsigned long size = 0;
	float posX = 0.0f;
	float pitchBendShift = 0.0f;
	float rainWidth = 0.0f;
	short pitchBendValue = 0;
	unsigned char pitchBendSensitivity = SM_DEFAULT_PITCHBEND_SENSITIVITY;
	SMNote note;
	D3DXVECTOR3 moveVector;
	MTNOTERAIN_VERTEX* pVertex = NULL;

	//�m�[�g���擾
	result = m_NoteList.GetNote(index, &note);
	if (result != 0) goto EXIT;

	//�s�b�`�x���h�ɂ��L�[�{�[�h�V�t�g��
	if (isEnablePitchBendShift) {
		pitchBendValue =       m_pNotePitchBend->GetValue(note.portNo, note.chNo);
		pitchBendSensitivity = m_pNotePitchBend->GetSensitivity(note.portNo, note.chNo);
		pitchBendShift = m_KeyboardDesign.GetPitchBendShift(pitchBendValue, pitchBendSensitivity);
	}

	//�m�[�gX���W
	moveVector = m_KeyboardDesign.GetKeyboardBasePos(note.portNo, note.chNo);
	posX = moveVector.x + m_KeyboardDesign.GetKeyCenterPosX(note.noteNo) + pitchBendShift;

	//���_�o�b�t�@�̃��b�N
	offset = NOTE_VERTEX_NUM * sizeof(MTNOTERAIN_VERTEX) * index;
	size   = NOTE_VERTEX_NUM * sizeof(MTNOTERAIN_VERTEX);
	result = m_PrimitiveAllNotes.LockVertex((void**)&pVertex, offset, size);
	if (result != 0) goto EXIT;

	//���_��X���W���X�V
	rainWidth = m_KeyboardDesign.GetBlackKeyWidth();
	pVertex[0].p.x = posX - rainWidth/2.0f;
	pVertex[1].p.x = posX + rainWidth/2.0f;
	pVertex[2].p.x = posX + rainWidth/2.0f;
	pVertex[3].p.x = posX - rainWidth/2.0f;

	//�o�b�t�@�̃��b�N����
	result = m_PrimitiveAllNotes.UnlockVertex();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �`��
//******************************************************************************
int MTNoteRain::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
   )
{
	int result = 0;
	DWORD value = 0;

	//�����_�����O�X�e�[�g���J�����O�Ȃ��ɂ���
	pD3DDevice->GetRenderState(D3DRS_CULLMODE, &value);
	pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	//�S�m�[�g�̕`��
	result = m_PrimitiveAllNotes.Draw(pD3DDevice);
	if (result != 0) goto EXIT;

	//�����_�����O�X�e�[�g��߂�
	pD3DDevice->SetRenderState(D3DRS_CULLMODE, value);

EXIT:;
	return result;
}

//******************************************************************************
// ���
//******************************************************************************
void MTNoteRain::Release()
{
	m_PrimitiveAllNotes.Release();
	m_NoteList.Clear();

	delete [] m_pNoteStatus;
	m_pNoteStatus = NULL;
}

//******************************************************************************
// �J�����g�`�b�N�^�C���ݒ�
//******************************************************************************
void MTNoteRain::SetCurTickTime(
		unsigned long curTickTime
	)
{
	m_CurTickTime = curTickTime;
}

//******************************************************************************
// ���Z�b�g
//******************************************************************************
void MTNoteRain::Reset()
{
	int result = 0;
	unsigned long i = 0;

	m_CurTickTime = 0;
	m_CurNoteIndex = 0;
	m_CurPos = 0.0f;

	for (i = 0; i < MTNOTERAIN_MAX_ACTIVENOTE_NUM; i++) {

		//�������m�[�g�̒��_�𕜌�����
		if (m_pNoteStatus[i].isActive) {
			result = _UpdateVertexOfNote(m_pNoteStatus[i].index);
			//if (result != 0) goto EXIT;
		}
		m_pNoteStatus[i].isActive = false;
		m_pNoteStatus[i].index = 0;
		m_pNoteStatus[i].startTime = 0;
	}

	return;
}

//******************************************************************************
// ���݈ʒu�擾
//******************************************************************************
float MTNoteRain::GetPos()
{
	return m_CurPos;
}

//******************************************************************************
// �X�L�b�v��Ԑݒ�
//******************************************************************************
void MTNoteRain::SetSkipStatus(
		bool isSkipping
	)
{
	m_isSkipping = isSkipping;
}

