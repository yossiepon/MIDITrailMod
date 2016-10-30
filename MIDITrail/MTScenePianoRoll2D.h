//******************************************************************************
//
// MIDITrail / MTScenePianoRoll2D
//
// �s�A�m���[��2D�V�[���`��N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once
#include "mtscenepianoroll3d.h"


//******************************************************************************
// �s�A�m���[��2D�V�[���`��N���X
//******************************************************************************
class MTScenePianoRoll2D : public MTScenePianoRoll3D
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTScenePianoRoll2D(void);
	virtual ~MTScenePianoRoll2D(void);

	//���̎擾
	const TCHAR* GetName();

	//����
	virtual int Create(
			HWND hWnd,
			LPDIRECT3DDEVICE9 pD3DDevice,
			SMSeqData* pSeqData
		);

};

