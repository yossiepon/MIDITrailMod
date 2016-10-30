//******************************************************************************
//
// MIDITrail / MTFontTexture
//
// �t�H���g�e�N�X�`���N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTFontTexture.h"

using namespace YNBaseLib;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTFontTexture::MTFontTexture(void)
{
	m_RGB = 0x00FFFFFF;
	m_TexHeight = 0;
	m_TexWidth = 0;
	m_pTexture = NULL;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTFontTexture::~MTFontTexture(void)
{
	Clear();
}

//******************************************************************************
// �N���A
//******************************************************************************
void MTFontTexture::Clear()
{
	if (m_pTexture) {
		m_pTexture->Release();
		m_pTexture = NULL;
	}
	m_Font2Bmp.Clear();
}

//******************************************************************************
// �t�H���g�ݒ�
//******************************************************************************
int MTFontTexture::SetFont(
		const TCHAR* pFontName,
		unsigned long fontSize,
		unsigned long rgb,
		bool isForceFixedPitch
	)
{
	int result = 0;

	result = m_Font2Bmp.SetFont(pFontName, fontSize, isForceFixedPitch);
	if (result != 0) goto EXIT;

	m_RGB = 0x00FFFFFF & rgb;

EXIT:;
	return result;
}

//******************************************************************************
// �e�N�X�`������
//******************************************************************************
int MTFontTexture::CreateTexture(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pStr
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;
	D3DLOCKED_RECT lockedRect;
	DWORD bmpHeight = 0;
	DWORD bmpWidth = 0;
	DWORD x, y = 0;
	DWORD alpha = 0;
	DWORD argb = 0;
	BYTE bmpPixcel = 0;
	DWORD* pDestPixcel = 0;
	int grayLevelNum = 0;

	if (m_pTexture) {
		m_pTexture->Release();
		m_pTexture = NULL;
	}

	//�t�H���gBMP�쐬
	result = m_Font2Bmp.CreateBmp(pStr);
	if (result != 0) goto EXIT;

	m_Font2Bmp.GetBmpSize(&bmpHeight, &bmpWidth);

	//�e�N�X�`���쐬
	hresult = pD3DDevice->CreateTexture(
							bmpWidth,			//�e�N�X�`���̕��i�s�N�Z���P�ʁj
							bmpHeight,			//�e�N�X�`���̍����i�s�N�Z���P�ʁj
							1,					//�e�N�X�`�����x��
							D3DUSAGE_DYNAMIC,	//�g�p���@�F���_�o�b�t�@�����I���������g�p����
							D3DFMT_A8R8G8B8,	//�t�H�[�}�b�g�F32bit�A���t�@�tARGB�t�H�[�}�b�g
							D3DPOOL_DEFAULT,	//�e�N�X�`���z�u�惁�����N���X�F�f�t�H���g
							&m_pTexture,		//�쐬���ꂽ�e�N�X�`��
							NULL				//�\��
						);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

	//�e�N�X�`�������b�N���ă|�C���^���擾����
	hresult = m_pTexture->LockRect(
						0,					//���\�[�X���x��
						&lockedRect,		//���b�N�ςݗ̈�
						NULL,				//���b�N�����`�F�e�N�X�`���S��
						D3DLOCK_DISCARD		//���b�N�̎�ށF�������ݐ�p
					);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

	//���b�N���ꂽ�̈���[���N���A
	ZeroMemory(lockedRect.pBits, lockedRect.Pitch * bmpHeight);

	//�O���C���x�����FGGO_GRAY4_BITMAP�Ȃ̂�17�i�K
	grayLevelNum = 17;

	//�e�N�X�`����Ƀr�b�g�}�b�v����������
	for (y = 0; y < bmpHeight; y++) {
		for (x = 0; x < bmpWidth; x++) {

			//�r�b�g�}�b�v����f�F8bit �� 0x00�`0x10(=16)���Ԃ� �� �A���t�@�l�Ƀ}�b�s���O
			bmpPixcel = m_Font2Bmp.GetBmpPixcel(x, y);

			//�e�N�X�`������f�F32bit ARGB
			alpha = (0xFF * bmpPixcel) / (grayLevelNum - 1);
			if (bmpPixcel == 0) {
				argb = (alpha << 24) | 0x00000000;
			}
			else {
				argb = (alpha << 24) | m_RGB;
			}

			//��������
			pDestPixcel = (DWORD*)((BYTE*)lockedRect.pBits + (lockedRect.Pitch * y) + (4 * x));
			*pDestPixcel = argb;
		}
	}
	
	//�A�����b�N
	hresult = m_pTexture->UnlockRect(0);  //���\�[�X���x��0
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

	m_TexHeight = bmpHeight;
	m_TexWidth = bmpWidth;

EXIT:;
	m_Font2Bmp.Clear();
	return result;
}

//******************************************************************************
// �e�N�X�`���|�C���^�擾
//******************************************************************************
LPDIRECT3DTEXTURE9 MTFontTexture::GetTexture()
{
	return m_pTexture;
}

//******************************************************************************
// �e�N�X�`���T�C�Y�擾
//******************************************************************************
void MTFontTexture::GetTextureSize(
		unsigned long* pHeight,
		unsigned long* pWidth
	)
{
	*pHeight = m_TexHeight;
	*pWidth = m_TexWidth;
}

