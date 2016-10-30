//******************************************************************************
//
// Simple MIDI Library / SMPortList
//
// �|�[�g���X�g�N���X
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
// �|�[�g���X�g�N���X
//******************************************************************************
class SMIDILIB_API SMPortList
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	SMPortList(void);
	virtual ~SMPortList(void);

	//�N���A
	void Clear();

	//�|�[�g�o�^
	int AddPort(unsigned char portNo);

	//�|�[�g�擾
	int GetPort(unsigned long index, unsigned char* pPortNo);

	//�|�[�g���擾
	unsigned long GetSize();

	//�R�s�[
	int CopyFrom(SMPortList* pSrcList);

private:

	SMSimpleList m_List;

	//����ƃR�s�[�R���X�g���N�^�̋֎~
	void operator=(const SMPortList&);
	SMPortList(const SMPortList&);

};

} // end of namespace

