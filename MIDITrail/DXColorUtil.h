//******************************************************************************
//
// MIDITrail / DXColorUtil
//
// �J���[���[�e�B���e�B�N���X
//
// Copyright (C) 2010-2013 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <d3d9.h>
#include <d3dx9.h>


//******************************************************************************
// �J���[���[�e�B���e�B�N���X
//******************************************************************************
class DXColorUtil
{
public:

	//RGBA�i16�i��������j����̐��l�ϊ�
	static D3DXCOLOR MakeColorFromHexRGBA(const TCHAR* pHexRGBA);

	//RGB�i16�i��������j����̐��l�ϊ�
	static D3DCOLOR MakeColorFromHexRGB(const TCHAR* pHexRGB);

private:

	//�R���X�g���N�^�^�f�X�g���N�^
	DXColorUtil(void);
	virtual ~DXColorUtil(void);

	//����ƃR�s�[�R���X�g���N�^�̋֎~
	void operator=(const DXColorUtil&);
	DXColorUtil(const DXColorUtil&);

};

