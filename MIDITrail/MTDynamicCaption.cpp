//******************************************************************************
//
// MIDITrail / MTDynamicCaption
//
// ���I�L���v�V�����`��N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTDynamicCaption.h"

using namespace YNBaseLib;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTDynamicCaption::MTDynamicCaption(void)
{
	m_pVertex = NULL;
	m_Chars[0] = _T('\0');
	m_CaptionSize = 0;
	m_Color = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTDynamicCaption::~MTDynamicCaption(void)
{
	Release();
}

//******************************************************************************
// �t�H���g�^�C������
//******************************************************************************
int MTDynamicCaption::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pFontName,
		unsigned long fontSize,
		const TCHAR* pCharacters,
		unsigned long captionSize
   )
{
	int result = 0;

	Release();

	if (pD3DDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	if ((pFontName == NULL) || (fontSize == 0)) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	if ((pCharacters == NULL) ||(captionSize == 0)) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	
	m_CaptionSize = captionSize;
	
	//�e�N�X�`������
	result = _CreateTexture(pD3DDevice, pFontName, fontSize, pCharacters);
	if (result != 0) goto EXIT;
	
	//�^�C���̒��_�𐶐�����
	result = _CreateVertex();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �e�N�X�`���T�C�Y�擾
//******************************************************************************
void MTDynamicCaption::GetTextureSize(
		unsigned long* pHeight,
		unsigned long* pWidth
	)
{
	m_FontTexture.GetTextureSize(pHeight, pWidth);
}

//******************************************************************************
// ������ݒ�
//******************************************************************************
int MTDynamicCaption::SetString(
		TCHAR* pStr
	)
{
	int result = 0;
	unsigned long i = 0;
	D3DXVECTOR2 v0, v1, v2, v3;
	
	if (pStr == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	
	for (i= 0; i < 6*m_CaptionSize; i++) {
		m_pVertex[i].t = D3DXVECTOR2(0.0f, 0.0f);
	}
	for (i= 0; i < m_CaptionSize; i++) {
		if (pStr[i] == _T('\0')) break;

		result = _GetTextureUV(pStr[i], &v0, &v1, &v2, &v3);
		if (result != 0) goto EXIT;
		
		// 0+--+1
		//  | /|
		//  |/ |
		// 2+--+3
		m_pVertex[6*i+0].t = v0;
		m_pVertex[6*i+1].t = v1;
		m_pVertex[6*i+2].t = v2;
		m_pVertex[6*i+3].t = v2;
		m_pVertex[6*i+4].t = v1;
		m_pVertex[6*i+5].t = v3;
	}

EXIT:;
	return result;
}

//******************************************************************************
// ������ݒ�
//******************************************************************************
void MTDynamicCaption::SetColor(
		D3DXCOLOR color
	)
{
	m_Color = color;
}

//******************************************************************************
// �`��
//******************************************************************************
int MTDynamicCaption::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice,
		float x,
		float y,
		float magRate
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;
	D3DXMATRIX mtxWorld;

	if (pD3DDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	if (m_pVertex == NULL) {
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
					D3DPT_TRIANGLELIST,				//�v���~�e�B�u���
					2 * m_CaptionSize,				//�v���~�e�B�u��
					m_pVertex,						//���_�f�[�^
					sizeof(MTDYNAMICCAPTION_VERTEX)	//���_�f�[�^�̃T�C�Y
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, m_CaptionSize);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// ���
//******************************************************************************
void MTDynamicCaption::Release()
{
	m_FontTexture.Clear();
	
	delete [] m_pVertex;
	m_pVertex = NULL;
}

//******************************************************************************
// �e�N�X�`������
//******************************************************************************
int MTDynamicCaption::_CreateTexture(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pFontName,
		unsigned long fontSize,
		const TCHAR* pCharacters
	)
{
	int result = 0;
	errno_t eresult = 0;
	unsigned long color = 0x00FFFFFF;
	bool isForceFixedPitch = true;

	//�^�C�������ꗗ���i�[
	eresult = _tcscpy_s(m_Chars, MTDYNAMICCAPTION_MAX_CHARS, pCharacters);
	if (eresult != 0) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//�t�H���g�ݒ�F�Œ�s�b�`����
	result = m_FontTexture.SetFont(pFontName, fontSize, color, isForceFixedPitch);
	if (result != 0) goto EXIT;

	//�^�C�������ꗗ�e�N�X�`���쐬
	result = m_FontTexture.CreateTexture(pD3DDevice, pCharacters);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �t�H���g�^�C�����_����
//******************************************************************************
int MTDynamicCaption::_CreateVertex()
{
	int result = 0;
	unsigned long i = 0;
	MTDYNAMICCAPTION_VERTEX* pVertex = NULL;

	//���_����
	try {
		pVertex = new MTDYNAMICCAPTION_VERTEX[6*m_CaptionSize];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", m_CaptionSize, 0);
		goto EXIT;
	}

	//���_���W�ݒ�
	_SetVertexPosition(
			pVertex,	//���_���W�z��
			0.0f,		//�`��ʒux
			0.0f,		//�`��ʒuy
			1.0f		//�g�嗦
		);

	for (i = 0; i < 6*m_CaptionSize; i++) {
		//�e���_�̏��Z��
		pVertex[i].rhw = 1.0f;
		//�e���_�̃f�B�t���[�Y�F
		pVertex[i].c = m_Color;
		//�e���_�̃e�N�X�`�����W
		pVertex[i].t = D3DXVECTOR2(0.0f, 0.0f);
	}

	m_pVertex = pVertex;
	pVertex = NULL;

EXIT:;
	delete [] pVertex;
	return result;
}

//******************************************************************************
// �e�N�X�`��UV���W�擾
//******************************************************************************
int MTDynamicCaption::_GetTextureUV(
		TCHAR target,
		D3DXVECTOR2* pV0,
		D3DXVECTOR2* pV1,
		D3DXVECTOR2* pV2,
		D3DXVECTOR2* pV3
	)
{
	int result = 0;
	bool isFound = false;
	unsigned long i = 0;
	unsigned long charsNum = 0;
	float fontNo = 0;
	float fontWidth = 0.0f;

	charsNum = _tcslen(m_Chars);
	for (i = 0; i < charsNum; i++) {
		if (m_Chars[i] == target) {
			isFound = true;
			fontNo = (float)i;
			break;
		}
	}

	fontWidth = 1.0f / (float)charsNum;

	//���������ꍇ�͊Y�����镶����UV���W��ݒ�
	if (isFound) {
		//����
		pV0->x = 1.0f * fontNo / (float)charsNum;
		pV0->y = 0.0f;
		//�E��
		pV1->x = 1.0f * (fontNo + 1.0f) / (float)charsNum;
		pV1->y = 0.0f;
		//����
		pV2->x = 1.0f * fontNo / (float)charsNum;
		pV2->y = 1.0f;
		//�E��
		pV3->x = 1.0f * (fontNo + 1.0f) / (float)charsNum;
		pV3->y = 1.0f;
	}
	//������Ȃ��ꍇ�̓e�N�X�`�������Ƃ���
	else {
		//����
		pV0->x = 0.0f;
		pV0->y = 0.0f;
		//�E��
		pV1->x = 0.0f;
		pV1->y = 0.0f;
		//����
		pV2->x = 0.0f;
		pV2->y = 0.0f;
		//�E��
		pV3->x = 0.0f;
		pV3->y = 0.0f;
	}

	return result;
}

//******************************************************************************
// ���_�ʒu�ݒ�
//******************************************************************************
void MTDynamicCaption::_SetVertexPosition(
		MTDYNAMICCAPTION_VERTEX* pVertex,
		float x,
		float y,
		float magRate
	)
{
	unsigned long i = 0;
	unsigned long texHeight = 0;
	unsigned long texWidth = 0;
	unsigned long charsNum = 0;
	float height = 0.0f;
	float width = 0.0f;

	charsNum = _tcslen(m_Chars);

	//�`��T�C�Y
	m_FontTexture.GetTextureSize(&texHeight, &texWidth);
	height = texHeight * magRate;
	width  = ((float)texWidth / (float)charsNum) * magRate;

	//���_���W
	for (i = 0; i < m_CaptionSize; i++) {
		pVertex[i*6+0].p = D3DXVECTOR3(width * (i     ), 0.0f,   0.0f);
		pVertex[i*6+1].p = D3DXVECTOR3(width * (i+1.0f), 0.0f,   0.0f);
		pVertex[i*6+2].p = D3DXVECTOR3(width * (i     ), height, 0.0f);
		pVertex[i*6+3].p = pVertex[i*6+2].p;
		pVertex[i*6+4].p = pVertex[i*6+1].p;
		pVertex[i*6+5].p = D3DXVECTOR3(width * (i+1.0f), height, 0.0f);
	}

	//�`��ʒu�Ɉړ�
	for (i = 0; i < 6*m_CaptionSize; i++) {
		pVertex[i].p.x += x;
		pVertex[i].p.y += y;
	}

	return;
}

//******************************************************************************
// ���_�F�ݒ�
//******************************************************************************
void MTDynamicCaption::_SetVertexColor(
		MTDYNAMICCAPTION_VERTEX* pVertex,
		D3DXCOLOR color
	)
{
	unsigned long i = 0;

	for (i = 0; i < 6*m_CaptionSize; i++) {
		pVertex[i].c = color;
	}

	return;
}

