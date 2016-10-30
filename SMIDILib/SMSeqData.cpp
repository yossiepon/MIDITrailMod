//******************************************************************************
//
// Simple Base Library / SMSeqData
//
// シーケンスデータクラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMEventMeta.h"
#include "SMSeqData.h"
#include "SMFPUCtrl.h"

using namespace YNBaseLib;

namespace SMIDILib {


//******************************************************************************
// コンストラクタ
//******************************************************************************
SMSeqData::SMSeqData()
{
	m_pMergedTrack = NULL;
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
	unsigned long i = 0;
	unsigned char portNo = 0;
	SMTrackListItr trackListItr;
	SMDeltaTimeBuf deltaTimeBuf;
	SMDeltaTimeBufList deltaTimeBufList;
	SMDeltaTimeBufListItr deltaTimeBufListItr;
	SMEvent event;
	SMTrack* pTrack = NULL;
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

	//デルタタイムバッファリストの作成
	for (trackListItr = m_TrackList.begin(); trackListItr != m_TrackList.end(); trackListItr++) {
		pTrack = *trackListItr;
		if (pTrack->GetSize() == 0) continue;

		deltaTimeBuf.index = 0;
		result = pTrack->GetDataSet(0, &deltaTimeBuf.deltaTime, NULL, NULL);
		if (result != 0) goto EXIT;

		deltaTimeBufList.push_back(deltaTimeBuf);
	}

	//マージ処理
	while (true) {

		//各トラックを参照して最もデルタタイムが短いイベントを取得する
		unsigned long deltaTimeMin = 0xFFFFFFFF;
		unsigned long targetTrackIndex = 0;
		bool isDataExist = false;

		trackListItr = m_TrackList.begin();
		deltaTimeBufListItr = deltaTimeBufList.begin();
		for (i = 0; i < m_TrackList.size(); i++) {

			pTrack = *trackListItr;                //カレントトラック
			deltaTimeBuf = *deltaTimeBufListItr;   //カレントトラックのデルタタイム情報

			//トラックを読み終わっていなければデルタタイムを参照する
			if (deltaTimeBuf.index < pTrack->GetSize()) {
				//最小デルタタイムのトラックをマークする
				if (deltaTimeBuf.deltaTime < deltaTimeMin) {
					targetTrackIndex = i;
					deltaTimeMin = deltaTimeBuf.deltaTime ;
				}
				isDataExist = true;
			}
			//次のトラック
			trackListItr++;
			deltaTimeBufListItr++;
		}

		//イベントが存在しなければマージ完了
		if (!isDataExist) break;

		//各トラックのデルタタイムを更新する
		trackListItr = m_TrackList.begin();
		deltaTimeBufListItr = deltaTimeBufList.begin();
		for (i = 0; i < m_TrackList.size(); i++) {

			pTrack = *trackListItr;               //カレントトラック
			deltaTimeBuf = *deltaTimeBufListItr;  //カレントトラックのデルタタイム情報

			//マークしたトラックはイベントをコピーしてマージトラックに登録
			if (i == targetTrackIndex) {
				result = pTrack->GetDataSet(deltaTimeBuf.index, NULL, &event, &portNo);
				if (result != 0) goto EXIT;

				result = pMergedTrack->AddDataSet(deltaTimeMin, &event, portNo);
				if (result != 0) goto EXIT;

				//マークしたトラックの次のデルタタイムを取得する
				deltaTimeBuf.index += 1;
				deltaTimeBuf.deltaTime = 0xFFFFFFFF;
				if (deltaTimeBuf.index < pTrack->GetSize()) {
					result = pTrack->GetDataSet(deltaTimeBuf.index, &deltaTimeBuf.deltaTime, NULL, NULL);
					if (result != 0) goto EXIT;
				}
			}
			//それ以外のトラックはデルタタイムを減算する
			else if (deltaTimeBuf.index < pTrack->GetSize()) {
				deltaTimeBuf.deltaTime -= deltaTimeMin;
			}
			*deltaTimeBufListItr = deltaTimeBuf;

			//次のトラック
			trackListItr++;
			deltaTimeBufListItr++;
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
	m_CopyRight = "";
	m_Title = "";

	delete m_pMergedTrack;
	m_pMergedTrack = NULL;

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
	return m_TrackList.size();
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
const char* SMSeqData::GetCopyRight()
{
	return m_CopyRight.c_str();
}

//******************************************************************************
// タイトルテキスト取得
//******************************************************************************
const char* SMSeqData::GetTitle()
{
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

		//拍子記号を取得
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
				result = metaEvent.GetText(&m_CopyRight);
				if (result != 0) goto EXIT;
				break;
			}
		}
	}

	//シーケンス名を検索
	for (index = 0; index < pTrack->GetSize(); index++) {

		result = pTrack->GetDataSet(index, &deltaTime, &event, NULL);
		if (result != 0) goto EXIT;

		if (event.GetType() == SMEvent::EventMeta) {
			metaEvent.Attach(&event);
			//任意テキスト
			if ((metaEvent.GetType() == 0x01) && (!isFoundText)) {
				result = metaEvent.GetText(&m_Title);
				if (result != 0) goto EXIT;

				//シーケンス名を優先するので検索は継続する
				isFoundText = true;
			}
			//シーケンス名
			if (metaEvent.GetType() == 0x03) {
				result = metaEvent.GetText(&m_Title);
				if (result != 0) goto EXIT;
				break;
			}
		}
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

} // end of namespace

