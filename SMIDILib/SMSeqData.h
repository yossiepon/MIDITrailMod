//******************************************************************************
//
// Simple MIDI Library / SMSeqData
//
// シーケンスデータクラス
//
// Copyright (C) 2010-2013 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

#include "SMCommon.h"
#include "SMTrack.h"
#include "SMBarList.h"
#include "SMPortList.h"
#include <string>
#include <list>

#pragma warning(disable:4251)

namespace SMIDILib {


//******************************************************************************
// シーケンスデータクラス
//******************************************************************************
class SMIDILIB_API SMSeqData
{
public:

	//コンストラクタ／デストラクタ
	SMSeqData();
	virtual ~SMSeqData(void);

	//----------------------------------------------------------------
	//データ作成系
	//----------------------------------------------------------------
	//SMFフォーマット登録
	void SetSMFFormat(unsigned long smfFormat);

	//時間解像度登録
	void SetTimeDivision(unsigned long timeDivision);

	//トラック登録
	int AddTrack(SMTrack* pTrack);

	//トラック登録終了
	int CloseTrack();

	//ファイル名登録
	void SetFileName(const char* pFileName);

	//クリア
	void Clear();

	//----------------------------------------------------------------
	//データ取得系
	//----------------------------------------------------------------
	//SMFフォーマット取得
	unsigned long GetSMFFormat();

	//時間解像度取得
	unsigned long GetTimeDivision();

	//トラック数取得
	unsigned long GetTrackNum();

	//トラック取得
	int GetTrack(unsigned long index, SMTrack* pTrack);

	//マージ済みトラック取得
	int GetMergedTrack(SMTrack* pMergedTrack);

	//トータルチックタイム取得
	unsigned long GetTotalTickTime();

	//トータル演奏時間取得
	unsigned long GetTotalPlayTime();

	//テンポ取得
	unsigned long GetTempo();

	//テンポ取得(BPM)
	unsigned long GetTempoBPM();

	//拍子記号取得：分子と分母
	unsigned long GetBeatNumerator();
	unsigned long GetBeatDenominator();

	//小節数取得
	unsigned long GetBarNum();

	//コピーライト文字列取得
	const char* GetCopyRight();

	//タイトル文字列取得
	const char* GetTitle();

	//小節リスト取得
	int GetBarList(SMBarList* pBarList);

	//ポートリスト取得
	int GetPortList(SMPortList* pPortList);

	//ファイル名取得
	const char* GetFileName();

private:

	typedef std::list<SMTrack*> SMTrackList;
	typedef std::list<SMTrack*>::iterator SMTrackListItr;

	typedef struct {
		unsigned long index;
		unsigned long deltaTime;
	} SMDeltaTimeBuf;

	typedef std::list<SMDeltaTimeBuf> SMDeltaTimeBufList;
	typedef std::list<SMDeltaTimeBuf>::iterator SMDeltaTimeBufListItr;

private:

	unsigned long m_SMFFormat;
	unsigned long m_TimeDivision;
	unsigned long m_TotalTickTime;
	unsigned long m_TotalPlayTime;
	unsigned long m_Tempo;
	unsigned long m_BeatNumerator;
	unsigned long m_BeatDenominator;
	unsigned long m_BarNum;
	std::string m_CopyRight;
	std::string m_Title;
	std::string m_FileName;
	SMTrackList m_TrackList;
	SMTrack* m_pMergedTrack;

	int _MergeTracks();
	double _GetDeltaTimeMsec(unsigned long tempo, unsigned long deltaTime);
	int _GetTempo(unsigned long* pTempo);
	int _GetBeat(unsigned long* pNumerator, unsigned long* pDenominator);
	int _GetBarNum(unsigned long* pBarNum);
	int _CalcTotalTime();
	int _SearchText();

	//代入とコピーコンストラクタの禁止
	void operator=(const SMSeqData&);
	SMSeqData(const SMSeqData&);

};

} // end of namespace

#pragma warning(default:4251)

