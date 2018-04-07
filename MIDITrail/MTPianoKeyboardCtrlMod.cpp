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
#include "MTNoteRippleMod.h"
#include "MTNoteLyrics.h"

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
	m_MaxKeyboardIndex = 0;
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

	//シーケンスデータ：ポートリスト取得
	result = pSeqData->GetPortList(&m_PortList);
	if (result != 0) goto EXIT;

	for (index = 0; index < SM_MAX_PORT_NUM; index++) {
		m_KeyboardIndex[index] = -1;
	}

	//シングルキーボードでない場合
	if (!isSingleKeyboard) {
		//ポート番号に昇順のキーボードインデックスを振る
		//ポート 0番 3番 5番 に出力する場合のインデックスはそれぞれ 0, 1, 2
		for (index = 0; index < m_PortList.GetSize(); index++) {
			m_PortList.GetPort(index, &portNo);
			m_KeyboardIndex[portNo] = keyboardIndex;
			keyboardIndex++;
			if(keyboardIndex == m_KeyboardDesignMod.GetKeyboardMaxDispNum()){
				break;
			}
		}
		m_MaxKeyboardIndex = (unsigned char)keyboardIndex;
	}
	//シングルキーボードの場合
	else {
		//キーボードデザインをシングルモードに設定
		m_KeyboardDesignMod.SetKeyboardSingle();
		//ポートとキーボードの対応を1:1に固定
		m_KeyboardIndex[0] = 0;
		m_MaxKeyboardIndex = 1;
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

	for (index = 0; index < m_MaxKeyboardIndex; index++) {
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
		float rollAngle
	)
{
	int result = 0;
	unsigned char portNo = 0;
	unsigned char chNo = 0;
	int index;
	D3DXVECTOR3 portWindowLU;
	D3DXVECTOR3 portWindowRU;
	D3DXVECTOR3 portWindowLD;
	D3DXVECTOR3 portWindowRD;
	D3DXVECTOR3 transformVector;
	D3DXVECTOR3 playbackPosVector;

	//アクティブポートフラグクリア
	for (index = 0; index < SM_MAX_PORT_NUM; index++) {
		m_isActivePort[index] = false;
	}

	//現在発音中ノートの頂点更新
	result = _TransformActiveNotes(pD3DDevice);
	if (result != 0) goto EXIT;

	//再生面頂点座標取得
	m_NoteDesignMod.GetPlaybackSectionVirtexPos(
			0,
			&portWindowLU,
			&portWindowRU,
			&portWindowLD,
			&portWindowRD
		);

	float boardHeight = portWindowLU.y - portWindowLD.y;
	float keyboardWidth = m_KeyboardDesignMod.GetPortOriginX() * -2.0f;

	float resizeSacle = boardHeight / keyboardWidth;

	float rippleSpacing = m_NoteDesignMod.GetRippleSpacing();
	float rippleMargin = rippleSpacing * (MTNOTELYRICS_MAX_LYRICS_NUM + MTNOTERIPPLE_MAX_RIPPLE_NUM); // * antiResizeScale;

	//移動ベクトル：再生面に追従する
	playbackPosVector = m_NoteDesignMod.GetWorldMoveVector();
	playbackPosVector.x += m_NoteDesignMod.GetPlayPosX(m_CurTickTime);

	unsigned char lastPortNo = 0;

	if (!m_isSingleKeyboard) {
		//シングルキーボードでない場合、最終ポート番号を取得
		m_PortList.GetPort(m_PortList.GetSize()-1, &lastPortNo);
	}
	else {
		//シングルキーボードの場合、最終ポート番号は0固定
		lastPortNo = 0;
	}

	for(portNo = 0; portNo <= lastPortNo; portNo ++) {

		//ポート番号からキーボードインデックスを取得
		//シングルキーボードの場合、インデックスは0固定
		int keyboardIndex = !m_isSingleKeyboard ? m_KeyboardIndex[portNo] : 0;

		if(keyboardIndex == -1) {
			continue;
		}

		//移動ベクトル：キーボード基準座標
		transformVector = m_KeyboardDesignMod.GetKeyboardBasePos(keyboardIndex, rippleMargin, boardHeight, rollAngle);

		//移動ベクトル：ピッチベンドシフトを反映
		transformVector.x += GetMaxPitchBendShift(portNo);

		//キーボード移動
		result = m_pPianoKeyboard[keyboardIndex]->Transform(pD3DDevice, transformVector, playbackPosVector, resizeSacle, portWindowLU.z, rollAngle);
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
		result = m_pPianoKeyboard[_GetKeyboardIndexFromNote(note)]->ResetKey(note.noteNo);
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
	unsigned char notePortNo;

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

		//シングルキーボードでない場合
		if (!m_isSingleKeyboard) {
			//ノートのポート番号を取得
			notePortNo = note.portNo;
		}
		//シングルキーボードの場合
		else {
			//ノートのポート番号を0固定に
			notePortNo = 0;
		}

		//発音対象キーを回転
		//  すでに同一ノートに対して頂点を更新している場合
		//  押下率が前回よりも上回る場合に限り頂点を更新する
		if (m_KeyDownRateMod[notePortNo][note.chNo][note.noteNo] < m_pNoteStatus[i].keyDownRate) {
			//複数チャンネルのキー状態をポート別に集約する
			result = m_pPianoKeyboard[_GetKeyboardIndexFromNote(note)]->PushKey(
																		note.chNo,
																		note.noteNo,
																		m_pNoteStatus[i].keyDownRate,
																		elapsedTime,
																		&noteColor
																	);
			if (result != 0) goto EXIT;
			m_KeyDownRateMod[notePortNo][note.chNo][note.noteNo] = m_pNoteStatus[i].keyDownRate;
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

	//レンダリングステート設定：Zバッファへの書き込みオフ
//	pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	//キーボードの描画
	for (index = 0; index < m_MaxKeyboardIndex; index++) {

		result = m_pPianoKeyboard[index]->Draw(pD3DDevice);
		if (result != 0) goto EXIT;
	}

	//レンダリングステート設定：Zバッファへの書き込みオン
//	pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

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
			result = m_pPianoKeyboard[_GetKeyboardIndexFromNote(note)]->ResetKey(note.noteNo);
			//if (result != 0) goto EXIT;
		}
		m_pNoteStatus[i].isActive = false;
		m_pNoteStatus[i].keyStatus = BeforeNoteON;
		m_pNoteStatus[i].index = 0;
		m_pNoteStatus[i].keyDownRate = 0.0f;
	}

	return;
}

int MTPianoKeyboardCtrlMod::_GetKeyboardIndexFromNote(const SMNote &note)
{
	//シングルキーボードでない場合
	if (!m_isSingleKeyboard) {
		//ノートのピアノ番号を取得
		return m_KeyboardIndex[note.portNo];
	}
	//シングルキーボードの場合
	else {
		//ノートのピアノ番号を0固定に
		return 0;
	}
}

//******************************************************************************
// ピッチベンドシフトの最大量を求める
//******************************************************************************
float MTPianoKeyboardCtrlMod::GetMaxPitchBendShift(unsigned char portNo) {

	float max = 0.0f;
	float cur = 0.0f;

	//シングルキーボードでない場合、指定のポート番号から求める
	//シングルキーボードの場合、シーケンスに含まれるポート番号すべてから求める
	int portListSize = !m_isSingleKeyboard ? 1 : m_PortList.GetSize();

	for (int i = 0; i < portListSize; i++) {

		if (m_isSingleKeyboard) {
			m_PortList.GetPort(i, &portNo);
		}

		if (!m_isActivePort[portNo]) {
			continue;
		}

		for (unsigned char chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {

			float pitchBendShift = _GetPichBendShiftPosX(portNo, chNo);
			if(max < fabs(pitchBendShift)) {

				cur = pitchBendShift;
				max = fabs(pitchBendShift);
			}
		}
	}

	return cur;
}