//******************************************************************************
//
// Simple MIDI Library / SMBarList
//
// ���߃��X�g�N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

#include "SMSimpleList.h"

namespace SMIDILib {

//******************************************************************************
// ���߃��X�g�N���X
//******************************************************************************
class SMIDILIB_API SMBarList
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	SMBarList(void);
	virtual ~SMBarList(void);

	//�N���A
	void Clear();

	//���ߒǉ�
	int AddBar(unsigned long tickTime);

	//���ߎ擾
	int GetBar(unsigned long index, unsigned long* pTickTime);

	//���ߐ��擾
	unsigned long GetSize();

	//�R�s�[
	int CopyFrom(SMBarList* pSrcList);

private:

	SMSimpleList m_List;

	//����ƃR�s�[�R���X�g���N�^�̋֎~
	void operator=(const SMBarList&);
	SMBarList(const SMBarList&);

};

} // end of namespace

