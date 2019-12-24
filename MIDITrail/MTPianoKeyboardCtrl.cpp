//******************************************************************************
//
// MIDITrail / MTPianoKeyboardCtrl
//
// ピアノキーボード制御クラス
//
// Copyright (C) 2010-2013 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTPianoKeyboard.h"
#include "MTPianoKeyboardCtrl.h"

using namespace YNBaseLib;


//******************************************************************************
// パラメータ定義
//******************************************************************************
#define MTPIANOKEYBOARD_MAX_ACTIVENOTE_NUM (256)


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTPianoKeyboardCtrl::MTPianoKeyboardCtrl(void)
{
	unsigned char chNo = 0;

	for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
		m_pPianoKeyboard[chNo] = NULL;
	}
	m_PlayTimeMSec = 0;
	m_CurTickTime = 0;
	m_CurNoteIndex = 0;
	m_pNoteStatus = NULL;
	m_isEnable = true;
	m_isSkipping = false;
	m_isSingleKeyboard = false;
	ZeroMemory(m_KeyDownRate, sizeof(float) * SM_MAX_CH_NUM * SM_MAX_NOTE_NUM);
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTPianoKeyboardCtrl::~MTPianoKeyboardCtrl(void)
{
	Release();
}

//******************************************************************************
// 生成処理
//******************************************************************************
int MTPianoKeyboardCtrl::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData,
		MTNotePitchBend* pNotePitchBend,
		bool isSingleKeyboard
	)
{
	int result = 0;
	SMTrack track;

	Release();

	if (pSeqData == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//ノートデザインオブジェクト初期化
	result = m_NoteDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	//キーボードデザイン初期化
	result = m_KeyboardDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	//トラック取得
	result = pSeqData->GetMergedTrack(&track);
	if (result != 0) goto EXIT;

	//ノートリスト取得：startTime, endTime はリアルタイム(msec)
	result = track.GetNoteListWithRealTime(&m_NoteListRT, pSeqData->GetTimeDivision());
	if (result != 0) goto EXIT;

	//ノート情報配列生成
	result = _CreateNoteStatus();
	if (result != 0) goto EXIT;

	//キーボード生成
	result = _CreateKeyboards(pD3DDevice, pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	//ピッチベンド情報
	m_pNotePitchBend = pNotePitchBend;

	//シングルキーボードフラグ
	m_isSingleKeyboard = isSingleKeyboard;

EXIT:;
	return result;
}

//******************************************************************************
// ノート情報配列生成
//******************************************************************************
int MTPianoKeyboardCtrl::_CreateNoteStatus()
{
	int result = 0;
	unsigned long i = 0;

	//ノート情報配列生成
	try {
		m_pNoteStatus = new NoteStatus[MTPIANOKEYBOARD_MAX_ACTIVENOTE_NUM];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

	//ノート状態リスト初期化
	for (i = 0; i < MTPIANOKEYBOARD_MAX_ACTIVENOTE_NUM; i++) {
		m_pNoteStatus[i].isActive = false;
		m_pNoteStatus[i].keyStatus = BeforeNoteON;
		m_pNoteStatus[i].index = 0;
		m_pNoteStatus[i].keyDownRate = 0.0f;
	}

EXIT:;
	return result;
}

//******************************************************************************
// キーボード描画オブジェクト生成
//******************************************************************************
int MTPianoKeyboardCtrl::_CreateKeyboards(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;
	unsigned char chNo = 0;
	LPDIRECT3DTEXTURE9 pTexture = NULL;

	for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
		try {
			m_pPianoKeyboard[chNo] = new MTPianoKeyboard;
		}
		catch (std::bad_alloc) {
			result = YN_SET_ERR("Could not allocate memory.", 0, 0);
			goto EXIT;
		}

		result = m_pPianoKeyboard[chNo]->Create(pD3DDevice, pSceneName, pSeqData, pTexture);
		if (result != 0) goto EXIT;

		//先頭オブジェクトで作成したテクスチャを再利用する
		pTexture = m_pPianoKeyboard[chNo]->GetTexture();
	}

EXIT:;
	return result;
}

//******************************************************************************
// 移動
//******************************************************************************
int MTPianoKeyboardCtrl::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice,
		float rollAngle
	)
{
	int result = 0;
	unsigned char portNo = 0;
	unsigned char chNo = 0;
	D3DXVECTOR3 moveVector;

	//現在発音中ノートの頂点更新
	result = _TransformActiveNotes(pD3DDevice);
	if (result != 0) goto EXIT;

	//各キーボードの移動
	for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {

		//移動ベクトル：キーボード基準座標
		moveVector = m_KeyboardDesign.GetKeyboardBasePos(portNo, chNo);

		//移動ベクトル：ピッチベンドシフトを反映
		moveVector.x += _GetPichBendShiftPosX(portNo, chNo);

		//移動ベクトル：再生面に追従する
		moveVector.y += m_NoteDesign.GetPlayPosX(m_CurTickTime);

		//キーボード移動
		result = m_pPianoKeyboard[chNo]->Transform(pD3DDevice, moveVector, rollAngle);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// 発音中ノートの頂点処理
//******************************************************************************
int MTPianoKeyboardCtrl::_TransformActiveNotes(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;

	//スキップ中なら何もしない
	if (m_isSkipping) goto EXIT;

	//発音中ノートの状態更新
	result = _UpdateStatusOfActiveNotes(pD3DDevice);
	if (result != 0) goto EXIT;

	//発音中ノートの頂点更新
	result = _UpdateVertexOfActiveNotes(pD3DDevice);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 発音中ノートの状態更新
//******************************************************************************
int MTPianoKeyboardCtrl::_UpdateStatusOfActiveNotes(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	unsigned long i = 0;
	unsigned long keyDownDuration = 0;
	unsigned long keyUpDuration = 0;
	bool isFound = false;
	bool isRegist = false;
	SMNote note;

	//キー上昇下降時間(msec)
	keyDownDuration = m_KeyboardDesign.GetKeyDownDuration();
	keyUpDuration   = m_KeyboardDesign.GetKeyUpDuration();

	//ノート情報を更新する
	for (i = 0; i < MTPIANOKEYBOARD_MAX_ACTIVENOTE_NUM; i++) {
		if (m_pNoteStatus[i].isActive) {
			//ノート情報取得
			result = m_NoteListRT.GetNote(m_pNoteStatus[i].index, &note);
			if (result != 0) goto EXIT;

			//発音中ノート状態更新
			result = _UpdateNoteStatus(
							m_PlayTimeMSec,
							keyDownDuration,
							keyUpDuration,
							note,
							&(m_pNoteStatus[i])
						);
			if (result != 0) goto EXIT;
		}
	}

	//前回検索終了位置から発音開始ノートを検索
	while (m_CurNoteIndex < m_NoteListRT.GetSize()) {
		//ノート情報取得
		result = m_NoteListRT.GetNote(m_CurNoteIndex, &note);
		if (result != 0) goto EXIT;

		//演奏時間がキー押下開始時間（発音開始直前）にたどりついていなければ検索終了
		if (note.startTime > keyDownDuration) {
			if (m_PlayTimeMSec < (note.startTime - keyDownDuration)) break;
		}

		//ノート情報登録判定
		isRegist = false;
		if (note.startTime < keyDownDuration) {
			isRegist = true;
		}
		else if (((note.startTime - keyDownDuration) <= m_PlayTimeMSec)
		      && (m_PlayTimeMSec <= (note.endTime + keyUpDuration))) {
			isRegist = true;
		}

		//ノート情報登録
		//  キー下降中／上昇中の情報も登録対象としているため
		//  同一ノートで複数エントリされる場合があることに注意する
		if (isRegist) {
			//すでに同一インデックスで登録済みの場合は何もしない
			isFound = false;
			for (i = 0; i < MTPIANOKEYBOARD_MAX_ACTIVENOTE_NUM; i++) {
				if ((m_pNoteStatus[i].isActive)
				 && (m_pNoteStatus[i].index == m_CurNoteIndex)) {
					isFound = true;
					break;
				}
			}
			//空いているところに追加する
			if (!isFound) {
				for (i = 0; i < MTPIANOKEYBOARD_MAX_ACTIVENOTE_NUM; i++) {
					if (!(m_pNoteStatus[i].isActive)) {
						m_pNoteStatus[i].isActive = true;
						m_pNoteStatus[i].keyStatus = BeforeNoteON;
						m_pNoteStatus[i].index = m_CurNoteIndex;
						m_pNoteStatus[i].keyDownRate = 0.0f;
						break;
					}
				}
			}
			//発音中ノート状態更新
			result = _UpdateNoteStatus(
							m_PlayTimeMSec,
							keyDownDuration,
							keyUpDuration,
							note,
							&(m_pNoteStatus[i])
						);
			if (result != 0) goto EXIT;
		}
		m_CurNoteIndex++;
	}

EXIT:;
	return result;
}

//******************************************************************************
// 発音中ノート状態更新
//******************************************************************************
int MTPianoKeyboardCtrl::_UpdateNoteStatus(
		unsigned long playTimeMSec,
		unsigned long keyDownDuration,
		unsigned long keyUpDuration,
		SMNote note,
		NoteStatus* pNoteStatus
	)
{
	int result= 0;
	unsigned char targetChNo = 0;

	//ノートON前（キー下降中）
	if (playTimeMSec < note.startTime) {
		pNoteStatus->keyStatus = BeforeNoteON;
		if (keyDownDuration == 0) {
			pNoteStatus->keyDownRate = 0.0f;
		}
		else {
			pNoteStatus->keyDownRate = 1.0f - ((float)(note.startTime - playTimeMSec) / (float)keyDownDuration);
		}
	}
	//ノートONからOFFまで
	else if ((note.startTime <= playTimeMSec) && (playTimeMSec <= note.endTime)) {
		pNoteStatus->keyStatus = NoteON;
		pNoteStatus->keyDownRate = 1.0f;
	}
	//ノートOFF後（キー上昇中）
	else if ((note.endTime < playTimeMSec) && (playTimeMSec <= (note.endTime + keyUpDuration))) {
		pNoteStatus->keyStatus = AfterNoteOFF;
		if (keyUpDuration == 0) {
			pNoteStatus->keyDownRate = 0.0f;
		}
		else {
			pNoteStatus->keyDownRate = 1.0f - ((float)(playTimeMSec - note.endTime) / (float)keyUpDuration);
		}
	}
	//ノートOFF後（キー復帰済み）
	else {
		//ノート情報を破棄
		//TODO: 複数ポート対応
		if (note.portNo == 0) {
			//シングルキーボードでは複数チャンネルのキー状態を先頭チャンネルに集約する
			targetChNo = note.chNo;
			if (m_isSingleKeyboard) {
				targetChNo = 0;
			}
			result = m_pPianoKeyboard[targetChNo]->ResetKey(note.noteNo);
			if (result != 0) goto EXIT;
		}
		pNoteStatus->isActive = false;
		pNoteStatus->keyStatus = BeforeNoteON;
		pNoteStatus->index = 0;
		pNoteStatus->keyDownRate = 0.0f;
	}

EXIT:;
	return result;
}

//******************************************************************************
// 発音中ノートの頂点更新
//******************************************************************************
int MTPianoKeyboardCtrl::_UpdateVertexOfActiveNotes(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	unsigned long i = 0;
	unsigned long elapsedTime = 0;
	SMNote note;
	unsigned char targetChNo = 0;
	D3DXCOLOR noteColor;

	ZeroMemory(m_KeyDownRate, sizeof(float) * SM_MAX_CH_NUM * SM_MAX_NOTE_NUM);

	//発音中ノートについて頂点を更新
	for (i = 0; i < MTPIANOKEYBOARD_MAX_ACTIVENOTE_NUM; i++) {
		//発音中でなければスキップ
		if (!(m_pNoteStatus[i].isActive)) continue;

		//ノート情報取得
		result = m_NoteListRT.GetNote(m_pNoteStatus[i].index, &note);
		if (result != 0) goto EXIT;

		//発音開始からの経過時間
		elapsedTime = 0;
		if (m_pNoteStatus[i].keyStatus == NoteON) {
			elapsedTime = m_PlayTimeMSec - note.startTime;
		}

		//キーの状態更新
		//  TODO: 複数ポート対応
		if (note.portNo == 0) {
			//シングルキーボードでは複数チャンネルのキー状態を先頭チャンネルに集約する
			targetChNo = note.chNo;
			if (m_isSingleKeyboard) {
				targetChNo = 0;
			}
			
			//ノートの色
			noteColor = m_NoteDesign.GetNoteBoxColor(note.portNo, note.chNo, note.noteNo);
			
			//発音対象キーを回転
			//  すでに同一ノートに対して頂点を更新している場合
			//  押下率が前回よりも上回る場合に限り頂点を更新する
			if (m_KeyDownRate[targetChNo][note.noteNo] < m_pNoteStatus[i].keyDownRate) {
				result = m_pPianoKeyboard[targetChNo]->PushKey(
														note.noteNo,
														m_pNoteStatus[i].keyDownRate,
														elapsedTime,
														&noteColor
													);
				if (result != 0) goto EXIT;
				m_KeyDownRate[targetChNo][note.noteNo] = m_pNoteStatus[i].keyDownRate;
			}
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// 描画
//******************************************************************************
int MTPianoKeyboardCtrl::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
   )
{
	int result = 0;
	unsigned char chNo = 0;
	unsigned long count = 0;
	unsigned long dispNum = 0;

	if (!m_isEnable) goto EXIT;

	//キーボード最大表示数
	dispNum = SM_MAX_CH_NUM;
	if (m_KeyboardDesign.GetKeyboardMaxDispNum() < dispNum) {
		dispNum = m_KeyboardDesign.GetKeyboardMaxDispNum();
	}

	//キーボードの描画
	for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
		//キーボード表示数の制限を確認
		count++;
		if (dispNum < count) break;

		//キーボード描画
		result = m_pPianoKeyboard[chNo]->Draw(pD3DDevice);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// ピッチベンド反映：キーボードシフト
//******************************************************************************
float MTPianoKeyboardCtrl::_GetPichBendShiftPosX(
		unsigned char portNo,
		unsigned char chNo
	)
{
	float shift = 0.0f;
	short pitchBendValue = 0;
	unsigned char pitchBendSensitivity = SM_DEFAULT_PITCHBEND_SENSITIVITY;

	//チャンネルのピッチベンド情報
	pitchBendValue =       m_pNotePitchBend->GetValue(portNo, chNo);
	pitchBendSensitivity = m_pNotePitchBend->GetSensitivity(portNo, chNo);

	//ピッチベンドによるキーボードシフト量
	shift = m_KeyboardDesign.GetPitchBendShift(pitchBendValue, pitchBendSensitivity);

	return shift;
}

//******************************************************************************
// 解放
//******************************************************************************
void MTPianoKeyboardCtrl::Release()
{
	unsigned char chNo = 0;

	for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
		if (m_pPianoKeyboard[chNo] != NULL) {
			m_pPianoKeyboard[chNo]->Release();
			delete m_pPianoKeyboard[chNo];
			m_pPianoKeyboard[chNo] = NULL;
		}
	}

// >>> modify 20120728 yossiepon begin
	//20120728 yossiepon: delete を delete[] に修正
	delete[] m_pNoteStatus;
	m_pNoteStatus = NULL;
// <<< modify 20120728 yossiepon end
}

//******************************************************************************
// カレントチックタイム設定
//******************************************************************************
void MTPianoKeyboardCtrl::SetCurTickTime(
		unsigned long curTickTime
	)
{
	m_CurTickTime = curTickTime;
}

//******************************************************************************
// 演奏時間設定
//******************************************************************************
void MTPianoKeyboardCtrl::SetPlayTimeMSec(
		unsigned long playTimeMsec
	)
{
	m_PlayTimeMSec = playTimeMsec;
}

//******************************************************************************
// リセット
//******************************************************************************
void MTPianoKeyboardCtrl::Reset()
{
	int result = 0;
	unsigned long i = 0;
	SMNote note;
	unsigned char targetChNo = 0;

	m_PlayTimeMSec = 0;
	m_CurTickTime = 0;
	m_CurNoteIndex = 0;

	for (i = 0; i < MTPIANOKEYBOARD_MAX_ACTIVENOTE_NUM; i++) {
		if (m_pNoteStatus[i].isActive) {
			result = m_NoteListRT.GetNote(m_pNoteStatus[i].index, &note);
			//if (result != 0) goto EXIT;

			//TODO: 複数ポート対応
			if (note.portNo == 0) {
				//シングルキーボードでは複数チャンネルのキー状態を先頭チャンネルに集約する
				targetChNo = note.chNo;
				if (m_isSingleKeyboard) {
					targetChNo = 0;
				}
				result = m_pPianoKeyboard[targetChNo]->ResetKey(note.noteNo);
				//if (result != 0) goto EXIT;
			}
		}
		m_pNoteStatus[i].isActive = false;
		m_pNoteStatus[i].keyStatus = BeforeNoteON;
		m_pNoteStatus[i].index = 0;
		m_pNoteStatus[i].keyDownRate = 0.0f;
	}

	return;
}

//******************************************************************************
// 表示設定
//******************************************************************************
void MTPianoKeyboardCtrl::SetEnable(
		bool isEnable
	)
{
	m_isEnable = isEnable;
}

//******************************************************************************
// スキップ状態設定
//******************************************************************************
void MTPianoKeyboardCtrl::SetSkipStatus(
		bool isSkipping
	)
{
	m_isSkipping = isSkipping;
}


