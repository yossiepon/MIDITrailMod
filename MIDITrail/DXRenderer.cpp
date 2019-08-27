//******************************************************************************
//
// MIDITrail / DXRenderer
//
// レンダラクラス
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "stdafx.h"
#include "YNBaseLib.h"
#include "DXScene.h"
#include "DXRenderer.h"

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
DXRenderer::DXRenderer()
{
	m_hWnd = NULL;
	m_pD3D = NULL;
	m_pD3DDevice = NULL;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
DXRenderer::~DXRenderer()
{
	Terminate();
}

//******************************************************************************
// 初期化
//******************************************************************************
int DXRenderer::Initialize(
		HWND hWnd,
		unsigned long multiSampleType	//省略時はゼロ：アンチエイリアシング無効
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;
	D3DDISPLAYMODE dispMode;
	bool isSupport = false;
	D3DMULTISAMPLE_TYPE type = D3DMULTISAMPLE_NONE;
	unsigned long qualityLevels = 0;

	m_hWnd = hWnd;

	if (DX_MULTI_SAMPLE_TYPE_MAX < multiSampleType) {
		result = YN_SET_ERR("Program error.", multiSampleType, 0);
		goto EXIT;
	}

	//Direct3D9オブジェクト作成
	m_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (m_pD3D == NULL) {
		result = YN_SET_ERR("DirectX API error.", 0, 0);
		goto EXIT;
	}

	//ディスプレイモード取得
	hresult = m_pD3D->GetAdapterDisplayMode(
					D3DADAPTER_DEFAULT,	//問い合わせ対象ディスプレイアダプタ
					&dispMode			//ディスプレイモード情報
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

	//プレゼンテーションパラメータ初期化
	ZeroMemory(&m_D3DPP, sizeof(D3DPRESENT_PARAMETERS));
	m_D3DPP.BackBufferCount = 1;					//バッファ数
	m_D3DPP.Windowed = TRUE;						//ウインドウ内表示の指定
	m_D3DPP.BackBufferFormat = dispMode.Format;		//バッファサーフェスフォーマット
	m_D3DPP.SwapEffect = D3DSWAPEFFECT_DISCARD;		//ダブルバッファリングスワップ指定
	m_D3DPP.EnableAutoDepthStencil = TRUE;			//深度ステンシルバッファ作成
	m_D3DPP.AutoDepthStencilFormat = D3DFMT_D16;	//自動深度ステンシルサーフェスフォーマット

	//アンチエイリアシング有効化
	if (multiSampleType >= DX_MULTI_SAMPLE_TYPE_MIN) {
		//アンチエイリアシング指定あり
		//ハードウェアのアンチエイリアシングサポート状況を確認する
		type = _EnumMultiSampleType(multiSampleType);
		result = _CheckAntialiasSupport(
						m_D3DPP,			//プレゼンテーションパラメータ
						type,				//マルチサンプル種別
						&isSupport,			//サポート確認結果
						&qualityLevels		//品質レベル数
					);
		if (result != 0) goto EXIT;
		
		//指定されたマルチサンプル数をサポートしている場合は
		//プレゼンテーションパラメータに反映する
		if (isSupport) {
			m_D3DPP.MultiSampleType = type;
			m_D3DPP.MultiSampleQuality = qualityLevels - 1;
		}
	}

	//ディスプレイアダプタデバイス作成（描画：ハード／頂点処理：ハード）
	hresult = m_pD3D->CreateDevice(
					D3DADAPTER_DEFAULT,	//ディスプレイアダプタ
					D3DDEVTYPE_HAL,		//デバイスタイプ：ハードウェアラスタ化
					hWnd,				//フォーカス対象ウィンドウハンドル
					D3DCREATE_HARDWARE_VERTEXPROCESSING,
										//フラグ：ハードウェア頂点処理
					&m_D3DPP,			//プレゼンテーションパラメータ
					&m_pD3DDevice		//作成されたデバイス
				);
	if (FAILED(hresult)) {
		//失敗しても次の方法を試す
	}
	else {
		//成功したら終了
		goto EXIT;
	}

	//ディスプレイアダプタデバイス作成（描画：ハード／頂点処理：CPU）
	hresult = m_pD3D->CreateDevice(
					D3DADAPTER_DEFAULT,	//ディスプレイアダプタ
					D3DDEVTYPE_HAL,		//デバイスタイプ：ハードウェアラスタ化
					hWnd,				//フォーカス対象ウィンドウハンドル
					D3DCREATE_SOFTWARE_VERTEXPROCESSING,
										//フラグ：ソフトウェア頂点処理
					&m_D3DPP,			//プレゼンテーションパラメータ
					&m_pD3DDevice		//作成されたデバイス
				);
	if (FAILED(hresult)) {
		//失敗しても次の方法を試す
	}
	else {
		//成功したら終了
		goto EXIT;
	}

	//HAL以外ではアンチエイリアシングは無効にする
	m_D3DPP.MultiSampleType = D3DMULTISAMPLE_NONE;
	m_D3DPP.MultiSampleQuality = 0;

	//ディスプレイアダプタデバイス作成（描画：CPU／頂点処理：CPU）
	hresult = m_pD3D->CreateDevice(
					D3DADAPTER_DEFAULT,	//ディスプレイアダプタ
					D3DDEVTYPE_REF,		//デバイスタイプ：ソフトウェア
					hWnd,				//フォーカス対象ウィンドウハンドル
					D3DCREATE_SOFTWARE_VERTEXPROCESSING,
										//フラグ：ソフトウェア頂点処理
					&m_D3DPP,			//プレゼンテーションパラメータ
					&m_pD3DDevice		//作成されたデバイス
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// デバイス取得
//******************************************************************************
LPDIRECT3DDEVICE9 DXRenderer::GetDevice()
{
	return m_pD3DDevice;
}

//*****************************************************************************
// シーン描画
//******************************************************************************
int DXRenderer::RenderScene(
		DXScene* pScene
	)
{
	int result = 0;
	HRESULT hresult = 0;
	D3DCOLOR bgColor;

	if (pScene == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//バッファ初期化色の指定：ARGB
	bgColor = pScene->GetBGColor();

	//ビューポートクリア＋深度バッファクリア＋ステンシルバッファ削除
	hresult = m_pD3DDevice->Clear(
					0,						//矩形数：全面
					NULL, 					//クリア対象矩形：全面
					D3DCLEAR_TARGET			//レンダリングターゲットクリア
					| D3DCLEAR_ZBUFFER, 	//深度バッファクリア
					bgColor,				//設定色(ARGB)
					//D3DCOLOR_XRGB(0,0,0), 			//設定色(ARGB)：黒
					//D3DCOLOR_XRGB(255,255,255), 		//設定色(ARGB)：白
					1.0f, 					//深度バッファ設定値
					0						//ステンシルバッファ設定値
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

	//描画開始
	hresult = m_pD3DDevice->BeginScene();
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

	//描画
	result = pScene->Draw(m_pD3DDevice);
	if (result != 0) {
		//失敗しても描画定型処理を続行する
		YN_SHOW_ERR(m_hWnd);
	}

	//描画終了
	hresult = m_pD3DDevice->EndScene();
	if (FAILED(hresult)) {
		//描失敗しても描画定型処理を続行する
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		YN_SHOW_ERR(m_hWnd);
	}

	//描画結果転送
	hresult = m_pD3DDevice->Present(
						NULL,	//転送元矩形：転送元サーフェイス全体を表示
						NULL,	//転送先矩形：クライアント領域全体
						NULL,	//転送先ウィンドウハンドル：プレゼンテーションパラメータに従う
						NULL	//更新対象矩形（最適化支援用）
					);
	if (FAILED(hresult)) {
		//デバイスロスト
		if (hresult == D3DERR_DEVICELOST) {
			result = DXRENDERER_ERR_DEVICE_LOST;
			goto EXIT;

			//リカバリ
			//result = _RecoverDevice();
			//if (result != 0) goto EXIT;
		}
		else {
			result = YN_SET_ERR("DirectX API error.", hresult, 0);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

//*****************************************************************************
// 終了処理
//******************************************************************************
void DXRenderer::Terminate()
{
	if (m_pD3DDevice != NULL) {
		m_pD3DDevice->Release();
		m_pD3DDevice = NULL;
	}
	if (m_pD3D != NULL) {
		m_pD3D->Release();
		m_pD3D = NULL;
	}
	m_hWnd = NULL;
}

//*****************************************************************************
// デバイスロストリカバリ処理
//******************************************************************************
int DXRenderer::_RecoverDevice()
{
	int result = 0;
	HRESULT hresult = D3D_OK;

	//デバイスロストに対処するには
	//  (1) デバイスがリセット可能になるまで待つ
	//      描画処理のAPI呼び出しは成功するが実際は何も処理されない
	//  (2) デバイスがリセット可能になったらリセットする
	//  (3) リセットしたらリソースを破棄して再構築する
	//      頂点バッファ／インデックスバッファ／テクスチャなど
	//
	//(3)の実装がつらいので現時点は未対応とする

	//デバイスの動作状態を取得
	hresult = m_pD3DDevice->TestCooperativeLevel();
	if (hresult == D3D_OK) {
		//正常動作中のためリカバリ不要
	}
	else {
		//デバイスロスト
		if (hresult == D3DERR_DEVICELOST) {
			//まだリセットできないので放置する
		}
		//リセット可能
		else if (hresult == D3DERR_DEVICENOTRESET) {
			//デバイスリセット
			hresult = m_pD3DDevice->Reset(&m_D3DPP);
			if (FAILED(hresult)) {
				result = YN_SET_ERR("DirectX API error.", hresult, 0);
				goto EXIT;
			}
			//全リソースを破棄
			// TODO: 未実装・・・どうしましょ
			//全リソースを再生成
			// TODO: 未実装・・・どうしましょ
		}
		//内部エラー
		else if (hresult == D3DERR_DRIVERINTERNALERROR) {
			result = YN_SET_ERR("DirectX API error.", hresult, 0);
			goto EXIT;
		}
		//未知のエラー
		else {
			result = YN_SET_ERR("DirectX API error.", hresult, 0);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

//*****************************************************************************
// アンチエイリアシングサポートチェック
//******************************************************************************
int DXRenderer::_CheckAntialiasSupport(
		D3DPRESENT_PARAMETERS d3dpp,
		D3DMULTISAMPLE_TYPE multiSampleType,
		bool* pIsSupport,
		unsigned long* pQualityLevels
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;
	unsigned long qualityLevelsBackBuffer = 0;
	unsigned long qualityLevelsZBuffer = 0;
	bool isSupportBackBuffer = false;
	bool isSupportZbuffer = false;

	//ハードウェアのアンチエイリアシングサポート状況のみ確認する

	//バックバッファフォーマットのアンチエイリアシングサポート確認
	hresult = m_pD3D->CheckDeviceMultiSampleType(
					D3DADAPTER_DEFAULT,			//問い合わせ対象ディスプレイアダプタ
					D3DDEVTYPE_HAL,				//デバイスタイプ：ハードウェア
					d3dpp.BackBufferFormat,		//サーフェイスフォーマット
					d3dpp.Windowed,				//ウィンドウ種別
					multiSampleType,			//マルチサンプリングテクニック
					&qualityLevelsBackBuffer	//利用可能な品質レベルの数：不要
				);
	if (SUCCEEDED(hresult)) {
		//サポートしている
		isSupportBackBuffer = true;
	}

	//深度バッファフォーマットのアンチエイリアシングサポート確認
	hresult = m_pD3D->CheckDeviceMultiSampleType(
					D3DADAPTER_DEFAULT,			//問い合わせ対象ディスプレイアダプタ
					D3DDEVTYPE_HAL,				//デバイスタイプ：ハードウェア
					d3dpp.BackBufferFormat,		//サーフェイスフォーマット
					d3dpp.Windowed,				//ウィンドウ種別
					multiSampleType,			//マルチサンプリングテクニック
					&qualityLevelsZBuffer		//利用可能な品質レベルの数：不要
				);
	if (SUCCEEDED(hresult)) {
		//サポートしている
		isSupportZbuffer = true;
	}

	//アンチエイリアシングチェック結果
	*pIsSupport = false;
	*pQualityLevels = 0;
	if (isSupportBackBuffer && isSupportZbuffer) {
		*pIsSupport = true;
		if (qualityLevelsBackBuffer < qualityLevelsZBuffer) {
			*pQualityLevels = qualityLevelsBackBuffer;
		}
		else {
			*pQualityLevels = qualityLevelsZBuffer;
		}
	}

	return result;
}

//*****************************************************************************
// マルチサンプル種別取得
//******************************************************************************
D3DMULTISAMPLE_TYPE DXRenderer::_EnumMultiSampleType(
		unsigned long multiSampleNum
	)
{
	D3DMULTISAMPLE_TYPE type = D3DMULTISAMPLE_NONE;
	D3DMULTISAMPLE_TYPE types[] = {
			D3DMULTISAMPLE_2_SAMPLES,  D3DMULTISAMPLE_3_SAMPLES,  D3DMULTISAMPLE_4_SAMPLES,
			D3DMULTISAMPLE_5_SAMPLES,  D3DMULTISAMPLE_6_SAMPLES,  D3DMULTISAMPLE_7_SAMPLES,
			D3DMULTISAMPLE_8_SAMPLES,  D3DMULTISAMPLE_9_SAMPLES,  D3DMULTISAMPLE_10_SAMPLES,
			D3DMULTISAMPLE_11_SAMPLES, D3DMULTISAMPLE_12_SAMPLES, D3DMULTISAMPLE_13_SAMPLES,
			D3DMULTISAMPLE_14_SAMPLES, D3DMULTISAMPLE_15_SAMPLES, D3DMULTISAMPLE_16_SAMPLES
		};

	if ((DX_MULTI_SAMPLE_TYPE_MIN <= multiSampleNum)
	 && (multiSampleNum <= DX_MULTI_SAMPLE_TYPE_MAX)) {
		type = types[multiSampleNum - 2];
	}

	return type;
}

//*****************************************************************************
// アンチエイリアシングサポートチェック
//******************************************************************************
int DXRenderer::IsSupportAntialias(
		unsigned long multiSampleType,
		bool* pIsSupport
	)
{
	int result = 0;
	unsigned long qualityLevels = 0;

	if (m_pD3D == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	if (pIsSupport == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	if ((multiSampleType < DX_MULTI_SAMPLE_TYPE_MIN)
	 || (DX_MULTI_SAMPLE_TYPE_MAX < multiSampleType)) {
		result = YN_SET_ERR("Program error.", multiSampleType, 0);
		goto EXIT;
	}

	result = _CheckAntialiasSupport(
					m_D3DPP,
					_EnumMultiSampleType(multiSampleType),
					pIsSupport,
					&qualityLevels
				);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//*****************************************************************************
// インデックスバッファサポートチェック
//******************************************************************************
int DXRenderer::IsSupportIndexBuffer(
		bool* pIsSupport,
		unsigned long* pMaxVertexIndex
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;
	D3DCAPS9 caps;

	if (m_pD3D == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	if ((pIsSupport == NULL) || (pMaxVertexIndex == NULL)) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	*pIsSupport = false;

	//デバイス情報取得
	hresult = m_pD3D->GetDeviceCaps(
					D3DADAPTER_DEFAULT,	//問い合わせ対象ディスプレイアダプタ
					D3DDEVTYPE_HAL,		//デバイスタイプ：ハードウェア
					&caps				//デバイス能力情報
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

	*pMaxVertexIndex = caps.MaxVertexIndex;

	if (caps.MaxVertexIndex > 0x0000FFFF) {
		//インデックスバッファをサポートしている
		*pIsSupport = true;
	}

EXIT:;
	return result;
}


