//******************************************************************************
//
// MIDITrail / MTStaticCaption
//
// �ÓI�L���v�V�����`��N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTStaticCaption.h"

using namespace YNBaseLib;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTStaticCaption::MTStaticCaption(void)
{
	m_pVertex = NULL;
	m_Color = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTStaticCaption::~MTStaticCaption(void)
{
	Release();
}

//******************************************************************************
// �ÓI�L���v�V��������
//******************************************************************************
int MTStaticCaption::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		TCHAR* pFontName,
		unsigned long fontSize,
		TCHAR* pCaption
   )
{
	int result = 0;

	if (pD3DDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	if ((pFontName == NULL) || (fontSize == 0) || (pCaption == NULL)) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	Release();

	//�e�N�X�`������
	result = _CreateTexture(pD3DDevice, pFontName, fontSize, pCaption);
	if (result != 0) goto EXIT;

	//���_�𐶐�����
	result = _CreateVertex();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �e�N�X�`���T�C�Y�擾
//******************************************************************************
void MTStaticCaption::GetTextureSize(
		unsigned long* pHeight,
		unsigned long* pWidth
	)
{
	m_FontTexture.GetTextureSize(pHeight, pWidth);
}

//******************************************************************************
// ������ݒ�
//******************************************************************************
void MTStaticCaption::SetColor(
		D3DXCOLOR color
	)
{
	m_Color = color;
}

//******************************************************************************
// �`��
//******************************************************************************
int MTStaticCaption::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice,
		float x,
		float y,
		float magRate
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;
	D3DXMATRIX mtxWorld;

	if ((pD3DDevice == NULL) || (m_pVertex == NULL)) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//���_���W�ݒ�
	_SetVertexPosition(
			m_pVertex,	//���_���W�z��
			x,			//�`��ʒux
			y,			//�`��ʒuy
			magRate		//�g�嗦
		);

	//���_�F�ݒ�
	_SetVertexColor(m_pVertex, m_Color);

	//�����_�����O�p�C�v���C���Ƀe�N�X�`����ݒ�
	hresult = pD3DDevice->SetTexture(
					0,							//�X�e�[�W���ʎq
					m_FontTexture.GetTexture()	//�e�N�X�`���I�u�W�F�N�g
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

	//�e�N�X�`���X�e�[�W�ݒ�
	//  �J���[���Z�F����1���g�p  ����1�F�|���S��
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	// �A���t�@���Z�F����1���g�p  ����1�F�e�N�X�`��
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

	//�e�N�X�`���t�B���^
	pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	//�����_�����O�p�C�v���C���ɒ��_�o�b�t�@FVF�t�H�[�}�b�g��ݒ�
	hresult = pD3DDevice->SetFVF(_GetFVFFormat());
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

	//�����_�����O�p�C�v���C���Ƀ}�e���A����ݒ�
	//�Ȃ�

	//�S�{�[�h�`��
	hresult = pD3DDevice->DrawPrimitiveUP(
					D3DPT_TRIANGLESTRIP,			//�v���~�e�B�u���
					2,								//�v���~�e�B�u��
					m_pVertex,						//���_�f�[�^
					sizeof(MTSTATICCAPTION_VERTEX)	//���_�f�[�^�̃T�C�Y
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// ���
//******************************************************************************
void MTStaticCaption::Release()
{
	m_FontTexture.Clear();

	delete [] m_pVertex;
	m_pVertex = NULL;
}

//******************************************************************************
// �e�N�X�`������
//******************************************************************************
int MTStaticCaption::_CreateTexture(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pFontName,
		unsigned long fontSize,
		const TCHAR* pCaption
	)
{
	int result = 0;
	bool isForceFixedPitch = false;
	unsigned long color = 0x00FFFFFF;

	//�t�H���g�ݒ�F�Œ�s�b�`����
	result = m_FontTexture.SetFont(pFontName, fontSize, color, isForceFixedPitch);
	if (result != 0) goto EXIT;

	//�^�C�������ꗗ�e�N�X�`���쐬
	result = m_FontTexture.CreateTexture(pD3DDevice, pCaption);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ���_����
//******************************************************************************
int MTStaticCaption::_CreateVertex()
{
	int result = 0;
	unsigned long i = 0;
	MTSTATICCAPTION_VERTEX* pVertex = NULL;
	
	//���_����
	try {
		pVertex = new MTSTATICCAPTION_VERTEX[4];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

	//���_���W�ݒ�
	_SetVertexPosition(
			pVertex,	//���_���W�z��
			0.0f,		//�`��ʒux
			0.0f,		//�`��ʒuy
			1.0f		//�g�嗦
		);

	for (i = 0; i < 4; i++) {
		//�e���_�̏��Z��
		pVertex[i].rhw = 1.0f;
		//�e���_�̃f�B�t���[�Y�F
		pVertex[i].c = m_Color;
	}

	//�e�N�X�`�����W
	pVertex[0].t = D3DXVECTOR2(0.0f, 0.0f);
	pVertex[1].t = D3DXVECTOR2(1.0f, 0.0f);
	pVertex[2].t = D3DXVECTOR2(0.0f, 1.0f);
	pVertex[3].t = D3DXVECTOR2(1.0f, 1.0f);

	m_pVertex = pVertex;
	pVertex = NULL;

EXIT:;
	delete [] pVertex;
	return result;
}

//******************************************************************************
// ���_�ʒu�ݒ�
//******************************************************************************
void MTStaticCaption::_SetVertexPosition(
		MTSTATICCAPTION_VERTEX* pVertex,
		float x,
		float y,
		float magRate
	)
{
	unsigned long i = 0;
	unsigned long texHeight = 0;
	unsigned long texWidth = 0;
	float height = 0.0f;
	float width = 0.0f;

	//�`��T�C�Y
	m_FontTexture.GetTextureSize(&texHeight, &texWidth);
	height = (float)texHeight * magRate;
	width  = (float)texWidth  * magRate;

	//���_���W
	pVertex[0].p = D3DXVECTOR3(0.0f , 0.0f,   0.0f);
	pVertex[1].p = D3DXVECTOR3(width, 0.0f,   0.0f);
	pVertex[2].p = D3DXVECTOR3(0.0f , height, 0.0f);
	pVertex[3].p = D3DXVECTOR3(width, height, 0.0f);

	//�`��ʒu�Ɉړ�
	for (i = 0; i < 4; i++) {
		pVertex[i].p.x += x;
		pVertex[i].p.y += y;
	}

	return;
}

//******************************************************************************
// ���_�F�ݒ�
//******************************************************************************
void MTStaticCaption::_SetVertexColor(
		MTSTATICCAPTION_VERTEX* pVertex,
		D3DXCOLOR color
	)
{
	unsigned long i = 0;

	for (i = 0; i < 4; i++) {
		pVertex[i].c = color;
	}

	return;
}

