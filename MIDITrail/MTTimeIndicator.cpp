//******************************************************************************
//
// MIDITrail / MTTimeIndicator
//
// �^�C���C���W�P�[�^�`��N���X
//
// Copyright (C) 2010-2013 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTTimeIndicator.h"

using namespace YNBaseLib;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTTimeIndicator::MTTimeIndicator(void)
{
	m_CurPos = 0.0f;
	m_CurTickTime = 0;
	m_isEnableLine = false;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTTimeIndicator::~MTTimeIndicator(void)
{
	Release();
}

//******************************************************************************
// �^�C���C���W�P�[�^����
//******************************************************************************
int MTTimeIndicator::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;
	SMBarList barList;

	Release();

	//�m�[�g�f�U�C���I�u�W�F�N�g������
	result = m_NoteDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	//�v���~�e�B�u����
	result = _CreatePrimitive(pD3DDevice);
	if (result != 0) goto EXIT;

	//�v���~�e�B�u�����F�^�C�����C��
	result = _CreatePrimitiveLine(pD3DDevice);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �v���~�e�B�u����
//******************************************************************************
int MTTimeIndicator::_CreatePrimitive(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	unsigned long vertexNum = 0;
	unsigned long indexNum = 0;
	MTTIMEINDICATOR_VERTEX* pVertex = NULL;
	unsigned long* pIndex = NULL;

	//�v���~�e�B�u������
	result = m_Primitive.Initialize(
					sizeof(MTTIMEINDICATOR_VERTEX),	//���_�T�C�Y
					_GetFVFFormat(),				//���_FVF�t�H�[�}�b�g
					D3DPT_TRIANGLESTRIP				//�v���~�e�B�u���
				);
	if (result != 0) goto EXIT;

	//���_�o�b�t�@����
	vertexNum = 4;
	result = m_Primitive.CreateVertexBuffer(pD3DDevice, vertexNum);
	if (result != 0) goto EXIT;

	//�C���f�b�N�X�o�b�t�@����
	indexNum = 4;
	result = m_Primitive.CreateIndexBuffer(pD3DDevice, indexNum);
	if (result != 0) goto EXIT;

	//�o�b�t�@�̃��b�N
	result = m_Primitive.LockVertex((void**)&pVertex);
	if (result != 0) goto EXIT;
	result = m_Primitive.LockIndex(&pIndex);
	if (result != 0) goto EXIT;

	//�o�b�t�@�ɒ��_�ƃC���f�b�N�X����������
	result = _CreateVertexOfIndicator(pVertex, pIndex);
	if (result != 0) goto EXIT;

	//�o�b�t�@�̃��b�N����
	result = m_Primitive.UnlockVertex();
	if (result != 0) goto EXIT;
	result = m_Primitive.UnlockIndex();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �v���~�e�B�u����
//******************************************************************************
int MTTimeIndicator::_CreatePrimitiveLine(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	unsigned long vertexNum = 0;
	MTTIMEINDICATOR_VERTEX* pVertex = NULL;

	//�v���~�e�B�u������
	result = m_PrimitiveLine.Initialize(
					sizeof(MTTIMEINDICATOR_VERTEX),	//���_�T�C�Y
					_GetFVFFormat(),				//���_FVF�t�H�[�}�b�g
					D3DPT_LINELIST					//�v���~�e�B�u���
				);
	if (result != 0) goto EXIT;

	//���_�o�b�t�@����
	vertexNum = 2;
	result = m_PrimitiveLine.CreateVertexBuffer(pD3DDevice, vertexNum);
	if (result != 0) goto EXIT;

	//�o�b�t�@�̃��b�N
	result = m_PrimitiveLine.LockVertex((void**)&pVertex);
	if (result != 0) goto EXIT;

	//�o�b�t�@�ɒ��_�ƃC���f�b�N�X����������
	result = _CreateVertexOfIndicatorLine(pVertex);
	if (result != 0) goto EXIT;

	//�o�b�t�@�̃��b�N����
	result = m_PrimitiveLine.UnlockVertex();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �ړ�
//******************************************************************************
int MTTimeIndicator::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice,
		D3DXVECTOR3 camVector,
		float rollAngle
	)
{
	int result = 0;
	D3DXVECTOR3 moveVector;
	D3DXMATRIX rotateMatrix;
	D3DXMATRIX moveMatrix;
	D3DXMATRIX worldMatrix;

	//�s�񏉊���
	D3DXMatrixIdentity(&rotateMatrix);
	D3DXMatrixIdentity(&moveMatrix);
	D3DXMatrixIdentity(&worldMatrix);

	//��]�s��
	D3DXMatrixRotationX(&rotateMatrix, D3DXToRadian(rollAngle));

	//���t�ʒu
	m_CurPos = m_NoteDesign.GetPlayPosX(m_CurTickTime);

	//�ړ��s��
	moveVector = m_NoteDesign.GetWorldMoveVector();
	D3DXMatrixTranslation(&moveMatrix, moveVector.x + m_CurPos, moveVector.y, moveVector.z);

	//�s��̍���
	D3DXMatrixMultiply(&worldMatrix, &rotateMatrix, &moveMatrix);

	//�ϊ��s��ݒ�
	m_Primitive.Transform(worldMatrix);
	m_PrimitiveLine.Transform(worldMatrix);

	return result;
}

//******************************************************************************
// �`��
//******************************************************************************
int MTTimeIndicator::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;

// >>> add 20120728 yossiepon begin

	//�e�N�X�`���X�e�[�W�ݒ�
	//  �J���[���Z�F����
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_DISABLE);
	// �A���t�@���Z�F����
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);

// <<< add 20120728 yossiepon end

	if (m_isEnableLine) {
		result = m_PrimitiveLine.Draw(pD3DDevice);
		if (result != 0) goto EXIT;
	}
	else {
		result = m_Primitive.Draw(pD3DDevice);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// ���
//******************************************************************************
void MTTimeIndicator::Release()
{
	m_Primitive.Release();
	m_PrimitiveLine.Release();
}

//******************************************************************************
// �^�C���C���W�P�[�^���_����
//******************************************************************************
int MTTimeIndicator::_CreateVertexOfIndicator(
		MTTIMEINDICATOR_VERTEX* pVertex,
		unsigned long* pIndex
	)
{
	int result = 0;
	unsigned long i;
	D3DXVECTOR3 vectorLU;
	D3DXVECTOR3 vectorRU;
	D3DXVECTOR3 vectorLD;
	D3DXVECTOR3 vectorRD;

	//              y x
	//  0+----+1    |/
	//   |    |  z--+0
	//   |    |
	//  2+----+3 �� 3 �����_(0,0,0)

	//�Đ��ʒ��_���W�擾
	m_NoteDesign.GetPlaybackSectionVirtexPos(
			0,
			&vectorLU,
			&vectorRU,
			&vectorLD,
			&vectorRD
		);

	//���_���W
	pVertex[0].p = vectorLU;
	pVertex[1].p = vectorRU;
	pVertex[2].p = vectorLD;
	pVertex[3].p = vectorRD;

	//�Đ��ʂ̕����[���̏ꍇ�̓��C����`�悷��
	if (vectorLU.z == vectorRU.z) {
		m_isEnableLine = true;
	}

	//�@��
	pVertex[0].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
	pVertex[1].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
	pVertex[2].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
	pVertex[3].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);

	//�e���_�̃f�B�t���[�Y�F
	for (i = 0; i < 4; i++) {
		pVertex[i].c = m_NoteDesign.GetPlaybackSectionColor();
	}

	//�C���f�b�N�X�FTRIANGLESTRIP
	pIndex[0] = 0;
	pIndex[1] = 1;
	pIndex[2] = 2;
	pIndex[3] = 3;

	return result;
}

//******************************************************************************
// �^�C���C���W�P�[�^���C�����_����
//******************************************************************************
int MTTimeIndicator::_CreateVertexOfIndicatorLine(
		MTTIMEINDICATOR_VERTEX* pVertex
	)
{
	int result = 0;
	unsigned long i;
	D3DXVECTOR3 vectorLU;
	D3DXVECTOR3 vectorRU;
	D3DXVECTOR3 vectorLD;
	D3DXVECTOR3 vectorRD;

	//              y x
	//  0+----+1    |/
	//   |    |  z--+0
	//   |    |
	//  2+----+3 �� 3 �����_(0,0,0)

	//�Đ��ʒ��_���W�擾
	m_NoteDesign.GetPlaybackSectionVirtexPos(
			0,
			&vectorLU,
			&vectorRU,
			&vectorLD,
			&vectorRD
		);

	//���_���W
	pVertex[0].p = vectorLU;
	pVertex[1].p = vectorLD;

	//�@��
	pVertex[0].n = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVertex[1].n = D3DXVECTOR3(0.0f, 0.0f, -1.0f);

	//�e���_�̃f�B�t���[�Y�F
	for (i = 0; i < 2; i++) {
		pVertex[i].c = m_NoteDesign.GetPlaybackSectionColor();
	}

	return result;
}

//******************************************************************************
// �`�b�N�^�C���ݒ�
//******************************************************************************
void MTTimeIndicator::SetCurTickTime(
		unsigned long curTickTime
	)
{
	m_CurTickTime = curTickTime;
	m_CurPos = m_NoteDesign.GetPlayPosX(m_CurTickTime);
}

//******************************************************************************
// ���Z�b�g
//******************************************************************************
void MTTimeIndicator::Reset()
{
	m_CurTickTime = 0;
	m_CurPos = 0.0f;
}

//******************************************************************************
// ���݈ʒu�擾
//******************************************************************************
float MTTimeIndicator::GetPos()
{
	return m_CurPos;
}

//******************************************************************************
// �ړ��x�N�g���擾
//******************************************************************************
D3DXVECTOR3 MTTimeIndicator::GetMoveVector()
{
	return D3DXVECTOR3(m_CurPos, 0.0f, 0.0f);
}

