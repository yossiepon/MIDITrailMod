//******************************************************************************
//
// MIDITrail / DXPrimitive
//
// �v���~�e�B�u�`��N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// DrawPrimitive, DrawIndexedPrimitive �̑�������b�v����N���X�B
// �C���f�b�N�X�o�b�t�@���쐬���Ȃ����DrawPrimitive
// �C���f�b�N�X�o�b�t�@���쐬�����DrawIndexedPrimitive
// ���g�p�����B

#pragma once

#include <d3d9.h>
#include <d3dx9.h>


//******************************************************************************
// �v���~�e�B�u�`��N���X
//******************************************************************************
class DXPrimitive
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	DXPrimitive(void);
	virtual ~DXPrimitive(void);

	//���\�[�X���
	void Release();
	
	//������
	int Initialize(
			unsigned long vertexSize,
			unsigned long fvfFormat,
			D3DPRIMITIVETYPE type
		);

	//���_�o�b�t�@�^�C���f�b�N�X�o�b�t�@�̐���
	int CreateVertexBuffer(LPDIRECT3DDEVICE9 pD3DDevice, unsigned long vertexNum);
	int CreateIndexBuffer(LPDIRECT3DDEVICE9 pD3DDevice, unsigned long indexNum);

	//���_�f�[�^�^�C���f�b�N�X�f�[�^�o�^
	//  �o�b�t�@�̃��b�N�^�A�����b�N����͎����I�ɍs����
	//  �{���\�b�h�Ɏw�肵���f�[�^�͗��p�ґ����j������
	int SetAllVertex(LPDIRECT3DDEVICE9 pD3DDevice, void* pVertex);
	int SetAllIndex(LPDIRECT3DDEVICE9 pD3DDevice, unsigned long* pIndex);

	//�}�e���A���o�^�i�ȗ��j
	void SetMaterial(D3DMATERIAL9 material);

	//�ړ�����
	void Transform(D3DXMATRIX worldMatrix);

	//�`��
	int Draw(
			LPDIRECT3DDEVICE9 pD3DDevice,
			LPDIRECT3DTEXTURE9 pTexture = NULL,
			int drawPrimitiveNum = -1
		);

// >>> add 20180413 yossiepon begin
	int Draw(
			LPDIRECT3DDEVICE9 pD3DDevice,
			LPDIRECT3DINDEXBUFFER9 pIndexBuffer,
			LPDIRECT3DTEXTURE9 pTexture = NULL,
			int drawPrimitiveNum = -1
		);
// <<< add 20180413 yossiepon end

// >>> add 20120728 yossiepon begin

	int DrawLyrics(
			LPDIRECT3DDEVICE9 pD3DDevice,
			LPDIRECT3DTEXTURE9 *pTextures = NULL,
			int drawPrimitiveNum = -1
		);

// <<< add 20120728 yossiepon end

	//���_�o�b�t�@�^�C���f�b�N�X�o�b�t�@�̃��b�N����
	//  �o�b�t�@�̓��e������������ɂ̓��b�N���ăo�b�t�@�̃|�C���^���擾����
	//  �o�b�t�@�̓��e�������I������A�����b�N����
	int LockVertex(void** pPtrVertex, unsigned long offset = 0, unsigned long size = 0);
	int UnlockVertex();
	int LockIndex(unsigned long** pPtrIndex, unsigned long offset = 0, unsigned long size = 0);
	int UnlockIndex();

private:

	//���_���
	unsigned long m_VertexSize;
	unsigned long m_FVFFormat;
	D3DPRIMITIVETYPE m_PrimitiveType;
	LPDIRECT3DVERTEXBUFFER9 m_pVertexBuffer;
	unsigned long m_VertexNum;
	bool m_IsVertexLocked;
	
	//�C���f�b�N�X���
	LPDIRECT3DINDEXBUFFER9 m_pIndexBuffer;
	unsigned long m_IndexNum;
	bool m_IsIndexLocked;
	
	//�`����
	D3DMATERIAL9 m_Material;
	D3DXMATRIX m_WorldMatrix;

	int _GetPrimitiveNum(unsigned long* pNum);
	void _GetDefaultMaterial(D3DMATERIAL9* pMaterial);

};

