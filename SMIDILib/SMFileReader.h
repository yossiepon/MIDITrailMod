//******************************************************************************
//
// Simple MIDI Library / SMFileReader
//
// �W��MIDI�t�@�C���ǂݍ��݃N���X
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

#include "mmsystem.h"
#include "SMEvent.h"
#include "SMEventMIDI.h"
#include "SMEventSysEx.h"
#include "SMEventMeta.h"
#include "SMTrack.h"
#include "SMSeqData.h"
#include <stdio.h>


namespace SMIDILib {

//******************************************************************************
// �W��MIDI�t�@�C���ǂݍ��݃N���X
//******************************************************************************
class SMIDILIB_API SMFileReader
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	SMFileReader(void);
	~SMFileReader(void);

	//���O�o�͐�t�@�C���p�X�o�^
	int SetLogPath(const TCHAR* pLogPath);

	//�W��MIDI�t�@�C���ǂݍ���
	int Load(const TCHAR* pSMFPath, SMSeqData* pMIDIData);

private:

	//�`�����N�w�b�_�\��

	#pragma pack(push,1)

	typedef struct {
		unsigned char chunkType[4];		//�`�����N�^�C�v MThd/MTrk
		unsigned long chunkSize;		//�`�����N�T�C�Y
	} SMFChunkTypeSection;

	typedef struct {
		unsigned short format;			//�t�H�[�}�b�g 0,1,2
		unsigned short ntracks;			//�g���b�N��
		unsigned short timeDivision;	//4������������̕���\
	} SMFChunkDataSection;

	#pragma pack(pop)

private:

	unsigned char m_PrevStatus;

	TCHAR m_LogPath[MAX_PATH];
	FILE* m_pLogFile;
	bool m_IsLogOut;

	int _ReadChunkHeader(
			HMMIO hFile,
			SMFChunkTypeSection* pChunkTypeSection,
			SMFChunkDataSection* pChunkDataSection
		);

	int _ReadTrackHeader(
			HMMIO hFile,
			unsigned long trackNo,
			SMFChunkTypeSection* pChunkTypeSection
		);

	int _ReadTrackEvents(
			HMMIO hFile,
			unsigned long chunkSize,
			SMTrack** pPtrTrack
		);

	int _ReadDeltaTime(
			HMMIO hFile,
			unsigned long* pDeltaTime,
			unsigned long* pOffset
		);

	int _ReadVariableDataSize(
			HMMIO hFile,
			unsigned long* pVariableDataSize,
			unsigned long* pOffset
		);

	int _ReadEvent(
			HMMIO hFile,
			SMEvent* pEvent,
			bool* pIsEndOfTrack,
			unsigned long* pOffset
		);

	int _ReadEventMIDI(
			HMMIO hFile,
			unsigned char status,
			SMEvent* pEvent,
			unsigned long* pOffset
		);

	int _ReadEventSysEx(
			HMMIO hFile,
			unsigned char status,
			SMEvent* pEvent,
			unsigned long* pOffset
		);

	int _ReadEventMeta(
			HMMIO hFile,
			unsigned char status,
			SMEvent* pEvent,
			bool* pIsEndOfTrack,
			unsigned long* pOffset
		);

	void _ReverseEndian(
			void* pData,
			unsigned long size
		);

	int _OpenLogFile();
	int _CloseLogFile();
	int _WriteLog(char* pText);
	int _WriteLogChunkHeader(
				SMFChunkTypeSection* pChunkTypeSection,
				SMFChunkDataSection* pChunkDataSection
			);
	int _WriteLogTrackHeader(
				unsigned long trackNo,
				SMFChunkTypeSection* pChunkTypeSection
			);
	int _WriteLogDeltaTime(
				unsigned long deltaTime
			);
	int _WriteLogEventMIDI(
				unsigned char status,
				unsigned char* pData,
				unsigned long size
			);
	int _WriteLogEventSysEx(
				unsigned char status,
				unsigned char* pData,
				unsigned long size
			);
	int _WriteLogEventMeta(
				unsigned char status,
				unsigned char type,
				unsigned char* pData,
				unsigned long size
			);

	//����ƃR�s�[�R���X�g���N�^�̋֎~
	void operator=(const SMFileReader&);
	SMFileReader(const SMFileReader&);

};

} // end of namespace

