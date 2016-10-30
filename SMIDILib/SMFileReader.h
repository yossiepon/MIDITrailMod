//******************************************************************************
//
// Simple MIDI Library / SMFileReader
//
// 標準MIDIファイル読み込みクラス
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
// 標準MIDIファイル読み込みクラス
//******************************************************************************
class SMIDILIB_API SMFileReader
{
public:

	//コンストラクタ／デストラクタ
	SMFileReader(void);
	~SMFileReader(void);

	//ログ出力先ファイルパス登録
	int SetLogPath(const TCHAR* pLogPath);

	//標準MIDIファイル読み込み
	int Load(const TCHAR* pSMFPath, SMSeqData* pMIDIData);

private:

	//チャンクヘッダ構造

	#pragma pack(push,1)

	typedef struct {
		unsigned char chunkType[4];		//チャンクタイプ MThd/MTrk
		unsigned long chunkSize;		//チャンクサイズ
	} SMFChunkTypeSection;

	typedef struct {
		unsigned short format;			//フォーマット 0,1,2
		unsigned short ntracks;			//トラック数
		unsigned short timeDivision;	//4分音符あたりの分解能
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

	//代入とコピーコンストラクタの禁止
	void operator=(const SMFileReader&);
	SMFileReader(const SMFileReader&);

};

} // end of namespace

