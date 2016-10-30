//******************************************************************************
//
// MIDITrail / DXScene
//
// �V�[�����N���X
//
// Copyright (C) 2010-2013 WADA Masashi. All Rights Reserved.
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

	//�w�i�F�ݒ�
	virtual void SetBGColor(D3DCOLOR color);

	//�w�i�F�擾
	virtual D3DCOLOR GetBGColor();

	//�`��
	virtual int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

private:

	//�w�i�F
	D3DCOLOR m_BGColor;

};

