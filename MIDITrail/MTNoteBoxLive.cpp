//******************************************************************************
//
// MIDITrail / MTNoteBoxLive
//
// ライブモニタ用ノートボックス描画クラス
//
// Copyright (C) 2012-2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTNoteBoxLive.h"

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
MTNoteBoxLive::MTNoteBoxLive(void)
{
	m_pNoteDesign = NULL;
	m_NoteNum = 0;
	m_pNoteStatus = NULL;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTNoteBoxLive::~MTNoteBoxLive(void)
{
	Release();
}

//******************************************************************************
// 生成処理
//******************************************************************************
int MTNoteBoxLive::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName,
		MTNotePitchBend* pNotePitchBend
	)
{
	int result = 0;
	
	Release();

	//ノートデザインオブジェクト生成
	result = _CreateNoteDesign();
	if (result != 0) goto EXIT;

	//ノートデザインオブジェクト初期化
	result = m_pNoteDesign->Initialize(pSceneName, NULL);
	if (result != 0) goto EXIT;
	
	//ライブモニタ表示期限
	m_LiveMonitorDisplayDuration = m_pNoteDesign->GetLiveMonitorDisplayDuration();
	
	//ノート情報配列生成
	result = _CreateNoteStatus();
	if (result != 0) goto EXIT;
	
	//ノートボックス生成（バッファ確保）
	result = _CreateNoteBox(pD3DDevice);
	if (result != 0) goto EXIT;
	
	//ピッチベンド情報
	m_pNotePitchBend = pNotePitchBend;
	
EXIT:;
	return result;
}

//******************************************************************************
// ノートデザイン生成
//******************************************************************************
int MTNoteBoxLive::_CreateNoteDesign()
{
	int result = 0;

	try {
		m_pNoteDesign = new MTNoteDesign();
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// ノート情報配列生成
//******************************************************************************
int MTNoteBoxLive::_CreateNoteStatus()
{
	int result = 0;
	unsigned long i = 0;
	
	//ノート情報配列生成
	try {
		m_pNoteStatus = new NoteStatus[MTNOTEBOX_MAX_LIVENOTE_NUM];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}
	
	//ノート状態リスト初期化
	for (i = 0; i < MTNOTEBOX_MAX_LIVENOTE_NUM; i++) {
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
// 発音中ノートボックス生成（バッファ確保）
//******************************************************************************
int MTNoteBoxLive::_CreateNoteBox(
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
	NoteStatus note;
	
	memset(&note, 0, sizeof(NoteStatus));
	
	//プリミティブ初期化
	result = m_PrimitiveNotes.Initialize(
					sizeof(MTNOTEBOX_VERTEX),	//頂点サイズ
					_GetFVFFormat(),			//頂点FVFフォーマット
					D3DPT_TRIANGLELIST			//プリミティブ種別
				);
	if (result != 0) goto EXIT;
	
	//頂点バッファ生成
	vertexNum = NOTE_VERTEX_NUM * MTNOTEBOX_MAX_LIVENOTE_NUM;
	result = m_PrimitiveNotes.CreateVertexBuffer(pD3DDevice, vertexNum);
	if (result != 0) goto EXIT;
	
	//インデックスバッファ生成
	indexNum = NOTE_INDEX_NUM * MTNOTEBOX_MAX_LIVENOTE_NUM;
	result = m_PrimitiveNotes.CreateIndexBuffer(pD3DDevice, indexNum);
	if (result != 0) goto EXIT;
	
	//バッファのロック
	result = m_PrimitiveNotes.LockVertex((void**)&pVertex);
	if (result != 0) goto EXIT;
	result = m_PrimitiveNotes.LockIndex(&pIndex);
	if (result != 0) goto EXIT;
	
	//バッファに頂点とインデックスを書き込む
	for (i = 0; i < MTNOTEBOX_MAX_LIVENOTE_NUM; i++) {
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
int MTNoteBoxLive::Transform(
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
	D3DXMatrixRotationX(&rotateMatrix, D3DXToRadian(rollAngle));
	
	//移動行列
	moveVector = m_pNoteDesign->GetWorldMoveVector();
	D3DXMatrixTranslation(&moveMatrix, moveVector.x, moveVector.y, moveVector.z);
	
	//行列の合成
	D3DXMatrixMultiply(&worldMatrix, &rotateMatrix, &moveMatrix);
	
	//変換行列設定
	m_PrimitiveNotes.Transform(worldMatrix);
	
EXIT:;
	return result;
}

//******************************************************************************
// ノートの頂点処理
//******************************************************************************
int MTNoteBoxLive::_TransformNotes(
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
int MTNoteBoxLive::_UpdateStatusOfNotes(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	unsigned long i = 0;
	unsigned long curTime = 0;
	
	curTime = timeGetTime();
	
	//古いノート情報を破棄する
	for (i = 0; i < MTNOTEBOX_MAX_LIVENOTE_NUM; i++) {
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
int MTNoteBoxLive::_UpdateVertexOfNotes(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	unsigned long i = 0;
	unsigned long noteNum = 0;
	unsigned long curTime = 0;
	MTNOTEBOX_VERTEX* pVertex = NULL;
	unsigned long* pIndex = NULL;
	
	curTime = timeGetTime();
	
	//バッファのロック
	result = m_PrimitiveNotes.LockVertex((void**)&pVertex);
	if (result != 0) goto EXIT;
	result = m_PrimitiveNotes.LockIndex(&pIndex);
	if (result != 0) goto EXIT;
	
	//発音中ノートについて頂点を更新
	for (i = 0; i < MTNOTEBOX_MAX_LIVENOTE_NUM; i++) {
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
int MTNoteBoxLive::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
   )
{
	int result = 0;
	
	//発音中ノートの描画
	if (m_NoteNum > 0) {
		result = m_PrimitiveNotes.Draw(
						pD3DDevice,						//デバイス
						NULL,							//テクスチャ：なし
						(NOTE_INDEX_NUM/3)*m_NoteNum	//プリミティブ数
					);
		if (result != 0) goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// 解放
//******************************************************************************
void MTNoteBoxLive::Release()
{
	delete m_pNoteDesign;
	m_pNoteDesign = NULL;

	m_PrimitiveNotes.Release();
	
	delete [] m_pNoteStatus;
	m_pNoteStatus = NULL;
}

//******************************************************************************
// ノートボックスの頂点生成
//******************************************************************************
int MTNoteBoxLive::_CreateVertexOfNote(
		NoteStatus note,
		MTNOTEBOX_VERTEX* pVertex,
		unsigned long vertexOffset,
		unsigned long* pIndex,
		unsigned long curTime,
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
	unsigned long elapsedTime = 0;
	
	if ((isEnablePitchBend) && (note.endTime == 0)) {
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
	elapsedTime = curTime - note.startTime;
	if (elapsedTime > m_LiveMonitorDisplayDuration) {
		elapsedTime = m_LiveMonitorDisplayDuration;
	}
	m_pNoteDesign->GetNoteBoxVirtexPosLive(
			elapsedTime,
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
	
	elapsedTime = 0;
	if (note.endTime != 0) {
		elapsedTime = curTime - note.endTime;
	}
	m_pNoteDesign->GetNoteBoxVirtexPosLive(
			elapsedTime,
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
	if (note.endTime != 0) {
		color = m_pNoteDesign->GetNoteBoxColor(note.portNo, note.chNo, note.noteNo);
	}
	else {
		//発音中は発音開始からの経過時間によって色が変化する
		elapsedTime = curTime - note.startTime;
		color = m_pNoteDesign->GetActiveNoteBoxColor(note.portNo, note.chNo, note.noteNo, elapsedTime);
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
unsigned long MTNoteBoxLive::_GetVertexIndexOfNote(
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
void MTNoteBoxLive::_MakeMaterial(
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
// リセット
//******************************************************************************
void MTNoteBoxLive::Reset()
{
	unsigned long i = 0;
	
	m_NoteNum = 0;
	
	for (i = 0; i < MTNOTEBOX_MAX_LIVENOTE_NUM; i++) {
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
void MTNoteBoxLive::SetNoteOn(
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
	for (i = 0; i < MTNOTEBOX_MAX_LIVENOTE_NUM; i++) {
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
void MTNoteBoxLive::SetNoteOff(
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo
	)
{
	unsigned long i = 0;
	
	//該当のノート情報に終了時刻を設定
	for (i = 0; i < MTNOTEBOX_MAX_LIVENOTE_NUM; i++) {
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
void MTNoteBoxLive::AllNoteOff()
{
	unsigned long i = 0;
	
	//ノートOFFが設定されていないノート情報に終了時刻を設定
	for (i = 0; i < MTNOTEBOX_MAX_LIVENOTE_NUM; i++) {
		if ((m_pNoteStatus[i].isActive) && (m_pNoteStatus[i].endTime == 0)) {
			m_pNoteStatus[i].endTime = timeGetTime();
		}
	}
	
	return;
}

//******************************************************************************
// 全ノートOFF（チャンネル指定）
//******************************************************************************
void MTNoteBoxLive::AllNoteOffOnCh(
		unsigned char portNo,
		unsigned char chNo
	)
{
	unsigned long i = 0;
	
	//指定チャンネルでノートOFFが設定されていないノート情報に終了時刻を設定
	for (i = 0; i < MTNOTEBOX_MAX_LIVENOTE_NUM; i++) {
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
void MTNoteBoxLive::_ClearOldestNoteStatus(
		unsigned long* pCleardIndex
	)
{
	unsigned long i = 0;
	unsigned long oldestIndex = 0;
	bool isFind = false;
	
	//ノートON時刻が最も古いノート情報をクリアする
	for (i = 0; i < MTNOTEBOX_MAX_LIVENOTE_NUM; i++) {
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

