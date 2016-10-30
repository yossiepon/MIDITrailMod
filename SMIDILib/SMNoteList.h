//******************************************************************************
//
// Simple MIDI Library / SMNoteList
//
// �m�[�g���X�g�N���X
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
// �m�[�g���\����
//******************************************************************************
//�m�[�g���
typedef struct {
	unsigned char portNo;
	unsigned char chNo;
	unsigned char noteNo;
	unsigned char velocity;
	unsigned long startTime;
	unsigned long endTime;
} SMNote;

//******************************************************************************
// �m�[�g���X�g�N���X
//******************************************************************************
class SMIDILIB_API SMNoteList
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	SMNoteList(void);
	virtual ~SMNoteList(void);

	//�N���A
	void Clear();

	//�m�[�g���ǉ�
	int AddNote(SMNote note);

	//�m�[�g���擾
	int GetNote(unsigned long index, SMNote* pNote);

	//�m�[�g���o�^�i�㏑���j
	int SetNote(unsigned long index, SMNote* pNote);

	//�m�[�g���擾
	unsigned long GetSize();

	//�R�s�[
	int CopyFrom(SMNoteList* pSrcList);

private:

	SMSimpleList m_List;

	//����ƃR�s�[�R���X�g���N�^�̋֎~
	void operator=(const SMNoteList&);
	SMNoteList(const SMNoteList&);

};

} // end of namespace

