//******************************************************************************
//
// MIDITrail / MTPictBoard
//
// ピクチャボード描画クラス
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTPictBoard.h"

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTPictBoard::MTPictBoard(void)
{
	m_pTexture = NULL;
	ZeroMemory(&m_ImgInfo, sizeof(D3DXIMAGE_INFO));
	m_CurTickTime = 0;
	m_isPlay = false;
	m_isEnable = true;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTPictBoard::~MTPictBoard(void)
{
	Release();
}

//******************************************************************************
// ピクチャボード生成
//******************************************************************************
int MTPictBoard::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;
	SMBarList barList;
	unsigned long vertexNum = 0;
	unsigned long indexNum = 0;
	MTPICTBOARD_VERTEX* pVertex = NULL;
	unsigned long* pIndex = NULL;

	Release();

	//ノートデザインオブジェクト初期化
	result = m_NoteDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	//テクスチャ読み込み
	result = _LoadTexture(pD3DDevice, pSceneName);
	if (result != 0) goto EXIT;

	//プリミティブ初期化
	result = m_Primitive.Initialize(
					sizeof(MTPICTBOARD_VERTEX),	//頂点サイズ
					_GetFVFFormat(),			//頂点FVFフォーマット
					D3DPT_TRIANGLESTRIP			//プリミティブ種別
				);
	if (result != 0) goto EXIT;

	//頂点バッファ生成
	vertexNum = 4;
	result = m_Primitive.CreateVertexBuffer(pD3DDevice, vertexNum);
	if (result != 0) goto EXIT;

	//インデックスバッファ生成
	indexNum = 4;
	result = m_Primitive.CreateIndexBuffer(pD3DDevice, indexNum);
	if (result != 0) goto EXIT;

	//バッファのロック
	result = m_Primitive.LockVertex((void**)&pVertex);
	if (result != 0) goto EXIT;
	result = m_Primitive.LockIndex(&pIndex);
	if (result != 0) goto EXIT;

	//バッファに頂点とインデックスを書き込む
	result = _CreateVertexOfBoard(
					pVertex,		//頂点バッファ書き込み位置
					pIndex			//インデックスバッファ書き込み位置
				);
	if (result != 0) goto EXIT;

	//バッファのロック解除
	result = m_Primitive.UnlockVertex();
	if (result != 0) goto EXIT;
	result = m_Primitive.UnlockIndex();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 移動
//******************************************************************************
int MTPictBoard::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice,
		D3DXVECTOR3 camVector,
		float rollAngle
	)
{
	int result = 0;
	float curPos = 0.0f;
	D3DXVECTOR3 moveVector;
	D3DXMATRIX rotateMatrix;
	D3DXMATRIX moveMatrix;
	D3DXMATRIX worldMatrix;

	//行列初期化
	D3DXMatrixIdentity(&rotateMatrix);
	D3DXMatrixIdentity(&moveMatrix);
	D3DXMatrixIdentity(&worldMatrix);

	//回転行列
	D3DXMatrixRotationX(&rotateMatrix, D3DXToRadian(rollAngle));

	//演奏位置
	curPos = m_NoteDesign.GetPlayPosX(m_CurTickTime);

	//移動行列
	moveVector = m_NoteDesign.GetWorldMoveVector();
	D3DXMatrixTranslation(&moveMatrix, moveVector.x + curPos, moveVector.y, moveVector.z);

	//行列の合成
	D3DXMatrixMultiply(&worldMatrix, &rotateMatrix, &moveMatrix);

	//変換行列設定
	m_Primitive.Transform(worldMatrix);

	return result;
}

//******************************************************************************
// 描画
//******************************************************************************
int MTPictBoard::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;

	if (!m_isEnable) goto EXIT;

	//テクスチャステージ設定
	//  カラー演算：引数1を使用  引数1：テクスチャ
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	// アルファ演算：引数1を使用  引数1：テクスチャ
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

	//テクスチャフィルタ
	pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	//描画
	result = m_Primitive.Draw(pD3DDevice, m_pTexture);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 解放
//******************************************************************************
void MTPictBoard::Release()
{
	m_Primitive.Release();
	
	if (m_pTexture != NULL) {
		m_pTexture->Release();
		m_pTexture = NULL;
	}
}

//******************************************************************************
// ピクチャボード頂点生成
//******************************************************************************
int MTPictBoard::_CreateVertexOfBoard(
		MTPICTBOARD_VERTEX* pVertex,
		unsigned long* pIndex
	)
{
	int result = 0;
	unsigned long i = 0;
	D3DXVECTOR3 vectorLU;
	D3DXVECTOR3 vectorRU;
	D3DXVECTOR3 vectorLD;
	D3DXVECTOR3 vectorRD;
	float boardHeight = 0.0f;
	float boardWidth = 0.0f;
	float chStep = 0.0f;

	//     +   1+----+3   +
	//    /|   / 上 /    /|gridH    y x
	//   + | 0+----+2   + |右       |/
	// 左| +   7+----+5 | +      z--+0
	//   |/    / 下 /   |/
	//   +   6+----+4   + ← 4 が原点(0,0,0)
	//        gridW

	//再生面頂点座標取得
	m_NoteDesign.GetPlaybackSectionVirtexPos(
			0,
			&vectorLU,
			&vectorRU,
			&vectorLD,
			&vectorRD
		);

	boardHeight = vectorLU.y - vectorLD.y;
	boardWidth = boardHeight * ((float)m_ImgInfo.Width / (float)m_ImgInfo.Height);
	chStep = m_NoteDesign.GetChStep();

	//頂点座標：左の面
	pVertex[0].p = D3DXVECTOR3(vectorLU.x,            vectorLU.y, vectorLU.z+chStep+0.01f); //0
	pVertex[1].p = D3DXVECTOR3(vectorLU.x+boardWidth, vectorLU.y, vectorLU.z+chStep+0.01f); //1
	pVertex[2].p = D3DXVECTOR3(vectorLD.x,            vectorLD.y, vectorLD.z+chStep+0.01f); //6
	pVertex[3].p = D3DXVECTOR3(vectorLD.x+boardWidth, vectorLD.y, vectorLD.z+chStep+0.01f); //7

	//再生面との相対位置にずらす
	//  相対位置 0.0f → 画像の左端が再生面と直交する
	//  相対位置 0.5f → 画像の中央が再生面と直交する
	//  相対位置 1.0f → 画像の右端が再生面と直交する
	for (i = 0; i < 4; i++) {
		pVertex[i].p.x += -(boardWidth * m_NoteDesign.GetPictBoardRelativePos());
	}

	//法線
	pVertex[0].n = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVertex[1].n = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVertex[2].n = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVertex[3].n = D3DXVECTOR3(0.0f, 0.0f, -1.0f);

	//各頂点のディフューズ色
	for (i = 0; i < 4; i++) {
		pVertex[i].c = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	}

	//各頂点のテクスチャ座標
	pVertex[0].t = D3DXVECTOR2(0.0f, 0.0f);
	pVertex[1].t = D3DXVECTOR2(1.0f, 0.0f);
	pVertex[2].t = D3DXVECTOR2(0.0f, 1.0f);
	pVertex[3].t = D3DXVECTOR2(1.0f, 1.0f);

	//インデックス：TRIANGLESTRIP
	pIndex[0] = 0;
	pIndex[1] = 1;
	pIndex[2] = 2;
	pIndex[3] = 3;

	return result;
}

//******************************************************************************
// テクスチャ画像読み込み
//******************************************************************************
int MTPictBoard::_LoadTexture(
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
	result = confFile.GetStr(_T("Board"), bmpFileName, _MAX_PATH, MT_IMGFILE_BOARD);
	if (result != 0) goto EXIT;

	//プロセス実行ファイルディレクトリパス取得
	result = YNPathUtil::GetModuleDirPath(imgFilePath, _MAX_PATH);
	if (result != 0) goto EXIT;

	//画像ファイルパス作成
	_tcscat_s(imgFilePath, _MAX_PATH, bmpFileName);

	//読み込む画像の縦横サイズを取得しておく
	hresult = D3DXGetImageInfoFromFile(imgFilePath, &m_ImgInfo);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

	//テクスチャ画像として読み込み
	hresult = D3DXCreateTextureFromFile(
					pD3DDevice,		//テクスチャに関連付けるデバイス
					imgFilePath,	//ファイル名
					&m_pTexture		//作成されたテクスチャオブジェクト
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// チックタイム設定
//******************************************************************************
void MTPictBoard::SetCurTickTime(
		unsigned long curTickTime
	)
{
	m_CurTickTime = curTickTime;
}

//******************************************************************************
// リセット
//******************************************************************************
void MTPictBoard::Reset()
{
	m_CurTickTime = 0;
}

//******************************************************************************
// 演奏開始
//******************************************************************************
void MTPictBoard::OnPlayStart()
{
	m_isPlay = true;
}

//******************************************************************************
// 演奏終了
//******************************************************************************
void MTPictBoard::OnPlayEnd()
{
	m_isPlay = false;
}

//******************************************************************************
// 表示設定
//******************************************************************************
void MTPictBoard::SetEnable(
		bool isEnable
	)
{
	m_isEnable = isEnable;
}


