//******************************************************************************
//
// MIDITrail / MTBackgroundImage
//
// 背景画像描画クラス
//
// Copyright (C) 2016 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "shlwapi.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTBackgroundImage.h"


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTBackgroundImage::MTBackgroundImage(void)
{
	m_pTexture = NULL;
	ZeroMemory(&m_ImgInfo, sizeof(D3DXIMAGE_INFO));
	m_hWnd = NULL;
	m_isEnable = true;
	m_isFilterLinear = true;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTBackgroundImage::~MTBackgroundImage(void)
{
	Release();
}

//******************************************************************************
// 背景画像生成
//******************************************************************************
int MTBackgroundImage::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		HWND hWnd
	)
{
	int result = 0;
	unsigned long vertexNum = 0;
	unsigned long indexNum = 0;
	MTBACKGROUNDIMAGE_VERTEX* pVertex = NULL;
	unsigned long* pIndex = NULL;

	Release();

	m_hWnd = hWnd;

	//設定ファイル初期化
	result = _InitConfFile();
	if (result != 0) goto EXIT;

	//テクスチャ読み込み
	result = _LoadTexture(pD3DDevice);
	if (result != 0) goto EXIT;

	//テクスチャを生成しなかった場合は何もしない
	if (m_pTexture == NULL) goto EXIT;

	//プリミティブ初期化
	result = m_Primitive.Initialize(
					sizeof(MTBACKGROUNDIMAGE_VERTEX),	//頂点サイズ
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
	result = _CreateVertexOfBackground(
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
// 描画
//******************************************************************************
int MTBackgroundImage::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	DWORD value = 0;

	if (!m_isEnable) goto EXIT;
	if (m_pTexture == NULL) goto EXIT;

	//Zバッファを一時的に無効化する
	pD3DDevice->GetRenderState(D3DRS_ZENABLE, &value);
	pD3DDevice->SetRenderState(D3DRS_ZENABLE, FALSE);

	//テクスチャステージ設定
	//  カラー演算：引数1を使用  引数1：テクスチャ
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	// アルファ演算：引数1を使用  引数1：テクスチャ
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

	//テクスチャフィルタ
	if (m_isFilterLinear) {
		pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	}
	else {
		//ピクセル等倍で描画する場合ボケないようにする
		pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
		pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	}

	//描画
	result = m_Primitive.Draw(pD3DDevice, m_pTexture);
	if (result != 0) goto EXIT;

	//Zバッファ有効化状態を戻す
	pD3DDevice->SetRenderState(D3DRS_ZENABLE, value);

EXIT:;
	return result;
}

//******************************************************************************
// 解放
//******************************************************************************
void MTBackgroundImage::Release()
{
	m_Primitive.Release();
	
	if (m_pTexture != NULL) {
		m_pTexture->Release();
		m_pTexture = NULL;
	}
}

//******************************************************************************
// 背景画像頂点生成
//******************************************************************************
int MTBackgroundImage::_CreateVertexOfBackground(
		MTBACKGROUNDIMAGE_VERTEX* pVertex,
		unsigned long* pIndex
	)
{
	int result = 0;
	BOOL bresult = 0;
	RECT rect;
	unsigned long i = 0;
	unsigned long cw = 0;
	unsigned long ch = 0;
	float ratio_cwh = 0.0f;
	float ratio_iwh = 0.0f;
	float x1 = 0.0f;
	float x2 = 0.0f;
	float y1 = 0.0f;
	float y2 = 0.0f;

	//クライアント領域のサイズを取得
	bresult = GetClientRect(m_hWnd, &rect);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	cw = rect.right - rect.left;
	ch = rect.bottom - rect.top;

	ratio_cwh = (float)cw / (float)ch;
	ratio_iwh = (float)m_ImgInfo.Width / (float)m_ImgInfo.Height;

	// クライアント領域より画像の方が横長の場合
	//     |----- cw -----|
	//  ---0--------------+-- +x
	//   | |              |
	//   | +--------------+
	//  ch |    image     |
	//   | +--------------+
	//   | |              |
	//  ---+--------------+
	//     |
	//    +y
	if (ratio_cwh < ratio_iwh) {
		x1 = 0.0f;
		x2 = (float)cw;
		y1 = ((float)ch - ((float)cw / ratio_iwh)) / 2.0f;
		y2 = (float)ch - y1;
		
		//ピクセル等倍で描画する場合はリニアフィルタを解除してボケないようにする
		if (cw == m_ImgInfo.Width) {
			m_isFilterLinear = false;
		}
	}
	// クライアント領域より画像の方が縦長の場合
	//     |----- cw -----|
	//  ---0--+--------+--+-- +x
	//   | |  |        |  |
	//   | |  |        |  |
	//  ch |  | image  |  |
	//   | |  |        |  |
	//   | |  |        |  |
	//  ---+--+--------+--+
	//     |
	//    +y
	else {
		x1 = ((float)cw - ((float)ch * ratio_iwh)) / 2.0f;
		x2 = (float)cw - x1 - 1.0f;
		y1 = 0.0f;
		y2 = (float)ch - 1.0f;
		
		//ピクセル等倍で描画する場合はリニアフィルタを解除してボケないようにする
		if (ch == m_ImgInfo.Height) {
			m_isFilterLinear = false;
		}
	}

	//ピクセル等倍で描画する場合を想定して座標を調整
	x1 -= 0.5f;
	x2 -= 0.5f;
	y1 -= 0.5f;
	y2 -= 0.5f;

	//頂点座標
	pVertex[0].p = D3DXVECTOR3(x1, y1, 0.0f);
	pVertex[1].p = D3DXVECTOR3(x2, y1, 0.0f);
	pVertex[2].p = D3DXVECTOR3(x1, y2, 0.0f);
	pVertex[3].p = D3DXVECTOR3(x2, y2, 0.0f);

	//各頂点のディフューズ色
	for (i = 0; i < 4; i++) {
		//各頂点の除算数
		pVertex[i].rhw = 1.0f;
		//各頂点のディフューズ色
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

EXIT:;
	return result;
}

//******************************************************************************
// 設定ファイル初期化
//******************************************************************************
int MTBackgroundImage::_InitConfFile()
{
	int result = 0;
	TCHAR userConfFilePath[_MAX_PATH] = {_T('\0')};

	result = YNPathUtil::GetAppDataDirPath(userConfFilePath, _MAX_PATH);
	if (result != 0) goto EXIT;

	_tcscat_s(userConfFilePath, _MAX_PATH, MT_USER_CONFFILE_DIR);
	_tcscat_s(userConfFilePath, _MAX_PATH, MT_USER_CONFFILE_GRAPHIC);

	result = m_ConfFile.Initialize(userConfFilePath);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// テクスチャ画像読み込み
//******************************************************************************
int MTBackgroundImage::_LoadTexture(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;
	TCHAR imageFilePath[_MAX_PATH] = {_T('\0')};

	//ビットマップファイル名
	result = m_ConfFile.SetCurSection(_T("Background-image"));
	if (result != 0) goto EXIT;
	result = m_ConfFile.GetStr(_T("ImageFilePath"), imageFilePath, _MAX_PATH, _T(""));
	if (result != 0) goto EXIT;

	//ファイル未指定なら何もしない
	if (_tcslen(imageFilePath) == 0) goto EXIT;

	//ファイルが存在しない場合は何もしない
	if (!PathFileExists(imageFilePath)) goto EXIT;

	//読み込む画像の縦横サイズを取得しておく
	hresult = D3DXGetImageInfoFromFile(imageFilePath, &m_ImgInfo);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

	//テクスチャ画像として読み込み
	//  ピクセル等倍で描画する場合にボケないようにするため
	//  画像サイズを指定して読み込み
	hresult = D3DXCreateTextureFromFileEx(
					pD3DDevice,			//デバイス
					imageFilePath,		//ファイルパス
					m_ImgInfo.Width,	//幅（ピクセル）：直接指定
					m_ImgInfo.Height,	//高さ（ピクセル）：直接指定
					1,					//ミップレベル
					0,					//使用方法
					D3DFMT_A8R8G8B8,	//ピクセルフォーマット
					D3DPOOL_MANAGED,	//テクスチャ配置先メモリクラス
					D3DX_FILTER_NONE,	//フィルタリング指定
					D3DX_FILTER_NONE,	//フィルタリング指定（ミップ）
					0xFF000000,			//透明色の指定：不透明黒
					NULL,				//ソースイメージ情報
					NULL,				//256色パレット
					&m_pTexture			//作成されたテクスチャオブジェクト
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// 表示設定
//******************************************************************************
void MTBackgroundImage::SetEnable(
		bool isEnable
	)
{
	m_isEnable = isEnable;
}


