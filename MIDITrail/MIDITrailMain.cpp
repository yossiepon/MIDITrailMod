//******************************************************************************
//
// MIDITrail / MIDITrailMain
//
// アプリケーションエントリポイント
// Phase 2: 最小 DX11 構成での動作確認用スタブ
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "stdafx.h"
#include "mmsystem.h"
#include "YNBaseLib.h"
#include "SMIDILib.h"
#include "DXRenderer11.h"
#include "DXPrimitive11.h"
#include "MTScenePianoRoll3D11.h"

using namespace SMIDILib;
using namespace YNBaseLib;

#define WINDOW_CLASS_NAME  _T("MIDITrailDX11")
#define WINDOW_TITLE       _T("MIDITrail (DX11 WIP)")

static DXRenderer11 g_Renderer;
static MTScenePianoRoll3D11* g_pScene = nullptr;
static SMMsgQueue g_MsgQueue;
static SMSequencer g_Sequencer;
static bool g_IsRunning = true;

//******************************************************************************
// ウィンドウプロシージャ
//******************************************************************************
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
		case WM_SIZE:
			g_Renderer.OnResize();
			return 0;
		case WM_DESTROY:
			g_IsRunning = false;
			PostQuitMessage(0);
			return 0;
		case WM_KEYDOWN:
			if (wParam == VK_ESCAPE) {
				DestroyWindow(hWnd);
				return 0;
			}
			break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

//******************************************************************************
// シーケンサメッセージ処理
//******************************************************************************
static int _SequencerMsgProc()
{
	int result = 0;
	bool isExist = false;
	unsigned long param1 = 0;
	unsigned long param2 = 0;

	while (true) {
		result = g_MsgQueue.GetMessage(&isExist, &param1, &param2);
		if (result != 0) goto EXIT;

		if (!isExist) break;

		if (g_pScene != NULL) {
			result = g_pScene->OnRecvSequencerMsg(param1, param2);
			if (result != 0) goto EXIT;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// エントリポイント
//******************************************************************************
int APIENTRY _tWinMain(
		HINSTANCE hInstance,
		HINSTANCE hPrevInstance,
		LPTSTR lpCmdLine,
		int nCmdShow
	)
{
	int result = 0;
	SMSeqData seqData;
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// ウィンドウクラス登録
	WNDCLASSEX wcex = {};
	wcex.cbSize        = sizeof(WNDCLASSEX);
	wcex.style         = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc   = WndProc;
	wcex.hInstance     = hInstance;
	wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wcex.lpszClassName = WINDOW_CLASS_NAME;
	RegisterClassEx(&wcex);

	// ウィンドウ作成
	HWND hWnd = CreateWindow(
				WINDOW_CLASS_NAME,
				WINDOW_TITLE,
				WS_OVERLAPPEDWINDOW,
				CW_USEDEFAULT, CW_USEDEFAULT,
				1280, 720,
				NULL, NULL, hInstance, NULL);

	if (hWnd == NULL) {
		return 0;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	// DX11 レンダラ初期化
	result = g_Renderer.Initialize(hWnd);
	if (result != 0) {
		YN_SHOW_ERR(hWnd);
		goto EXIT;
	}

	// シェーダパイプライン初期化
	result = DXPrimitive11::InitPipeline(g_Renderer.GetDevice());
	if (result != 0) {
		YN_SHOW_ERR(hWnd);
		goto EXIT;
	}

	// テスト用 MIDI ファイル読み込み
	{
		SMFileReader reader;
		result = reader.Load(L"C:\\Users\\yoshy\\Source\\Claude\\20260804_MIDITrailModMod\\temp\\test.mid", &seqData);
		if (result != 0) {
			YN_SHOW_ERR(hWnd);
			result = 0;
		}
	}

	// シーン生成
	g_pScene = new MTScenePianoRoll3D11(false, false);
	result = g_pScene->Create(hWnd, g_Renderer.GetDevice(),
	                           g_Renderer.GetContext(), &seqData);
	if (result != 0) {
		YN_SHOW_ERR(hWnd);
		goto EXIT;
	}

	// シーケンサ初期化・再生開始
	result = g_MsgQueue.Initialize(10000);
	if (result != 0) {
		YN_SHOW_ERR(hWnd);
		goto EXIT;
	}

	result = g_Sequencer.Initialize(&g_MsgQueue);
	if (result != 0) {
		YN_SHOW_ERR(hWnd);
		goto EXIT;
	}

	result = g_Sequencer.SetSeqData(&seqData);
	if (result != 0) {
		YN_SHOW_ERR(hWnd);
		goto EXIT;
	}

	// MIDI 出力デバイス未設定（音なし再生）
	g_pScene->OnPlayStart();

	result = g_Sequencer.Play();
	if (result != 0) {
		YN_SHOW_ERR(hWnd);
		goto EXIT;
	}

	// メッセージループ
	{
		MSG msg = {};

		while (g_IsRunning) {
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				if (msg.message == WM_QUIT) break;
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else {
				// シーケンサメッセージ処理
				result = _SequencerMsgProc();
				if (result != 0) {
					YN_SHOW_ERR(hWnd);
				}

				// シーン更新
				result = g_pScene->Update();
				if (result != 0) goto EXIT;

				// フレーム描画
				result = g_Renderer.RenderScene(g_pScene, g_pScene->GetCamera());
				if (result != 0) goto EXIT;
			}
		}
	}

	g_Sequencer.Stop();
	g_pScene->OnPlayEnd();

EXIT:;
	if (g_pScene != NULL) {
		g_pScene->Release();
		delete g_pScene;
		g_pScene = NULL;
	}
	DXPrimitive11::ReleasePipeline();
	g_Renderer.Terminate();
	return 0;
}
