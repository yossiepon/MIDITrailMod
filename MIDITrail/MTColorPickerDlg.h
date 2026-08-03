//******************************************************************************
//
// MIDITrail / MTColorPickerDlg
//
// ced 20260628: RGBA カラーピッカー。
//   - HSV の2Dフィールド（彩度×明度）＋色相バー：ドラッグで色選択
//   - R/G/B/A のスライダー＋数値欄＋hex 欄
//   - 市松模様プレビュー（透明度を反映）
//   Windows 標準の ChooseColor はアルファ非対応のため自作。
//   オーナー描画は DIB 一括転送でちらつき無し。
//
//******************************************************************************

#pragma once

#include <windows.h>
#include "DXColorUtil.h"


class MTColorPickerDlg
{
public:

	MTColorPickerDlg(void);
	virtual ~MTColorPickerDlg(void);

	// inColor を初期値として表示。OK で *pOutColor に結果を返し *pChosen=true。
	int Show(HWND hParentWnd, D3DXCOLOR inColor, D3DXCOLOR* pOutColor, bool* pChosen);

private:

	static MTColorPickerDlg* m_pThis;

	HWND m_hDlg;
	HWND m_hSV;        // 彩度×明度 2Dフィールド（SS_OWNERDRAW）
	HWND m_hHue;       // 色相バー（SS_OWNERDRAW）
	HWND m_hAlpha;     // 透明度バー（SS_OWNERDRAW）

	// 作業状態：色相(0..360)/彩度(0..1)/明度(0..1)/アルファ(0..255)
	float m_H;
	float m_S;
	float m_V;
	int   m_A;
	// 出力 RGB（HSV から算出、0..255）
	int   m_R;
	int   m_G;
	int   m_B;

	bool  m_Chosen;
	bool  m_Updating;  // スライダー/数値欄/hex/ドラッグの相互更新による再入を防ぐ

	static INT_PTR CALLBACK _WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	INT_PTR _WndProcImpl(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

	// SV/Hue コントロールのサブクラス（ドラッグ取得）
	static LRESULT CALLBACK _CtrlSubProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam,
			UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
	void _OnCtrlDrag(int ctrlId, HWND hCtrl, LPARAM lParam);

	void _OnInitDlg(HWND hDlg);

	// HSV -> RGB を算出して m_R/m_G/m_B に反映
	void _RecalcRGB();
	// RGB -> HSV を反映（グレー/黒では色相/彩度を維持）
	void _SyncHSVfromRGB();

	// 表示更新
	void _UpdateControls(bool updateSliders, bool updateEdits, bool updateHex);
	void _RedrawColorAreas();
	void _UpdateTracks();
	void _UpdateEdits();
	void _UpdateHex();

	// 入力ハンドラ
	void _OnEditChanged(int editId, bool isAlpha, int rgbIndex);
	void _OnHexChanged();
	void _SyncFromTracks();

	// オーナー描画
	void _DrawSV(DRAWITEMSTRUCT* di);
	void _DrawHue(DRAWITEMSTRUCT* di);
	void _DrawAlpha(DRAWITEMSTRUCT* di);
	void _DrawPreview(DRAWITEMSTRUCT* di);

	int  _GetTrackPos(int ctrlId);
	void _SetTrackPos(int ctrlId, int pos);

	void operator=(const MTColorPickerDlg&);
	MTColorPickerDlg(const MTColorPickerDlg&);
};
