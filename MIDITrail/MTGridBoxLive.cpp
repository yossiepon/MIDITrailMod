//******************************************************************************
//
// MIDITrail / MTGridBoxLive
//
// ���C�u���j�^�p�O���b�h�{�b�N�X�`��N���X
//
// Copyright (C) 2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTGridBoxLive.h"

using namespace YNBaseLib;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTGridBoxLive::MTGridBoxLive(void)
{
	m_isVisible = true;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTGridBoxLive::~MTGridBoxLive(void)
{
	Release();
}

//******************************************************************************
// �O���b�h����
//******************************************************************************
int MTGridBoxLive::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName
   )
{
	int result = 0;
	SMBarList barList;
	unsigned long vertexNum = 0;
	unsigned long indexNum = 0;
	MTGRIDBOXLIVE_VERTEX* pVertex = NULL;
	unsigned long* pIndex = NULL;
	D3DMATERIAL9 material;
	D3DXCOLOR lineColor;
	
	Release();
	
	if (pD3DDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	
	//�m�[�g�f�U�C���I�u�W�F�N�g������
	result = m_NoteDesign.Initialize(pSceneName, NULL);
	if (result != 0) goto EXIT;
	
	//�v���~�e�B�u������
	result = m_Primitive.Initialize(
					sizeof(MTGRIDBOXLIVE_VERTEX),	//���_�T�C�Y
					_GetFVFFormat(),			//���_FVF�t�H�[�}�b�g
					D3DPT_LINELIST				//�v���~�e�B�u���
				);
	if (result != 0) goto EXIT;
	
	//���_�o�b�t�@�����F1������8���_
	vertexNum = 8;
	result = m_Primitive.CreateVertexBuffer(pD3DDevice, vertexNum);
	if (result != 0) goto EXIT;
	
	//�C���f�b�N�X�o�b�t�@�����F(1������12�� * 2���_)
	indexNum = 24;
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
					pIndex			//�C���f�b�N�X�o�b�t�@�������݈ʒu
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
int MTGridBoxLive::Transform(
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
int MTGridBoxLive::Draw(
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
void MTGridBoxLive::Release()
{
	m_Primitive.Release();
}

//******************************************************************************
// �O���b�h���_����
//******************************************************************************
int MTGridBoxLive::_CreateVertexOfGrid(
		MTGRIDBOXLIVE_VERTEX* pVertex,
		unsigned long* pIndex
	)
{
	int result = 0;
	unsigned long i = 0;
	D3DXVECTOR3 vectorStart[4];
	D3DXVECTOR3 vectorEnd[4];
	unsigned long elapsedTime = 0;
	unsigned char portNo = 0;
	
	//     +   1+----+3   +
	//    /|   / �� /    /|gridH    y x
	//   + | 0+----+2   + |�E       |/
	// ��| +   7+----+5 | +      z--+0
	//   |/    / �� /   |/
	//   +   6+----+4   + �� 4 �����_(0,0,0)
	//        gridW
	
	//�O���b�h�{�b�N�X���_���W�擾
	elapsedTime = 0;
	m_NoteDesign.GetGridBoxVirtexPosLive(
			elapsedTime,
			portNo,
			&(vectorStart[0]),
			&(vectorStart[1]),
			&(vectorStart[2]),
			&(vectorStart[3])
		);
	elapsedTime = m_NoteDesign.GetLiveMonitorDisplayDuration();
	m_NoteDesign.GetGridBoxVirtexPosLive(
			elapsedTime,
			portNo,
			&(vectorEnd[0]),
			&(vectorEnd[1]),
			&(vectorEnd[2]),
			&(vectorEnd[3])
		);
	
	//���_���W�E�E�E�@�����قȂ�̂Œ��_��8�ɏW��ł��Ȃ�
	//��̖�
	pVertex[0].p = vectorStart[0];
	pVertex[1].p = vectorEnd[0];
	pVertex[2].p = vectorStart[1];
	pVertex[3].p = vectorEnd[1];
	//���̖�
	pVertex[4].p = vectorStart[3];
	pVertex[5].p = vectorEnd[3];
	pVertex[6].p = vectorStart[2];
	pVertex[7].p = vectorEnd[2];
	
	//�e���_�̖@��
	for (i = 0; i < 8; i++) {
		pVertex[i].n = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	}
	
	//�e���_�̃f�B�t���[�Y�F
	for (i = 0; i < 8; i++) {
		pVertex[i].c = m_NoteDesign.GetGridLineColor();
	}
	
	//�C���f�b�N�X�FDrawIndexdPrimitive�Ăяo����1��ōςނ悤��LINELIST�Ƃ���
	unsigned long index[24] = {
		0, 1,  // 1 ��ʂ̕�
		1, 3,  // 2 �F
		3, 2,  // 3 �F
		2, 0,  // 4 �F
		6, 7,  // 5 ���ʂ̕�
		7, 5,  // 6 �F
		5, 4,  // 7 �F
		4, 6,  // 8 �F
		0, 6,  // 9 �c�̐�
		1, 7,  //10 �F
		3, 5,  //11 �F
		2, 4   //12 �F
	};
	for (i = 0; i < 24; i++) {
		pIndex[i] = index[i];
	}
	
	return result;
}

//******************************************************************************
// �}�e���A���쐬
//******************************************************************************
void MTGridBoxLive::_MakeMaterial(
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
	pMaterial->Power = 10.0f;
	//�����F
	pMaterial->Emissive.r = 0.0f;
	pMaterial->Emissive.g = 0.0f;
	pMaterial->Emissive.b = 0.0f;
	pMaterial->Emissive.a = 0.0f;
}


