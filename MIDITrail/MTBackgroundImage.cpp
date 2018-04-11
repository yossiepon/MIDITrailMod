//******************************************************************************
//
// MIDITrail / MTBackgroundImage
//
// �w�i�摜�`��N���X
//
// Copyright (C) 2016 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "shlwapi.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTBackgroundImage.h"


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTBackgroundImage::MTBackgroundImage(void)
{
	m_pTexture = NULL;
	ZeroMemory(&m_ImgInfo, sizeof(D3DXIMAGE_INFO));
	m_hWnd = NULL;
	m_isEnable = true;
	m_isFilterLinear = true;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTBackgroundImage::~MTBackgroundImage(void)
{
	Release();
}

//******************************************************************************
// �w�i�摜����
//******************************************************************************
int MTBackgroundImage::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		HWND hWnd
	)
{
	int result = 0;
	unsigned long vertexNum = 0;
	unsigned long indexNum = 0;
	MTBACKGROUNDIMAGE_VERTEX* pVertex = NULL;
	unsigned long* pIndex = NULL;

	Release();

	m_hWnd = hWnd;

	//�ݒ�t�@�C��������
	result = _InitConfFile();
	if (result != 0) goto EXIT;

	//�e�N�X�`���ǂݍ���
	result = _LoadTexture(pD3DDevice);
	if (result != 0) goto EXIT;

	//�e�N�X�`���𐶐����Ȃ������ꍇ�͉������Ȃ�
	if (m_pTexture == NULL) goto EXIT;

	//�v���~�e�B�u������
	result = m_Primitive.Initialize(
					sizeof(MTBACKGROUNDIMAGE_VERTEX),	//���_�T�C�Y
					_GetFVFFormat(),			//���_FVF�t�H�[�}�b�g
					D3DPT_TRIANGLESTRIP			//�v���~�e�B�u���
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
	result = _CreateVertexOfBackground(
					pVertex,		//���_�o�b�t�@�������݈ʒu
					pIndex			//�C���f�b�N�X�o�b�t�@�������݈ʒu
				);
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
// �`��
//******************************************************************************
int MTBackgroundImage::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	DWORD value = 0;

	if (!m_isEnable) goto EXIT;
	if (m_pTexture == NULL) goto EXIT;

	//Z�o�b�t�@���ꎞ�I�ɖ���������
	pD3DDevice->GetRenderState(D3DRS_ZENABLE, &value);
	pD3DDevice->SetRenderState(D3DRS_ZENABLE, FALSE);

	//�e�N�X�`���X�e�[�W�ݒ�
	//  �J���[���Z�F����1���g�p  ����1�F�e�N�X�`��
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	// �A���t�@���Z�F����1���g�p  ����1�F�e�N�X�`��
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

	//�e�N�X�`���t�B���^
	if (m_isFilterLinear) {
		pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	}
	else {
		//�s�N�Z�����{�ŕ`�悷��ꍇ�{�P�Ȃ��悤�ɂ���
		pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
		pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	}

	//�`��
	result = m_Primitive.Draw(pD3DDevice, m_pTexture);
	if (result != 0) goto EXIT;

	//Z�o�b�t�@�L������Ԃ�߂�
	pD3DDevice->SetRenderState(D3DRS_ZENABLE, value);

EXIT:;
	return result;
}

//******************************************************************************
// ���
//******************************************************************************
void MTBackgroundImage::Release()
{
	m_Primitive.Release();
	
	if (m_pTexture != NULL) {
		m_pTexture->Release();
		m_pTexture = NULL;
	}
}

//******************************************************************************
// �w�i�摜���_����
//******************************************************************************
int MTBackgroundImage::_CreateVertexOfBackground(
		MTBACKGROUNDIMAGE_VERTEX* pVertex,
		unsigned long* pIndex
	)
{
	int result = 0;
	BOOL bresult = 0;
	RECT rect;
	unsigned long i = 0;
	unsigned long cw = 0;
	unsigned long ch = 0;
	float ratio_cwh = 0.0f;
	float ratio_iwh = 0.0f;
	float x1 = 0.0f;
	float x2 = 0.0f;
	float y1 = 0.0f;
	float y2 = 0.0f;

	//�N���C�A���g�̈�̃T�C�Y���擾
	bresult = GetClientRect(m_hWnd, &rect);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	cw = rect.right - rect.left;
	ch = rect.bottom - rect.top;

	ratio_cwh = (float)cw / (float)ch;
	ratio_iwh = (float)m_ImgInfo.Width / (float)m_ImgInfo.Height;

	// �N���C�A���g�̈���摜�̕��������̏ꍇ
	//     |----- cw -----|
	//  ---0--------------+-- +x
	//   | |              |
	//   | +--------------+
	//  ch |    image     |
	//   | +--------------+
	//   | |              |
	//  ---+--------------+
	//     |
	//    +y
	if (ratio_cwh < ratio_iwh) {
		x1 = 0.0f;
		x2 = (float)cw;
		y1 = ((float)ch - ((float)cw / ratio_iwh)) / 2.0f;
		y2 = (float)ch - y1;
		
		//�s�N�Z�����{�ŕ`�悷��ꍇ�̓��j�A�t�B���^���������ă{�P�Ȃ��悤�ɂ���
		if (cw == m_ImgInfo.Width) {
			m_isFilterLinear = false;
		}
	}
	// �N���C�A���g�̈���摜�̕����c���̏ꍇ
	//     |----- cw -----|
	//  ---0--+--------+--+-- +x
	//   | |  |        |  |
	//   | |  |        |  |
	//  ch |  | image  |  |
	//   | |  |        |  |
	//   | |  |        |  |
	//  ---+--+--------+--+
	//     |
	//    +y
	else {
		x1 = ((float)cw - ((float)ch * ratio_iwh)) / 2.0f;
		x2 = (float)cw - x1 - 1.0f;
		y1 = 0.0f;
		y2 = (float)ch - 1.0f;
		
		//�s�N�Z�����{�ŕ`�悷��ꍇ�̓��j�A�t�B���^���������ă{�P�Ȃ��悤�ɂ���
		if (ch == m_ImgInfo.Height) {
			m_isFilterLinear = false;
		}
	}

	//�s�N�Z�����{�ŕ`�悷��ꍇ��z�肵�č��W�𒲐�
	x1 -= 0.5f;
	x2 -= 0.5f;
	y1 -= 0.5f;
	y2 -= 0.5f;

	//���_���W
	pVertex[0].p = D3DXVECTOR3(x1, y1, 0.0f);
	pVertex[1].p = D3DXVECTOR3(x2, y1, 0.0f);
	pVertex[2].p = D3DXVECTOR3(x1, y2, 0.0f);
	pVertex[3].p = D3DXVECTOR3(x2, y2, 0.0f);

	//�e���_�̃f�B�t���[�Y�F
	for (i = 0; i < 4; i++) {
		//�e���_�̏��Z��
		pVertex[i].rhw = 1.0f;
		//�e���_�̃f�B�t���[�Y�F
		pVertex[i].c = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	}

	//�e���_�̃e�N�X�`�����W
	pVertex[0].t = D3DXVECTOR2(0.0f, 0.0f);
	pVertex[1].t = D3DXVECTOR2(1.0f, 0.0f);
	pVertex[2].t = D3DXVECTOR2(0.0f, 1.0f);
	pVertex[3].t = D3DXVECTOR2(1.0f, 1.0f);

	//�C���f�b�N�X�FTRIANGLESTRIP
	pIndex[0] = 0;
	pIndex[1] = 1;
	pIndex[2] = 2;
	pIndex[3] = 3;

EXIT:;
	return result;
}

//******************************************************************************
// �ݒ�t�@�C��������
//******************************************************************************
int MTBackgroundImage::_InitConfFile()
{
	int result = 0;
	TCHAR userConfFilePath[_MAX_PATH] = {_T('\0')};

	result = YNPathUtil::GetAppDataDirPath(userConfFilePath, _MAX_PATH);
	if (result != 0) goto EXIT;

	_tcscat_s(userConfFilePath, _MAX_PATH, MT_USER_CONFFILE_DIR);
	_tcscat_s(userConfFilePath, _MAX_PATH, MT_USER_CONFFILE_GRAPHIC);

	result = m_ConfFile.Initialize(userConfFilePath);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �e�N�X�`���摜�ǂݍ���
//******************************************************************************
int MTBackgroundImage::_LoadTexture(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;
	TCHAR imageFilePath[_MAX_PATH] = {_T('\0')};

	//�r�b�g�}�b�v�t�@�C����
	result = m_ConfFile.SetCurSection(_T("Background-image"));
	if (result != 0) goto EXIT;
	result = m_ConfFile.GetStr(_T("ImageFilePath"), imageFilePath, _MAX_PATH, _T(""));
	if (result != 0) goto EXIT;

	//�t�@�C�����w��Ȃ牽�����Ȃ�
	if (_tcslen(imageFilePath) == 0) goto EXIT;

	//�t�@�C�������݂��Ȃ��ꍇ�͉������Ȃ�
	if (!PathFileExists(imageFilePath)) goto EXIT;

	//�ǂݍ��މ摜�̏c���T�C�Y���擾���Ă���
	hresult = D3DXGetImageInfoFromFile(imageFilePath, &m_ImgInfo);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

	//�e�N�X�`���摜�Ƃ��ēǂݍ���
	//  �s�N�Z�����{�ŕ`�悷��ꍇ�Ƀ{�P�Ȃ��悤�ɂ��邽��
	//  �摜�T�C�Y���w�肵�ēǂݍ���
	hresult = D3DXCreateTextureFromFileEx(
					pD3DDevice,			//�f�o�C�X
					imageFilePath,		//�t�@�C���p�X
					m_ImgInfo.Width,	//���i�s�N�Z���j�F���ڎw��
					m_ImgInfo.Height,	//�����i�s�N�Z���j�F���ڎw��
					1,					//�~�b�v���x��
					0,					//�g�p���@
					D3DFMT_A8R8G8B8,	//�s�N�Z���t�H�[�}�b�g
					D3DPOOL_MANAGED,	//�e�N�X�`���z�u�惁�����N���X
					D3DX_FILTER_NONE,	//�t�B���^�����O�w��
					D3DX_FILTER_NONE,	//�t�B���^�����O�w��i�~�b�v�j
					0xFF000000,			//�����F�̎w��F�s������
					NULL,				//�\�[�X�C���[�W���
					NULL,				//256�F�p���b�g
					&m_pTexture			//�쐬���ꂽ�e�N�X�`���I�u�W�F�N�g
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �\���ݒ�
//******************************************************************************
void MTBackgroundImage::SetEnable(
		bool isEnable
	)
{
	m_isEnable = isEnable;
}


