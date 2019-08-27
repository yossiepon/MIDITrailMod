//******************************************************************************
//
// MIDITrail / MTGridBox
//
// �O���b�h�{�b�N�X�`��N���X
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTGridBox.h"

using namespace YNBaseLib;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTGridBox::MTGridBox(void)
{
	m_BarNum = 0;
	m_isVisible = true;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTGridBox::~MTGridBox(void)
{
	Release();
}

//******************************************************************************
// �O���b�h����
//******************************************************************************
int MTGridBox::Create(
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

	//���_�o�b�t�@�����F1������8���_ + (���ߐ�2���_ * ���ߐ�) + (�|�[�g������4���_ * (�|�[�g��-1))
	vertexNum = 8 + (2 * m_BarNum) + (4 * (m_PortList.GetSize() - 1));
	result = m_Primitive.CreateVertexBuffer(pD3DDevice, vertexNum);
	if (result != 0) goto EXIT;

	//�C���f�b�N�X�o�b�t�@�����F(1������12�� * 2���_) + (���ߐ�2���_ * ���ߐ�) + (�|�[�g������4���_ * (�|�[�g��-1))
	indexNum = 24 + (2 * m_BarNum) + (4 * (m_PortList.GetSize() - 1));
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

	//���ߐ��̒��_�ƃC���f�b�N�X�𐶐�
	result = _CreateVertexOfBar(
					&(pVertex[8]),	//���_�o�b�t�@�������݈ʒu
					&(pIndex[24]),	//�C���f�b�N�X�o�b�t�@�������݈ʒu
					8,				//���_�C���f�b�N�X�I�t�Z�b�g
					&barList		//���߃��X�g
				);
	if (result != 0) goto EXIT;

	//�|�[�g��؂���̒��_�ƃC���f�b�N�X�𐶐�
	result = _CreateVertexOfPortSplitLine(
					&(pVertex[8 + (2 * m_BarNum)]),
					&(pIndex[24 + (2 * m_BarNum)]),
					8 + (2 * m_BarNum),
					totalTickTime
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
int MTGridBox::Transform(
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
int MTGridBox::Draw(
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
void MTGridBox::Release()
{
	m_Primitive.Release();
}

//******************************************************************************
// �O���b�h���_����
//******************************************************************************
int MTGridBox::_CreateVertexOfGrid(
		MTGRIDBOX_VERTEX* pVertex,
		unsigned long* pIndex,
		unsigned long totalTickTime
	)
{
	int result = 0;
	unsigned long i = 0;
	unsigned char lastPortNo = 0;
	D3DXVECTOR3 vectorFirstPortStart[4];
	D3DXVECTOR3 vectorFirstPortEnd[4];
	D3DXVECTOR3 vectorFinalPortStart[4];
	D3DXVECTOR3 vectorFinalPortEnd[4];

	//     +   1+----+3   +
	//    /|   / �� /    /|gridH    y x
	//   + | 0+----+2   + |�E       |/
	// ��| +   7+----+5 | +      z--+0
	//   |/    / �� /   |/
	//   +   6+----+4   + �� 4 �����_(0,0,0)
	//        gridW

	m_PortList.GetPort(m_PortList.GetSize()-1, &lastPortNo);

	//�O���b�h�{�b�N�X���_���W�擾
	//  �|�[�g���ƂɃO���b�h��`�悵������
	//  ���̂Ƃ���͑S�|�[�g�������O���b�h�����`�悷��
	m_NoteDesign.GetGridBoxVirtexPos(
			0,
			0,
			&(vectorFirstPortStart[0]),
			&(vectorFirstPortStart[1]),
			&(vectorFirstPortStart[2]),
			&(vectorFirstPortStart[3])
		);
	m_NoteDesign.GetGridBoxVirtexPos(
			totalTickTime,
			0,
			&(vectorFirstPortEnd[0]),
			&(vectorFirstPortEnd[1]),
			&(vectorFirstPortEnd[2]),
			&(vectorFirstPortEnd[3])
		);
	m_NoteDesign.GetGridBoxVirtexPos(
			0,
			lastPortNo,
			&(vectorFinalPortStart[0]),
			&(vectorFinalPortStart[1]),
			&(vectorFinalPortStart[2]),
			&(vectorFinalPortStart[3])
		);
	m_NoteDesign.GetGridBoxVirtexPos(
			totalTickTime,
			lastPortNo,
			&(vectorFinalPortEnd[0]),
			&(vectorFinalPortEnd[1]),
			&(vectorFinalPortEnd[2]),
			&(vectorFinalPortEnd[3])
		);

	//���_���W�E�E�E�@�����قȂ�̂Œ��_��8�ɏW��ł��Ȃ�
	//��̖�
	pVertex[0].p = vectorFinalPortStart[0];
	pVertex[1].p = vectorFinalPortEnd[0];
	pVertex[2].p = vectorFirstPortStart[1];
	pVertex[3].p = vectorFirstPortEnd[1];
	//���̖�
	pVertex[4].p = vectorFirstPortStart[3];
	pVertex[5].p = vectorFirstPortEnd[3];
	pVertex[6].p = vectorFinalPortStart[2];
	pVertex[7].p = vectorFinalPortEnd[2];

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
// ���ߐ����_����
//******************************************************************************
int MTGridBox::_CreateVertexOfBar(
		MTGRIDBOX_VERTEX* pVertex,
		unsigned long* pIndex,
		unsigned long vartexIndexOffset,
		SMBarList* pBarList
	)
{
	int result = 0;
	unsigned long i = 0;
	unsigned long tickTime = 0;
	unsigned char lastPortNo = 0;
	D3DXVECTOR3 vectorStart[4];

	//     +   1+----+3   +
	//    /|   / �� /    /|gridH    y x
	//   + | 0+----+2   + |�E       |/
	// ��| +   7+----+5 | +      z--+0
	//   |/    / �� /   |/
	//   +   6+----+4   + �� 4 �����_(0,0,0)
	//        gridW

	m_PortList.GetPort(m_PortList.GetSize()-1, &lastPortNo);

	//���_���W�F���ߐ��͍��ʂ�y���ɉ���
	for (i = 0; i < pBarList->GetSize(); i++) {
		result = pBarList->GetBar(i, &tickTime);
		if (result != 0) goto EXIT;

		//�O���b�h�{�b�N�X���_���W�擾
		m_NoteDesign.GetGridBoxVirtexPos(
				tickTime,
				lastPortNo,
				&(vectorStart[0]),
				&(vectorStart[1]),
				&(vectorStart[2]),
				&(vectorStart[3])
			);

		pVertex[(i*2)+0].p = vectorStart[0];
		pVertex[(i*2)+1].p = vectorStart[2];
	}

	//�e���_�̖@��
	for (i = 0; i < pBarList->GetSize(); i++) {
		pVertex[(i*2)+0].n = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
		pVertex[(i*2)+1].n = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	}

	//�e���_�̃f�B�t���[�Y�F
	for (i = 0; i < pBarList->GetSize(); i++) {
		pVertex[(i*2)+0].c = m_NoteDesign.GetGridLineColor();
		pVertex[(i*2)+1].c = m_NoteDesign.GetGridLineColor();
	}

	//�C���f�b�N�X�E�E�EDrawIndexdPrimitive�Ăяo����1��ōςނ悤��LINELIST�Ƃ���
	for (i = 0; i < pBarList->GetSize(); i++) {
		pIndex[(i*2)+0] = vartexIndexOffset + (i*2);
		pIndex[(i*2)+1] = vartexIndexOffset + (i*2)+1;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �|�[�g��؂�����_����
//******************************************************************************
int MTGridBox::_CreateVertexOfPortSplitLine(
		MTGRIDBOX_VERTEX* pVertex,
		unsigned long* pIndex,
		unsigned long vartexIndexOffset,
		unsigned long totalTickTime
	)
{
	int result = 0;
	unsigned long i = 0;
	unsigned long j = 0;
	unsigned long count = 0;
	unsigned char portNo = 0;
	unsigned char lastPortNo = 0;
	D3DXVECTOR3 vectorStart[4];
	D3DXVECTOR3 vectorEnd[4];

	//     +   1+----+3   +
	//    /|   / �� /    /|gridH    y x
	//   + | 0+----+2   + |�E       |/
	// ��| +   7+----+5 | +      z--+0
	//   |/    / �� /   |/
	//   +   6+----+4   + �� 4 �����_(0,0,0)
	//        gridW

	m_PortList.GetPort(m_PortList.GetSize()-1, &lastPortNo);

	//���_���W�F2�|�[�g�ڂ����؂���𐶐�����
	count = 0;
	for (i = 1; i < m_PortList.GetSize(); i++) {
		result = m_PortList.GetPort(i, &portNo);
		if (result != 0) goto EXIT;

		//�O���b�h�{�b�N�X���_���W�擾
		m_NoteDesign.GetGridBoxVirtexPos(
				0,
				portNo,
				&(vectorStart[0]),
				&(vectorStart[1]),
				&(vectorStart[2]),
				&(vectorStart[3])
			);
		m_NoteDesign.GetGridBoxVirtexPos(
				totalTickTime,
				portNo,
				&(vectorEnd[0]),
				&(vectorEnd[1]),
				&(vectorEnd[2]),
				&(vectorEnd[3])
			);

		pVertex[(count*4)+0].p = vectorStart[1];
		pVertex[(count*4)+1].p = vectorEnd[1];
		pVertex[(count*4)+2].p = vectorStart[3];
		pVertex[(count*4)+3].p = vectorEnd[3];
		count++;
	}

	//���W�ȊO�̏��o�^
	count = 0;
	for (i = 1; i < m_PortList.GetSize(); i++) {
		for (j = 0; j < 4; j++) {

			//�@��
			pVertex[(count*4)+j].n = D3DXVECTOR3(0.0f, 0.0f, -1.0f);

			//�f�B�t���[�Y�F
			pVertex[(count*4)+j].c = m_NoteDesign.GetGridLineColor();

			//�C���f�b�N�X
			pIndex[(count*4)+j] = vartexIndexOffset + (count*4) + j;
		}
		count++;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �}�e���A���쐬
//******************************************************************************
void MTGridBox::_MakeMaterial(
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

