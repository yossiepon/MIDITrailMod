//******************************************************************************
//
// MIDITrail / MTConfFile
//
// �ݒ�t�@�C���N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "YNBaseLib.h"

using namespace YNBaseLib;


//******************************************************************************
// �ݒ�t�@�C���N���X
//******************************************************************************
class MTConfFile : public YNConfFile
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTConfFile(void);
	virtual ~MTConfFile(void);

	//������
	int Initialize(const TCHAR* pCategory);

};


