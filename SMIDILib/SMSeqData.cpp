//******************************************************************************
//
// Simple Base Library / SMSeqData
//
// シーケンスデータクラス
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMEventMeta.h"
#include "SMSeqData.h"
#include "SMNoteList.h"
#include "SMFPUCtrl.h"
#include <vector>
#include <queue>
#include <algorithm>
#include <iterator>
#include <mbctype.h>

using namespace YNBaseLib;

namespace SMIDILib {

//------------------------------------------------------------------------------
// k-way merge heap item (min-heap by absolute time; ties broken by track index,
// matching the original linear-scan order)
//------------------------------------------------------------------------------
namespace {
	struct MergeHeapItem {
		unsigned long long absTime;
		int                trackIdx;
	};
	struct MergeHeapCmp {
		bool operator()(const MergeHeapItem& a, const MergeHeapItem& b) const {
			if (a.absTime != b.absTime) return a.absTime > b.absTime;
			return a.trackIdx > b.trackIdx;
		}
	};
}


//******************************************************************************
// コンストラクタ
//******************************************************************************
SMSeqData::SMSeqData()
{
	m_pMergedTrack = NULL;
	m_pMergedNoteList = NULL;
	m_pMergedNoteListTrack = NULL;
	m_pMergedTrackNo = NULL;
	m_pMergedNoteListRT = NULL;
	m_pMergedNoteListWithTrackRT = NULL;
	Clear();
}

//******************************************************************************
// デストラクタ
//******************************************************************************
SMSeqData::~SMSeqData(void)
{
	Clear();
}

//******************************************************************************
// SMFフォーマット登録
//******************************************************************************
void SMSeqData::SetSMFFormat(
		unsigned long smfFormat
	)
{
	m_SMFFormat = smfFormat;
}

//******************************************************************************
// 分解能登録
//******************************************************************************
void SMSeqData::SetTimeDivision(
		unsigned long timeDivision
	)
{
	m_TimeDivision = timeDivision;
}

//******************************************************************************
// トラック登録
//******************************************************************************
int SMSeqData::AddTrack(
		SMTrack* pTrack
	)
{
	m_TrackList.push_back(pTrack);
	return 0;
}

//******************************************************************************
// トラック登録完了
//******************************************************************************
int SMSeqData::CloseTrack()
{
	int result = 0;

	//トラックマージ処理
	result = _MergeTracks();
	if (result != 0) goto EXIT;

	//合計演奏時間算出
	result = _CalcTotalTime();
	if (result != 0) goto EXIT;

	//テンポ取得
	result = _GetTempo(&m_Tempo);
	if (result != 0) goto EXIT;

	//拍子記号取得
	result = _GetBeat(&m_BeatNumerator, &m_BeatDenominator);
	if (result != 0) goto EXIT;

	//小節数取得
	result = _GetBarNum(&m_BarNum);
	if (result != 0) goto EXIT;

	//テキスト情報取得
	result = _SearchText();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// トラックマージ処理
//******************************************************************************
int SMSeqData::_MergeTracks()
{
	int result = 0;
	SMTrackListItr trackListItr;
	SMEvent event;
	SMTrack* pMergedTrack = NULL;

	delete m_pMergedTrack;
	m_pMergedTrack = NULL;

	try {
		pMergedTrack = new SMTrack();
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

	{
		// k-way merge of the per-track event streams by absolute time using a
		// min-heap -> O(events * log tracks) instead of the old O(events * tracks)
		// linear scan (a large win when loading many-track Black MIDI).
		std::vector<SMTrack*> tr;
		for (trackListItr = m_TrackList.begin(); trackListItr != m_TrackList.end(); trackListItr++) {
			tr.push_back(*trackListItr);
		}
		std::vector<unsigned long> idx(tr.size(), 0);
		std::priority_queue<MergeHeapItem, std::vector<MergeHeapItem>, MergeHeapCmp> pq;

		for (size_t k = 0; k < tr.size(); k++) {
			if (tr[k]->GetSize() == 0) continue;
			unsigned long dt = 0;
			result = tr[k]->GetDataSet(0, &dt, NULL, NULL);
			if (result != 0) goto EXIT;
			MergeHeapItem it;
			it.absTime = (unsigned long long)dt;
			it.trackIdx = (int)k;
			pq.push(it);
		}

		unsigned long long lastEmit = 0;
		while (!pq.empty()) {
			MergeHeapItem top = pq.top();
			pq.pop();
			int k = top.trackIdx;
			unsigned long long absT = top.absTime;
			unsigned char portNo = 0;

			result = tr[k]->GetDataSet(idx[k], NULL, &event, &portNo);
			if (result != 0) goto EXIT;
			result = pMergedTrack->AddDataSet((unsigned long)(absT - lastEmit), &event, portNo);
			if (result != 0) goto EXIT;
			lastEmit = absT;

			idx[k] += 1;
			if (idx[k] < tr[k]->GetSize()) {
				unsigned long ndt = 0;
				result = tr[k]->GetDataSet(idx[k], &ndt, NULL, NULL);
				if (result != 0) goto EXIT;
				MergeHeapItem nit;
				nit.absTime = absT + (unsigned long long)ndt;
				nit.trackIdx = k;
				pq.push(nit);
			}
		}
	}

	m_pMergedTrack = pMergedTrack;

EXIT:;
	if (result != NULL) {
		delete pMergedTrack;
		pMergedTrack = NULL;
	}
	return result;
}

//******************************************************************************
// データクリア
//******************************************************************************
void SMSeqData::Clear()
{
	SMTrackListItr itr;

	m_SMFFormat = 0;
	m_TimeDivision = 0;
	m_TotalTickTime = 0;
	m_TotalPlayTime = 0;
	m_Tempo = SM_DEFAULT_TEMPO;
	m_BeatNumerator = SM_DEFAULT_TIME_SIGNATURE_NUMERATOR;
	m_BeatDenominator = SM_DEFAULT_TIME_SIGNATURE_DENOMINATOR;
	m_BarNum = 0;
	m_CopyRight = L"";
	m_Title = L"";
	m_FileName = L"";

	delete m_pMergedTrack;
	m_pMergedTrack = NULL;

	delete m_pMergedNoteList;
	m_pMergedNoteList = NULL;

	delete m_pMergedNoteListTrack;
	m_pMergedNoteListTrack = NULL;
	delete [] m_pMergedTrackNo;
	m_pMergedTrackNo = NULL;
	delete m_pMergedNoteListRT;
	m_pMergedNoteListRT = NULL;
	delete m_pMergedNoteListWithTrackRT;
	m_pMergedNoteListWithTrackRT = NULL;

	for (itr = m_TrackList.begin(); itr != m_TrackList.end(); itr++) {
		delete *itr;
		*itr = NULL;
	}
	m_TrackList.clear();

	return;
}

// >>> add 20120728 yossiepon begin

//******************************************************************************
// シーケンス追加
//******************************************************************************
void SMSeqData::AddSequence(SMSeqData &other, short portNo, short chNo)
{
	SMTrackListItr itr = other.m_TrackList.begin();
	std::advance(itr, 1);

	for (; itr != other.m_TrackList.end(); itr++) {

		(*itr)->OverwritePortNo(portNo);
		(*itr)->OverwriteChNo(chNo);

		m_TrackList.push_back(*itr);
	}

	other.m_TrackList.clear();

	CloseTrack();

	return;
}

// <<< add 20120728 yossiepon end

//******************************************************************************
// SMFフォーマット取得
//******************************************************************************
unsigned long SMSeqData::GetSMFFormat()
{
	return m_SMFFormat;
}

//******************************************************************************
// 分解能取得
//******************************************************************************
unsigned long SMSeqData::GetTimeDivision()
{
	return m_TimeDivision;
}

//******************************************************************************
// トラック数取得
//******************************************************************************
unsigned long SMSeqData::GetTrackNum()
{
	return (unsigned long)m_TrackList.size();
}

//******************************************************************************
// トラック取得
//******************************************************************************
int SMSeqData::GetTrack(
		unsigned long index,
		SMTrack* pTrack
	)
{
	int result = 0;
	SMTrackListItr itr;
	SMTrack *pSrcTrack;

	if (pTrack == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	if (index >= GetTrackNum()) {
		result = YN_SET_ERR("Program error.", index, GetTrackNum());
		goto EXIT;
	}

	itr = m_TrackList.begin();
	advance(itr, index);
	pSrcTrack = *itr;

	result = pTrack->CopyFrom(pSrcTrack);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// マージトラック取得
//******************************************************************************
int SMSeqData::GetMergedTrack(
		SMTrack* pMergedTrack
	)
{
	int result = 0;

	if (pMergedTrack == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	if (m_pMergedTrack == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	result = pMergedTrack->CopyFrom(m_pMergedTrack);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// merged note list (built once from the merged track, then cached)
//******************************************************************************
int SMSeqData::GetMergedNoteList(
		SMNoteList** ppNoteList
	)
{
	int result = 0;

	if (ppNoteList == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	if (m_pMergedTrack == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	if (m_pMergedNoteList == NULL) {
		try {
			m_pMergedNoteList = new SMNoteList();
		}
		catch (std::bad_alloc) {
			result = YN_SET_ERR("Could not allocate memory.", 0, 0);
			goto EXIT;
		}
		result = m_pMergedTrack->GetNoteList(m_pMergedNoteList);
		if (result != 0) {
			delete m_pMergedNoteList;
			m_pMergedNoteList = NULL;
			goto EXIT;
		}
	}

	*ppNoteList = m_pMergedNoteList;

EXIT:;
	return result;
}

void SMSeqData::ReleaseMergedNoteList()
{
	delete m_pMergedNoteList;
	m_pMergedNoteList = NULL;
	delete m_pMergedNoteListTrack;
	m_pMergedNoteListTrack = NULL;
	delete [] m_pMergedTrackNo;
	m_pMergedTrackNo = NULL;
	delete m_pMergedNoteListRT;
	m_pMergedNoteListRT = NULL;
	delete m_pMergedNoteListWithTrackRT;
	m_pMergedNoteListWithTrackRT = NULL;
}

//******************************************************************************
// derive a realtime (ms) note list from a tick-based one (tick->ms tempo conv)
//******************************************************************************
int SMSeqData::_BuildRealTimeNoteList(
		SMNoteList* pTickList,
		SMNoteList** ppRTList
	)
{
	int result = 0;
	SMNoteList* pOut = NULL;
	unsigned long timeDiv = m_TimeDivision ? m_TimeDivision : 480;
	std::vector<unsigned long> segTick;   // tempo-segment boundaries (tick)
	std::vector<double>        segMs;      // ms at each boundary
	std::vector<double>        segMsPerTick;
	unsigned long i = 0;
	SMNote note;

	if ((pTickList == NULL) || (ppRTList == NULL) || (m_pMergedTrack == NULL)) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	// build tick->ms tempo segments by walking the merged track's tempo events
	{
		double tempoUs = (double)SM_DEFAULT_TEMPO;
		double msPerTick = tempoUs / ((double)timeDiv * 1000.0);
		unsigned long absTick = 0;
		double absMs = 0.0;
		SMEvent ev;
		SMEventMeta meta;
		unsigned long n = m_pMergedTrack->GetSize();

		segTick.push_back(0); segMs.push_back(0.0); segMsPerTick.push_back(msPerTick);
		for (i = 0; i < n; i++) {
			unsigned long dt = 0;
			if (m_pMergedTrack->GetDataSet(i, &dt, &ev, NULL) != 0) continue;
			absTick += dt;
			absMs   += (double)dt * msPerTick;
			if (ev.GetType() == SMEvent::EventMeta) {
				meta.Attach(&ev);
				if (meta.GetType() == 0x51) {
					tempoUs = (double)meta.GetTempo();
					if (tempoUs <= 0.0) tempoUs = (double)SM_DEFAULT_TEMPO;
					msPerTick = tempoUs / ((double)timeDiv * 1000.0);
					segTick.push_back(absTick); segMs.push_back(absMs); segMsPerTick.push_back(msPerTick);
				}
			}
		}
	}

	try {
		pOut = new SMNoteList();
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

	{
		unsigned long cnt = pTickList->GetSize();
		for (i = 0; i < cnt; i++) {
			if (pTickList->GetNote(i, &note) != 0) continue;
			// tick -> ms via the last segment at or before the tick
			size_t s = (size_t)(std::upper_bound(segTick.begin(), segTick.end(), note.startTime) - segTick.begin());
			s = (s > 0) ? (s - 1) : 0;
			double sMs = segMs[s] + (double)((long)note.startTime - (long)segTick[s]) * segMsPerTick[s];
			size_t e = (size_t)(std::upper_bound(segTick.begin(), segTick.end(), note.endTime) - segTick.begin());
			e = (e > 0) ? (e - 1) : 0;
			double eMs = segMs[e] + (double)((long)note.endTime - (long)segTick[e]) * segMsPerTick[e];
			note.startTime = (unsigned long)(sMs + 0.5);
			note.endTime   = (unsigned long)(eMs + 0.5);
			result = pOut->AddNote(note);
			if (result != 0) goto EXIT;
		}
	}

	*ppRTList = pOut;
	pOut = NULL;

EXIT:;
	delete pOut;   // non-NULL only on error
	return result;
}

//******************************************************************************
// merged note list with realtime (ms), derived from the cached tick list
//******************************************************************************
int SMSeqData::GetMergedNoteListRealTime(
		SMNoteList** ppNoteList
	)
{
	int result = 0;

	if (ppNoteList == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	if (m_pMergedNoteListRT == NULL) {
		SMNoteList* pTick = NULL;
		result = GetMergedNoteList(&pTick);   // shared tick list (cached)
		if (result != 0) goto EXIT;
		result = _BuildRealTimeNoteList(pTick, &m_pMergedNoteListRT);
		if (result != 0) goto EXIT;
	}

	*ppNoteList = m_pMergedNoteListRT;

EXIT:;
	return result;
}

//******************************************************************************
// realtime note list aligned with the source-track array (for ripple color)
//******************************************************************************
int SMSeqData::GetMergedNoteListWithTrackRealTime(
		SMNoteList** ppNoteList,
		const unsigned char** ppTrackNo
	)
{
	int result = 0;

	if ((ppNoteList == NULL) || (ppTrackNo == NULL)) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	if (m_pMergedNoteListWithTrackRT == NULL) {
		SMNoteList* pTick = NULL;
		const unsigned char* pTrk = NULL;
		result = GetMergedNoteListWithTrack(&pTick, &pTrk);   // tick list + trackNo
		if (result != 0) goto EXIT;
		result = _BuildRealTimeNoteList(pTick, &m_pMergedNoteListWithTrackRT);
		if (result != 0) goto EXIT;
	}

	*ppNoteList = m_pMergedNoteListWithTrackRT;
	*ppTrackNo  = m_pMergedTrackNo;   // same order as the with-track tick list

EXIT:;
	return result;
}

//******************************************************************************
// one track's note list (no track copy, unlike GetTrack + GetNoteList)
//******************************************************************************
int SMSeqData::GetTrackNoteList(
		unsigned long index,
		SMNoteList* pNoteList
	)
{
	int result = 0;
	SMTrackListItr itr;

	if (pNoteList == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	if (index >= GetTrackNum()) {
		result = YN_SET_ERR("Program error.", index, GetTrackNum());
		goto EXIT;
	}

	itr = m_TrackList.begin();
	std::advance(itr, index);
	result = (*itr)->GetNoteList(pNoteList);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// merged note list WITH source track number (built once, cached)
//******************************************************************************
int SMSeqData::GetMergedNoteListWithTrack(
		SMNoteList** ppNoteList,
		const unsigned char** ppTrackNo
	)
{
	int result = 0;

	if ((ppNoteList == NULL) || (ppTrackNo == NULL)) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	if (m_pMergedNoteListTrack == NULL) {
		std::vector<SMNote> tmpNotes;
		std::vector<unsigned char> tmpTrk;
		std::vector<std::pair<unsigned long, unsigned long> > keys;
		SMNoteList tl;
		SMNote note;
		unsigned long trackNum = GetTrackNum();
		unsigned long t = 0, n = 0;
		size_t k = 0;

		// gather (note, sourceTrack) across all tracks (no per-track copy)
		for (t = 0; t < trackNum; t++) {
			result = GetTrackNoteList(t, &tl);
			if (result != 0) goto EXIT;
			unsigned long sz = tl.GetSize();
			for (n = 0; n < sz; n++) {
				result = tl.GetNote(n, &note);
				if (result != 0) goto EXIT;
				tmpNotes.push_back(note);
				tmpTrk.push_back((unsigned char)(t & 0xFF));
			}
		}

		// sort by start tick via small (startTime,index) keys (stable by start)
		keys.resize(tmpNotes.size());
		for (k = 0; k < tmpNotes.size(); k++) {
			keys[k].first = tmpNotes[k].startTime;
			keys[k].second = (unsigned long)k;
		}
		std::sort(keys.begin(), keys.end());

		try {
			m_pMergedNoteListTrack = new SMNoteList();
			m_pMergedTrackNo = new unsigned char[(keys.size() > 0) ? keys.size() : 1];
		}
		catch (std::bad_alloc) {
			delete m_pMergedNoteListTrack; m_pMergedNoteListTrack = NULL;
			delete [] m_pMergedTrackNo; m_pMergedTrackNo = NULL;
			result = YN_SET_ERR("Could not allocate memory.", 0, 0);
			goto EXIT;
		}

		for (k = 0; k < keys.size(); k++) {
			unsigned long src = keys[k].second;
			result = m_pMergedNoteListTrack->AddNote(tmpNotes[src]);
			if (result != 0) {
				delete m_pMergedNoteListTrack; m_pMergedNoteListTrack = NULL;
				delete [] m_pMergedTrackNo; m_pMergedTrackNo = NULL;
				goto EXIT;
			}
			m_pMergedTrackNo[k] = tmpTrk[src];
		}
	}

	*ppNoteList = m_pMergedNoteListTrack;
	*ppTrackNo = m_pMergedTrackNo;

EXIT:;
	return result;
}

//******************************************************************************
// 合計チックタイム取得
//******************************************************************************
unsigned long SMSeqData::GetTotalTickTime()
{
	return m_TotalTickTime;
}

//******************************************************************************
// 合計演奏時間取得（msec.）
//******************************************************************************
unsigned long SMSeqData::GetTotalPlayTime()
{
	return m_TotalPlayTime;
}

//******************************************************************************
// テンポ取得(μsec.)
//******************************************************************************
unsigned long SMSeqData::GetTempo()
{
	return m_Tempo;
}

//******************************************************************************
// テンポ取得(BPM)
//******************************************************************************
unsigned long SMSeqData::GetTempoBPM()
{
	return ((60 * 1000 * 1000) / m_Tempo);
}

//******************************************************************************
// 拍子記号取得：分子
//******************************************************************************
unsigned long SMSeqData::GetBeatNumerator()
{
	return m_BeatNumerator;
}

//******************************************************************************
// 拍子記号取得：分母
//******************************************************************************
unsigned long SMSeqData::GetBeatDenominator()
{
	return m_BeatDenominator;
}

//******************************************************************************
// 小節数取得
//******************************************************************************
unsigned long SMSeqData::GetBarNum()
{
	return m_BarNum;
}

//******************************************************************************
// 著作権テキスト取得
//******************************************************************************
const WCHAR* SMSeqData::GetCopyRight()
{
	return m_CopyRight.c_str();
}

//******************************************************************************
// タイトルテキスト取得
//******************************************************************************
const WCHAR* SMSeqData::GetTitle()
{
// >>> add 20170528 yossiepon begin
	if (m_Title.length() == 0) {
		return m_FileName.c_str();
	}
// <<< add 20170528 yossiepon end

	return m_Title.c_str();
}

//******************************************************************************
// 合計演奏時間算出
//******************************************************************************
int SMSeqData::_CalcTotalTime()
{
	int result = 0;	
	unsigned long tempo = 0;
	unsigned long deltaTime = 0;
	unsigned long index = 0;
	double totalPlayTime = 0.0f;
	SMEvent event;
	SMEventMeta metaEvent;
	SMFPUCtrl fpuCtrl;

	if (m_pMergedTrack == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//浮動小数点演算精度を倍精度に設定
	result = fpuCtrl.Start(SMFPUCtrl::FPUDouble);
	if (result != 0) goto EXIT;

	tempo = SM_DEFAULT_TEMPO;
	m_TotalTickTime = 0;
	m_TotalPlayTime = 0;

	for (index = 0; index < m_pMergedTrack->GetSize(); index++) {

		//トラックからデータセット取得
		result = m_pMergedTrack->GetDataSet(index, &deltaTime, &event, NULL);
		if (result != 0) goto EXIT;

		//デルタタイムを実時間に変換して演奏時間に加算
		//  1msec未満を切り捨てると誤差が蓄積するためdoubleで積算する
		m_TotalTickTime += deltaTime;
		totalPlayTime += _GetDeltaTimeMsec(tempo, deltaTime);

		//メタイベントが現れたらテンポの更新を確認する
		if (event.GetType() == SMEvent::EventMeta) {
			metaEvent.Attach(&event);
			if (metaEvent.GetType() == 0x51) {
				tempo = metaEvent.GetTempo();
			}
		}
	}

	m_TotalPlayTime = (unsigned long)totalPlayTime;

	result = fpuCtrl.End();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// デルタタイム取得（ミリ秒）
//******************************************************************************
double SMSeqData::_GetDeltaTimeMsec(
		unsigned long tempo,
		unsigned long deltaTime
	)
{
	double deltaTimeMsec = 0;

	//(1) 四分音符あたりの分解能 division
	//    例：48
	//(2) トラックデータのデルタタイム delta
	//    分解能の値を用いて表現する時間差
	//    分解能が48でデルタタイムが24なら八分音符分の時間差
	//(3) テンポ設定（マイクロ秒） tempo
	//    四分音符の実時間間隔
	//
	// デルタタイムに対応する実時間間隔（ミリ秒）
	//  = (delta / division) * tempo / 1000
	//  = (delta * tempo) / (division * 1000)

	deltaTimeMsec = ((double)deltaTime * (double)tempo) / (1000.0 * (double)m_TimeDivision);

	return deltaTimeMsec;
}

//******************************************************************************
// テンポ取得
//******************************************************************************
int SMSeqData::_GetTempo(
		unsigned long* pTempo
	)
{
	int result = 0;
	unsigned long index = 0;
	unsigned long deltaTime = 0;
	SMEvent event;
	SMEventMeta metaEvent;

	if (m_pMergedTrack == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//MIDI仕様においてテンポのデフォルトはBPM120 = 500msec = 500,000μsec
	*pTempo = SM_DEFAULT_TEMPO;

	//シーケンスの先頭（デルタタイムゼロ）からテンポを検索
	//見つからなければデフォルト値が採用される
	for (index = 0; index < m_pMergedTrack->GetSize(); index++) {

		result = m_pMergedTrack->GetDataSet(index, &deltaTime, &event, NULL);
		if (result != 0) goto EXIT;

		if (deltaTime != 0) break;

		//メタイベント以外は無視
		if (event.GetType() != SMEvent::EventMeta) continue;

		//テンポを取得
		metaEvent.Attach(&event);
		if (metaEvent.GetType() == 0x51) {
			*pTempo = metaEvent.GetTempo();
			break;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// 拍子記号取得
//******************************************************************************
int SMSeqData::_GetBeat(
		unsigned long* pNumerator,
		unsigned long* pDenominator
	)
{
	int result = 0;
	unsigned long index = 0;
	unsigned long deltaTime = 0;
	SMEvent event;
	SMEventMeta metaEvent;

	if (m_pMergedTrack == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//MIDI仕様において拍子記号のデフォルトは4/4
	*pNumerator   = SM_DEFAULT_TIME_SIGNATURE_NUMERATOR;
	*pDenominator = SM_DEFAULT_TIME_SIGNATURE_DENOMINATOR;

	//シーケンスの先頭（デルタタイムゼロ）から拍子記号を検索
	//見つからなければデフォルト値が採用される
	for (index = 0; index < m_pMergedTrack->GetSize(); index++) {

		result = m_pMergedTrack->GetDataSet(index, &deltaTime, &event, NULL);
		if (result != 0) goto EXIT;

		if (deltaTime != 0) break;

		//メタイベント以外は無視
		if (event.GetType() != SMEvent::EventMeta) continue;

		//拍子記号を取得
		metaEvent.Attach(&event);
		if (metaEvent.GetType() == 0x58) {
			metaEvent.GetTimeSignature(pNumerator, pDenominator);
			break;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// 小節数取得
//******************************************************************************
int SMSeqData::_GetBarNum(
		unsigned long* pBarNum
	)
{
	int result = 0;
	SMBarList barList;

	result = GetBarList(&barList);
	if (result != 0) goto EXIT;

	*pBarNum = barList.GetSize();

EXIT:;
	return result;
}

//******************************************************************************
// テキスト情報検索
//******************************************************************************
int SMSeqData::_SearchText()
{
	int result = 0;	
	unsigned long index = 0;
	unsigned long deltaTime = 0;
	bool isFoundText = false;
	SMTrackListItr itr;
	SMTrack* pTrack = NULL;
	SMEvent event;
	SMEventMeta metaEvent;
	std::string copyRight;
	std::string title;

	//トラックが存在しなければ何もしない
	if (m_TrackList.size() == 0) goto EXIT;

	//第1トラック(Conductor Track)を参照する
	itr = m_TrackList.begin();
	pTrack = *itr;

	//著作権表示を検索
	for (index = 0; index < pTrack->GetSize(); index++) {

		result = pTrack->GetDataSet(index, &deltaTime, &event, NULL);
		if (result != 0) goto EXIT;

		//著作権表示はデルタタイムゼロに記録される
		if (deltaTime != 0) break;

		if (event.GetType() == SMEvent::EventMeta) {
			metaEvent.Attach(&event);
			if (metaEvent.GetType() == 0x02) {
				result = metaEvent.GetText(&copyRight);
				if (result != 0) goto EXIT;
				break;
			}
		}
	}
	//ワイド文字列変換
	if (copyRight.length() > 0) {
		result = _StringToWstring(&copyRight, &m_CopyRight);
		if (result != 0) goto EXIT;
	}

	//シーケンス名を検索
	for (index = 0; index < pTrack->GetSize(); index++) {

		result = pTrack->GetDataSet(index, &deltaTime, &event, NULL);
		if (result != 0) goto EXIT;

		if (event.GetType() == SMEvent::EventMeta) {
			metaEvent.Attach(&event);
			//任意テキスト
			if ((metaEvent.GetType() == 0x01) && (!isFoundText)) {
				result = metaEvent.GetText(&title);
				if (result != 0) goto EXIT;

				//シーケンス名を優先するので検索は継続する
				isFoundText = true;
			}
			//シーケンス名
			if (metaEvent.GetType() == 0x03) {
				result = metaEvent.GetText(&title);
				if (result != 0) goto EXIT;
				break;
			}
		}
	}
	//ワイド文字列変換
	if (title.length() > 0) {
		result = _StringToWstring(&title, &m_Title);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// 小節リスト取得
//******************************************************************************
int SMSeqData::GetBarList(
		SMBarList* pBarList
	)
{
	int result = 0;	
	unsigned long index = 0;
	unsigned long deltaTime = 0;
	unsigned long prevBarTime = 0;
	unsigned long nextBarTime = 0;
	unsigned long totalTickTime = 0;
	unsigned long numerator = 0;
	unsigned long denominator = 0;
	unsigned long tickTimeOfBar = 0;
	SMEvent event;

	if (pBarList == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	if (m_pMergedTrack == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	pBarList->Clear();

	//1小節あたりのチックタイム
	tickTimeOfBar = (SM_DEFAULT_TIME_SIGNATURE_NUMERATOR * m_TimeDivision * 4) / SM_DEFAULT_TIME_SIGNATURE_DENOMINATOR;

	//1小節目開始地点として登録
	totalTickTime = 0;
	prevBarTime = totalTickTime;
	result = pBarList->AddBar(totalTickTime);
	if (result != 0) goto EXIT;

	for (index = 0; index < m_pMergedTrack->GetSize(); index++) {
		SMEventMeta metaEvent;

		result = m_pMergedTrack->GetDataSet(index, &deltaTime, &event, NULL);
		if (result != 0) goto EXIT;

		totalTickTime += deltaTime;

		//経過時間内で小節の区切りを見つけて登録する
		while(true) {
			nextBarTime = prevBarTime + tickTimeOfBar;
			if (nextBarTime <= totalTickTime) {
				pBarList->AddBar(nextBarTime);
				prevBarTime = nextBarTime;
			}
			else {
				break;
			}
		}

		//以降は拍子記号が現れた場合の対応

		//メタイベント以外は無視
		if (event.GetType() != SMEvent::EventMeta) continue;

		//拍子記号以外は無視
		metaEvent.Attach(&event);
		if (metaEvent.GetType() != 0x58) continue;

		//拍子記号を取得
		metaEvent.GetTimeSignature(&numerator, &denominator);
		if (denominator == 0) {
			//データ異常
			result = YN_SET_ERR("Invalid data found.", index, numerator);
			goto EXIT;
		}

		//1小節あたりのチックタイムを更新
		tickTimeOfBar = (numerator * m_TimeDivision * 4) / denominator;

		//拍子記号更新のため1小節目開始地点として登録
		if (prevBarTime != totalTickTime) {
			prevBarTime = totalTickTime;
			result = pBarList->AddBar(totalTickTime);
			if (result != 0) goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// ポートリスト取得
//******************************************************************************
int SMSeqData::GetPortList(
		SMPortList* pPortList
	)
{
	int result = 0;	
	unsigned long index = 0;
	unsigned char portNo = 0;
	unsigned char port[256];
	SMEvent event;

	if (pPortList == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	if (m_pMergedTrack == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	pPortList->Clear();

	for (index = 0; index < 256; index++) {
		port[index] = 0;
	}

	for (index = 0; index < m_pMergedTrack->GetSize(); index++) {
		result = m_pMergedTrack->GetDataSet(index, NULL, &event, &portNo);
		if (result != 0) goto EXIT;

		port[portNo] = 1;
	}

	for (index = 0; index < 256; index++) {
		if (port[index] != 0) {
			pPortList->AddPort((unsigned char)index);
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// ファイル名登録
//******************************************************************************
void SMSeqData::SetFileName(
		const WCHAR* pFileName
	)
{
	m_FileName = pFileName;
	return;
}

//******************************************************************************
// ファイル名取得
//******************************************************************************
const WCHAR* SMSeqData::GetFileName()
{
	return m_FileName.c_str();
}

//******************************************************************************
// ワイド文字列変換
//******************************************************************************
int SMSeqData::_StringToWstring(std::string* pStr, std::wstring* pWstr)
{
	int result = 0;
	int apiresult = 0;
	int buffSize = 0;
	WCHAR* wstrBuff = NULL;

	//空文字の場合は変換なし
	if (pStr->length() == 0) {
		*pWstr = L"";
		goto EXIT;
	}

	//サロゲートペアと0終端を考慮したバッファサイズ
	buffSize = (int)(pStr->length()) * 2 + 1;

	try {
		wstrBuff = new WCHAR[buffSize];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", buffSize, 0);
		goto EXIT;
	}

	memset(wstrBuff, 0, sizeof(WCHAR) * buffSize);

	apiresult = MultiByteToWideChar(
						_getmbcp(),			//コードページ
						MB_PRECOMPOSED,		//フラグ：
						pStr->c_str(),		//変換元マルチバイト文字列
						(int)(pStr->length()),	//変換元マルチバイト文字列バイト数
						wstrBuff,			//変換先ワイド文字列バッファ
						buffSize - 1		//バッファサイズ（ワイド文字数単位）
					);
	if (apiresult == 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	*pWstr = wstrBuff;

EXIT:;
	delete [] wstrBuff;
	return result;
}

int SMSeqData::StringToWstring(std::string* pStr, std::wstring* pWstr)
{
	return _StringToWstring(pStr, pWstr);
}

} // end of namespace

