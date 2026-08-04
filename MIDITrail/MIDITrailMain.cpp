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
#include "YNBaseLib.h"
#include "DXRenderer11.h"
#include "DXPrimitive11.h"

using namespace YNBaseLib;

#define WINDOW_CLASS_NAME  _T("MIDITrailDX11")
#define WINDOW_TITLE       _T("MIDITrail (DX11 WIP)")

static DXRenderer11 g_Renderer;
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

	// 背景色設定（暗い青）
	g_Renderer.SetBGColor(0xFF1A1A2E);

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
				// フレーム描画（シーンなし = 背景色クリア + Present のみ）
				g_Renderer.RenderScene(NULL, NULL);
			}
		}
	}

EXIT:;
	DXPrimitive11::ReleasePipeline();
	g_Renderer.Terminate();
	return 0;
}
