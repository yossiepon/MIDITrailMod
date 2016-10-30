//******************************************************************************
//
// MIDITrail / MTLogo
//
// MIDITrail ���S�`��N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// �|���S�����^�C����ɕ��ׂă��S�̃e�N�X�`����\��t����B
// �|���S���̐F���^�C�����ƂɍX�V���邱�ƂŃ��S���O���f�[�V����������B
//
//   +-++-++-++-+
//   |/||/||/||/|...
//   +-++-++-++-+

#include "StdAfx.h"
#include <mmsystem.h>
#include "YNBaseLib.h"
#include "MTLogo.h"

using namespace YNBaseLib;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTLogo::MTLogo(void)
{
	m_pVertex = NULL;
	m_StartTime = 0;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTLogo::~MTLogo(void)
{
	Release();
}

//******************************************************************************
// �V�[������
//******************************************************************************
int MTLogo::Create(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;

	Release();

	//�e�N�X�`������
	result = _CreateTexture(pD3DDevice);
	if (result != 0) goto EXIT;

	//���_����
	result = _CreateVertex();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �ϊ�����
//******************************************************************************
int MTLogo::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;

	//�^�C�g���O���f�[�V�����ݒ�
	_SetGradationColor();

	return result;
}

//******************************************************************************
// �`��
//******************************************************************************
int MTLogo::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;

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

	//�^�C�g�������`��
	hresult = pD3DDevice->DrawPrimitiveUP(
					D3DPT_TRIANGLELIST,		//�v���~�e�B�u���
					2 * MTLOGO_TILE_NUM,		//�v���~�e�B�u��
					m_pVertex,				//���_�f�[�^
					sizeof(MTLOGO_VERTEX)	//���_�f�[�^�̃T�C�Y
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, MTLOGO_TILE_NUM);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �j��
//******************************************************************************
void MTLogo::Release()
{
	m_FontTexture.Clear();

	delete [] m_pVertex;
	m_pVertex = NULL;
}

//******************************************************************************
// �e�N�X�`������
//******************************************************************************
int MTLogo::_CreateTexture(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	unsigned long color = 0x00FFFFFF;
	bool isForceFixedPitch = false;

	//�t�H���g�ݒ�
	result = m_FontTexture.SetFont(
					_T("Arial"),		//�t�H���g����
					40,					//�t�H���g�T�C�Y
					color,				//�F
					isForceFixedPitch	//�Œ�s�b�`����
				);
	if (result != 0) goto EXIT;

	//�^�C�������ꗗ�e�N�X�`���쐬
	result = m_FontTexture.CreateTexture(pD3DDevice, MTLOGO_TITLE);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �t�H���g�^�C�����_����
//******************************************************************************
int MTLogo::_CreateVertex()
{
	int result = 0;
	unsigned long i = 0;
	MTLOGO_VERTEX* pVertex = NULL;

	//���_����
	try {
		pVertex = new MTLOGO_VERTEX[6*MTLOGO_TILE_NUM];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

	//���_���W�ݒ�
	_SetVertexPosition(
			pVertex,		//���_���W�z��
			MTLOGO_POS_X,	//�`��ʒux
			MTLOGO_POS_Y,	//�`��ʒuy
			MTLOGO_MAG		//�g�嗦
		);

	for (i = 0; i < 6*MTLOGO_TILE_NUM; i++) {
		//�e���_�̃f�B�t���[�Y�F
		pVertex[i].c = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f); //R,G,B,A
	}

	m_pVertex = pVertex;
	pVertex = NULL;

EXIT:;
	delete [] pVertex;
	return result;
}

//******************************************************************************
// ���_�ʒu�ݒ�
//******************************************************************************
void MTLogo::_SetVertexPosition(
		MTLOGO_VERTEX* pVertex,
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
	float tileNo = 0.0f;

	//�^�C���T�C�Y
	m_FontTexture.GetTextureSize(&texHeight, &texWidth);
	height = texHeight * magRate;
	width  = ((float)texWidth / (float)MTLOGO_TILE_NUM) * magRate;

	//���_���W�FXY���ʂ�(0, 0)������Ƃ���
	for (i = 0; i < MTLOGO_TILE_NUM; i++) {
		//���_���W
		pVertex[i*6+0].p = D3DXVECTOR3(width * (i     ),  0.0f,   0.0f);
		pVertex[i*6+1].p = D3DXVECTOR3(width * (i+1.0f),  0.0f,   0.0f);
		pVertex[i*6+2].p = D3DXVECTOR3(width * (i     ), -height, 0.0f);
		pVertex[i*6+3].p = pVertex[i*6+2].p;
		pVertex[i*6+4].p = pVertex[i*6+1].p;
		pVertex[i*6+5].p = D3DXVECTOR3(width * (i+1.0f), -height, 0.0f);

		//�e�N�X�`�����W
		//����
		pVertex[i*6+0].t.x = 1.0f * tileNo / (float)MTLOGO_TILE_NUM;
		pVertex[i*6+0].t.y = 0.0f;
		//�E��
		pVertex[i*6+1].t.x = 1.0f * (tileNo + 1.0f) / (float)MTLOGO_TILE_NUM;
		pVertex[i*6+1].t.y = 0.0f;
		//����
		pVertex[i*6+2].t.x = 1.0f * tileNo / (float)MTLOGO_TILE_NUM;
		pVertex[i*6+2].t.y = 1.0f;
		//����
		pVertex[i*6+3].t = pVertex[i*6+2].t;
		//�E��
		pVertex[i*6+4].t = pVertex[i*6+1].t;
		//�E��
		pVertex[i*6+5].t.x = 1.0f * (tileNo + 1.0f) / (float)MTLOGO_TILE_NUM;
		pVertex[i*6+5].t.y = 1.0f;

		tileNo += 1.0f;
	}

	//�@��
	for (i = 0; i < 6*MTLOGO_TILE_NUM; i++) {
		pVertex[i].n = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	}

	//�w��ʒu�Ɉړ�
	for (i = 0; i < 6*MTLOGO_TILE_NUM; i++) {
		pVertex[i].p.x += x;
		pVertex[i].p.y += y;
	}

	return;
}

//******************************************************************************
// �O���f�[�V�����ݒ�
//******************************************************************************
void MTLogo::_SetGradationColor()
{
	unsigned long i = 0;
	unsigned long sceneTime = 0;
	unsigned long delay = 0;
	unsigned long tileTime = 0;
	float color = 0.0f;
	MTLOGO_VERTEX* pVertex = NULL;

	//�V�[���o�ߎ���
	if (m_StartTime == 0) {
		m_StartTime = timeGetTime();
	}
	sceneTime = timeGetTime() - m_StartTime;

	//�O���f�[�V��������
	for (i = 0; i < MTLOGO_TILE_NUM; i++) {

		//�^�C�����Ƃ̒x������
		delay = i * (MTLOGO_GRADATION_TIME / MTLOGO_TILE_NUM);

		//�^�C���ɂƂ��Ă̌o�ߎ���
		if (sceneTime < delay) {
			tileTime = 0;
		}
		else {
			tileTime = sceneTime - delay;
		}

		//�^�C���̐F
		if (tileTime < MTLOGO_GRADATION_TIME) {
			//������
			color = (float)tileTime / (float)MTLOGO_GRADATION_TIME;
		}
		else if (tileTime < (MTLOGO_GRADATION_TIME*2)) {
			//������
			color = 1.0f - ((float)(tileTime - MTLOGO_GRADATION_TIME) / (float)MTLOGO_GRADATION_TIME);
		}
		else {
			//��
			color = 0.0f;
		}

		//�^�C���̐F�𒸓_�ɐݒ�
		pVertex = m_pVertex + (6*i);
		_SetTileColor(pVertex, color);
	}

	return;
}

//******************************************************************************
// �^�C���F�ݒ�
//******************************************************************************
void MTLogo::_SetTileColor(
		MTLOGO_VERTEX* pVertex,
		float color
	)
{
	unsigned long i = 0;

	for (i = 0; i < 6; i++) {
		pVertex[i].c = D3DXCOLOR(color, color, color, 1.0f); //R,G,B,A
	}
}

