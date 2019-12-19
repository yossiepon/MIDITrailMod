//******************************************************************************
//
// MIDITrail / MTPictBoardRing
//
// ピクチャボードリング描画クラス
//
// Copyright (C) 2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTPictBoardRing.h"
#include "DXH.h"

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTPictBoardRing::MTPictBoardRing(void)
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
MTPictBoardRing::~MTPictBoardRing(void)
{
	Release();
}

//******************************************************************************
// ピクチャボード生成
//******************************************************************************
int MTPictBoardRing::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData,
		bool isReverseMode
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
	vertexNum = (SM_MAX_NOTE_NUM + 1) * 2;
	result = m_Primitive.CreateVertexBuffer(pD3DDevice, vertexNum);
	if (result != 0) goto EXIT;

	//インデックスバッファ生成
	indexNum = SM_MAX_NOTE_NUM * 3 * 2;
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
					pIndex,			//インデックスバッファ書き込み位置
					isReverseMode
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
int MTPictBoardRing::Transform(
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
int MTPictBoardRing::Draw(
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
void MTPictBoardRing::Release()
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
int MTPictBoardRing::_CreateVertexOfBoard(
		MTPICTBOARD_VERTEX* pVertex,
		unsigned long* pIndex,
		bool isReverseMode
	)
{
	int result = 0;
	unsigned long i = 0;
	unsigned long virtexIndex = 0;
	unsigned long virtexIndexStart = 0;
	D3DXVECTOR3 basePos;
	D3DXVECTOR3 rotatedPos;
	float boardHeight = 0.0f;
	float boardWidth = 0.0f;
	float chStep = 0.0f;
	float angle = 0.0f;
	float direction = 0.0f;
	D3DXVECTOR2 clipAreaP1;
	D3DXVECTOR2 clipAreaP2;
	D3DXVECTOR2 textureP1;
	D3DXVECTOR2 textureP2;

	//テクスチャクリップ領域の座標
	clipAreaP1 = D3DXVECTOR2(0.0f, 0.0f);  //左上
	clipAreaP2 = D3DXVECTOR2(1.0f, 1.0f);  //右下

	//テスクチャX座標
	if (isReverseMode) {
		textureP1.x = clipAreaP1.x;
		textureP1.y = clipAreaP2.y;
		textureP2.x = clipAreaP2.x;
		textureP2.y = clipAreaP1.y;
		direction = -1.0f;
	}
	else {
		textureP1.x = clipAreaP2.x;
		textureP1.y = clipAreaP1.y;
		textureP2.x = clipAreaP1.x;
		textureP2.y = clipAreaP2.y;
		direction = 1.0f;
	}

	//基準座標
	chStep = m_NoteDesign.GetChStep();
	basePos = D3DXVECTOR3(
				m_NoteDesign.GetPlayPosX(0),
				m_NoteDesign.GetPortOriginY(0) + (chStep * (float)SM_MAX_CH_NUM) + chStep + 0.01f,
				m_NoteDesign.GetPortOriginZ(0));
	boardHeight = 2.0f * 3.1415926f * basePos.y;
	boardWidth = boardHeight * ((float)m_ImgInfo.Width / (float)m_ImgInfo.Height);
	basePos.x -= (boardWidth * m_NoteDesign.GetPictBoardRelativePos());

	//頂点作成：X軸回りの円筒
	virtexIndexStart = virtexIndex;
	pVertex[virtexIndex].p = basePos;
	pVertex[virtexIndex].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
	pVertex[virtexIndex].c = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	pVertex[virtexIndex].t = D3DXVECTOR2(textureP1.x, textureP1.y);
	virtexIndex++;
	pVertex[virtexIndex].p = basePos;
	pVertex[virtexIndex].p.x += boardWidth;
	pVertex[virtexIndex].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
	pVertex[virtexIndex].c = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	pVertex[virtexIndex].t = D3DXVECTOR2(textureP2.x, textureP1.y);
	for (i = 1; i < SM_MAX_NOTE_NUM; i++) {
		virtexIndex++;
		
		//回転後の頂点
		angle = (360.0f / (float)SM_MAX_NOTE_NUM) * (float)i;
		rotatedPos = DXH::RotateYZ(0.0f, 0.0f, basePos, angle);
		pVertex[virtexIndex].p = rotatedPos;
		pVertex[virtexIndex].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
		pVertex[virtexIndex].c = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
		pVertex[virtexIndex].t = D3DXVECTOR2(textureP1.x, textureP1.y + (direction * (float)i / (float)SM_MAX_NOTE_NUM));
		virtexIndex++;
		pVertex[virtexIndex].p = rotatedPos;
		pVertex[virtexIndex].p.x += boardWidth;
		pVertex[virtexIndex].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
		pVertex[virtexIndex].c = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
		pVertex[virtexIndex].t = D3DXVECTOR2(textureP2.x, textureP1.y + (direction * (float)i / (float)SM_MAX_NOTE_NUM));
		
		//インデックスバッファ
		//  直前の頂点0,1と追加した頂点2,3で三角形0-1-3と0-3-2を追加
		//  1+--+3-+--+--+--
		//   | /| /| /| /|  ..
		//   |/ |/ |/ |/ |  ..
		//  0+--+2-+--+--+--
		pIndex[(i-1)*6 + 0] = (i-1)*2 + 0;  //0 1つ目の三角形
		pIndex[(i-1)*6 + 1] = (i-1)*2 + 1;  //1 1つ目の三角形
		pIndex[(i-1)*6 + 2] = (i-1)*2 + 3;  //3 1つ目の三角形
		pIndex[(i-1)*6 + 3] = (i-1)*2 + 0;  //0 2つ目の三角形
		pIndex[(i-1)*6 + 4] = (i-1)*2 + 3;  //3 2つ目の三角形
		pIndex[(i-1)*6 + 5] = (i-1)*2 + 2;  //2 2つ目の三角形
	}
	//最後の頂点2,3は最初0,1の頂点と同じ（リングを閉じる）
	virtexIndex++;
	pVertex[virtexIndex] =pVertex[0];
	pVertex[virtexIndex].t = D3DXVECTOR2(textureP1.x, textureP2.y);
	virtexIndex++;
	pVertex[virtexIndex] =pVertex[1];
	pVertex[virtexIndex].t = D3DXVECTOR2(textureP2.x, textureP2.y);

	//インデックスバッファ（リングを閉じる）
	pIndex[(i-1)*6 + 0] = (i-1)*2 + 0;  //0 1つ目の三角形
	pIndex[(i-1)*6 + 1] = (i-1)*2 + 1;  //1 1つ目の三角形
	pIndex[(i-1)*6 + 2] = (i-1)*2 + 3;  //3 1つ目の三角形
	pIndex[(i-1)*6 + 3] = (i-1)*2 + 0;  //0 2つ目の三角形
	pIndex[(i-1)*6 + 4] = (i-1)*2 + 3;  //3 2つ目の三角形
	pIndex[(i-1)*6 + 5] = (i-1)*2 + 2;  //2 2つ目の三角形

	return result;
}

//******************************************************************************
// テクスチャ画像読み込み
//******************************************************************************
int MTPictBoardRing::_LoadTexture(
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
void MTPictBoardRing::SetCurTickTime(
		unsigned long curTickTime
	)
{
	m_CurTickTime = curTickTime;
}

//******************************************************************************
// リセット
//******************************************************************************
void MTPictBoardRing::Reset()
{
	m_CurTickTime = 0;
}

//******************************************************************************
// 演奏開始
//******************************************************************************
void MTPictBoardRing::OnPlayStart()
{
	m_isPlay = true;
}

//******************************************************************************
// 演奏終了
//******************************************************************************
void MTPictBoardRing::OnPlayEnd()
{
	m_isPlay = false;
}

//******************************************************************************
// 表示設定
//******************************************************************************
void MTPictBoardRing::SetEnable(
		bool isEnable
	)
{
	m_isEnable = isEnable;
}


