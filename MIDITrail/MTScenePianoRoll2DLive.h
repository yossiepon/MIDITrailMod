//******************************************************************************
//
// MIDITrail / MTScenePianoRoll2DLive
//
// ���C�u���j�^�p�s�A�m���[��2D�V�[���`��N���X
//
// Copyright (C) 2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once
#include "MTScenePianoRoll3DLive.h"


//******************************************************************************
// �s�A�m���[��2D�V�[���`��N���X
//******************************************************************************
class MTScenePianoRoll2DLive : public MTScenePianoRoll3DLive
{
public:
	
	//�R���X�g���N�^�^�f�X�g���N�^
	MTScenePianoRoll2DLive(void);
	virtual ~MTScenePianoRoll2DLive(void);
	
	//���̎擾
	const TCHAR* GetName();
	
	//����
	virtual int Create(
				HWND hWnd,
				LPDIRECT3DDEVICE9 pD3DDevice,
				SMSeqData* pSeqData
			);
	
};


