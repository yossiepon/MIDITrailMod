//******************************************************************************
//
// MIDITrail / DXScene
//
// �V�[�����N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// DXRenderer�ɑΉ����钊�ۃN���X�B

#pragma once

#include <d3d9.h>
#include <d3dx9.h>


//******************************************************************************
// �V�[�����N���X
//******************************************************************************
class DXScene
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	DXScene(void);
	virtual ~DXScene(void);

	//�`��
	virtual int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

};

