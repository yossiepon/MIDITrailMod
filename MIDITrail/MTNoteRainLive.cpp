//******************************************************************************
//
// MIDITrail / MTNoteRainLive
//
// ライブモニタ用ノートレイン描画クラス
//
// Copyright (C) 2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// Windows版のソースを移植しているため、座標は左手系(DirectX)で処理している。
// 左手系(DirectX)=>右手系(OpenGL)への変換は LH2RH マクロで実現する。

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTNoteRainLive.h"

using namespace YNBaseLib;


//******************************************************************************
// パラメータ定義
//******************************************************************************
//1ノートあたりの頂点数 = 1長方形4頂点 * 1面
#define NOTE_VERTEX_NUM  (4 * 1)

//1ノートあたりのインデックス数 = 1三角形3頂点 * 2個 * 1面
#define NOTE_INDEX_NUM   (3 * 2 * 1)

//******************************************************************************
// コンストラクタ
//******************************************************************************
MTNoteRainLive::MTNoteRainLive(void)
{
	m_NoteNum = 0;
	m_pNoteStatus = NULL;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTNoteRainLive::~MTNoteRainLive(void)
{
	Release();
}

//******************************************************************************
// 生成処理
//******************************************************************************
int MTNoteRainLive::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName,
		MTNotePitchBend* pNotePitchBend
	)
{
	int result = 0;
	
	Release();
	
	//ノートデザインオブジェクト初期化
	result = m_NoteDesign.Initialize(pSceneName, NULL);
	if (result != 0) goto EXIT;
	
	//キーボードデザインオブジェクト初期化
	result = m_KeyboardDesign.Initialize(pSceneName, NULL);
	if (result != 0) goto EXIT;
	
	//ライブモニタ表示期限
	m_LiveMonitorDisplayDuration = m_NoteDesign.GetLiveMonitorDisplayDuration();
	
	//ノート情報配列生成
	result = _CreateNoteStatus();
	if (result != 0) goto EXIT;
	
	//ノートボックス生成（バッファ確保）
	result = _CreateNoteRain(pD3DDevice);
	if (result != 0) goto EXIT;
	
	//ピッチベンド情報
	m_pNotePitchBend = pNotePitchBend;
	
EXIT:;
	return result;
}

//******************************************************************************
// ノート情報配列生成
//******************************************************************************
int MTNoteRainLive::_CreateNoteStatus()
{
	int result = 0;
	unsigned long i = 0;
	
	//ノート情報配列生成
	try {
		m_pNoteStatus = new NoteStatus[MTNOTERAIN_MAX_LIVENOTE_NUM];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}
	
	//ノート状態リスト初期化
	for (i = 0; i < MTNOTERAIN_MAX_LIVENOTE_NUM; i++) {
		m_pNoteStatus[i].isActive = false;
		m_pNoteStatus[i].portNo = 0;
		m_pNoteStatus[i].chNo = 0;
		m_pNoteStatus[i].noteNo = 0;
		m_pNoteStatus[i].startTime = 0;
		m_pNoteStatus[i].endTime = 0;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// 発音中ノートレイン生成（バッファ確保）
//******************************************************************************
int MTNoteRainLive::_CreateNoteRain(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	unsigned long vertexNum = 0;
	unsigned long indexNum = 0;
	MTNOTERAIN_VERTEX* pVertex = NULL;
	unsigned long* pIndex = NULL;
	unsigned long i = 0;
	D3DMATERIAL9 material;
	NoteStatus note;
	
	memset(&note, 0, sizeof(NoteStatus));
	
	//プリミティブ初期化
	result = m_PrimitiveNotes.Initialize(
					sizeof(MTNOTERAIN_VERTEX),	//頂点サイズ
					_GetFVFFormat(),			//頂点FVFフォーマット
					D3DPT_TRIANGLELIST			//プリミティブ種別
				);
	if (result != 0) goto EXIT;
	
	//頂点バッファ生成
	vertexNum = NOTE_VERTEX_NUM * MTNOTERAIN_MAX_LIVENOTE_NUM;
	result = m_PrimitiveNotes.CreateVertexBuffer(pD3DDevice, vertexNum);
	if (result != 0) goto EXIT;
	
	//インデックスバッファ生成
	indexNum = NOTE_INDEX_NUM * MTNOTERAIN_MAX_LIVENOTE_NUM;
	result = m_PrimitiveNotes.CreateIndexBuffer(pD3DDevice, indexNum);
	if (result != 0) goto EXIT;
	
	//バッファのロック
	result = m_PrimitiveNotes.LockVertex((void**)&pVertex);
	if (result != 0) goto EXIT;
	result = m_PrimitiveNotes.LockIndex(&pIndex);
	if (result != 0) goto EXIT;
	
	//バッファに頂点とインデックスを書き込む
	for (i = 0; i < MTNOTERAIN_MAX_LIVENOTE_NUM; i++) {
		result = _CreateVertexOfNote(
						m_pNoteStatus[i],				//ノート状態
						&(pVertex[NOTE_VERTEX_NUM * i]),//頂点バッファ書き込み位置
						NOTE_VERTEX_NUM * i,			//頂点バッファインデックスオフセット
						&(pIndex[NOTE_INDEX_NUM * i]),	//インデックスバッファ書き込み位置
						0								//現在時刻
					);
		if (result != 0) goto EXIT;
	}
	
	//バッファのロック解除
	result = m_PrimitiveNotes.UnlockVertex();
	if (result != 0) goto EXIT;
	result = m_PrimitiveNotes.UnlockIndex();
	if (result != 0) goto EXIT;
	
	//マテリアル作成
	_MakeMaterial(&material);
	m_PrimitiveNotes.SetMaterial(material);
	
EXIT:;
	return result;
}

//******************************************************************************
// 移動
//******************************************************************************
int MTNoteRainLive::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice,
		float rollAngle
	)
{
	int result = 0;
	D3DXVECTOR3 moveVector;
	D3DXMATRIX rotateMatrix;
	D3DXMATRIX moveMatrix;
	D3DXMATRIX worldMatrix;
	
	//ノートの頂点生成
	result = _TransformNotes(pD3DDevice);
	if (result != 0) goto EXIT;
	
	//行列初期化
	D3DXMatrixIdentity(&rotateMatrix);
	D3DXMatrixIdentity(&moveMatrix);
	D3DXMatrixIdentity(&worldMatrix);
	
	//回転行列
	D3DXMatrixRotationY(&rotateMatrix, D3DXToRadian(rollAngle));
	
	//移動行列
	//  ノートを移動させる場合
	moveVector = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	//  ノートを移動させずにカメラとキーボードを移動させる場合
	//moveVector = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	D3DXMatrixTranslation(&moveMatrix, moveVector.x, moveVector.y, moveVector.z);
	
	//行列の合成：移動→回転
	D3DXMatrixMultiply(&worldMatrix, &moveMatrix, &rotateMatrix);
	
	//変換行列設定
	m_PrimitiveNotes.Transform(worldMatrix);
	
EXIT:;
	return result;
}

//******************************************************************************
// ノートの頂点処理
//******************************************************************************
int MTNoteRainLive::_TransformNotes(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	
	//ノートの状態更新
	result = _UpdateStatusOfNotes(pD3DDevice);
	if (result != 0) goto EXIT;
	
	//ノートの頂点更新
	result = _UpdateVertexOfNotes(pD3DDevice);
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// ノートの状態更新
//******************************************************************************
int MTNoteRainLive::_UpdateStatusOfNotes(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	unsigned long i = 0;
	unsigned long curTime = 0;
	
	curTime = timeGetTime();
	
	//古いノート情報を破棄する
	for (i = 0; i < MTNOTERAIN_MAX_LIVENOTE_NUM; i++) {
		if (m_pNoteStatus[i].isActive) {
			if ((m_pNoteStatus[i].endTime != 0)
				&& ((curTime - m_pNoteStatus[i].endTime) > m_LiveMonitorDisplayDuration)) {
				m_pNoteStatus[i].isActive = false;
				m_pNoteStatus[i].portNo = 0;
				m_pNoteStatus[i].chNo = 0;
				m_pNoteStatus[i].noteNo = 0;
				m_pNoteStatus[i].startTime = 0;
				m_pNoteStatus[i].endTime = 0;
			}
		}
	}
	
//EXIT:;
	return result;
}

//******************************************************************************
// ノートの頂点更新
//******************************************************************************
int MTNoteRainLive::_UpdateVertexOfNotes(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	unsigned long i = 0;
	unsigned long noteNum = 0;
	unsigned long curTime = 0;
	MTNOTERAIN_VERTEX* pVertex = NULL;
	unsigned long* pIndex = NULL;
	
	curTime = timeGetTime();
	
	//バッファのロック
	result = m_PrimitiveNotes.LockVertex((void**)&pVertex);
	if (result != 0) goto EXIT;
	result = m_PrimitiveNotes.LockIndex(&pIndex);
	if (result != 0) goto EXIT;
	
	//発音中ノートについて頂点を更新
	for (i = 0; i < MTNOTERAIN_MAX_LIVENOTE_NUM; i++) {
		if (m_pNoteStatus[i].isActive) {
			//頂点更新
			result = _CreateVertexOfNote(
							m_pNoteStatus[i],						//ノート状態
							&(pVertex[NOTE_VERTEX_NUM * noteNum]),	//頂点バッファ書き込み位置
							NOTE_VERTEX_NUM * noteNum,				//頂点バッファインデックスオフセット
							&(pIndex[NOTE_INDEX_NUM * noteNum]),	//インデックスバッファ書き込み位置
							curTime,								//現在の時間
							true									//ピッチベンド適用
						);
			if (result != 0) goto EXIT;
			
			noteNum++;
		}
	}
	m_NoteNum = noteNum;
	
	//バッファのロック解除
	result = m_PrimitiveNotes.UnlockVertex();
	if (result != 0) goto EXIT;
	result = m_PrimitiveNotes.UnlockIndex();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 描画
//******************************************************************************
int MTNoteRainLive::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	DWORD value = 0;
	
	//レンダリングステートをカリングなしにする
	pD3DDevice->GetRenderState(D3DRS_CULLMODE, &value);
	pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	
	//発音中ノートの描画
	if (m_NoteNum > 0) {
		result = m_PrimitiveNotes.Draw(
						pD3DDevice,						//デバイス
						NULL,							//テクスチャ：なし
						(NOTE_INDEX_NUM/3)*m_NoteNum	//プリミティブ数
					);
		if (result != 0) goto EXIT;
	}
	
	//レンダリングステートを戻す
	pD3DDevice->SetRenderState(D3DRS_CULLMODE, value);
	
EXIT:;
	return result;
}

//******************************************************************************
// 解放
//******************************************************************************
void MTNoteRainLive::Release()
{
	m_PrimitiveNotes.Release();
	
	delete [] m_pNoteStatus;
	m_pNoteStatus = NULL;
}

//******************************************************************************
// ノートレインの頂点生成
//******************************************************************************
int MTNoteRainLive::_CreateVertexOfNote(
		NoteStatus note,
		MTNOTERAIN_VERTEX* pVertex,
		unsigned long vertexOffset,
		unsigned long* pIndex,
		unsigned long curTime,
		bool isEnablePitchBend
	)
{
	int result = 0;
	D3DXVECTOR3 startVector;
	D3DXVECTOR3 endVector;
	D3DXVECTOR3 moveVector;
	D3DXCOLOR color;
	float rainWidth = m_KeyboardDesign.GetBlackKeyWidth();
	float pitchBendShift = 0.0f;
	short pitchBendValue = 0;
	unsigned char pitchBendSensitivity = SM_DEFAULT_PITCHBEND_SENSITIVITY;
	unsigned long elapsedTime = 0;
	
	if ((isEnablePitchBend) && (note.endTime == 0)) {
		pitchBendValue =       m_pNotePitchBend->GetValue(note.portNo, note.chNo);
		pitchBendSensitivity = m_pNotePitchBend->GetSensitivity(note.portNo, note.chNo);
		pitchBendShift = m_KeyboardDesign.GetPitchBendShift(pitchBendValue, pitchBendSensitivity);
	}
	
	//ノートON座標
	elapsedTime = curTime - note.startTime;
	if (elapsedTime > m_LiveMonitorDisplayDuration) {
		elapsedTime = m_LiveMonitorDisplayDuration;
	}
	startVector.x = pitchBendShift;
	startVector.y = m_NoteDesign.GetLivePosX(elapsedTime);
	startVector.z = 0.0f;
	
	//ノートOFF座標
	elapsedTime = 0;
	if (note.endTime != 0) {
		elapsedTime = curTime - note.endTime;
	}
	endVector.x = pitchBendShift;
	endVector.y = m_NoteDesign.GetLivePosX(elapsedTime);
	endVector.z = 0.0f;
	
	//移動ベクトル
	moveVector    = m_KeyboardDesign.GetKeyboardBasePos(note.portNo, note.chNo);
	moveVector.x += m_KeyboardDesign.GetKeyCenterPosX(note.noteNo);
	moveVector.y += m_KeyboardDesign.GetWhiteKeyHeight() / 2.0f;
	moveVector.z += m_KeyboardDesign.GetNoteDropPosZ(note.noteNo);
	
	//座標更新
	startVector = startVector + moveVector;
	endVector   = endVector   + moveVector;
	
	//頂点
	pVertex[0].p = D3DXVECTOR3(startVector.x - rainWidth/2.0f, startVector.y, startVector.z);
	pVertex[1].p = D3DXVECTOR3(startVector.x + rainWidth/2.0f, startVector.y, startVector.z);
	pVertex[2].p = D3DXVECTOR3(endVector.x   + rainWidth/2.0f, endVector.y,   endVector.z);
	pVertex[3].p = D3DXVECTOR3(endVector.x   - rainWidth/2.0f, endVector.y,   endVector.z);
	
	//法線
	//実際の面の方向に合わせて(0,0,-1)とするとライトを適用したときに暗くなる
	//鍵盤の上面に合わせることで明るくする
	pVertex[0].n = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	pVertex[1].n = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	pVertex[2].n = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	pVertex[3].n = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	
	//色
	color = m_NoteDesign.GetNoteBoxColor(note.portNo, note.chNo, note.noteNo);
	pVertex[0].c = D3DXCOLOR(color.r, color.g, color.b, 1.0f);
	pVertex[1].c = D3DXCOLOR(color.r, color.g, color.b, 1.0f);
	pVertex[2].c = D3DXCOLOR(color.r, color.g, color.b, 0.5f); //ノートOFFに近づくほど半透明にする
	pVertex[3].c = D3DXCOLOR(color.r, color.g, color.b, 0.5f); //ノートOFFに近づくほど半透明にする
	
	//インデックス
	pIndex[0] = vertexOffset + 0;
	pIndex[1] = vertexOffset + 2;
	pIndex[2] = vertexOffset + 1;
	pIndex[3] = vertexOffset + 0;
	pIndex[4] = vertexOffset + 3;
	pIndex[5] = vertexOffset + 2;
	
	return result;
}

//******************************************************************************
// マテリアル作成
//******************************************************************************
void MTNoteRainLive::_MakeMaterial(
		D3DMATERIAL9* pMaterial
	)
{
	memset(pMaterial, 0, sizeof(D3DMATERIAL9));
	
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
// リセット
//******************************************************************************
void MTNoteRainLive::Reset()
{
	unsigned long i = 0;
	
	m_NoteNum = 0;
	
	for (i = 0; i < MTNOTERAIN_MAX_LIVENOTE_NUM; i++) {
		m_pNoteStatus[i].isActive = false;
		m_pNoteStatus[i].portNo = 0;
		m_pNoteStatus[i].chNo = 0;
		m_pNoteStatus[i].noteNo = 0;
		m_pNoteStatus[i].startTime = 0;
		m_pNoteStatus[i].endTime = 0;
	}
	
	return;
}

//******************************************************************************
// ノートON登録
//******************************************************************************
void MTNoteRainLive::SetNoteOn(
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		unsigned char velocity
	)
{
	unsigned long i = 0;
	unsigned long cleardIndex = 0;
	bool isFind = false;
	unsigned long curTime = 0;
	
	curTime = timeGetTime();
	
	//空きスペースにノート情報を登録
	//空きが見つからなければ登録をあきらめる
	for (i = 0; i < MTNOTERAIN_MAX_LIVENOTE_NUM; i++) {
		if (m_pNoteStatus[i].isActive) {
			if ((m_pNoteStatus[i].endTime != 0)
				&& ((curTime - m_pNoteStatus[i].endTime) > m_LiveMonitorDisplayDuration)) {
				m_pNoteStatus[i].isActive = false;
				cleardIndex = i;
				isFind = true;
				break;
			}
		}
		else {
			m_pNoteStatus[i].isActive = false;
			cleardIndex = i;
			isFind = true;
			break;
		}
	}
	
	//空きスペースが見つからない場合は最も古いノート情報をクリアする
	if (!isFind) {
		_ClearOldestNoteStatus(&cleardIndex);
	}
	
	//ノート情報登録
	m_pNoteStatus[cleardIndex].isActive = true;
	m_pNoteStatus[cleardIndex].portNo = portNo;
	m_pNoteStatus[cleardIndex].chNo = chNo;
	m_pNoteStatus[cleardIndex].noteNo = noteNo;
	m_pNoteStatus[cleardIndex].startTime = timeGetTime();
	m_pNoteStatus[cleardIndex].endTime = 0;
	
	return;
}

//******************************************************************************
// ノートOFF登録
//******************************************************************************
void MTNoteRainLive::SetNoteOff(
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo
	)
{
	unsigned long i = 0;
	
	//該当のノート情報に終了時刻を設定
	for (i = 0; i < MTNOTERAIN_MAX_LIVENOTE_NUM; i++) {
		if ((m_pNoteStatus[i].isActive)
			&& (m_pNoteStatus[i].portNo == portNo)
			&& (m_pNoteStatus[i].chNo == chNo)
			&& (m_pNoteStatus[i].noteNo == noteNo)
			&& (m_pNoteStatus[i].endTime == 0)) {
			m_pNoteStatus[i].endTime = timeGetTime();
			break;
		}
	}
	
	return;
}

//******************************************************************************
// 全ノートOFF
//******************************************************************************
void MTNoteRainLive::AllNoteOff()
{
	unsigned long i = 0;
	
	//ノートOFFが設定されていないノート情報に終了時刻を設定
	for (i = 0; i < MTNOTERAIN_MAX_LIVENOTE_NUM; i++) {
		if ((m_pNoteStatus[i].isActive) && (m_pNoteStatus[i].endTime == 0)) {
			m_pNoteStatus[i].endTime = timeGetTime();
		}
	}
	
	return;
}

//******************************************************************************
// 全ノートOFF（チャンネル指定）
//******************************************************************************
void MTNoteRainLive::AllNoteOffOnCh(
		unsigned char portNo,
		unsigned char chNo
	)
{
	unsigned long i = 0;
	
	//指定チャンネルでノートOFFが設定されていないノート情報に終了時刻を設定
	for (i = 0; i < MTNOTERAIN_MAX_LIVENOTE_NUM; i++) {
		if ((m_pNoteStatus[i].isActive) && (m_pNoteStatus[i].endTime == 0)
			&& (m_pNoteStatus[i].portNo == portNo) && (m_pNoteStatus[i].chNo == chNo)) {
			m_pNoteStatus[i].endTime = timeGetTime();
		}
	}
	
	return;
}

//******************************************************************************
// 最も古いノート情報のクリア
//******************************************************************************
void MTNoteRainLive::_ClearOldestNoteStatus(
		unsigned long* pCleardIndex
	)
{
	unsigned long i = 0;
	unsigned long oldestIndex = 0;
	bool isFind = false;
	
	//ノートON時刻が最も古いノート情報をクリアする
	for (i = 0; i < MTNOTERAIN_MAX_LIVENOTE_NUM; i++) {
		if (m_pNoteStatus[i].isActive) {
			if (!isFind) {
				//有効なノートが現れた場合：初回
				oldestIndex = i;
				isFind = true;
			}
			else {
				//有効なノートが現れた場合：2回目以降
				if (m_pNoteStatus[i].startTime < m_pNoteStatus[oldestIndex].startTime) {
					oldestIndex = i;
				}
			}
		}
	}
	
	//ノート情報クリア
	//  ノートが一つも登録されていない場合は配列の先頭がクリア対象となる
	m_pNoteStatus[oldestIndex].isActive = false;
	*pCleardIndex = oldestIndex;
	
	return;
}

