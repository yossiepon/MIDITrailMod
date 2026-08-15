//******************************************************************************
//
// Simple MIDI Library / SMFileReader
//
// Standard MIDI file reader class.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
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
// Standard MIDI file reader class
//******************************************************************************
class SMIDILIB_API SMFileReader
{
public:

	//Progress callback
	typedef void (*LoadProgressFunc)(unsigned long current, unsigned long total, void* userData);

	//Constructor / Destructor
	SMFileReader(void);
	~SMFileReader(void);

	//Register log output file path
	int SetLogPath(const WCHAR* pLogPath);

	//Load standard MIDI file
	int Load(const WCHAR* pSMFPath, SMSeqData* pMIDIData);

	//Set progress callback (static, process-wide)
	static void SetLoadProgressCallback(LoadProgressFunc func, void* userData);

private:

	//Chunk header structure

	#pragma pack(push,1)

	//SMF chunk type
	typedef struct {
		unsigned char chunkType[4];		//Chunk type MThd/MTrk
		unsigned long chunkSize;		//Chunk size
	} SMFChunkTypeSection;

	//SMF chunk data
	typedef struct {
		unsigned short format;			//Format 0,1,2
		unsigned short ntracks;			//Track count
		unsigned short timeDivision;	//Resolution per quarter note
	} SMFChunkDataSection;

	//RIFF chunk
	typedef struct {
		unsigned char chunkID[4];		//Chunk ID
		unsigned long chunkSize;		//Chunk size
		unsigned char format[4];		//Format
	} SMFRIFFChunkHeader;

	//RIFF sub chunk
	typedef struct {
		unsigned char chunkID[4];		//Chunk ID
		unsigned long chunkSize;		//Chunk size
	} SMFRIFFSubChunkHeader;

	#pragma pack(pop)

private:

	unsigned char m_PrevStatus;

	WCHAR m_LogPath[MAX_PATH];
	FILE* m_pLogFile;
	bool m_IsLogOut;

	static LoadProgressFunc s_ProgressFunc;
	static void* s_ProgressUserData;

	int _SkipRIFFHeader(
			HMMIO hFile
		);
	
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
			SMTrack** pPtrTrack,
			unsigned long trackIndex,
			unsigned long trackCount
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
	int _WriteLog(const char* pText);
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

	//Prohibit assignment and copy constructor
	void operator=(const SMFileReader&);
	SMFileReader(const SMFileReader&);

};

} // end of namespace

