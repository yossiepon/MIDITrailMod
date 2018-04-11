//******************************************************************************
//
// MIDITrail / MTNoteBox
//
// ノートボックス描画クラス
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTNoteBox.h"

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
MTNoteBox::MTNoteBox(void)
{
	m_CurTickTime = 0;
	m_CurNoteIndex = 0;
	m_ActiveNoteNum = 0;
	m_pNoteStatus = NULL;
	m_isSkipping = false;
	ZeroMemory(m_KeyDownRate, sizeof(float) * MT_NOTEBOX_MAX_PORT_NUM * SM_MAX_CH_NUM * SM_MAX_NOTE_NUM);
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTNoteBox::~MTNoteBox(void)
{
	Release();
}

//******************************************************************************
// 生成処理
//******************************************************************************
int MTNoteBox::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData,
		MTNotePitchBend* pNotePitchBend
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

	//トラック取得
	result = pSeqData->GetMergedTrack(&track);
	if (result != 0) goto EXIT;

	//ノートリスト取得
	result = track.GetNoteList(&m_NoteList);
	if (result != 0) goto EXIT;

	//ノートリスト取得：startTime, endTime はリアルタイム(msec)
	result = track.GetNoteListWithRealTime(&m_NoteListRT, pSeqData->GetTimeDivision());
	if (result != 0) goto EXIT;

	//全ノートボックス生成
	result = _CreateAllNoteBox(pD3DDevice);
	if (result != 0) goto EXIT;

	//発音中ノートボックス生成（バッファ確保）
	result = _CreateActiveNoteBox(pD3DDevice);
	if (result != 0) goto EXIT;

	//ノート情報配列生成
	result = _CreateNoteStatus();
	if (result != 0) goto EXIT;

	//ピッチベンド情報
	m_pNotePitchBend = pNotePitchBend;

EXIT:;
	return result;
}

//******************************************************************************
// 全ノートボックス生成
//******************************************************************************
int MTNoteBox::_CreateAllNoteBox(
		LPDIRECT3DDEVICE9 pD3DDevice
   )
{
	int result = 0;
	unsigned long vertexNum = 0;
	unsigned long indexNum = 0;
	MTNOTEBOX_VERTEX* pVertex = NULL;
	unsigned long* pIndex = NULL;
	unsigned long i = 0;
	D3DMATERIAL9 material;
	SMNote note;

	//プリミティブ初期化
	result = m_PrimitiveAllNotes.Initialize(
					sizeof(MTNOTEBOX_VERTEX),	//頂点サイズ
					_GetFVFFormat(),			//頂点FVFフォーマット
					D3DPT_TRIANGLELIST			//プリミティブ種別
				);
	if (result != 0) goto EXIT;

	//頂点バッファ生成
	vertexNum = NOTE_VERTEX_NUM * m_NoteList.GetSize();
	result = m_PrimitiveAllNotes.CreateVertexBuffer(pD3DDevice, vertexNum);
	if (result != 0) goto EXIT;

	//インデックスバッファ生成
	indexNum = NOTE_INDEX_NUM * m_NoteList.GetSize();
	result = m_PrimitiveAllNotes.CreateIndexBuffer(pD3DDevice, indexNum);
	if (result != 0) goto EXIT;

	//バッファのロック
	result = m_PrimitiveAllNotes.LockVertex((void**)&pVertex);
	if (result != 0) goto EXIT;
	result = m_PrimitiveAllNotes.LockIndex(&pIndex);
	if (result != 0) goto EXIT;

	//バッファに頂点とインデックスを書き込む
	for (i = 0; i < m_NoteList.GetSize(); i++) {
		result = m_NoteList.GetNote(i, &note);
		if (result != 0) goto EXIT;

		result = _CreateVertexOfNote(
						note,							//ノート情報
						&(pVertex[NOTE_VERTEX_NUM * i]),//頂点バッファ書き込み位置
						NOTE_VERTEX_NUM * i,			//頂点バッファインデックスオフセット
						&(pIndex[NOTE_INDEX_NUM * i])	//インデックスバッファ書き込み位置
					);
		if (result != 0) goto EXIT;
	}

	//バッファのロック解除
	result = m_PrimitiveAllNotes.UnlockVertex();
	if (result != 0) goto EXIT;
	result = m_PrimitiveAllNotes.UnlockIndex();
	if (result != 0) goto EXIT;

	//マテリアル作成
	_MakeMaterial(&material);
	m_PrimitiveAllNotes.SetMaterial(material);

EXIT:;
	return result;
}

//******************************************************************************
// 発音中ノートボックス生成（バッファ確保）
//******************************************************************************
int MTNoteBox::_CreateActiveNoteBox(
		LPDIRECT3DDEVICE9 pD3DDevice
   )
{
	int result = 0;
	SMTrack track;
	unsigned long vertexNum = 0;
	unsigned long indexNum = 0;
	MTNOTEBOX_VERTEX* pVertex = NULL;
	unsigned long* pIndex = NULL;
	unsigned long i = 0;
	D3DMATERIAL9 material;
	SMNote note;

	ZeroMemory(&note, sizeof(SMNote));

	m_CurTickTime = 0;
	m_CurNoteIndex = 0;

	//プリミティブ初期化
	result = m_PrimitiveActiveNotes.Initialize(
					sizeof(MTNOTEBOX_VERTEX),	//頂点サイズ
					_GetFVFFormat(),			//頂点FVFフォーマット
					D3DPT_TRIANGLELIST			//プリミティブ種別
				);
	if (result != 0) goto EXIT;

	//頂点バッファ生成
	vertexNum = NOTE_VERTEX_NUM * MTNOTEBOX_MAX_ACTIVENOTE_NUM;
	result = m_PrimitiveActiveNotes.CreateVertexBuffer(pD3DDevice, vertexNum);
	if (result != 0) goto EXIT;

	//インデックスバッファ生成
	indexNum = NOTE_INDEX_NUM * MTNOTEBOX_MAX_ACTIVENOTE_NUM;
	result = m_PrimitiveActiveNotes.CreateIndexBuffer(pD3DDevice, indexNum);
	if (result != 0) goto EXIT;

	//バッファのロック
	result = m_PrimitiveActiveNotes.LockVertex((void**)&pVertex);
	if (result != 0) goto EXIT;
	result = m_PrimitiveActiveNotes.LockIndex(&pIndex);
	if (result != 0) goto EXIT;

	//バッファに頂点とインデックスを書き込む
	for (i = 0; i < MTNOTEBOX_MAX_ACTIVENOTE_NUM; i++) {
		result = _CreateVertexOfNote(
						note,							//ノート情報
						&(pVertex[NOTE_VERTEX_NUM * i]),//頂点バッファ書き込み位置
						NOTE_VERTEX_NUM * i,			//頂点バッファインデックスオフセット
						&(pIndex[NOTE_INDEX_NUM * i])	//インデックスバッファ書き込み位置
					);
		if (result != 0) goto EXIT;
	}

	//バッファのロック解除
	result = m_PrimitiveActiveNotes.UnlockVertex();
	if (result != 0) goto EXIT;
	result = m_PrimitiveActiveNotes.UnlockIndex();
	if (result != 0) goto EXIT;

	//マテリアル作成
	_MakeMaterialForActiveNote(&material);
	m_PrimitiveActiveNotes.SetMaterial(material);

EXIT:;
	return result;
}

//******************************************************************************
// ノート情報配列生成
//******************************************************************************
int MTNoteBox::_CreateNoteStatus()
{
	int result = 0;
	unsigned long i = 0;

	//ノート情報配列生成
	try {
		m_pNoteStatus = new NoteStatus[MTNOTEBOX_MAX_ACTIVENOTE_NUM];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

	//ノート状態リスト初期化
	for (i = 0; i < MTNOTEBOX_MAX_ACTIVENOTE_NUM; i++) {
		m_pNoteStatus[i].isActive = false;
		m_pNoteStatus[i].isHide = false;
		m_pNoteStatus[i].keyStatus = BeforeNoteON;
		m_pNoteStatus[i].index = 0;
		m_pNoteStatus[i].keyDownRate = 0.0f;
		//m_pNoteStatus[i].startTime = 0;
	}

EXIT:;
	return result;
}

//******************************************************************************
// 移動
//******************************************************************************
int MTNoteBox::Transform(
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
// 発音中ノートの頂点処理
//******************************************************************************
int MTNoteBox::_TransformActiveNotes(
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
int MTNoteBox::_UpdateStatusOfActiveNotes(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	unsigned long i = 0;
	bool isFound = false;
	SMNote note;

	//波紋ディケイ・リリース時間(msec)
	unsigned long decayDuration = m_NoteDesign.GetRippleDecayDuration();
	unsigned long releaseDuration   = m_NoteDesign.GetRippleReleaseDuration();

	//ノート情報を更新する
	for (i = 0; i < MTNOTEBOX_MAX_ACTIVENOTE_NUM; i++) {
		if (m_pNoteStatus[i].isActive) {
			//ノート情報取得
			result = m_NoteListRT.GetNote(m_pNoteStatus[i].index, &note);
			if (result != 0) goto EXIT;

			//発音中ノート状態更新
			result = _UpdateNoteStatus(
							m_PlayTimeMSec,
							decayDuration,
							releaseDuration,
							note,
							&(m_pNoteStatus[i])
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
				if ((m_pNoteStatus[i].isActive)
				 && (m_pNoteStatus[i].index == m_CurNoteIndex)) {
					isFound = true;
					break;
				}
			}
			//空いているところに追加する
			if (!isFound) {
				for (i = 0; i < MTNOTEBOX_MAX_ACTIVENOTE_NUM; i++) {
					if (!(m_pNoteStatus[i].isActive)) {
						m_pNoteStatus[i].isActive = true;
						m_pNoteStatus[i].isHide = false;
						m_pNoteStatus[i].keyStatus = BeforeNoteON;
						m_pNoteStatus[i].index = m_CurNoteIndex;
						m_pNoteStatus[i].keyDownRate = 0.0f;
						//m_pNoteStatus[i].startTime = curTime;
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
int MTNoteBox::_UpdateNoteStatus(
		unsigned long playTimeMSec,
		unsigned long decayDuration,
		unsigned long releaseDuration,
		SMNote note,
		NoteStatus* pNoteStatus
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
int MTNoteBox::_UpdateVertexOfActiveNotes(
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
		if (m_pNoteStatus[i].isActive) {
			//ノート情報取得
			result = m_NoteList.GetNote(m_pNoteStatus[i].index, &note);
			if (result != 0) goto EXIT;

			//頂点更新
			result = _CreateVertexOfNote(
							note,										//ノート情報
							&(pVertex[NOTE_VERTEX_NUM * activeNoteNum]),//頂点バッファ書き込み位置
							NOTE_VERTEX_NUM * activeNoteNum,			//頂点バッファインデックスオフセット
							&(pIndex[NOTE_INDEX_NUM * activeNoteNum]),	//インデックスバッファ書き込み位置
							m_pNoteStatus[i].keyDownRate,				//ノート状態
							true										//ピッチベンド適用
						);
			if (result != 0) goto EXIT;

			//発音中ノートがピッチベンドで移動する場合
			//発音終了までオリジナルのノートを非表示にする
			if (!(m_pNoteStatus[i].isHide)) {
				if ((m_pNotePitchBend->GetValue(note.portNo, note.chNo) != 0)
				 && (m_pNotePitchBend->GetSensitivity(note.portNo, note.chNo) != 0)) {
					result = _HideNoteBox(m_pNoteStatus[i].index);
					if (result != 0) goto EXIT;
					m_pNoteStatus[i].isHide = true;
				}
			}

			activeNoteNum++;
			m_KeyDownRate[note.portNo][note.chNo][note.noteNo] = m_pNoteStatus[i].keyDownRate;
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
// 描画
//******************************************************************************
int MTNoteBox::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
   )
{
	int result = 0;

	//全ノートの描画
	result = m_PrimitiveAllNotes.Draw(pD3DDevice);
	if (result != 0) goto EXIT;

	//発音中ノートの描画
	if (m_ActiveNoteNum > 0) {
		result = m_PrimitiveActiveNotes.Draw(
						pD3DDevice,							//デバイス
						NULL,								//テクスチャ：なし
						(NOTE_INDEX_NUM/3)*m_ActiveNoteNum	//プリミティブ数
					);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// 解放
//******************************************************************************
void MTNoteBox::Release()
{
	m_PrimitiveAllNotes.Release();
	m_PrimitiveActiveNotes.Release();
	m_NoteList.Clear();

	delete [] m_pNoteStatus;
	m_pNoteStatus = NULL;
}

//******************************************************************************
// ノートボックスの頂点生成
//******************************************************************************
int MTNoteBox::_CreateVertexOfNote(
		SMNote note,
		MTNOTEBOX_VERTEX* pVertex,
		unsigned long vertexOffset,
		unsigned long* pIndex,
		float keyDownRate,
		bool isEnablePitchBend
	)
{
	int result = 0;
	unsigned long i;
	D3DXVECTOR3 vectorStartLU;
	D3DXVECTOR3 vectorStartRU;
	D3DXVECTOR3 vectorStartLD;
	D3DXVECTOR3 vectorStartRD;
	D3DXVECTOR3 vectorEndLU;
	D3DXVECTOR3 vectorEndRU;
	D3DXVECTOR3 vectorEndLD;
	D3DXVECTOR3 vectorEndRD;
	D3DXCOLOR color;
	short pitchBendValue = 0;
	unsigned char pitchBendSensitivity = SM_DEFAULT_PITCHBEND_SENSITIVITY;

	if (isEnablePitchBend) {
		pitchBendValue =       m_pNotePitchBend->GetValue(note.portNo, note.chNo);
		pitchBendSensitivity = m_pNotePitchBend->GetSensitivity(note.portNo, note.chNo);
	}

	//     +   1+----+3   +
	//    /|   / 上 /    /|         y x
	//   + | 0+----+2   + |右       |/
	// 左| +   7+----+5 | +      z--+0
	//   |/    / 下 /   |/
	//   +   6+----+4   + ← 4 が原点(0,0,0)
	//

	//ノートボックス頂点座標取得
	m_NoteDesign.GetNoteBoxVirtexPos(
			note.startTime,
			note.portNo,
			note.chNo,
			note.noteNo,
			&vectorStartLU,
			&vectorStartRU,
			&vectorStartLD,
			&vectorStartRD,
			pitchBendValue,
			pitchBendSensitivity
		);
	m_NoteDesign.GetNoteBoxVirtexPos(
			note.endTime,
			note.portNo,
			note.chNo,
			note.noteNo,
			&vectorEndLU,
			&vectorEndRU,
			&vectorEndLD,
			&vectorEndRD,
			pitchBendValue,
			pitchBendSensitivity
		);

	//頂点座標・・・法線が異なるので頂点を8個に集約できない
	//上の面
	pVertex[0].p = vectorStartLU;
	pVertex[1].p = vectorEndLU;
	pVertex[2].p = vectorStartRU;
	pVertex[3].p = vectorEndRU;
	//下の面
	pVertex[4].p = vectorStartRD;
	pVertex[5].p = vectorEndRD;
	pVertex[6].p = vectorStartLD;
	pVertex[7].p = vectorEndLD;
	//右の面
	pVertex[8].p  = pVertex[2].p;
	pVertex[9].p  = pVertex[3].p;
	pVertex[10].p = pVertex[4].p;
	pVertex[11].p = pVertex[5].p;
	//左の面
	pVertex[12].p = pVertex[6].p;
	pVertex[13].p = pVertex[7].p;
	pVertex[14].p = pVertex[0].p;
	pVertex[15].p = pVertex[1].p;
	//前の面
	pVertex[16].p = pVertex[0].p;
	pVertex[17].p = pVertex[2].p;
	pVertex[18].p = pVertex[6].p;
	pVertex[19].p = pVertex[4].p;
	//後の面
	pVertex[20].p = pVertex[3].p;
	pVertex[21].p = pVertex[1].p;
	pVertex[22].p = pVertex[5].p;
	pVertex[23].p = pVertex[7].p;

	//法線
	//上の面
	pVertex[0].n = D3DXVECTOR3( 0.0f, 1.0f, 0.0f);
	pVertex[1].n = D3DXVECTOR3( 0.0f, 1.0f, 0.0f);
	pVertex[2].n = D3DXVECTOR3( 0.0f, 1.0f, 0.0f);
	pVertex[3].n = D3DXVECTOR3( 0.0f, 1.0f, 0.0f);
	//下の面
	pVertex[4].n = D3DXVECTOR3( 0.0f,-1.0f, 0.0f);
	pVertex[5].n = D3DXVECTOR3( 0.0f,-1.0f, 0.0f);
	pVertex[6].n = D3DXVECTOR3( 0.0f,-1.0f, 0.0f);
	pVertex[7].n = D3DXVECTOR3( 0.0f,-1.0f, 0.0f);
	//右の面
	pVertex[8].n  = D3DXVECTOR3( 0.0f, 0.0f,-1.0f);
	pVertex[9].n  = D3DXVECTOR3( 0.0f, 0.0f,-1.0f);
	pVertex[10].n = D3DXVECTOR3( 0.0f, 0.0f,-1.0f);
	pVertex[11].n = D3DXVECTOR3( 0.0f, 0.0f,-1.0f);
	//左の面
	pVertex[12].n = D3DXVECTOR3( 0.0f, 0.0f, 1.0f);
	pVertex[13].n = D3DXVECTOR3( 0.0f, 0.0f, 1.0f);
	pVertex[14].n = D3DXVECTOR3( 0.0f, 0.0f, 1.0f);
	pVertex[15].n = D3DXVECTOR3( 0.0f, 0.0f, 1.0f);
	//前の面
	pVertex[16].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
	pVertex[17].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
	pVertex[18].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
	pVertex[19].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
	//後の面
	pVertex[20].n = D3DXVECTOR3( 1.0f, 0.0f, 0.0f);
	pVertex[21].n = D3DXVECTOR3( 1.0f, 0.0f, 0.0f);
	pVertex[22].n = D3DXVECTOR3( 1.0f, 0.0f, 0.0f);
	pVertex[23].n = D3DXVECTOR3( 1.0f, 0.0f, 0.0f);

	//各頂点のディフューズ色
	if (keyDownRate == 0.0f) {
		color = m_NoteDesign.GetNoteBoxColor(note.portNo, note.chNo, note.noteNo);
	}
	else {
		//発音中は発音開始からの経過時間によって色が変化する
		color = m_NoteDesign.GetActiveNoteBoxColor(note.portNo, note.chNo, note.noteNo, keyDownRate);
	}

	//頂点の色設定完了
	for (i = 0; i < NOTE_VERTEX_NUM; i++) {
		pVertex[i].c = color;
	}

	//インデックス：DrawIndexdPrimitive呼び出しが1回で済むようにTRIANGLELISTとする
	for (i = 0; i < NOTE_INDEX_NUM; i++) {
		pIndex[i] = vertexOffset + _GetVertexIndexOfNote(i);
	}

	return result;
}

//******************************************************************************
// ノート頂点インデックス取得
//******************************************************************************
unsigned long MTNoteBox::_GetVertexIndexOfNote(
		unsigned long index
	)
{
	unsigned long vertexIndex = 0;

	unsigned long vertexIndexes[NOTE_INDEX_NUM] = {
		//TRIANGLE-1   TRIANGLE-2
		 0,  1,  2,     2,  1,  3,	//上
		 4,  5,  6,     6,  5,  7,	//下
		 8,  9, 10,    10,  9, 11,	//右
		12, 13, 14,    14, 13, 15,	//左
		16, 17, 18,    18, 17, 19,	//前
		20, 21, 22,    22, 21, 23,	//後
	};

	if (index < NOTE_INDEX_NUM) {
		vertexIndex = vertexIndexes[index];
	}

	return vertexIndex;
}

//******************************************************************************
// マテリアル作成
//******************************************************************************
void MTNoteBox::_MakeMaterial(
		D3DMATERIAL9* pMaterial
	)
{
	ZeroMemory(pMaterial, sizeof(D3DMATERIAL9));
	
	//拡散光
	pMaterial->Diffuse.r = 1.0f;
	pMaterial->Diffuse.g = 1.0f;
	pMaterial->Diffuse.b = 1.0f;
	pMaterial->Diffuse.a = 1.0f;
	//環境光：影の色
	pMaterial->Ambient.r = 0.5f;
	pMaterial->Ambient.g = 0.5f;
	pMaterial->Ambient.b = 0.5f;
	pMaterial->Ambient.a = 1.0f;
	//鏡面反射光
	pMaterial->Specular.r = 0.2f;
	pMaterial->Specular.g = 0.2f;
	pMaterial->Specular.b = 0.2f;
	pMaterial->Specular.a = 1.0f;
	//鏡面反射光の鮮明度
	pMaterial->Power = 40.0f;
	//発光色
	pMaterial->Emissive.r = 0.0f;
	pMaterial->Emissive.g = 0.0f;
	pMaterial->Emissive.b = 0.0f;
	pMaterial->Emissive.a = 0.0f;
}

//******************************************************************************
// マテリアル作成：発音中ノート用
//******************************************************************************
void MTNoteBox::_MakeMaterialForActiveNote(
		D3DMATERIAL9* pMaterial
	)
{
	ZeroMemory(pMaterial, sizeof(D3DMATERIAL9));
	
	//拡散光
	pMaterial->Diffuse.r = 1.0f;
	pMaterial->Diffuse.g = 1.0f;
	pMaterial->Diffuse.b = 1.0f;
	pMaterial->Diffuse.a = 1.0f;
	//環境光：影の色
	pMaterial->Ambient.r = 0.5f;
	pMaterial->Ambient.g = 0.5f;
	pMaterial->Ambient.b = 0.5f;
	pMaterial->Ambient.a = 1.0f;
	//鏡面反射光
	pMaterial->Specular.r = 0.2f;
	pMaterial->Specular.g = 0.2f;
	pMaterial->Specular.b = 0.2f;
	pMaterial->Specular.a = 1.0f;
	//鏡面反射光の鮮明度
	pMaterial->Power = 40.0f;
	//発光色
	pMaterial->Emissive = m_NoteDesign.GetActiveNoteEmissive();
}

//******************************************************************************
// カレントチックタイム設定
//******************************************************************************
void MTNoteBox::SetCurTickTime(
		unsigned long curTickTime
	)
{
	m_CurTickTime = curTickTime;
}

//******************************************************************************
// 演奏時間設定
//******************************************************************************
void MTNoteBox::SetPlayTimeMSec(
		unsigned long playTimeMsec
	)
{
	m_PlayTimeMSec = playTimeMsec;
}

//******************************************************************************
// リセット
//******************************************************************************
void MTNoteBox::Reset()
{
	int result = 0;
	unsigned long i = 0;

	m_CurTickTime = 0;
	m_CurNoteIndex = 0;
	m_ActiveNoteNum = 0;

	for (i = 0; i < MTNOTEBOX_MAX_ACTIVENOTE_NUM; i++) {

		//非表示にしているノートを復元する
		if (m_pNoteStatus[i].isHide) {
			result = _ShowNoteBox(m_pNoteStatus[i].index);
			//if (result != 0) goto EXIT;
		}

		m_pNoteStatus[i].isActive = false;
		m_pNoteStatus[i].isHide = false;
		m_pNoteStatus[i].keyStatus = BeforeNoteON;
		m_pNoteStatus[i].index = 0;
		m_pNoteStatus[i].keyDownRate = 0.0f;
		//m_pNoteStatus[i].startTime = 0;
	}

	return;
}

//******************************************************************************
// ノートボックス非表示
//******************************************************************************
int MTNoteBox::_HideNoteBox(
		unsigned long index
	)
{
	int result = 0;
	unsigned long offset = 0;
	unsigned long size = 0;
	unsigned long i = 0;
	unsigned long* pIndex = NULL;

	offset = sizeof(unsigned long) * NOTE_INDEX_NUM * index;
	size = sizeof(unsigned long) * NOTE_INDEX_NUM;

	result = m_PrimitiveAllNotes.LockIndex(&pIndex, offset, size);
	if (result != 0) goto EXIT;

	for (i = 0; i < NOTE_INDEX_NUM; i++) {
		pIndex[i] = NOTE_VERTEX_NUM * index;
	}

	result = m_PrimitiveAllNotes.UnlockIndex();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ノートボックス表示
//******************************************************************************
int MTNoteBox::_ShowNoteBox(
		unsigned long index
	)
{
	int result = 0;
	unsigned long offset = 0;
	unsigned long size = 0;
	unsigned long i = 0;
	unsigned long* pIndex = NULL;

	offset = sizeof(unsigned long) * NOTE_INDEX_NUM * index;
	size = sizeof(unsigned long) * NOTE_INDEX_NUM;

	result = m_PrimitiveAllNotes.LockIndex(&pIndex, offset, size);
	if (result != 0) goto EXIT;

	for (i = 0; i < NOTE_INDEX_NUM; i++) {
		pIndex[i] = NOTE_VERTEX_NUM * index + _GetVertexIndexOfNote(i);
	}

	result = m_PrimitiveAllNotes.UnlockIndex();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// スキップ状態設定
//******************************************************************************
void MTNoteBox::SetSkipStatus(
		bool isSkipping
	)
{
	m_isSkipping = isSkipping;
}

