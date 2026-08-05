//******************************************************************************
//
// MIDITrail / MTScenePianoRoll3D11
//
// DX11 PianoRoll 3D/2D scene.
//
// Copyright (C) 2010-2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
// Based on the DX11 migration design by ced (Zel9278)
// https://github.com/Zel9278/MIDITrailModMod
//
//******************************************************************************

#include "StdAfx.h"
#include "MTScenePianoRoll3D11.h"

using namespace DirectX::SimpleMath;


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTScenePianoRoll3D11::MTScenePianoRoll3D11(bool isLive, bool is2D)
{
	m_IsLive = isLive;
	m_Is2D = is2D;

	// シーン固有のプロパティ
	m_Traits.cameraDir = MTCameraDirX;
	m_Traits.cameraTracksPlayback = true;
	m_Traits.lightEnabled = !is2D;  // 3D: ライトON, 2D: ライトOFF
	m_Traits.lightCount = 2;
	m_Traits.viewpointCompensation = true;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTScenePianoRoll3D11::~MTScenePianoRoll3D11()
{
}

//******************************************************************************
// シーン名取得
//******************************************************************************
const TCHAR* MTScenePianoRoll3D11::GetName() const
{
	if (m_IsLive) {
		return m_Is2D ? _T("PianoRoll2DLive") : _T("PianoRoll3DLive");
	}
	return m_Is2D ? _T("PianoRoll2D") : _T("PianoRoll3D");
}

//******************************************************************************
// 生成
//******************************************************************************
int MTScenePianoRoll3D11::Create(
		HWND hWnd,
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		SMIDILib::SMSeqData* pSeqData
	)
{
	int result = 0;

	m_pDevice = pDevice;
	m_pContext = pContext;

	// 設定読み込み
	_LoadConf();

	// カメラ初期化
	result = m_Camera.Initialize(hWnd, GetName(), pSeqData);
	if (result != 0) goto EXIT;

	m_Camera.SetProgressDirection(MTFirstPersonCam::DirX);

	// デフォルト視点を設定
	{
		MTViewParamMap defaultView;
		_ComputeDefaultViewParam(&defaultView);
		m_Camera.SetViewParam(&defaultView);
	}

	// 星
	result = m_Stars.Create(pDevice, pContext, GetName());
	if (result != 0) goto EXIT;

	// グリッド
	result = m_Grid.Create(pDevice, pContext, GetName(), pSeqData);
	if (result != 0) goto EXIT;

	// タイムインジケータ
	result = m_TimeIndicator.Create(pDevice, pContext, GetName(), pSeqData);
	if (result != 0) goto EXIT;

	// ピクチャボード
	result = m_PictBoard.Create(pDevice, pContext, GetName(), pSeqData);
	if (result != 0) goto EXIT;

	// 背景画像
	result = m_BackgroundImage.Create(pDevice, pContext, hWnd);
	if (result != 0) goto EXIT;

	// Phase 2: 残りのコンポーネント生成（段階的に追加）

EXIT:;
	return result;
}

//******************************************************************************
// 解放
//******************************************************************************
void MTScenePianoRoll3D11::Release()
{
	m_Stars.Release();
	m_Grid.Release();
	m_TimeIndicator.Release();
	m_PictBoard.Release();
	m_BackgroundImage.Release();

	MTSceneBase11::Release();
}

//******************************************************************************
// フレーム更新
//******************************************************************************
void MTScenePianoRoll3D11::Transform(
		unsigned long curTickTime,
		unsigned long playTimeMSec
	)
{
	// カメラ入力処理
	m_Camera.TransformInput();

	// カメラ位置取得
	Vector3 camPos;
	m_Camera.GetPosition(&camPos);
	float rollAngle = m_Camera.GetRollAngle();

	// 星：カメラに追従
	m_Stars.Transform(camPos);

	// グリッド
	m_Grid.Transform(rollAngle);

	// タイムインジケータ
	m_TimeIndicator.Transform(rollAngle);

	// ピクチャボード
	m_PictBoard.Transform(camPos, rollAngle);
}

//******************************************************************************
// 描画
//******************************************************************************
int MTScenePianoRoll3D11::Draw(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		float rollAngle,
		const Vector3& camPos
	)
{
	// 背景画像（最初に描画）
	m_BackgroundImage.DrawDX11(pContext);

	// シーン固有コンポーネント描画
	_DrawSceneComponents(pContext, viewProj, rollAngle, camPos);

	// Phase 2: Dashboard を最後に描画

	return 0;
}

//******************************************************************************
// シーン固有コンポーネント描画
//******************************************************************************
void MTScenePianoRoll3D11::_DrawSceneComponents(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		float rollAngle,
		const Vector3& camPos
	)
{
	Vector4 lightDir(1.0f, -1.0f, 2.0f, 0.0f);

	// グリッド
	m_Grid.DrawDX11(pContext, viewProj, lightDir, rollAngle);

	// Phase 2: ノート → 波紋

	// ピクチャボード
	m_PictBoard.DrawDX11(pContext, viewProj, lightDir, rollAngle);

	// タイムインジケータ
	m_TimeIndicator.DrawDX11(pContext, viewProj, lightDir, rollAngle);

	// 星
	m_Stars.DrawDX11(pContext, viewProj, rollAngle);
}

//******************************************************************************
// 再生開始
//******************************************************************************
void MTScenePianoRoll3D11::OnPlayStart()
{
	_Reset();
}

//******************************************************************************
// 再生終了
//******************************************************************************
void MTScenePianoRoll3D11::OnPlayEnd()
{
}

//******************************************************************************
// シーケンサメッセージ受信
//******************************************************************************
int MTScenePianoRoll3D11::OnRecvSequencerMsg(
		unsigned long param1,
		unsigned long param2
	)
{
	// Phase 2: メッセージをコンポーネントに配信
	return 0;
}

//******************************************************************************
// 表示トグル
//******************************************************************************
void MTScenePianoRoll3D11::SetEffect(MTEffectType type, bool isEnable)
{
	// Phase 2: EffectType → コンポーネント enable/disable マッピング
}

//******************************************************************************
// 再生速度設定
//******************************************************************************
void MTScenePianoRoll3D11::SetPlaySpeedRatio(unsigned long ratio)
{
	// Phase 2: Dashboard に転送
}

//******************************************************************************
// ノート数取得
//******************************************************************************
unsigned long MTScenePianoRoll3D11::GetNoteCount() const
{
	// Phase 2: NoteBox から取得
	return 0;
}

//******************************************************************************
// デフォルト視点計算
//******************************************************************************
void MTScenePianoRoll3D11::_ComputeDefaultViewParam(MTViewParamMap* pParamMap)
{
	// DX9 MTScenePianoRoll3D::GetDefaultViewParam と同じ
	// E4 (noteNo=64) の中心、Z=-18 後方
	float noteStep = 0.1f;  // Phase 2: m_NoteDesign から取得
	float defaultY = noteStep * 64.0f;
	float defaultZ = -18.0f;

	(*pParamMap)["X"] = 0.0f;
	(*pParamMap)["Y"] = defaultY;
	(*pParamMap)["Z"] = defaultZ;
	(*pParamMap)["Phi"] = 90.0f;
	(*pParamMap)["Theta"] = 90.0f;
	(*pParamMap)["ManualRollAngle"] = 0.0f;
	(*pParamMap)["AutoRollVelocity"] = 0.0f;
}

//******************************************************************************
// 視点補正量
//******************************************************************************
float MTScenePianoRoll3D11::_GetViewpointCompensation() const
{
	return m_TimeIndicator.GetPos();
}

//******************************************************************************
// リセット
//******************************************************************************
void MTScenePianoRoll3D11::_Reset()
{
	MTSceneBase11::_Reset();
	// Phase 2: シーン固有コンポーネントのリセット
}
