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
	m_MaxPortIndex = 0;
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
	unsigned long portIndex = 0;
	unsigned char portNo = 0;
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
	result = m_KeyboardDesignMod.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	//シーケンスデータ：ポートリスト取得
	result = pSeqData->GetPortList(&m_PortList);
	if (result != 0) goto EXIT;

	//ポート番号に昇順のインデックスを振る
	//ポート 0番 3番 5番 に出力する場合のインデックスはそれぞれ 0, 1, 2
	for (index = 0; index < SM_MAX_PORT_NUM; index++) {
		m_PortIndex[index] = -1;
	}
	for (index = 0; index < m_PortList.GetSize(); index++) {
		m_PortList.GetPort(index, &portNo);
		m_PortIndex[portNo] = portIndex;
		portIndex++;
		if(portIndex == m_KeyboardDesignMod.GetKeyboardMaxDispNum()){
			break;
		}
	}
	m_MaxPortIndex = (unsigned char)portIndex;

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
	//※フラグを受け取っても使用しない。ポート別シングルキーボードで常に動作する
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
	unsigned char portIndex = 0;
	LPDIRECT3DTEXTURE9 pTexture = NULL;

	for (portIndex = 0; portIndex < m_MaxPortIndex; portIndex++) {
		try {
			m_pPianoKeyboard[portIndex] = new MTPianoKeyboardMod;
		}
		catch (std::bad_alloc) {
			result = YN_SET_ERR("Could not allocate memory.", 0, 0);
			goto EXIT;
		}

		result = m_pPianoKeyboard[portIndex]->Create(pD3DDevice, pSceneName, pSeqData, pTexture);
		if (result != 0) goto EXIT;

		//先頭オブジェクトで作成したテクスチャを再利用する
		pTexture = m_pPianoKeyboard[portIndex]->GetTexture();
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
	D3DXVECTOR3 vectorLU;
	D3DXVECTOR3 vectorRU;
	D3DXVECTOR3 vectorLD;
	D3DXVECTOR3 vectorRD;
	D3DXVECTOR3 moveVector1;
	D3DXVECTOR3 moveVector2;

	//現在発音中ノートの頂点更新
	result = _TransformActiveNotes(pD3DDevice);
	if (result != 0) goto EXIT;

	//再生面頂点座標取得
	m_NoteDesign.GetPlaybackSectionVirtexPos(
			0,
			&vectorLU,
			&vectorRU,
			&vectorLD,
			&vectorRD
		);

	float boardHeight = vectorLU.y - vectorLD.y;
	float keyboardWidth = m_KeyboardDesignMod.GetPortOriginX(0) * -2.0f;

	//移動ベクトル：再生面に追従する
	moveVector2 = m_NoteDesign.GetWorldMoveVector();
	moveVector2.x += m_NoteDesign.GetPlayPosX(m_CurTickTime);

	unsigned char lastPortNo = 0;

	m_PortList.GetPort(m_PortList.GetSize()-1, &lastPortNo);

	for(portNo = 0; portNo <= lastPortNo; portNo ++) {

		int portIndex = m_PortIndex[portNo];

		if(portIndex == -1) {
			continue;
		}

		//ピッチベンドシフトの最大量を求める
		float maxAbsPitchBendShift = 0.0f;
		float curMaxPitchBendShift = 0.0f;

		for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {

			float pitchBendShift = _GetPichBendShiftPosX(portNo, chNo);
			if(maxAbsPitchBendShift < fabs(pitchBendShift)) {

				curMaxPitchBendShift = pitchBendShift;
				maxAbsPitchBendShift = fabs(pitchBendShift);
			}
		}

		//移動ベクトル：キーボード基準座標
		moveVector1 = m_KeyboardDesignMod.GetKeyboardBasePos(portIndex, 0, keyboardWidth / boardHeight);

		//移動ベクトル：ピッチベンドシフトを反映
		moveVector1.x += curMaxPitchBendShift;

		if(rollAngle < 0.0f) {
			rollAngle += 360.0f;
		}

		float portWidth =  m_KeyboardDesignMod.GetChStep() * 16.0f;

		if((rollAngle > 120.0f) && (rollAngle < 300.0f)) {

			//鍵盤の1/2の幅だけ高音側に
			moveVector1.x += m_KeyboardDesignMod.GetWhiteKeyStep() / 2.0f;

			//ポート原点Y
			moveVector1.y -= portWidth * (m_PortList.GetSize() - portIndex - 1) * (keyboardWidth / boardHeight);

			//鍵盤の原点をCh15に
			moveVector1.y -= m_KeyboardDesignMod.GetChStep()  * (keyboardWidth / boardHeight) * 15.0f;

			//鍵盤の1/4の高さだけ下に
			moveVector1.y -= m_KeyboardDesignMod.GetWhiteKeyHeight() / 4.0f;

			//鍵盤の長さ＋リップルマージン＋歌詞マージンだけ手前に
			moveVector1.z -= m_KeyboardDesignMod.GetWhiteKeyLen() + 0.002f * (MTNOTELYRICS_MAX_LYRICS_NUM + MTNOTERIPPLE_MAX_RIPPLE_NUM) * (keyboardWidth / boardHeight);

		} else {

			//鍵盤の1/2の幅だけ高音側に
			moveVector1.x += m_KeyboardDesignMod.GetWhiteKeyStep() / 2.0f;

			//ポート原点Y
			moveVector1.y += portWidth * (m_PortList.GetSize() - portIndex - 1) * (keyboardWidth / boardHeight);

			//鍵盤の1/4の高さだけ下に
			moveVector1.y -= m_KeyboardDesignMod.GetWhiteKeyHeight() / 4.0f;

			//リップルマージン＋歌詞マージンだけ奥に
			moveVector1.z += 0.002f * (MTNOTELYRICS_MAX_LYRICS_NUM + MTNOTERIPPLE_MAX_RIPPLE_NUM) * (keyboardWidth / boardHeight);
		}

		//キーボード移動
		result = m_pPianoKeyboard[portIndex]->Transform(pD3DDevice, moveVector1, moveVector2, boardHeight / keyboardWidth, vectorLU.z, rollAngle);
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
		//複数チャンネルのキー状態をポート別に集約する
		result = m_pPianoKeyboard[m_PortIndex[note.portNo]]->ResetKey(note.noteNo);
		if (result != 0) goto EXIT;

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
		noteColor = m_NoteDesign.GetNoteBoxColor(note.portNo, note.chNo, note.noteNo);
		
		//発音対象キーを回転
		//  すでに同一ノートに対して頂点を更新している場合
		//  押下率が前回よりも上回る場合に限り頂点を更新する
		if (m_KeyDownRateMod[note.portNo][note.chNo][note.noteNo] < m_pNoteStatus[i].keyDownRate) {
			//複数チャンネルのキー状態をポート別に集約する
			result = m_pPianoKeyboard[m_PortIndex[note.portNo]]->PushKey(
																	note.chNo,
																	note.noteNo,
																	m_pNoteStatus[i].keyDownRate,
																	elapsedTime,
																	&noteColor
																);
			if (result != 0) goto EXIT;
			m_KeyDownRateMod[note.portNo][note.chNo][note.noteNo] = m_pNoteStatus[i].keyDownRate;
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
	unsigned char portIndex = 0;
	unsigned long count = 0;
	unsigned long dispNum = 0;

	if (!m_isEnable) goto EXIT;

	//キーボードの描画
	for (portIndex = 0; portIndex < m_MaxPortIndex; portIndex++) {

		result = m_pPianoKeyboard[portIndex]->Draw(pD3DDevice);
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

			//複数チャンネルのキー状態をポート別に集約する
			result = m_pPianoKeyboard[m_PortIndex[note.portNo]]->ResetKey(note.noteNo);
			//if (result != 0) goto EXIT;
		}
		m_pNoteStatus[i].isActive = false;
		m_pNoteStatus[i].keyStatus = BeforeNoteON;
		m_pNoteStatus[i].index = 0;
		m_pNoteStatus[i].keyDownRate = 0.0f;
	}

	return;
}
