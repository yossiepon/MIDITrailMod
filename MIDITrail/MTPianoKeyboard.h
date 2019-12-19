//******************************************************************************
//
// MIDITrail / MTPianoKeyboard
//
// �s�A�m�L�[�{�[�h�`��N���X
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// �s�A�m�L�[�{�[�h(1ch��)�̕`��𐧌䂷��N���X�B

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "DXPrimitive.h"
#include "SMIDILib.h"
#include "MTPianoKeyboardDesign.h"
#include "MTNotePitchBend.h"

using namespace SMIDILib;


//******************************************************************************
// �s�A�m�L�[�{�[�h�`��N���X
//******************************************************************************
class MTPianoKeyboard
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTPianoKeyboard(void);
	virtual ~MTPianoKeyboard(void);

	//����
	int Create(
			LPDIRECT3DDEVICE9 pD3DDevice,
			const TCHAR* pSceneName,
			SMSeqData* pSeqData,
			LPDIRECT3DTEXTURE9 pTexture = NULL
		);

	//�X�V
	int Transform(LPDIRECT3DDEVICE9 pD3DDevice, D3DXVECTOR3 moveVector, float rollAngle);

	//�`��
	int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//���
	void Release();

	//�L�[��ԕύX
	int ResetKey(unsigned char noteNo);
	int PushKey(
			unsigned char noteNo,
			float keyDownRate,
			unsigned long elapsedTime,
			D3DXCOLOR* pActiveKeyColor = NULL
		);

	//���L�p�e�N�X�`���擾
	LPDIRECT3DTEXTURE9 GetTexture();

private:

	//���_�o�b�t�@�\����
	struct MTPIANOKEYBOARD_VERTEX {
		D3DXVECTOR3 p;	//���_���W
		D3DXVECTOR3 n;	//�@��
		DWORD		c;	//�f�B�t���[�Y�F
		D3DXVECTOR2 t;	//�e�N�X�`���摜�ʒu
	};

	//�o�b�t�@���
	typedef struct {
		unsigned long vertexPos;
		unsigned long vertexNum;
		unsigned long indexPos;
		unsigned long indexNum;
	} MTBufInfo;

private:

	//�L�[�{�[�h�v���~�e�B�u
	DXPrimitive m_PrimitiveKeyboard;

	//�e�N�X�`��
	LPDIRECT3DTEXTURE9 m_pTexture;
	D3DXIMAGE_INFO m_ImgInfo;

	//�L�[�{�[�h�f�U�C��
	MTPianoKeyboardDesign m_KeyboardDesign;

	//�o�b�t�@���
	MTBufInfo m_BufInfo[SM_MAX_NOTE_NUM];

	//���_�o�b�t�@FVF�t�H�[�}�b�g
	DWORD _GetFVFFormat(){ return (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1); }

	int _CreateKeyboard(LPDIRECT3DDEVICE9 pD3DDevice);
	void _CreateBufInfo();
	int _CreateVertexOfKeyboard(LPDIRECT3DDEVICE9 pD3DDevice);
	int _CreateVertexOfKey(unsigned char noteNo);
	int _CreateVertexOfKeyWhite1(
				unsigned char noteNo,
				MTPIANOKEYBOARD_VERTEX* pVertex,
				unsigned long* pIndex,
				D3DXCOLOR* pColor = NULL
			);
	int _CreateVertexOfKeyWhite2(
				unsigned char noteNo,
				MTPIANOKEYBOARD_VERTEX* pVertex,
				unsigned long* pIndex,
				D3DXCOLOR* pColor = NULL
			);
	int _CreateVertexOfKeyWhite3(
				unsigned char noteNo,
				MTPIANOKEYBOARD_VERTEX* pVertex,
				unsigned long* pIndex,
				D3DXCOLOR* pColor = NULL
			);
	int _CreateVertexOfKeyBlack(
				unsigned char noteNo,
				MTPIANOKEYBOARD_VERTEX* pVertex,
				unsigned long* pIndex,
				D3DXCOLOR* pColor = NULL
			);
	int _LoadTexture(LPDIRECT3DDEVICE9 pD3DDevice, const TCHAR* pSceneName);
	void _MakeMaterial(D3DMATERIAL9* pMaterial);

	int _RotateKey(unsigned char noteNo, float angle, D3DXCOLOR* pColor = NULL);

	int _HideKey(unsigned char noteNo);

};


