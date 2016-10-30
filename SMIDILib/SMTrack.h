//******************************************************************************
//
// Simple MIDI Library / SMTrack
//
// �g���b�N�N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// SysEX�C�x���g�ƃ��^�C�x���g�͉ϒ��T�C�Y�̂��߁A�P�����X�g�N���X��
// ���̂܂ܗ��p�ł��Ȃ��B�����������̃C�x���g�́A�K��4byte�Ɏ��܂�MIDI
// �C�x���g�ɔ�ׂĈ��|�I�ɏ��Ȃ��̂ŁA�X��new����邱�Ƃ�e�F���A
// map�ŊǗ�����B
//
// TODO:
// SMEvent�N���X�Ƀf���^�^�C���ƃ|�[�g�ԍ�����������ׂ��B
// �C�x���g�^�f���^�^�C���^�|�[�g�ԍ��𕪗����Ă��邽�߁A
// SMTrack�N���X���p�҂̏������ώG�ɂȂ��Ă���B

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

#include "SMSimpleList.h"
#include "SMEvent.h"
#include "SMNoteList.h"
#include <map>

#pragma warning(disable:4251)

namespace SMIDILib {


//******************************************************************************
// �g���b�N�N���X
//******************************************************************************
class SMIDILIB_API SMTrack
{

public:

	//�R���X�g���N�^�^�f�X�g���N�^
	SMTrack(void);
	virtual ~SMTrack(void);

	//�N���A
	void Clear();

	//�f�[�^�Z�b�g�o�^
	int AddDataSet(unsigned long deltaTime, SMEvent* pEvent, unsigned char portNo);

	//�f�[�^�Z�b�g�擾
	int GetDataSet(unsigned long index, unsigned long* pDeltaTime, SMEvent* pEvent, unsigned char* pPortNo);

	//�f�[�^�Z�b�g���擾
	unsigned long GetSize();

	//�m�[�g���X�g�擾�FstartTime, endTime �̓`�b�N�^�C��
	int GetNoteList(SMNoteList* pNoteList);

	//�m�[�g���X�g�擾�FstartTime, endTime �̓��A���^�C��(msec)
	int GetNoteListWithRealTime(SMNoteList* pNoteList, unsigned long timeDivision);

	//�R�s�[
	int CopyFrom(SMTrack* pSrcTrack);

// >>> add 20120728 yossiepon begin

	//�|�[�g�ԍ��㏑��
	int OverwritePortNo(short portNo);

	//�`�����l���ԍ��㏑��
	int OverwriteChNo(short chNo);

// <<< add 20120728 yossiepon end

private:

	//�C�x���g�f�[�^
	typedef struct {
		SMEvent::EventType type;
		unsigned char status;
		unsigned char meta;
		unsigned long size;
		unsigned char data[4];
	} SMEventData;

	//�f�[�^�Z�b�g
	typedef struct {
		unsigned long deltaTime;
		SMEventData eventData;
		unsigned char portNo;
	} SMDataSet;

	//�g���f�[�^�}�b�v�F�C���f�b�N�X���f�[�^�ʒu
	typedef std::map<unsigned long, unsigned char*> SMExDataMap;
	typedef std::pair<unsigned long, unsigned char*> SMExDataMapPair;

	//�m�[�g���}�b�v�F�m�[�g����L�[���m�[�g���X�g�C���f�b�N�X
	typedef std::map<unsigned long, unsigned long> SMNoteMap;
	typedef std::pair<unsigned long, unsigned long> SMNoteMapPair;

private:

	SMSimpleList m_List;
	SMExDataMap m_ExDataMap;

// >>> add 20120728 yossiepon begin

	short m_OverwritePortNo;

// <<< add 20120728 yossiepon end

	unsigned long _GetNoteKey(unsigned char portNo, unsigned char chNo, unsigned char noteNo);
	int _GetNoteList(SMNoteList* pNoteList, unsigned long timeDivision);
	double _ConvTick2TimeMsec(unsigned long tickTime, unsigned long tempo, unsigned long timeDivision);

	//����ƃR�s�[�R���X�g���N�^�̋֎~
	void operator=(const SMTrack&);
	SMTrack(const SMTrack&);

};

} // end of namespace

#pragma warning(default:4251)

