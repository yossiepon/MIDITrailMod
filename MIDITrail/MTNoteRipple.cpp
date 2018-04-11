//******************************************************************************
//
// MIDITrail / MTNoteRipple
//
// ノート波紋描画クラス
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// 波紋は、位置とサイズが個別に変化する。
// このため初回リリース前は、DrawPrimitiveUPで描画していた。
// つまり描画のたびに波紋の頂点をGPU側に流し込んでいた。
// しかし演奏開始後の初回の波紋描画時に限り、処理が引っかかる（FPSが
// 落ちる）現象が発生したため、この方式を取りやめた。
// 代わりに頂点バッファを使用し、描画のたびにバッファをロックして頂点
// を書き換える方式に切り替えた。
// もっとエレガントな方法があるかもしれない・・・。

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTNoteRipple.h"
#include <new>

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTNoteRipple::MTNoteRipple(void)
{
	m_pTexture = NULL;
	m_pNoteStatus = NULL;
	m_PlayTimeMSec = 0;
	m_CurTickTime = 0;
	m_CurNoteIndex = 0;
	m_ActiveNoteNum = 0;
	m_CamVector = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	ZeroMemory(&m_Material, sizeof(D3DMATERIAL9));
	m_isEnable = true;
	m_isSkipping = false;
	ZeroMemory(m_KeyDownRate, sizeof(float) * MTNOTERIPPLE_MAX_PORT_NUM * SM_MAX_CH_NUM * SM_MAX_NOTE_NUM);
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTNoteRipple::~MTNoteRipple(void)
{
	Release();
}

//******************************************************************************
// ノート波紋生成
//******************************************************************************
int MTNoteRipple::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData,
		MTNotePitchBend* pNotePitchBend
   )
{
	int result = 0;
	SMTrack track;

	Release();

	//ノートデザインオブジェクト初期化
	result = m_NoteDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	//テクスチャ生成
	result = _CreateTexture(pD3DDevice, pSceneName);
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

	//頂点生成
	result = _CreateVertex(pD3DDevice);
	if (result != 0) goto EXIT;

	//マテリアル作成
	_MakeMaterial(&m_Material);

	//ピッチベンド情報
	m_pNotePitchBend = pNotePitchBend;

EXIT:;
	return result;
}

//******************************************************************************
// 移動
//******************************************************************************
int MTNoteRipple::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice,
		D3DXVECTOR3 camVector,
		float rollAngle
	)
{
	int result = 0;
	D3DXVECTOR3 moveVector;
	D3DXMATRIX rotateMatrix;
	D3DXMATRIX moveMatrix;
	D3DXMATRIX worldMatrix;

	m_CamVector = camVector;

	//波紋の頂点更新
	result = _TransformRipple(pD3DDevice);
	if (result != 0) goto EXIT;

	//行列初期化
	D3DXMatrixIdentity(&rotateMatrix);
	D3DXMatrixIdentity(&moveMatrix);
	D3DXMatrixIdentity(&worldMatrix);

	//回転行列
	D3DXMatrixRotationX(&rotateMatrix, D3DXToRadian(rollAngle + 180.0f));

	//移動行列
	moveVector = m_NoteDesign.GetWorldMoveVector();
	D3DXMatrixTranslation(&moveMatrix, moveVector.x, moveVector.y, moveVector.z);

	//行列の合成
	D3DXMatrixMultiply(&worldMatrix, &rotateMatrix, &moveMatrix);

	//変換行列設定
	m_Primitive.Transform(worldMatrix);

EXIT:;
	return result;
}

//******************************************************************************
// 波紋の頂点更新
//******************************************************************************
int MTNoteRipple::_TransformRipple(
		LPDIRECT3DDEVICE9 pD3DDevice
   )
{
	int result = 0;

	//スキップ中なら何もしない
	if (m_isSkipping) goto EXIT;

	//波紋の状態更新
	result = _UpdateStatusOfRipple(pD3DDevice);
	if (result != 0) goto EXIT;

	//波紋の頂点更新
	result = _UpdateVertexOfRipple(pD3DDevice);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 波紋の状態更新
//******************************************************************************
int MTNoteRipple::_UpdateStatusOfRipple(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	unsigned long i = 0;
	bool isFound = false;
	bool isRegist = false;
	SMNote note;

	//波紋ディケイ・リリース時間(msec)
	unsigned long decayDuration = m_NoteDesign.GetRippleDecayDuration();
	unsigned long releaseDuration   = m_NoteDesign.GetRippleReleaseDuration();

	//ノート情報を更新する
	for (i = 0; i < MTNOTERIPPLE_MAX_RIPPLE_NUM; i++) {
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
	while (m_CurNoteIndex < m_NoteListRT.GetSize()) {
		//ノート情報取得
		result = m_NoteListRT.GetNote(m_CurNoteIndex, &note);
		if (result != 0) goto EXIT;

		//演奏時間がキー押下開始時間（発音開始直前）にたどりついていなければ検索終了
		if (m_PlayTimeMSec < note.startTime) {
			break;
		}

		//ノート情報登録判定
		isRegist = false;
		if ((note.startTime <= m_PlayTimeMSec) && (m_PlayTimeMSec <= note.endTime)) {
			isRegist = true;
		}

		//ノート情報登録
		//  キー下降中／上昇中の情報も登録対象としているため
		//  同一ノートで複数エントリされる場合があることに注意する
		if (isRegist) {
			//すでに同一インデックスで登録済みの場合は何もしない
			isFound = false;
			for (i = 0; i < MTNOTERIPPLE_MAX_RIPPLE_NUM; i++) {
				if ((m_pNoteStatus[i].isActive)
				 && (m_pNoteStatus[i].index == m_CurNoteIndex)) {
					isFound = true;
					break;
				}
			}
			//空いているところに追加する
			if (!isFound) {
				for (i = 0; i < MTNOTERIPPLE_MAX_RIPPLE_NUM; i++) {
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
							decayDuration,
							releaseDuration,
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
int MTNoteRipple::_UpdateNoteStatus(
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
		//ノート情報を破棄
		pNoteStatus->isActive = false;
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
	//発音長が（波紋ディケイ＋リリース時間）×２以内の場合、切り替え点をディケイ終了時間とリリース開始時間の中間にする
	else if(noteLen < (decayDuration + releaseDuration) * 2) {

		unsigned long midTime = (note.startTime + decayDuration) / 2 + (note.endTime - releaseDuration) / 2;

		decayDuration = midTime - note.startTime;
		releaseDuration = note.endTime - midTime;

		decayRatio = 0.5f;
		sustainRatio = 0.0f;
		releaseRatio = 0.5f;
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
// 波紋の頂点更新
//******************************************************************************
int MTNoteRipple::_UpdateVertexOfRipple(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	MTNOTERIPPLE_VERTEX* pVertex = NULL;
	D3DXMATRIX mtxWorld;
	unsigned long i = 0;
	unsigned long activeNoteNum = 0;
	bool isTimeout = false;

	if (pD3DDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//バッファのロック
	result = m_Primitive.LockVertex((void**)&pVertex);
	if (result != 0) goto EXIT;

	ZeroMemory(m_KeyDownRate, sizeof(float) * MTNOTERIPPLE_MAX_PORT_NUM * SM_MAX_CH_NUM * SM_MAX_NOTE_NUM);

	//発音中ノートの波紋について頂点を更新
	for (i = 0; i < MTNOTERIPPLE_MAX_RIPPLE_NUM; i++) {
		if (m_pNoteStatus[i].isActive) {
			//ノート情報取得
			SMNote note;
			result = m_NoteListRT.GetNote(m_pNoteStatus[i].index, &note);
			if (result != 0) goto EXIT;

			//発音対象キーを回転
			//  すでに同一ノートに対して頂点を更新している場合
			//  押下率が前回よりも上回る場合に限り頂点を更新する
			if ((note.portNo < MTNOTERIPPLE_MAX_PORT_NUM)
			 && (m_KeyDownRate[note.portNo][note.chNo][note.noteNo] < m_pNoteStatus[i].keyDownRate)) {
				//頂点更新：波紋の描画位置とサイズを変える
				_SetVertexPosition(
						&(pVertex[activeNoteNum*6]),	//頂点バッファ書き込み位置
						note,							//ノート情報
						&(m_pNoteStatus[i]),			//ノート状態
						i								//ノート状態登録インデックス位置
					);
		 		activeNoteNum++;
				_SetVertexPosition(
						&(pVertex[activeNoteNum*6]),	//頂点バッファ書き込み位置
						note,							//ノート情報
						&(m_pNoteStatus[i]),			//ノート状態
						i								//ノート状態登録インデックス位置
					);
		 		activeNoteNum++;
				_SetVertexPosition(
						&(pVertex[activeNoteNum*6]),	//頂点バッファ書き込み位置
						note,							//ノート情報
						&(m_pNoteStatus[i]),			//ノート状態
						i								//ノート状態登録インデックス位置
					);
				activeNoteNum++;

				m_KeyDownRate[note.portNo][note.chNo][note.noteNo] = m_pNoteStatus[i].keyDownRate;
			}

		}
	}
	m_ActiveNoteNum = activeNoteNum;

	//バッファのロック解除
	result = m_Primitive.UnlockVertex();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 描画
//******************************************************************************
int MTNoteRipple::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
   )
{
	int result = 0;

	if (pD3DDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	if (!m_isEnable) goto EXIT;

	//テクスチャステージ設定
	//  カラー演算：乗算  引数1：テクスチャ  引数2：ポリゴン
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	// アルファ演算：引数1を使用  引数1：ポリゴン
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	//テクスチャフィルタ
	pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	//レンダリングステート設定：加算合成
	//pD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	//プリミティブ描画
	if (m_ActiveNoteNum > 0) {
		//バッファ全体でなく波紋の数に合わせて描画するプリミティブを減らす
		result = m_Primitive.Draw(pD3DDevice, m_pTexture, 2 * m_ActiveNoteNum);
		if (result != 0) goto EXIT;
	}

	//レンダリングステート設定：通常のアルファ合成
	//pD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

EXIT:;
	return result;
}

//******************************************************************************
// 解放
//******************************************************************************
void MTNoteRipple::Release()
{
	m_Primitive.Release();

	if (m_pTexture != NULL) {
		m_pTexture->Release();
		m_pTexture = NULL;
	}

	delete [] m_pNoteStatus;
	m_pNoteStatus = NULL;
}

//******************************************************************************
// テクスチャ生成
//******************************************************************************
int MTNoteRipple::_CreateTexture(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;
	TCHAR imgFilePath[_MAX_PATH] = {_T('\0')};
	TCHAR bmpFileName[_MAX_PATH] = {_T('\0')};
	MTConfFile confFile;

	result = confFile.Initialize(pSceneName);
	if (result != 0) goto EXIT;

	//ビットマップファイル名
	result = confFile.SetCurSection(_T("Bitmap"));
	if (result != 0) goto EXIT;
	result = confFile.GetStr(_T("Ripple"), bmpFileName, _MAX_PATH, MT_IMGFILE_RIPPLE);
	if (result != 0) goto EXIT;

	//プロセス実行ファイルディレクトリパス取得
	result = YNPathUtil::GetModuleDirPath(imgFilePath, _MAX_PATH);
	if (result != 0) goto EXIT;

	//画像ファイルパス
	_tcscat_s(imgFilePath, _MAX_PATH, bmpFileName);

	hresult = D3DXCreateTextureFromFileEx(
					pD3DDevice,			//デバイス
					imgFilePath,		//ファイルパス
					0,					//幅：ファイルから取得
					0,					//高さ：ファイルから取得
					0,					//ミップレベル
					0,					//使用方法
					D3DFMT_A8R8G8B8,	//ピクセルフォーマット
					D3DPOOL_MANAGED,	//テクスチャ配置先メモリクラス
					D3DX_FILTER_LINEAR,	//フィルタリング指定
					D3DX_FILTER_LINEAR,	//フィルタリング指定（ミップ）
					0xFF000000,			//透明色の指定：不透明黒
					NULL,				//ソースイメージ情報
					NULL,				//256パレット情報
					&m_pTexture			//作成したテクスチャオブジェクト
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// ノート情報配列生成
//******************************************************************************
int MTNoteRipple::_CreateNoteStatus()
{
	int result = 0;
	unsigned long i = 0;

	//ノート情報配列生成
	try {
		m_pNoteStatus = new NoteStatus[MTNOTERIPPLE_MAX_RIPPLE_NUM];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

	ZeroMemory(m_pNoteStatus, sizeof(NoteStatus) * MTNOTERIPPLE_MAX_RIPPLE_NUM);

	for (i = 0; i < MTNOTERIPPLE_MAX_RIPPLE_NUM; i++) {
		m_pNoteStatus[i].isActive = false;
		m_pNoteStatus[i].keyStatus = BeforeNoteON;
		m_pNoteStatus[i].index = 0;
		m_pNoteStatus[i].keyDownRate = 0.0f;
	}

EXIT:;
	return result;
}

//******************************************************************************
// 頂点生成
//******************************************************************************
int MTNoteRipple::_CreateVertex(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	unsigned long vertexNum = 0;
	MTNOTERIPPLE_VERTEX* pVertex = NULL;

	//プリミティブ初期化
	result = m_Primitive.Initialize(
					sizeof(MTNOTERIPPLE_VERTEX),//頂点サイズ
					_GetFVFFormat(),			//頂点FVFフォーマット
					D3DPT_TRIANGLELIST			//プリミティブ種別
				);
	if (result != 0) goto EXIT;

	//頂点バッファ生成
	vertexNum = 6 * MTNOTERIPPLE_MAX_RIPPLE_NUM * 3;
	result = m_Primitive.CreateVertexBuffer(pD3DDevice, vertexNum);
	if (result != 0) goto EXIT;

	//バッファのロック
	result = m_Primitive.LockVertex((void**)&pVertex);
	if (result != 0) goto EXIT;

	ZeroMemory(pVertex, sizeof(MTNOTERIPPLE_VERTEX) * 6 * MTNOTERIPPLE_MAX_RIPPLE_NUM * 3);

	//バッファのロック解除
	result = m_Primitive.UnlockVertex();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 頂点の座標設定
//******************************************************************************
int MTNoteRipple::_SetVertexPosition(
		MTNOTERIPPLE_VERTEX* pVertex,
		SMNote note,
		NoteStatus* pNoteStatus,
		unsigned long rippleNo
	)
{
	int result = 0;
	unsigned long i = 0;
	float rh, rw = 0.0f;
	float alpha = 0.0f;
	D3DXVECTOR3 center;
	D3DXCOLOR color;
	short pbValue = 0;
	unsigned char pbSensitivity = SM_DEFAULT_PITCHBEND_SENSITIVITY;

	pbValue =       m_pNotePitchBend->GetValue(note.portNo, note.chNo);
	pbSensitivity = m_pNotePitchBend->GetSensitivity(note.portNo, note.chNo);

	//ノートボックス中心座標取得
	center = m_NoteDesign.GetNoteBoxCenterPosX(
					m_CurTickTime,
					note.portNo,
					note.chNo,
					note.noteNo,
					pbValue,
					pbSensitivity
				);

	//波紋サイズ
	rh = m_NoteDesign.GetRippleHeight(pNoteStatus->keyDownRate);
	rw = m_NoteDesign.GetRippleWidth(pNoteStatus->keyDownRate);

	//描画終了確認
	if ((rh <= 0.0f) || (rw <= 0.0f)) {
		goto EXIT;
	}

	//波紋を再生平面上からカメラ側に少しだけ浮かせて描画する
	//また波紋同士が同一平面上で重ならないように描画する
	//  Zファイティングによって発生するちらつきやかすれを回避する
	//  グラフィックカードによって現象が異なる
	if (center.x < m_CamVector.x) {
		center.x += (+(float)(rippleNo + 1) * 0.002f);
	}
	else {
		center.x += (-(float)(rippleNo + 1) * 0.002f);
	}

	//頂点座標
	pVertex[0].p = D3DXVECTOR3(center.x, center.y+(rh/2.0f), center.z+(rw/2.0f));
	pVertex[1].p = D3DXVECTOR3(center.x, center.y+(rh/2.0f), center.z-(rw/2.0f));
	pVertex[2].p = D3DXVECTOR3(center.x, center.y-(rh/2.0f), center.z+(rw/2.0f));
	pVertex[3].p = pVertex[2].p;
	pVertex[4].p = pVertex[1].p;
	pVertex[5].p = D3DXVECTOR3(center.x, center.y-(rh/2.0f), center.z-(rw/2.0f));

	//法線
	for (i = 0; i < 6; i++) {
		pVertex[i].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
	}

	//透明度を徐々に落とす
	alpha = m_NoteDesign.GetRippleAlpha(pNoteStatus->keyDownRate);

	//各頂点のディフューズ色
	for (i = 0; i < 6; i++) {
		color = m_NoteDesign.GetNoteBoxColor(
								note.portNo,
								note.chNo,
								note.noteNo
							);
		pVertex[i].c = D3DXCOLOR(color.r, color.g, color.b, alpha);
	}

	//テクスチャ座標
	pVertex[0].t = D3DXVECTOR2(0.0f, 0.0f);
	pVertex[1].t = D3DXVECTOR2(1.0f, 0.0f);
	pVertex[2].t = D3DXVECTOR2(0.0f, 1.0f);
	pVertex[3].t = pVertex[2].t;
	pVertex[4].t = pVertex[1].t;
	pVertex[5].t = D3DXVECTOR2(1.0f, 1.0f);

EXIT:
	return result;
}

//******************************************************************************
// マテリアル作成
//******************************************************************************
void MTNoteRipple::_MakeMaterial(
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
	pMaterial->Power = 10.0f;
	//発光色
	pMaterial->Emissive.r = 0.5f;
	pMaterial->Emissive.g = 0.5f;
	pMaterial->Emissive.b = 0.5f;
	pMaterial->Emissive.a = 1.0f;
}

////******************************************************************************
//// ノートOFF登録
////******************************************************************************
//void MTNoteRipple::SetNoteOff(
//		unsigned char portNo,
//		unsigned char chNo,
//		unsigned char noteNo
//	)
//{
//	unsigned long i = 0;
//
//	//該当のノート情報を無効化
//	for (i = 0; i < MTNOTERIPPLE_MAX_RIPPLE_NUM; i++) {
//		if ((m_pNoteStatus[i].isActive)
//		 && (m_pNoteStatus[i].portNo == portNo)
//		 && (m_pNoteStatus[i].chNo == chNo)
//		 && (m_pNoteStatus[i].noteNo == noteNo)) {
//			m_pNoteStatus[i].isActive = false;
//			break;
//		}
//	}
//
//	return;
//}
//
////******************************************************************************
//// ノートON登録
////******************************************************************************
//void MTNoteRipple::SetNoteOn(
//		unsigned char portNo,
//		unsigned char chNo,
//		unsigned char noteNo,
//		unsigned char velocity
//	)
//{
//	unsigned long i = 0;
//
//	//空きスペースにノート情報を登録
//	//空きが見つからなければ波紋の表示はあきらめる
//	for (i = 0; i < MTNOTERIPPLE_MAX_RIPPLE_NUM; i++) {
//		if (!(m_pNoteStatus[i].isActive)) {
//			m_pNoteStatus[i].isActive = true;
//		 	m_pNoteStatus[i].portNo = portNo;
//		 	m_pNoteStatus[i].chNo = chNo;
//		 	m_pNoteStatus[i].noteNo = noteNo;
//		 	m_pNoteStatus[i].velocity = velocity;
//		 	m_pNoteStatus[i].regTime = timeGetTime();
//			break;
//		}
//	}
//
//	return;
//}

//******************************************************************************
// カレントチックタイム設定
//******************************************************************************
void MTNoteRipple::SetCurTickTime(
		unsigned long curTickTime
	)
{
	m_CurTickTime = curTickTime;
}

//******************************************************************************
// 演奏時間設定
//******************************************************************************
void MTNoteRipple::SetPlayTimeMSec(
		unsigned long playTimeMsec
	)
{
	m_PlayTimeMSec = playTimeMsec;
}

//******************************************************************************
// リセット
//******************************************************************************
void MTNoteRipple::Reset()
{
	unsigned long i = 0;

	m_PlayTimeMSec = 0;
	m_CurTickTime = 0;
	m_CurNoteIndex = 0;
	m_ActiveNoteNum = 0;

	for (i = 0; i < MTNOTERIPPLE_MAX_RIPPLE_NUM; i++) {
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
void MTNoteRipple::SetEnable(
		bool isEnable
	)
{
	m_isEnable = isEnable;
}

//******************************************************************************
// スキップ状態設定
//******************************************************************************
void MTNoteRipple::SetSkipStatus(
		bool isSkipping
	)
{
	m_isSkipping = isSkipping;
}


