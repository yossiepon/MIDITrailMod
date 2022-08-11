//******************************************************************************
//
// MIDITrail / MTTimeIndicatorRing
//
// �^�C���C���W�P�[�^�����O�`��N���X
//
// Copyright (C) 2019-2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTTimeIndicatorRing.h"
#include "DXH.h"

using namespace YNBaseLib;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTTimeIndicatorRing::MTTimeIndicatorRing(void)
{
	m_CurPos = 0.0f;
	m_CurTickTime = 0;
	m_isEnable = true;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTTimeIndicatorRing::~MTTimeIndicatorRing(void)
{
	Release();
}

//******************************************************************************
// �^�C���C���W�P�[�^����
//******************************************************************************
int MTTimeIndicatorRing::Create(
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

	//�v���~�e�B�u�����F�^�C�����C��
	result = _CreatePrimitiveLine(pD3DDevice);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �v���~�e�B�u����
//******************************************************************************
int MTTimeIndicatorRing::_CreatePrimitiveLine(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	unsigned long vertexNum = 0;
	unsigned long indexNum = 0;
	MTTIMEINDICATOR_VERTEX* pVertex = NULL;
	unsigned long* pIndex = NULL;

	//�v���~�e�B�u������
	result = m_PrimitiveLine.Initialize(
					sizeof(MTTIMEINDICATOR_VERTEX),	//���_�T�C�Y
					_GetFVFFormat(),				//���_FVF�t�H�[�}�b�g
					D3DPT_LINELIST					//�v���~�e�B�u���
				);
	if (result != 0) goto EXIT;

	//���_�o�b�t�@�����F1�T�[�N��128���_
	vertexNum = 128;
	result = m_PrimitiveLine.CreateVertexBuffer(pD3DDevice, vertexNum);
	if (result != 0) goto EXIT;

	//�C���f�b�N�X�o�b�t�@�����F1�T�[�N��128�� * 2(�n�_/�I�_)
	indexNum = 128 * 2;
	result = m_PrimitiveLine.CreateIndexBuffer(pD3DDevice, indexNum);
	if (result != 0) goto EXIT;

	//�o�b�t�@�̃��b�N
	result = m_PrimitiveLine.LockVertex((void**)&pVertex);
	if (result != 0) goto EXIT;
	result = m_PrimitiveLine.LockIndex(&pIndex);
	if (result != 0) goto EXIT;

	//�o�b�t�@�ɒ��_�ƃC���f�b�N�X����������
	result = _CreateVertexOfIndicatorLine(pVertex, pIndex);
	if (result != 0) goto EXIT;

	//�o�b�t�@�̃��b�N����
	result = m_PrimitiveLine.UnlockVertex();
	if (result != 0) goto EXIT;
	result = m_PrimitiveLine.UnlockIndex();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �ړ�
//******************************************************************************
int MTTimeIndicatorRing::Transform(
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
	m_PrimitiveLine.Transform(worldMatrix);

	return result;
}

//******************************************************************************
// �`��
//******************************************************************************
int MTTimeIndicatorRing::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;

	if (!m_isEnable) goto EXIT;

	//�e�N�X�`���X�e�[�W�ݒ�
	//  �J���[���Z�F����1���g�p  ����1�F�e�N�X�`��
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	// �A���t�@���Z�F����1���g�p  ����1�F�e�N�X�`��
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

	//�e�N�X�`���t�B���^
	pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	result = m_PrimitiveLine.Draw(pD3DDevice);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ���
//******************************************************************************
void MTTimeIndicatorRing::Release()
{
	m_PrimitiveLine.Release();
}

//******************************************************************************
// �^�C���C���W�P�[�^���C�����_����
//******************************************************************************
int MTTimeIndicatorRing::_CreateVertexOfIndicatorLine(
		MTTIMEINDICATOR_VERTEX* pVertex,
		unsigned long* pIndex
	)
{
	int result = 0;
	unsigned long i = 0;
	unsigned long virtexIndex = 0;
	unsigned long virtexIndexStart = 0;
	D3DXVECTOR3 basePos;
	D3DXVECTOR3 rotatedPos;
	float angle = 0.0f;

	//����W
	basePos = D3DXVECTOR3(m_NoteDesign.GetPlayPosX(0),
							m_NoteDesign.GetPortOriginY(0),
							m_NoteDesign.GetPortOriginZ(0));

	//���_�쐬
	virtexIndexStart = virtexIndex;
	pVertex[virtexIndex].p = basePos;
	pVertex[virtexIndex].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
	pVertex[virtexIndex].c = m_NoteDesign.GetGridLineColor();
	for (i = 1; i < 128; i++) {
		virtexIndex++;
		
		//��]��̒��_
		angle = (360.0f / 128.0f) * (float)i;
		rotatedPos = DXH::RotateYZ(0.0f, 0.0f, basePos, angle);
		pVertex[virtexIndex].p = rotatedPos;
		pVertex[virtexIndex].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
		pVertex[virtexIndex].c = m_NoteDesign.GetGridLineColor();
		
		//�C���f�b�N�X�o�b�t�@�i�O��̒��_���獡��̒��_�j
		pIndex[(virtexIndex - 1) * 2]     = virtexIndex - 1;
		pIndex[(virtexIndex - 1) * 2 + 1] = virtexIndex;
	}
	//�I�_�Ǝn�_���Ȃ���
	pIndex[virtexIndex * 2]     = virtexIndex;
	pIndex[virtexIndex * 2 + 1] = virtexIndexStart;

	return result;
}

//******************************************************************************
// �`�b�N�^�C���ݒ�
//******************************************************************************
void MTTimeIndicatorRing::SetCurTickTime(
		unsigned long curTickTime
	)
{
	m_CurTickTime = curTickTime;
	m_CurPos = m_NoteDesign.GetPlayPosX(m_CurTickTime);
}

//******************************************************************************
// ���Z�b�g
//******************************************************************************
void MTTimeIndicatorRing::Reset()
{
	m_CurTickTime = 0;
	m_CurPos = 0.0f;
}

//******************************************************************************
// ���݈ʒu�擾
//******************************************************************************
float MTTimeIndicatorRing::GetPos()
{
	return m_CurPos;
}

//******************************************************************************
// �ړ��x�N�g���擾
//******************************************************************************
D3DXVECTOR3 MTTimeIndicatorRing::GetMoveVector()
{
	return D3DXVECTOR3(m_CurPos, 0.0f, 0.0f);
}

//******************************************************************************
// �\���ݒ�
//******************************************************************************
void MTTimeIndicatorRing::SetEnable(
		bool isEnable
	)
{
	m_isEnable = isEnable;
}


