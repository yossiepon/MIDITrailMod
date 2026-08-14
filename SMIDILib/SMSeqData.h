//******************************************************************************
//
// Simple MIDI Library / SMSeqData
//
// Sequence data class.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2012-2025 Yossiepon Oniichan. All Rights Reserved.
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
// Sequence data class
//******************************************************************************
class SMIDILIB_API SMSeqData
{
public:

	//Constructor / Destructor
	SMSeqData();
	virtual ~SMSeqData(void);

	//----------------------------------------------------------------
	//Data creation related
	//----------------------------------------------------------------
	//Register SMF format
	void SetSMFFormat(unsigned long smfFormat);

	//Register time division
	void SetTimeDivision(unsigned long timeDivision);

	//Register track
	int AddTrack(SMTrack* pTrack);

	//End track registration
	int CloseTrack();

	//Register file name
	void SetFileName(const WCHAR* pFileName);

	//Clear
	void Clear();


	//Add sequence
	void AddSequence(SMSeqData &other, short portNo = -1, short chNo = -1);


	//----------------------------------------------------------------
	//Get data related
	//----------------------------------------------------------------
	//Get SMF format
	unsigned long GetSMFFormat();

	//Get time division
	unsigned long GetTimeDivision();

	//Get track count
	unsigned long GetTrackNum();

	//Get track
	int GetTrack(unsigned long index, SMTrack* pTrack);

	//Get merged track
	int GetMergedTrack(SMTrack* pMergedTrack);

	//Get total tick time
	unsigned long GetTotalTickTime();

	//Get total playback time
	unsigned long GetTotalPlayTime();

	//Get tempo
	unsigned long GetTempo();

	//Get tempo(BPM)
	unsigned long GetTempoBPM();

	//Get time signature: numerator and denominator
	unsigned long GetBeatNumerator();
	unsigned long GetBeatDenominator();

	//Get bar count
	unsigned long GetBarNum();

	//Get copyright string
	const WCHAR* GetCopyRight();

	//Get title string
	const WCHAR* GetTitle();

	//Get bar list
	int GetBarList(SMBarList* pBarList);

	//Get port list
	int GetPortList(SMPortList* pPortList);

	//Get file name
	const WCHAR* GetFileName();

	static int StringToWstring(std::string* pStr, std::wstring* pWstr);

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
	std::wstring m_CopyRight;
	std::wstring m_Title;
	std::wstring m_FileName;
	SMTrackList m_TrackList;
	SMTrack* m_pMergedTrack;

	int _MergeTracks();
	double _GetDeltaTimeMsec(unsigned long tempo, unsigned long deltaTime);
	int _GetTempo(unsigned long* pTempo);
	int _GetBeat(unsigned long* pNumerator, unsigned long* pDenominator);
	int _GetBarNum(unsigned long* pBarNum);
	int _CalcTotalTime();
	int _SearchText();
	static int _StringToWstring(std::string* pStr, std::wstring* pWstr);

	//Prohibit assignment and copy constructor
	void operator=(const SMSeqData&);
	SMSeqData(const SMSeqData&);
};

} // end of namespace

#pragma warning(default:4251)

