//******************************************************************************
//
// MIDITrail / MTPianoKeyboardCtrlMod
//
// ピアノキーボード制御Modクラス
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTPianoKeyboardCtrlMod.h"
#include "MTPianoKeyboardMod.h"

using namespace YNBaseLib;


//******************************************************************************
// パラメータ定義
//******************************************************************************
#define MTPIANOKEYBOARD_MAX_ACTIVENOTE_NUM (256)


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTPianoKeyboardCtrlMod::MTPianoKeyboardCtrlMod(void)
{
	ZeroMemory(m_KeyDownRateMod, sizeof(float) * SM_MAX_CH_NUM* SM_MAX_CH_NUM * SM_MAX_NOTE_NUM);
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTPianoKeyboardCtrlMod::~MTPianoKeyboardCtrlMod(void)
{
	Release();
}

//******************************************************************************
// 生成処理
//******************************************************************************
int MTPianoKeyboardCtrlMod::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData,
		MTNotePitchBend* pNotePitchBend,
		bool isSingleKeyboard
	)
{
	int result = 0;
	unsigned long index = 0;
	unsigned long keyboardIndex = 0;
	unsigned char portNo = 0;
	SMTrack track;

	Release();

	if (pSeqData == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//ノートデザインオブジェクト初期化
	result = m_NoteDesignMod.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	//キーボードデザイン初期化
	result = m_KeyboardDesignMod.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	if (isSingleKeyboard) {
		m_KeyboardDesignMod.SetKeyboardSingle();
	}

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
// キーボード描画オブジェクト生成
//******************************************************************************
int MTPianoKeyboardCtrlMod::_CreateKeyboards(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;
	unsigned char index = 0;
	LPDIRECT3DTEXTURE9 pTexture = NULL;

	for (index = 0; index < m_KeyboardDesignMod.GetKeyboardDispNum(); index++) {
		try {
			m_pPianoKeyboard[index] = new MTPianoKeyboardMod;
		}
		catch (std::bad_alloc) {
			result = YN_SET_ERR("Could not allocate memory.", 0, 0);
			goto EXIT;
		}

		result = m_pPianoKeyboard[index]->Create(pD3DDevice, pSceneName, pSeqData, pTexture);
		if (result != 0) goto EXIT;

		//先頭オブジェクトで作成したテクスチャを再利用する
		pTexture = m_pPianoKeyboard[index]->GetTexture();
	}

EXIT:;
	return result;
}

//******************************************************************************
// 移動
//******************************************************************************
int MTPianoKeyboardCtrlMod::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice,
		D3DXVECTOR3 camVector,
		float rollAngle
	)
{
	int result = 0;
	unsigned char portNo = 0;
	unsigned char chNo = 0;
	int index;
	D3DXVECTOR3 basePosVector;
	D3DXVECTOR3 playbackPosVector;

	//アクティブポートフラグクリア
	for (index = 0; index < SM_MAX_PORT_NUM; index++) {
		m_isActivePort[index] = false;
	}

	//現在発音中ノートの頂点更新
	result = _TransformActiveNotes(pD3DDevice);
	if (result != 0) goto EXIT;

	//移動ベクトル：再生面に追従する
	playbackPosVector = m_NoteDesignMod.GetWorldMoveVector();
	playbackPosVector.x += m_NoteDesignMod.GetPlayPosX(m_CurTickTime);

	for(index = 0; index < m_KeyboardDesignMod.GetKeyboardDispNum(); index ++) {

		//移動ベクトル：キーボード基準座標
		basePosVector = m_KeyboardDesignMod.GetKeyboardBasePos(index, rollAngle);

		//移動ベクトル：ピッチベンドシフトを反映
		basePosVector.x += _GetMaxPitchBendShift(index);

		//キーボード移動
		result = m_pPianoKeyboard[index]->Transform(pD3DDevice, basePosVector, playbackPosVector, rollAngle);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// 発音中ノート状態更新
//******************************************************************************
int MTPianoKeyboardCtrlMod::_UpdateNoteStatus(
		unsigned long playTimeMSec,
		unsigned long keyDownDuration,
		unsigned long keyUpDuration,
		SMNote note,
		NoteStatus* pNoteStatus
	)
{
	int result= 0;

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
		//発音中のキーをリセットする
		result = m_pPianoKeyboard[m_KeyboardDesignMod.GetKeyboardIndex(note)]->ResetKey(note.noteNo);
		if (result != 0) goto EXIT;

		pNoteStatus->isActive = false;
		pNoteStatus->keyStatus = BeforeNoteON;
		pNoteStatus->index = 0;
		pNoteStatus->keyDownRate = 0.0f;
	}

	//状態更新後、発音中であれば
	if (pNoteStatus->isActive) {
		//アクティブポートフラグを立てる
		m_isActivePort[note.portNo] = true;
	}

EXIT:;
	return result;
}

//******************************************************************************
// 発音中ノートの頂点更新
//******************************************************************************
int MTPianoKeyboardCtrlMod::_UpdateVertexOfActiveNotes(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	unsigned long i = 0;
	unsigned long elapsedTime = 0;
	SMNote note;
	D3DXCOLOR noteColor;

	ZeroMemory(m_KeyDownRateMod, sizeof(float) * SM_MAX_CH_NUM* SM_MAX_CH_NUM * SM_MAX_NOTE_NUM);

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

		//ノートの色
		noteColor = m_NoteDesignMod.GetNoteBoxColor(note.portNo, note.chNo, note.noteNo);

		int portNo = m_KeyboardDesignMod.GetPortNo(note);
		int keyboardIndex = m_KeyboardDesignMod.GetKeyboardIndex(note);

		//発音対象キーを回転
		//  すでに同一ノートに対して頂点を更新している場合
		//  押下率が前回よりも上回る場合に限り頂点を更新する
		if (m_KeyDownRateMod[portNo][note.chNo][note.noteNo] < m_pNoteStatus[i].keyDownRate) {
			//複数チャンネルのキー状態をポート別に集約する
			result = m_pPianoKeyboard[keyboardIndex]->PushKey(
					note.chNo,
					note.noteNo,
					m_pNoteStatus[i].keyDownRate,
					elapsedTime,
					&noteColor
				);
			if (result != 0) goto EXIT;
			m_KeyDownRateMod[portNo][note.chNo][note.noteNo] = m_pNoteStatus[i].keyDownRate;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// 描画
//******************************************************************************
int MTPianoKeyboardCtrlMod::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
   )
{
	int result = 0;
	unsigned char index = 0;
	unsigned long count = 0;
	unsigned long dispNum = 0;

	if (!m_isEnable) goto EXIT;

	//キーボードの描画
	for (index = 0; index < m_KeyboardDesignMod.GetKeyboardDispNum(); index++) {

		result = m_pPianoKeyboard[index]->Draw(pD3DDevice);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// リセット
//******************************************************************************
void MTPianoKeyboardCtrlMod::Reset()
{
	int result = 0;
	unsigned long i = 0;
	SMNote note;

	m_PlayTimeMSec = 0;
	m_CurTickTime = 0;
	m_CurNoteIndex = 0;

	for (i = 0; i < MTPIANOKEYBOARD_MAX_ACTIVENOTE_NUM; i++) {
		if (m_pNoteStatus[i].isActive) {
			result = m_NoteListRT.GetNote(m_pNoteStatus[i].index, &note);
			//if (result != 0) goto EXIT;

			//発音中のキーをリセットする
			result = m_pPianoKeyboard[m_KeyboardDesignMod.GetKeyboardIndex(note)]->ResetKey(note.noteNo);
			//if (result != 0) goto EXIT;
		}
		m_pNoteStatus[i].isActive = false;
		m_pNoteStatus[i].keyStatus = BeforeNoteON;
		m_pNoteStatus[i].index = 0;
		m_pNoteStatus[i].keyDownRate = 0.0f;
	}

	return;
}

//******************************************************************************
// ピッチベンドシフトの最大量を求める
//******************************************************************************
float MTPianoKeyboardCtrlMod::_GetMaxPitchBendShift(int keyboardIndex) {

	if (!m_KeyboardDesignMod.IsKeyboardSingle()) {
		return _GetMaxPitchBendShift(keyboardIndex, 0.0f);
	}

	float max = 0.0f;

	int portListSize = m_KeyboardDesignMod.GetPortListSize();

	for (int i = 0; i < portListSize; i++) {

		max = _GetMaxPitchBendShift(i, max);
	}

	return max;
}

float MTPianoKeyboardCtrlMod::_GetMaxPitchBendShift(int keyboardIndex, float max) {

	unsigned char portNo = m_KeyboardDesignMod.GetPortNoFromKeyboardIndex(keyboardIndex);

	if (!m_isActivePort[portNo]) {
		return max;
	}

	for (unsigned char chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {

		float pitchBendShift = _GetPichBendShiftPosX(portNo, chNo);

		if(fabs(max) < fabs(pitchBendShift)) {

			max = pitchBendShift;
		}
	}

	return max;
}