//******************************************************************************
//
// Simple MIDI Library / SMTrack
//
// トラッククラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMCommon.h"
#include "SMTrack.h"
#include "SMEventMIDI.h"
#include "SMEventMeta.h"
#include "SMFPUCtrl.h"

using namespace YNBaseLib;

namespace SMIDILib {


//******************************************************************************
// コンストラクタ
//******************************************************************************
SMTrack::SMTrack(void)
// >>> modify 20120728 yossiepon begin
 : m_List(sizeof(SMDataSet), 1000), m_OverwritePortNo(-1)
// <<< modify 20120728 yossiepon end
{
}

//******************************************************************************
// デストラクタ
//******************************************************************************
SMTrack::~SMTrack(void)
{
	Clear();
}

//******************************************************************************
// データクリア
//******************************************************************************
void SMTrack::Clear()
{
	SMExDataMap::iterator exdataitr;

	m_List.Clear();

	for (exdataitr = m_ExDataMap.begin(); exdataitr != m_ExDataMap.end(); exdataitr++) {
		delete [] (exdataitr->second);
	}
	m_ExDataMap.clear();

// >>> add 20120728 yossiepon begin
	m_OverwritePortNo = -1;
// <<< add 20120728 yossiepon end

	return;
}

//******************************************************************************
// データセット追加
//******************************************************************************
int SMTrack::AddDataSet(
		unsigned long deltaTime,
		SMEvent* pEvent,
		unsigned char portNo
	)
{
	int result = 0;
	unsigned long index = 0;
	unsigned char* pExData = NULL;
	SMDataSet dataSet;

	index = m_List.GetSize();

	//データセット作成
	ZeroMemory(&dataSet, sizeof(SMDataSet));
	dataSet.deltaTime = deltaTime;
	dataSet.eventData.type   = pEvent->GetType();
	dataSet.eventData.status = pEvent->GetStatus();
	dataSet.eventData.meta   = pEvent->GetMetaType();
	dataSet.eventData.size   = pEvent->GetDataSize();
	dataSet.portNo = portNo;

	//イベントデータが4byte以内なら構造体内に格納する
	if (pEvent->GetDataSize() <= 4) {
		memcpy(&(dataSet.eventData.data), pEvent->GetDataPtr(), pEvent->GetDataSize());
	}
	//それ以外は別途ヒープに保持してマップで管理する
	else {
		try {
			pExData = new unsigned char[pEvent->GetDataSize()];
		}
		catch (std::bad_alloc) {
			result = YN_SET_ERR("Could not allocate memory.", pEvent->GetDataSize(), 0);
			goto EXIT;
		}
		memcpy(pExData, pEvent->GetDataPtr(), pEvent->GetDataSize());
		m_ExDataMap.insert(SMExDataMapPair(index, pExData));
		pExData = NULL;
	}

	result = m_List.AddItem(&dataSet);
	if (result != 0) goto EXIT;

EXIT:;
	delete [] pExData;
	return result;
}

//******************************************************************************
// データセット取得
//******************************************************************************
int SMTrack::GetDataSet(
		unsigned long index,
		unsigned long* pDeltaTime,
		SMEvent* pEvent,
		unsigned char* pProtNo
	)
{
	int result = 0;
	unsigned char* pEventData = NULL;
	SMDataSet dataSet;
	SMExDataMap::iterator exdataitr;

	result = m_List.GetItem(index, &dataSet);
	if (result != 0) goto EXIT;

	//デルタタイム
	if (pDeltaTime != NULL) {
		*pDeltaTime = dataSet.deltaTime;
	}

	//イベントデータ位置
	if (dataSet.eventData.size <= 4) {
		pEventData = dataSet.eventData.data;
	}
	else {
		exdataitr = m_ExDataMap.find(index);
		if (exdataitr == m_ExDataMap.end()) {
			result = YN_SET_ERR("Program error.", index, 0);
			goto EXIT;
		}
		pEventData = exdataitr->second;
	}

	//イベントデータ登録
	if (pEvent != NULL) {
		result = pEvent->SetData(
						dataSet.eventData.type,
						dataSet.eventData.status,
						dataSet.eventData.meta,
						pEventData,
						dataSet.eventData.size
					);
		if (result != 0) goto EXIT;
	}

	//ポート番号
	if (pProtNo != NULL) {
// >>> modify 20120728 yossiepon begin
		if(m_OverwritePortNo == -1) {
			*pProtNo = dataSet.portNo;
		} else {
			*pProtNo = (unsigned char)m_OverwritePortNo;
		}
// <<< modify 20120728 yossiepon end	
	}

EXIT:;
	return result;
}

//******************************************************************************
// >>> modify 20120728 yossiepon begin
// サイズ取得
// <<< modify 20120728 yossiepon end
//******************************************************************************
unsigned long SMTrack::GetSize()
{
	return m_List.GetSize();
}

//******************************************************************************
// コピー
//******************************************************************************
int SMTrack::CopyFrom(
		SMTrack* pSrcTrack
	)
{
	int result = 0;
	unsigned long index = 0;
	unsigned long deltaTime = 0;
	SMEvent event;
	unsigned char portNo = 0;

	//TODO: もう少しインテリジェントなコピーにする

	if (pSrcTrack == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//自分自身がコピー元なら何もしない
	if (pSrcTrack == this) {
		goto EXIT;
	}

	Clear();

	for (index = 0; index < pSrcTrack->GetSize(); index++) {
		result = pSrcTrack->GetDataSet(index, &deltaTime, &event, &portNo);
		if (result != 0) goto EXIT;

		result = AddDataSet(deltaTime, &event, portNo);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

// >>> add 20120728 yossiepon begin

//******************************************************************************
// ポート番号上書き
//******************************************************************************
int SMTrack::OverwritePortNo(short portNo)
{
	int result = 0;

	m_OverwritePortNo = portNo;

	return result;
}

//******************************************************************************
// チャンネル番号上書き
//******************************************************************************
int SMTrack::OverwriteChNo(short chNo)
{
	int result = 0;
	unsigned long index = 0;
	SMEvent event;

	if(chNo == -1) {
		goto EXIT;
	}

	for (index = 0; index < GetSize(); index++) {

		//リストからノートを取得する
		result = m_List.GetItem(index, &event);
		if (result != 0) goto EXIT;

		if(event.GetType() != SMEvent::EventMIDI) {
			continue;
		}

		//チャンネル番号を上書きしてリストに書き戻す
		unsigned char status = event.GetStatus();
		event.SetStatus((status & 0xf0) | (chNo & 0x0f));

		result = m_List.SetItem(index, &event);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

// <<< add 20120728 yossiepon end

//******************************************************************************
// ノートリスト取得
//******************************************************************************
int SMTrack::GetNoteList(
		SMNoteList* pNoteList
	)
{
	int result = 0;

	result = _GetNoteList(pNoteList, 0);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ノートリスト取得
//******************************************************************************
int SMTrack::GetNoteListWithRealTime(
		SMNoteList* pNoteList,
		unsigned long timeDivision
	)
{
	int result = 0;

	if (timeDivision == 0) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	result = _GetNoteList(pNoteList, timeDivision);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ノートリスト取得
//******************************************************************************
int SMTrack::_GetNoteList(
		SMNoteList* pNoteList,
		unsigned long timeDivision
	)
{
	int result = 0;
	unsigned long index = 0;
	unsigned long deltaTime = 0;
	unsigned long totalTime = 0;
	unsigned char portNo = 0;
	unsigned long key = 0;
	unsigned long tempo = SM_DEFAULT_TEMPO;
	double totalRealtime = 0;
	SMNoteMap noteMap;
	SMNoteMap::iterator itr;
	SMNote note;
	SMEvent event;
	SMEventMIDI midiEvent;
	SMEventMeta metaEvent;
	SMFPUCtrl fpuCtrl;

	// timeDivision  = 0 の場合：startTime, endTime はチックタイムを設定
	// timeDivision != 0 の場合：startTime, endTime はリアルタイムを設定(msec)

	if (pNoteList == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//浮動小数点演算精度を倍精度に設定
	result = fpuCtrl.Start(SMFPUCtrl::FPUDouble);
	if (result != 0) goto EXIT;

	//ノート情報はトラック登録順でノートリストに追加する
	//すなわちノート開始チックタイムでソートされるようにリストを作成する
	pNoteList->Clear();

	for (index = 0; index < GetSize(); index++) {

		result = GetDataSet(index, &deltaTime, &event, &portNo);
		if (result != 0) goto EXIT;

		totalTime += deltaTime;
		totalRealtime += _ConvTick2TimeMsec(deltaTime, tempo, timeDivision);

// >>> modify 20120728 yossiepon begin

		//METAイベント
		if (event.GetType() == SMEvent::EventMeta) {

			metaEvent.Attach(&event);

			if (metaEvent.GetType() == 0x51) {
				//テンポの設定
				tempo = metaEvent.GetTempo();
			} else if (metaEvent.GetType() == 0x05) {

				//最後のノートを取得
				result = pNoteList->GetNote(pNoteList->GetSize() - 1, &note);
				if (result != 0) goto EXIT;

				//歌詞を取得
				std::string lyric;
				metaEvent.GetText(&lyric);
				//歌詞の先頭がSPC(0x20)以降であれば、歌詞を格納する
				if(((unsigned char)lyric.c_str()[0]) > 0x20) {
					::strncpy_s(note.lyric, sizeof(note.lyric), lyric.c_str(), _TRUNCATE);
					result = pNoteList->SetNote(pNoteList->GetSize() - 1, &note);
					if (result != 0) goto EXIT;
				}
			}
		}

// <<< modify 20120728 yossiepon end

		//MIDIイベント以外はスキップ
		if (event.GetType() != SMEvent::EventMIDI) continue;

		midiEvent.Attach(&event);

		//ノートオン
		if (midiEvent.GetChMsg() == SMEventMIDI::NoteOn) {
			//マップから当該ノートを検索
			key = _GetNoteKey(portNo, midiEvent.GetChNo(), midiEvent.GetNoteNo());
			itr = noteMap.find(key);

			//未登録の場合
			if (itr == noteMap.end()) {
				note.portNo = portNo;
				note.chNo = midiEvent.GetChNo();
				note.noteNo = midiEvent.GetNoteNo();
				note.velocity = midiEvent.GetVelocity();
				note.startTime = ((timeDivision == 0) ? totalTime : (unsigned long)totalRealtime);
				note.endTime = 0;
// >>> add 20120728 yossiepon begin
				note.lyric[0] = '\0';
// <<< add 20120728 yossiepon end
			}
			//登録済みの場合
			else {
				//同一ノート番号でノートOFFなしにノートONが連続した場合に相当する
				//MIDIの仕様上どういう扱いになるかは不明
				//ノートを区切って新しいノートの開始とする
				result = pNoteList->GetNote(itr->second, &note);
				if (result != 0) goto EXIT;

				//終了チックタイムを記録してリストに書き戻す
				note.endTime = ((timeDivision == 0) ? totalTime : (unsigned long)totalRealtime);
				result = pNoteList->SetNote(itr->second, &note);
				if (result != 0) goto EXIT;

				noteMap.erase(itr);

				//新しいノート
				note.velocity = midiEvent.GetVelocity();
				note.startTime = ((timeDivision == 0) ? totalTime : (unsigned long)totalRealtime);
				note.endTime = 0;
			}
			//終了チックタイム未定のままノートリストに登録する
			pNoteList->AddNote(note);
			//ノートリストのインデックス位置をマップに記録する
			noteMap.insert(SMNoteMapPair(key, (pNoteList->GetSize()-1)));
		}
		//ノートオフ
		if (midiEvent.GetChMsg() == SMEventMIDI::NoteOff) {
			//マップから当該ノートを検索
			key = _GetNoteKey(portNo, midiEvent.GetChNo(), midiEvent.GetNoteNo());
			itr = noteMap.find(key);

			if (itr != noteMap.end()) {
				result = pNoteList->GetNote(itr->second, &note);
				if (result != 0) goto EXIT;

				//終了チックタイムを記録してリストに書き戻す
				note.endTime = ((timeDivision == 0) ? totalTime : (unsigned long)totalRealtime);
				result = pNoteList->SetNote(itr->second, &note);
				if (result != 0) goto EXIT;

				noteMap.erase(itr);
			}
		}
	}

	//ノートオンのまま終了した場合はノートを区切ってリストに追加する
	for (itr = noteMap.begin(); itr != noteMap.end(); itr++) {
		result = pNoteList->GetNote(itr->second, &note);
		if (result != 0) goto EXIT;

		note.endTime = ((timeDivision == 0) ? totalTime : (unsigned long)totalRealtime);
		result = pNoteList->SetNote(itr->second, &note);
		if (result != 0) goto EXIT;
	}

	result = fpuCtrl.End();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// チックタイムから実時間への変換（ミリ秒）
//******************************************************************************
double SMTrack::_ConvTick2TimeMsec(
		unsigned long tickTime,
		unsigned long tempo,
		unsigned long timeDivision
	)
{
	double timeMsec = 0;

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

	timeMsec = ((double)tickTime * (double)tempo) / (1000.0 * (double)timeDivision);

	return timeMsec;
}

//******************************************************************************
// ノート特定キー取得
//******************************************************************************
unsigned long SMTrack::_GetNoteKey(
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo
	)
{
// >>> add 20120728 yossiepon begin
	if(m_OverwritePortNo != -1) {
		portNo = (unsigned char)m_OverwritePortNo;
	}
// <<< add 20120728 yossiepon end

	return ((portNo << 16) | (chNo << 8) | noteNo);
}

} // end of namespace

