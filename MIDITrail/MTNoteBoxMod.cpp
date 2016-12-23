//******************************************************************************
//
// MIDITrail / MTNoteBoxMod
//
// ノートボックス描画Modクラス
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTNoteBoxMod.h"

using namespace YNBaseLib;


//******************************************************************************
// パラメータ定義
//******************************************************************************
//1ノートあたりの頂点数 = 1長方形4頂点 * 6面 
#define NOTE_VERTEX_NUM  (4 * 6)

//1ノートあたりのインデックス数 = 1三角形3頂点 * 2個 * 6面
#define NOTE_INDEX_NUM   (3 * 2 * 6)

//******************************************************************************
// コンストラクタ
//******************************************************************************
MTNoteBoxMod::MTNoteBoxMod(void) : MTNoteBox()
{
	m_pNoteStatusMod = NULL;
	ZeroMemory(m_KeyDownRate, sizeof(float) * MT_NOTEBOX_MAX_PORT_NUM * SM_MAX_CH_NUM * SM_MAX_NOTE_NUM);
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTNoteBoxMod::~MTNoteBoxMod(void)
{
	Release();
}

//******************************************************************************
// 生成処理
//******************************************************************************
int MTNoteBoxMod::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData,
		MTNotePitchBend* pNotePitchBend
	)
{
	int result = 0;
	SMTrack track;

	Release();

	// 基底クラスの生成処理を呼び出す
	result = MTNoteBox::Create(pD3DDevice, pSceneName, pSeqData, pNotePitchBend);
	if (result != 0) goto EXIT;

	//ノートデザインModオブジェクト初期化
	result = m_NoteDesignMod.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	//トラック取得
	result = pSeqData->GetMergedTrack(&track);
	if (result != 0) goto EXIT;

	//ノートリスト取得：startTime, endTime はリアルタイム(msec)
	result = track.GetNoteListWithRealTime(&m_NoteListRT, pSeqData->GetTimeDivision());
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ノート情報配列生成
//******************************************************************************
int MTNoteBoxMod::_CreateNoteStatus()
{
	int result = 0;
	unsigned long i = 0;

	//ノート情報配列生成
	try {
		m_pNoteStatusMod = new NoteStatusMod[MTNOTEBOX_MAX_ACTIVENOTE_NUM];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

	//ノート状態リスト初期化
	for (i = 0; i < MTNOTEBOX_MAX_ACTIVENOTE_NUM; i++) {
		m_pNoteStatusMod[i].isActive = false;
		m_pNoteStatusMod[i].isHide = false;
		m_pNoteStatusMod[i].index = 0;
		m_pNoteStatusMod[i].keyStatus = BeforeNoteON;
		m_pNoteStatusMod[i].keyDownRate = 0.0f;
	}

EXIT:;
	return result;
}

//******************************************************************************
// 移動
//******************************************************************************
int MTNoteBoxMod::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice,
		float rollAngle
	)
{
	int result = 0;
	D3DXVECTOR3 moveVector;
	D3DXMATRIX rotateMatrix;
	D3DXMATRIX moveMatrix;
	D3DXMATRIX worldMatrix;

	//現在発音中ノートの頂点生成
	result = _TransformActiveNotes(pD3DDevice);
	if (result != 0) goto EXIT;

	//行列初期化
	D3DXMatrixIdentity(&rotateMatrix);
	D3DXMatrixIdentity(&moveMatrix);
	D3DXMatrixIdentity(&worldMatrix);

	//回転行列
	//TODO: ini で切り替えられるようにする
	//D3DXMatrixRotationX(&rotateMatrix, D3DXToRadian(rollAngle + 180.0f));
	D3DXMatrixRotationX(&rotateMatrix, D3DXToRadian(rollAngle));

	//移動行列
	moveVector = m_NoteDesign.GetWorldMoveVector();
	D3DXMatrixTranslation(&moveMatrix, moveVector.x, moveVector.y, moveVector.z);

	//行列の合成
	D3DXMatrixMultiply(&worldMatrix, &rotateMatrix, &moveMatrix);

	//変換行列設定
	m_PrimitiveAllNotes.Transform(worldMatrix);
	m_PrimitiveActiveNotes.Transform(worldMatrix);

EXIT:;
	return result;
}

//******************************************************************************
// 発音中ノートの状態更新
//******************************************************************************
int MTNoteBoxMod::_UpdateStatusOfActiveNotes(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	unsigned long i = 0;
	bool isFound = false;
	SMNote note;

	//波紋ディケイ・リリース時間(msec)
	unsigned long decayDuration = m_NoteDesignMod.GetRippleDecayDuration();
	unsigned long releaseDuration   = m_NoteDesignMod.GetRippleReleaseDuration();

	//ノート情報を更新する
	for (i = 0; i < MTNOTEBOX_MAX_ACTIVENOTE_NUM; i++) {
		if (m_pNoteStatusMod[i].isActive) {
			//ノート情報取得
			result = m_NoteListRT.GetNote(m_pNoteStatusMod[i].index, &note);
			if (result != 0) goto EXIT;

			//発音中ノート状態更新
			result = _UpdateNoteStatus(
							m_PlayTimeMSec,
							decayDuration,
							releaseDuration,
							note,
							&(m_pNoteStatusMod[i])
						);
			if (result != 0) goto EXIT;
		}
	}

	//前回検索終了位置から発音開始ノートを検索
	while (m_CurNoteIndex < m_NoteList.GetSize()) {
		//ノート情報取得
		result = m_NoteList.GetNote(m_CurNoteIndex, &note);
		if (result != 0) goto EXIT;

		//現在チックタイムが発音開始チックタイムにたどりついていなければ検索終了
		if (m_CurTickTime < note.startTime) break;

		//発音中ノートを登録
		if ((note.startTime <= m_CurTickTime) && (m_CurTickTime <= note.endTime)) {
			//すでに登録済みなら何もしない
			isFound = false;
			for (i = 0; i < MTNOTEBOX_MAX_ACTIVENOTE_NUM; i++) {
				if ((m_pNoteStatusMod[i].isActive)
				 && (m_pNoteStatusMod[i].index == m_CurNoteIndex)) {
					isFound = true;
					break;
				}
			}
			//空いているところに追加する
			if (!isFound) {
				for (i = 0; i < MTNOTEBOX_MAX_ACTIVENOTE_NUM; i++) {
					if (!(m_pNoteStatusMod[i].isActive)) {
						m_pNoteStatusMod[i].isActive = true;
						m_pNoteStatusMod[i].isHide = false;
						m_pNoteStatusMod[i].index = m_CurNoteIndex;
						m_pNoteStatusMod[i].keyStatus = BeforeNoteON;
						m_pNoteStatusMod[i].keyDownRate = 0.0f;
						break;
					}
				}
			}
		}
		m_CurNoteIndex++;
	}

EXIT:;
	return result;
}

//******************************************************************************
// 発音中ノート状態更新
//******************************************************************************
int MTNoteBoxMod::_UpdateNoteStatus(
		unsigned long playTimeMSec,
		unsigned long decayDuration,
		unsigned long releaseDuration,
		SMNote note,
		NoteStatusMod* pNoteStatus
	)
{
	int result= 0;

	//発音終了ノート
	if(playTimeMSec > note.endTime) {
		if(pNoteStatus->isHide) {
			result = _ShowNoteBox(pNoteStatus->index);
			if (result != 0) goto EXIT;
		}
		//ノート情報を破棄
		pNoteStatus->isActive = false;
		pNoteStatus->isHide = false;
		pNoteStatus->keyStatus = BeforeNoteON;
		pNoteStatus->index = 0;
		pNoteStatus->keyDownRate = 0.0f;

		goto EXIT;
	}

	unsigned long noteLen = note.endTime - note.startTime;

	float decayRatio = 0.3f;
	float sustainRatio = 0.4f;
	float releaseRatio = 0.3f;

	//波紋ディケイ時間が発音長より長い場合、ディケイを消音時間までに修正する
	if(noteLen < decayDuration) {

		//decayDuration = noteLen;
		//releaseDuration = 0;

		//decayRatio = 0.3f;
		//sustainRatio = 0.0f;
		//releaseRatio = 0.0f;
	}
	//波紋ディケイ＋リリース時間が発音長より長い場合、リリース開始時間をディケイ時間経過直後に修正する
	else if(noteLen < (decayDuration + releaseDuration)) {

		releaseDuration = noteLen - decayDuration;
		decayRatio = 0.5f;
		sustainRatio = 0.0f;
		releaseRatio = 0.5f;
	}
	//発音長が（波紋ディケイ＋リリース時間）×２以内の場合、SustainRatioを0.0～0.4の範囲で変化させる
	else if(noteLen < (decayDuration + releaseDuration) * 2) {

		sustainRatio = 0.4f * (float)(noteLen - (decayDuration + releaseDuration)) / (float)noteLen;
		decayRatio = (1.0f - sustainRatio) / 2.0f;
		releaseRatio = decayRatio;
	}

	//ノートON後（減衰中）
	if (playTimeMSec < (note.startTime + decayDuration)) {
		pNoteStatus->keyStatus = BeforeNoteON;
		if (decayDuration == 0) {
			pNoteStatus->keyDownRate = 0.0f;
		}
		else {
			pNoteStatus->keyDownRate = decayRatio * (float)(playTimeMSec - note.startTime) / (float)decayDuration;
		}
	}
	//ノートON減衰後からリリース前まで
	else if (((note.startTime + decayDuration) <= playTimeMSec)
			&& (playTimeMSec <= (note.endTime - releaseDuration))) {
		pNoteStatus->keyStatus = NoteON;

		unsigned long denominator = noteLen - (decayDuration + releaseDuration);
		if(denominator > 0) {
			pNoteStatus->keyDownRate = decayRatio + sustainRatio
					* (float)(playTimeMSec - (note.startTime + decayDuration)) / (float)denominator;
		} else {
			pNoteStatus->keyDownRate = decayRatio + sustainRatio;
		}
	}
	//ノートOFF前（リリース中）
	else if (((note.endTime - releaseDuration) < playTimeMSec) && (playTimeMSec <= note.endTime)) {
		pNoteStatus->keyStatus = AfterNoteOFF;
		if (releaseDuration == 0) {
			pNoteStatus->keyDownRate = 1.0f;
		}
		else {
			pNoteStatus->keyDownRate = decayRatio + sustainRatio + releaseRatio
					* (float)(playTimeMSec - (note.endTime - releaseDuration)) / (float)releaseDuration;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// 発音中ノートの頂点更新
//******************************************************************************
int MTNoteBoxMod::_UpdateVertexOfActiveNotes(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	unsigned long i = 0;
	unsigned long activeNoteNum = 0;
	MTNOTEBOX_VERTEX* pVertex = NULL;
	unsigned long* pIndex = NULL;
	SMNote note;

	//バッファのロック
	result = m_PrimitiveActiveNotes.LockVertex((void**)&pVertex);
	if (result != 0) goto EXIT;
	result = m_PrimitiveActiveNotes.LockIndex(&pIndex);
	if (result != 0) goto EXIT;

	ZeroMemory(m_KeyDownRate, sizeof(float) * MT_NOTEBOX_MAX_PORT_NUM * SM_MAX_CH_NUM * SM_MAX_NOTE_NUM);

	//発音中ノートについて頂点を更新
	for (i = 0; i < MTNOTEBOX_MAX_ACTIVENOTE_NUM; i++) {
		if (m_pNoteStatusMod[i].isActive) {
			//ノート情報取得
			result = m_NoteList.GetNote(m_pNoteStatusMod[i].index, &note);
			if (result != 0) goto EXIT;

			//頂点更新
			result = _CreateVertexOfNote(
							note,										//ノート情報
							&(pVertex[NOTE_VERTEX_NUM * activeNoteNum]),//頂点バッファ書き込み位置
							NOTE_VERTEX_NUM * activeNoteNum,			//頂点バッファインデックスオフセット
							&(pIndex[NOTE_INDEX_NUM * activeNoteNum]),	//インデックスバッファ書き込み位置
							m_pNoteStatusMod[i].keyDownRate,				//ノート状態
							true										//ピッチベンド適用
						);
			if (result != 0) goto EXIT;

			//発音中ノートがピッチベンドで移動する場合
			//発音終了までオリジナルのノートを非表示にする
			if (!(m_pNoteStatusMod[i].isHide)) {
				if ((m_pNotePitchBend->GetValue(note.portNo, note.chNo) != 0)
				 && (m_pNotePitchBend->GetSensitivity(note.portNo, note.chNo) != 0)) {
					result = _HideNoteBox(m_pNoteStatusMod[i].index);
					if (result != 0) goto EXIT;
					m_pNoteStatusMod[i].isHide = true;
				}
			}

			activeNoteNum++;
			m_KeyDownRate[note.portNo][note.chNo][note.noteNo] = m_pNoteStatusMod[i].keyDownRate;
		}
	}
	m_ActiveNoteNum = activeNoteNum;

	//バッファのロック解除
	result = m_PrimitiveActiveNotes.UnlockVertex();
	if (result != 0) goto EXIT;
	result = m_PrimitiveActiveNotes.UnlockIndex();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 解放
//******************************************************************************
void MTNoteBoxMod::Release()
{
	if(m_pNoteStatusMod != NULL) {
		delete [] m_pNoteStatusMod;
		m_pNoteStatusMod = NULL;
	}
}

//******************************************************************************
// ノートボックスの頂点生成
//******************************************************************************
int MTNoteBoxMod::_CreateVertexOfNote(
		SMNote note,
		MTNOTEBOX_VERTEX* pVertex,
		unsigned long vertexOffset,
		unsigned long* pIndex,
		float keyDownRate,
		bool isEnablePitchBend
	)
{
	unsigned long i;
	D3DXCOLOR color;

	// 基底クラスの頂点生成処理の呼び出し
	MTNoteBox::_CreateVertexOfNote(note, pVertex, vertexOffset, pIndex, -1, isEnablePitchBend);

	//基底クラスの設定色を上書きする

	//各頂点のディフューズ色
	if (keyDownRate == 0.0f) {
		color = m_NoteDesign.GetNoteBoxColor(note.portNo, note.chNo, note.noteNo);
	}
	else {
		//発音中は発音開始からの経過時間によって色が変化する
		color = m_NoteDesignMod.GetActiveNoteBoxColor(note.portNo, note.chNo, note.noteNo, keyDownRate);
	}

	//頂点の色設定完了
	for (i = 0; i < NOTE_VERTEX_NUM; i++) {
		pVertex[i].c = color;
	}

	return 0;
}

//******************************************************************************
// 演奏時間設定
//******************************************************************************
void MTNoteBoxMod::SetPlayTimeMSec(
		unsigned long playTimeMsec
	)
{
	m_PlayTimeMSec = playTimeMsec;
}

//******************************************************************************
// リセット
//******************************************************************************
void MTNoteBoxMod::Reset()
{
	int result = 0;
	unsigned long i = 0;

	m_CurTickTime = 0;
	m_CurNoteIndex = 0;
	m_ActiveNoteNum = 0;

	for (i = 0; i < MTNOTEBOX_MAX_ACTIVENOTE_NUM; i++) {

		//非表示にしているノートを復元する
		if (m_pNoteStatusMod[i].isHide) {
			result = _ShowNoteBox(m_pNoteStatusMod[i].index);
			//if (result != 0) goto EXIT;
		}

		m_pNoteStatusMod[i].isActive = false;
		m_pNoteStatusMod[i].isHide = false;
		m_pNoteStatusMod[i].index = 0;
		m_pNoteStatusMod[i].keyStatus = BeforeNoteON;
		m_pNoteStatusMod[i].keyDownRate = 0.0f;
	}

	return;
}
