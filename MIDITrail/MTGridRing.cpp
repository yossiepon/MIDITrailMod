//******************************************************************************
//
// MIDITrail / MTGridRing
//
// �O���b�h�����O�`��N���X
//
// Copyright (C) 2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTGridRing.h"
#include "DXH.h"

using namespace YNBaseLib;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTGridRing::MTGridRing(void)
{
	m_BarNum = 0;
	m_isVisible = true;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTGridRing::~MTGridRing(void)
{
	Release();
}

//******************************************************************************
// �O���b�h����
//******************************************************************************
int MTGridRing::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
   )
{
	int result = 0;
	SMBarList barList;
	unsigned long vertexNum = 0;
	unsigned long indexNum = 0;
	MTGRIDBOX_VERTEX* pVertex = NULL;
	unsigned long* pIndex = NULL;
	unsigned long totalTickTime = 0;
	D3DMATERIAL9 material;
	D3DXCOLOR lineColor;

	Release();

	if ((pD3DDevice == NULL) || (pSeqData == NULL)) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//�m�[�g�f�U�C���I�u�W�F�N�g������
	result = m_NoteDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	//�V�[�P���X�f�[�^�F���ԏ��擾
	totalTickTime = pSeqData->GetTotalTickTime();

	//�V�[�P���X�f�[�^�F���߃��X�g�擾
	result = pSeqData->GetBarList(&barList);
	if (result != 0) goto EXIT;

	//�V�[�P���X�f�[�^�F�|�[�g���X�g�擾
	result = pSeqData->GetPortList(&m_PortList);
	if (result != 0) goto EXIT;

	//���ߐ�
	m_BarNum = barList.GetSize();

	//�v���~�e�B�u������
	result = m_Primitive.Initialize(
					sizeof(MTGRIDBOX_VERTEX),	//���_�T�C�Y
					_GetFVFFormat(),			//���_FVF�t�H�[�}�b�g
					D3DPT_LINELIST				//�v���~�e�B�u���
				);
	if (result != 0) goto EXIT;

	//���_�o�b�t�@�����F1�����O128���_ * 2(��[/�I�[)
	vertexNum = 128 * 2;
	result = m_Primitive.CreateVertexBuffer(pD3DDevice, vertexNum);
	if (result != 0) goto EXIT;

	//�C���f�b�N�X�o�b�t�@�����F1�����O128�� * 2(�n�_/�I�_) * 2(��[/�I�[)
	indexNum = 128 * 2 * 2;
	result = m_Primitive.CreateIndexBuffer(pD3DDevice, indexNum);
	if (result != 0) goto EXIT;

	//�o�b�t�@�̃��b�N
	result = m_Primitive.LockVertex((void**)&pVertex);
	if (result != 0) goto EXIT;
	result = m_Primitive.LockIndex(&pIndex);
	if (result != 0) goto EXIT;

	//�O���b�h�{�b�N�X�̒��_�ƃC���f�b�N�X�𐶐�
	result = _CreateVertexOfGrid(
					pVertex,		//���_�o�b�t�@�������݈ʒu
					pIndex,			//�C���f�b�N�X�o�b�t�@�������݈ʒu
					totalTickTime	//�g�[�^���`�b�N�^�C��
				);
	if (result != 0) goto EXIT;

	//�o�b�t�@�̃��b�N����
	result = m_Primitive.UnlockVertex();
	if (result != 0) goto EXIT;
	result = m_Primitive.UnlockIndex();
	if (result != 0) goto EXIT;

	//�}�e���A���쐬
	_MakeMaterial(&material);
	m_Primitive.SetMaterial(material);

	//�O���b�h�̐F���m�F
	lineColor = m_NoteDesign.GetGridLineColor();
	if (((DWORD)lineColor & 0xFF000000) == 0) {
		//�����Ȃ�`�悵�Ȃ�
		m_isVisible = false;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �ړ�
//******************************************************************************
int MTGridRing::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice,
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

	//�ړ��s��
	moveVector = m_NoteDesign.GetWorldMoveVector();
	D3DXMatrixTranslation(&moveMatrix, moveVector.x, moveVector.y, moveVector.z);

	//�s��̍���
	D3DXMatrixMultiply(&worldMatrix, &rotateMatrix, &moveMatrix);

	//�ϊ��s��ݒ�
	m_Primitive.Transform(worldMatrix);

	return result;
}

//******************************************************************************
// �`��
//******************************************************************************
int MTGridRing::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;

	if (m_isVisible) {
		result = m_Primitive.Draw(pD3DDevice);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// ���
//******************************************************************************
void MTGridRing::Release()
{
	m_Primitive.Release();
}

//******************************************************************************
// �O���b�h���_����
//******************************************************************************
int MTGridRing::_CreateVertexOfGrid(
		MTGRIDBOX_VERTEX* pVertex,
		unsigned long* pIndex,
		unsigned long totalTickTime
	)
{
	int result = 0;
	unsigned long i = 0;
	unsigned long virtexIndex = 0;
	unsigned long virtexIndexStart = 0;
	D3DXVECTOR3 basePosStart;
	D3DXVECTOR3 basePosEnd;
	D3DXVECTOR3 rotatedPos;
	float angle = 0.0f;

	//����W�擾
	m_NoteDesign.GetGridRingBasePos(totalTickTime, &basePosStart, &basePosEnd);

	//----------------------------------
	//��[�����O
	//----------------------------------
	//���_�쐬
	virtexIndexStart = virtexIndex;
	pVertex[virtexIndex].p = basePosStart;
	pVertex[virtexIndex].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
	pVertex[virtexIndex].c = m_NoteDesign.GetGridLineColor();
	for (i = 1; i < 128; i++) {
		virtexIndex++;
		
		//��]��̒��_
		angle = (360.0f / 128.0f) * (float)i;
		rotatedPos = DXH::RotateYZ(0.0f, 0.0f, basePosStart, angle);
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

	virtexIndex++;

	//----------------------------------
	//�I�[�����O
	//----------------------------------
	//���_�쐬
	virtexIndexStart = virtexIndex;
	pVertex[virtexIndex].p = basePosEnd;
	pVertex[virtexIndex].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
	pVertex[virtexIndex].c = m_NoteDesign.GetGridLineColor();
	for (i = 1; i < 128; i++) {
		virtexIndex++;
		
		//��]��̒��_
		angle = (360.0f / 128.0f) * (float)i;
		rotatedPos = DXH::RotateYZ(0.0f, 0.0f, basePosEnd, angle);
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
// �}�e���A���쐬
//******************************************************************************
void MTGridRing::_MakeMaterial(
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
	pMaterial->Power = 10.0f;
	//�����F
	pMaterial->Emissive.r = 0.0f;
	pMaterial->Emissive.g = 0.0f;
	pMaterial->Emissive.b = 0.0f;
	pMaterial->Emissive.a = 0.0f;
}


