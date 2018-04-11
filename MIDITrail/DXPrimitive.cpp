//******************************************************************************
//
// MIDITrail / DXPrimitive
//
// �v���~�e�B�u�`��N���X
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "DXPrimitive.h"

using namespace YNBaseLib;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
DXPrimitive::DXPrimitive(void)
{
	m_VertexSize = 0;
	m_FVFFormat = 0;
	m_PrimitiveType = D3DPT_TRIANGLELIST;
	m_pVertexBuffer = NULL;
	m_pIndexBuffer = NULL;
	ZeroMemory(&m_Material, sizeof(D3DMATERIAL9));
	D3DXMatrixIdentity(&m_WorldMatrix);
	m_VertexNum = 0;
	m_IndexNum = 0;
	m_IsVertexLocked = false;
	m_IsIndexLocked = false;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
DXPrimitive::~DXPrimitive(void)
{
	Release();
}

//******************************************************************************
// ���
//******************************************************************************
void DXPrimitive::Release()
{
	if (m_pVertexBuffer != NULL) {
		m_pVertexBuffer->Release();
		m_pVertexBuffer = NULL;
	}
	if (m_pIndexBuffer != NULL) {
		m_pIndexBuffer->Release();
		m_pIndexBuffer = NULL;
	}
	m_VertexNum = 0;
	m_IndexNum = 0;
	m_IsVertexLocked = false;
	m_IsIndexLocked = false;
}

//******************************************************************************
// ������
//******************************************************************************
int DXPrimitive::Initialize(
		unsigned long vertexSize,
		unsigned long fvfFormat,
		D3DPRIMITIVETYPE type
	)
{
	int result = 0;

	Release();

	m_VertexSize = vertexSize;
	m_FVFFormat = fvfFormat;
	m_PrimitiveType = type;
	_GetDefaultMaterial(&m_Material);

	return result;
}

//******************************************************************************
// ���_�o�b�t�@����
//******************************************************************************
int DXPrimitive::CreateVertexBuffer(
		LPDIRECT3DDEVICE9 pD3DDevice,
		unsigned long vertexNum
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;

	if (pD3DDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	if (m_pVertexBuffer != NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//���_�o�b�t�@����
	if (vertexNum > 0) {
		hresult = pD3DDevice->CreateVertexBuffer(
						m_VertexSize * vertexNum,	//���_�o�b�t�@�̑S�̃T�C�Y(byte)
						D3DUSAGE_WRITEONLY,			//���_�o�b�t�@�̎g�p���@
						m_FVFFormat,				//���_��FVF�t�H�[�}�b�g
						D3DPOOL_MANAGED,			//���\�[�X�z�u�ꏊ�ƂȂ郁�����N���X
						&m_pVertexBuffer,			//�쐬���ꂽ���_�o�b�t�@
						NULL						//�\��p�����[�^
					);
		if (FAILED(hresult)) {
			result = YN_SET_ERR("DirectX API error.", hresult, vertexNum);
			goto EXIT;
		}
	}

	m_VertexNum = vertexNum;

EXIT:;
	return result;
}

//******************************************************************************
// �C���f�b�N�X�o�b�t�@����
//******************************************************************************
int DXPrimitive::CreateIndexBuffer(
		LPDIRECT3DDEVICE9 pD3DDevice,
		unsigned long indexNum
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;
	
	if (pD3DDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	if (m_pIndexBuffer != NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	
	//�C���f�b�N�X�o�b�t�@����
	if (indexNum > 0) {
		hresult = pD3DDevice->CreateIndexBuffer(
						sizeof(unsigned long) * indexNum,
												//�C���f�b�N�X�o�b�t�@�̑S�̃T�C�Y(byte)
						D3DUSAGE_WRITEONLY,		//�g�p���@
						D3DFMT_INDEX32,			//�C���f�b�N�X�o�b�t�@�̃t�H�[�}�b�g
						D3DPOOL_MANAGED,		//���\�[�X�z�u�ꏊ�ƂȂ郁�����N���X
						&m_pIndexBuffer,		//�쐬���ꂽ�C���f�b�N�X�o�b�t�@
						NULL					//�\��p�����[�^
					);
		if (FAILED(hresult)) {
			result = YN_SET_ERR("DirectX API error.", hresult, indexNum);
			goto EXIT;
		}
	}

	m_IndexNum = indexNum;

EXIT:;
	return result;
}

//******************************************************************************
// ���_�o�^
//******************************************************************************
int DXPrimitive::SetAllVertex(
		LPDIRECT3DDEVICE9 pD3DDevice,
		void* pVertex
	)
{
	int result = 0;
	void* pBuf = NULL;

	//���_�o�b�t�@�̃��b�N
	result = LockVertex(&pBuf);
	if (result != 0) goto EXIT;

	//�o�b�t�@�ɒ��_�f�[�^����������
	try {
		memcpy(pBuf, pVertex, (m_VertexSize * m_VertexNum));
	}
	catch (...) {
		result = YN_SET_ERR("Memory access error.", (DWORD)pVertex, m_VertexNum);
		goto EXIT;
	}

EXIT:;
	UnlockVertex();
	return result;
}

//******************************************************************************
// �C���f�b�N�X�o�^
//******************************************************************************
int DXPrimitive::SetAllIndex(
		LPDIRECT3DDEVICE9 pD3DDevice,
		unsigned long* pIndex
	)
{
	int result = 0;
	unsigned long* pBuf = NULL;

	//���_�o�b�t�@�̃��b�N
	result = LockIndex(&pBuf);
	if (result != 0) goto EXIT;

	//�o�b�t�@�ɒ��_�f�[�^����������
	try {
		memcpy(pBuf, pIndex, (sizeof(unsigned long)* m_IndexNum));
	}
	catch (...) {
		result = YN_SET_ERR("Memory access error.", (DWORD)pIndex, m_IndexNum);
		goto EXIT;
	}

EXIT:;
	UnlockIndex();
	return result;
}

//******************************************************************************
// �}�e���A���ݒ�
//******************************************************************************
void DXPrimitive::SetMaterial(
		D3DMATERIAL9 material
	)
{
	m_Material = material;
}

//******************************************************************************
// �ړ�
//******************************************************************************
void DXPrimitive::Transform(
		D3DXMATRIX worldMatrix
	)
{
	m_WorldMatrix = worldMatrix;
}

//******************************************************************************
// �`��
//******************************************************************************
int DXPrimitive::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice,
		LPDIRECT3DTEXTURE9 pTexture,
		int drawPrimitiveNum
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;
	unsigned long primitiveNum = 0;

	if (m_IsVertexLocked || m_IsIndexLocked) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//���_�����݂��Ȃ���Ή������Ȃ�
	if (m_pVertexBuffer == NULL) goto EXIT;

	//�����_�����O�p�C�v���C���ɒ��_�o�b�t�@��ݒ�
	hresult = pD3DDevice->SetStreamSource(
					0,					//�X�g���[���ԍ�
					m_pVertexBuffer,	//�X�g���[���f�[�^
					0,					//���_�f�[�^�J�n�I�t�Z�b�g�ʒu(bytes)
					m_VertexSize		//���_�f�[�^�\���̃T�C�Y
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, m_VertexSize);
		goto EXIT;
	}

	//�����_�����O�p�C�v���C���ɃC���f�b�N�X�o�b�t�@��ݒ�
	if (m_pIndexBuffer != NULL) {
		hresult = pD3DDevice->SetIndices(m_pIndexBuffer);
		if (FAILED(hresult)) {
			result = YN_SET_ERR("DirectX API error.", (DWORD)hresult, (DWORD)m_pIndexBuffer);
			goto EXIT;
		}
	}

	//�����_�����O�p�C�v���C���ɒ��_�o�b�t�@FVF�t�H�[�}�b�g��ݒ�
	hresult = pD3DDevice->SetFVF(m_FVFFormat);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, m_FVFFormat);
		goto EXIT;
	}

	//�����_�����O�p�C�v���C���Ƀ}�e���A����ݒ�
	hresult = pD3DDevice->SetMaterial(&m_Material);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

	//�����_�����O�p�C�v���C���Ƀe�N�X�`����ݒ�F�X�e�[�W0
	hresult = pD3DDevice->SetTexture(0, pTexture);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, (DWORD)pTexture);
		goto EXIT;
	}

	//�����_�����O�p�C�v���C���Ɉړ��}�g���b�N�X���Z�b�g
	hresult = pD3DDevice->SetTransform(D3DTS_WORLD, &m_WorldMatrix);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

	//�v���~�e�B�u���擾
	result = _GetPrimitiveNum(&primitiveNum);
	if (result != 0) goto EXIT;

	//���ڃv���~�e�B�u�����w�肳�ꂽ�ꍇ�͂���ɏ]��
	if (drawPrimitiveNum > 0) {
		//�o�b�t�@�T�C�Y�𒴂����w��̓G���[
		if ((unsigned long)drawPrimitiveNum > primitiveNum) {
			result = YN_SET_ERR("Program error.", drawPrimitiveNum, primitiveNum);
			goto EXIT;
		}
		primitiveNum = drawPrimitiveNum;
	}

	//�C���f�b�N�X�t���v���~�e�B�u�̕`��
	if (m_pIndexBuffer != NULL) {
		hresult = pD3DDevice->DrawIndexedPrimitive(
						m_PrimitiveType,	//�v���~�e�B�u���
						0,					//���_�o�b�t�@�J�n�C���f�b�N�X
						0,					//���_�o�b�t�@�ŏ��C���f�b�N�X
						m_VertexNum,		//�Q�Ƃ��钸�_�̐�
						0,					//�C���f�b�N�X�o�b�t�@�J�n�C���f�b�N�X
						primitiveNum		//�v���~�e�B�u��
					);
		if (FAILED(hresult)) {
			result = YN_SET_ERR("DirectX API error.", hresult, primitiveNum);
			goto EXIT;
		}
	}
	//�C���f�b�N�X�Ȃ��v���~�e�B�u�̕`��
	else {
		hresult = pD3DDevice->DrawPrimitive(
						m_PrimitiveType,	//�v���~�e�B�u���
						0,					//���_�o�b�t�@�J�n�C���f�b�N�X
						primitiveNum		//�v���~�e�B�u��
					);
		if (FAILED(hresult)) {
			result = YN_SET_ERR("DirectX API error.", hresult, primitiveNum);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

// >>> add 20120728 yossiepon begin

//******************************************************************************
// �̎��`��
//******************************************************************************
int DXPrimitive::DrawLyrics(
		LPDIRECT3DDEVICE9 pD3DDevice,
		LPDIRECT3DTEXTURE9 *pTextures,
		int drawPrimitiveNum
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;
	unsigned long primitiveNum = 0;

	if (m_IsVertexLocked || m_IsIndexLocked) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//���_�����݂��Ȃ���Ή������Ȃ�
	if (m_pVertexBuffer == NULL) goto EXIT;

	//�v���~�e�B�u���擾
	primitiveNum = drawPrimitiveNum;

	for(unsigned long i = 0; i < primitiveNum / 2; i++) {

		//�����_�����O�p�C�v���C���ɒ��_�o�b�t�@��ݒ�
		hresult = pD3DDevice->SetStreamSource(
						0,					//�X�g���[���ԍ�
						m_pVertexBuffer,	//�X�g���[���f�[�^
						m_VertexSize * 6 * i,	//���_�f�[�^�J�n�I�t�Z�b�g�ʒu(bytes)
						m_VertexSize		//���_�f�[�^�\���̃T�C�Y
					);
		if (FAILED(hresult)) {
			result = YN_SET_ERR("DirectX API error.", hresult, m_VertexSize);
			goto EXIT;
		}

		//�����_�����O�p�C�v���C���ɒ��_�o�b�t�@FVF�t�H�[�}�b�g��ݒ�
		hresult = pD3DDevice->SetFVF(m_FVFFormat);
		if (FAILED(hresult)) {
			result = YN_SET_ERR("DirectX API error.", hresult, m_FVFFormat);
			goto EXIT;
		}

		//�����_�����O�p�C�v���C���Ƀ}�e���A����ݒ�
		hresult = pD3DDevice->SetMaterial(&m_Material);
		if (FAILED(hresult)) {
			result = YN_SET_ERR("DirectX API error.", hresult, 0);
			goto EXIT;
		}

		//�����_�����O�p�C�v���C���Ƀe�N�X�`����ݒ�F�X�e�[�W0
		hresult = pD3DDevice->SetTexture(0, pTextures[i]);
		if (FAILED(hresult)) {
			result = YN_SET_ERR("DirectX API error.", hresult, (DWORD)pTextures[i]);
			goto EXIT;
		}

		//�����_�����O�p�C�v���C���Ɉړ��}�g���b�N�X���Z�b�g
		hresult = pD3DDevice->SetTransform(D3DTS_WORLD, &m_WorldMatrix);
		if (FAILED(hresult)) {
			result = YN_SET_ERR("DirectX API error.", hresult, 0);
			goto EXIT;
		}

		//�C���f�b�N�X�Ȃ��v���~�e�B�u�̕`��
		hresult = pD3DDevice->DrawPrimitive(
						m_PrimitiveType,	//�v���~�e�B�u���
						0, //i * 2,				//���_�o�b�t�@�J�n�C���f�b�N�X
						2					//�v���~�e�B�u��
					);
		if (FAILED(hresult)) {
			result = YN_SET_ERR("DirectX API error.", hresult, primitiveNum);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

// <<< add 20120728 yossiepon end

//******************************************************************************
// ���_�o�b�t�@���b�N
//******************************************************************************
int DXPrimitive::LockVertex(
		void** pPtrVertex,
		unsigned long offset,	//�ȗ����̓[��
		unsigned long size		//�ȗ����̓[��
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;

	if (m_IsVertexLocked) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	if ((m_VertexSize * m_VertexNum) < (offset + size)) {
		result = YN_SET_ERR("Program error.", offset, size);
		goto EXIT;
	}

	//���_�o�b�t�@�̃��b�N�ƃo�b�t�@�������|�C���^�擾
	if (m_pVertexBuffer != NULL) {
		hresult = m_pVertexBuffer->Lock(
						offset,		//���b�N���钸�_�̃I�t�Z�b�g
						size,		//���b�N���钸�_�̃T�C�Y(byte)
						pPtrVertex,	//�o�b�t�@�������|�C���^
						0			//���b�L���O�t���O
					);
		if (FAILED(hresult)) {
			result = YN_SET_ERR("DirectX API error.", hresult, (DWORD)pPtrVertex);
			goto EXIT;
		}
	}

	m_IsVertexLocked = true;

EXIT:;
	return result;
}

//******************************************************************************
// ���_�o�b�t�@���b�N����
//******************************************************************************
int DXPrimitive::UnlockVertex()
{
	int result = 0;
	HRESULT hresult = D3D_OK;
	
	if (m_IsVertexLocked) {
		if (m_pVertexBuffer != NULL) {
			hresult = m_pVertexBuffer->Unlock();
			if (FAILED(hresult)) {
				result = YN_SET_ERR("DirectX API error.", hresult, 0);
				goto EXIT;
			}
		}
		m_IsVertexLocked = false;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// �C���f�b�N�X�o�b�t�@���b�N
//******************************************************************************
int DXPrimitive::LockIndex(
		unsigned long** pPtrIndex,
		unsigned long offset,	//�ȗ����̓[��
		unsigned long size		//�ȗ����̓[��
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;

	if (m_IsIndexLocked) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	if ((sizeof(unsigned long) * m_IndexNum) < (offset + size)) {
		result = YN_SET_ERR("Program error.", offset, size);
		goto EXIT;
	}

	//�C���f�b�N�X�o�b�t�@�̃��b�N�ƃo�b�t�@�������|�C���^�擾
	if (m_pIndexBuffer != NULL) {
		hresult = m_pIndexBuffer->Lock(
						offset,		//���b�N����C���f�b�N�X�̃I�t�Z�b�g(byte)
						size,		//���b�N����C���f�b�N�X�̃T�C�Y(byte)
						(void**)pPtrIndex,	//�o�b�t�@�������|�C���^
						0			//���b�L���O�t���O
					);
		if (FAILED(hresult)) {
			result = YN_SET_ERR("DirectX API error.", hresult, (DWORD)pPtrIndex);
			goto EXIT;
		}
	}

	m_IsIndexLocked = true;

EXIT:;
	return result;
}

//******************************************************************************
// �C���f�b�N�X�o�b�t�@���b�N����
//******************************************************************************
int DXPrimitive::UnlockIndex()
{
	int result = 0;
	HRESULT hresult = D3D_OK;

	if (m_IsIndexLocked) {
		if (m_pIndexBuffer != NULL) {
			hresult = m_pIndexBuffer->Unlock();
			if (FAILED(hresult)) {
			result = YN_SET_ERR("DirectX API error.", hresult, 0);
				goto EXIT;
			}
		}
		m_IsIndexLocked = false;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �v���~�e�B�u���擾
//******************************************************************************
int DXPrimitive::_GetPrimitiveNum(
		unsigned long* pNum
	)
{
	int result = 0;
	unsigned long vertexNum = 0;

	vertexNum = m_VertexNum;
	if (m_pIndexBuffer != NULL) {
		vertexNum = m_IndexNum;
	}

	if (vertexNum == 0) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	switch (m_PrimitiveType) {
		case D3DPT_POINTLIST:
			*pNum = vertexNum;
			break;
			
		case D3DPT_LINELIST:
			if ((vertexNum % 2) != 0) {
				result = YN_SET_ERR("Program error.", vertexNum, 0);
				goto EXIT;
			}
			*pNum = vertexNum / 2;
			break;
			
		case D3DPT_LINESTRIP:
			if (vertexNum < 2) {
				result = YN_SET_ERR("Program error.", vertexNum, 0);
				goto EXIT;
			}
			*pNum = vertexNum - 1;
			break;
			
		case D3DPT_TRIANGLELIST:
			if ((vertexNum % 3) != 0) {
				result = YN_SET_ERR("Program error.", vertexNum, 0);
				goto EXIT;
			}
			*pNum = vertexNum / 3;
			break;
			
		case D3DPT_TRIANGLESTRIP:
			if (vertexNum < 3) {
				result = YN_SET_ERR("Program error.", vertexNum, 0);
				goto EXIT;
			}
			*pNum = vertexNum - 2;
			break;
			
		case D3DPT_TRIANGLEFAN:
			if (vertexNum < 3) {
				result = YN_SET_ERR("Program error.", vertexNum, 0);
				goto EXIT;
			}
			*pNum = vertexNum - 2;
			break;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �f�t�H���g�}�e���A��
//******************************************************************************
void DXPrimitive::_GetDefaultMaterial(
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
	pMaterial->Specular.r = 0.0f;
	pMaterial->Specular.g = 0.0f;
	pMaterial->Specular.b = 0.0f;
	pMaterial->Specular.a = 0.0f;
	//���ʔ��ˌ��̑N���x
	pMaterial->Power = 0.0f;
	//�����F
	pMaterial->Emissive.r = 0.0f;
	pMaterial->Emissive.g = 0.0f;
	pMaterial->Emissive.b = 0.0f;
	pMaterial->Emissive.a = 0.0f;
}


