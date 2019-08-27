//******************************************************************************
//
// MIDITrail / MTPianoKeyboardMod
//
// �s�A�m�L�[�{�[�h�`��Mod�N���X
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTPianoKeyboard.h"
#include "MTPianoKeyboardDesignMod.h"


//******************************************************************************
// �p�����[�^��`
//******************************************************************************
#define MTPIANOKEYBOARDMOD_INDEX_BUFFER_MAX (12)

//******************************************************************************
// �s�A�m�L�[�{�[�h�`��Mod�N���X
//******************************************************************************
class MTPianoKeyboardMod : public MTPianoKeyboard
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTPianoKeyboardMod(void);
	virtual ~MTPianoKeyboardMod(void);

	//����
	virtual int Create(
			LPDIRECT3DDEVICE9 pD3DDevice,
			const TCHAR* pSceneName,
			SMSeqData* pSeqData,
			LPDIRECT3DTEXTURE9 pTexture = NULL
		);

	//�`��
	virtual int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//�X�V
	int Transform(
			LPDIRECT3DDEVICE9 pD3DDevice,
			D3DXVECTOR3 basePosVector,
			D3DXVECTOR3 playbackPosVector,
			D3DXVECTOR3 camVector,
			D3DXVECTOR3 lookVector,
			float rollAngle
		);

	//�L�[��ԕύX
	virtual int PushKey(
			unsigned char chNo,
			unsigned char noteNo,
			float keyDownRate,
			unsigned long elapsedTime,
			D3DXCOLOR* pActiveKeyColor = NULL
		);

private:

	//�`����̍쐬
	int _MakeRenderingInfo(
			D3DXVECTOR3 basePosVector,
			D3DXVECTOR3 playbackPosVector,
			D3DXVECTOR3 camVector,
			D3DXVECTOR3 lookVector,
			float rollAngle
		);

	//�`��C���f�b�N�X�̐���
	int _CreateRenderingIndex(LPDIRECT3DDEVICE9 pD3DDevice);

	//�L�[�P�ʂ̕`��C���f�b�N�X�̐���
	int _CreateRenderingIndexOfKey(
			unsigned char noteNo,
			int bufferIdx,
			unsigned long* pIndex,
			unsigned long* pRenderingIndex
		);

	//�`��C���f�b�N�X�o�b�t�@�̐���
	int _CreateRenderingIndexBuffer(
				LPDIRECT3DDEVICE9 pD3DDevice,
				int bufferIdx,
				unsigned long indexNum
			);

	//�`��C���f�b�N�X�o�b�t�@�̃��b�N����
	int _LockRenderingIndex(
				unsigned long** pPtrIndex,
				int bufferIdx,
				unsigned long offset = 0,
				unsigned long size = 0
			);
	int _UnlockRenderingIndex(
				int bufferIdx
			);

	virtual int _CreateVertexOfKeyWhite1(
				unsigned char noteNo,
				MTPIANOKEYBOARD_VERTEX* pVertex,
				unsigned long* pIndex,
				D3DXCOLOR* pColor = NULL
			);
	virtual int _CreateVertexOfKeyWhite2(
				unsigned char noteNo,
				MTPIANOKEYBOARD_VERTEX* pVertex,
				unsigned long* pIndex,
				D3DXCOLOR* pColor = NULL
			);
	virtual int _CreateVertexOfKeyWhite3(
				unsigned char noteNo,
				MTPIANOKEYBOARD_VERTEX* pVertex,
				unsigned long* pIndex,
				D3DXCOLOR* pColor = NULL
			);
	virtual int _CreateVertexOfKeyBlack(
				unsigned char noteNo,
				MTPIANOKEYBOARD_VERTEX* pVertex,
				unsigned long* pIndex,
				D3DXCOLOR* pColor = NULL
			);

private:

	//�L�[�{�[�h�f�U�C��
	MTPianoKeyboardDesignMod m_KeyboardDesignMod;

	//�`��C���f�b�N�X���
	LPDIRECT3DINDEXBUFFER9 m_pRenderingIndexBuffer[ MTPIANOKEYBOARDMOD_INDEX_BUFFER_MAX ];
	unsigned long m_RenderingIndexNum[ MTPIANOKEYBOARDMOD_INDEX_BUFFER_MAX ];
	bool m_IsRenderingIndexLocked[ MTPIANOKEYBOARDMOD_INDEX_BUFFER_MAX ];

	//�L�[�{�[�h�`����
	int m_noteNoLow;
	int m_noteNoHigh;
	int m_camDirLR;
	int m_camPosIdx;
};


