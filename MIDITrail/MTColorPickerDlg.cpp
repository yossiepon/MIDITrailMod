//******************************************************************************
//
// MIDITrail / MTColorPickerDlg
//
// ced 20260628: RGBA カラーピッカー（HSV 2Dフィールド＋色相バー＋RGBAスライダー）。
//
//******************************************************************************

#include "MTColorPickerDlg.h"
#include "resource.h"
#include <commctrl.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#pragma comment(lib, "comctl32.lib")


MTColorPickerDlg* MTColorPickerDlg::m_pThis = NULL;

//------------------------------------------------------------------------------
// HSV <-> RGB
//------------------------------------------------------------------------------
static void HSVtoRGB(float h, float s, float v, int* pR, int* pG, int* pB)
{
	float r = v, g = v, b = v;
	if (s > 0.0f) {
		if (h >= 360.0f) h = 0.0f;
		if (h < 0.0f) h = 0.0f;
		h /= 60.0f;
		int i = (int)h;
		float f = h - (float)i;
		float p = v * (1.0f - s);
		float q = v * (1.0f - s * f);
		float t = v * (1.0f - s * (1.0f - f));
		switch (i) {
			case 0:  r = v; g = t; b = p; break;
			case 1:  r = q; g = v; b = p; break;
			case 2:  r = p; g = v; b = t; break;
			case 3:  r = p; g = q; b = v; break;
			case 4:  r = t; g = p; b = v; break;
			default: r = v; g = p; b = q; break;
		}
	}
	int R = (int)(r * 255.0f + 0.5f); if (R < 0) R = 0; if (R > 255) R = 255;
	int G = (int)(g * 255.0f + 0.5f); if (G < 0) G = 0; if (G > 255) G = 255;
	int B = (int)(b * 255.0f + 0.5f); if (B < 0) B = 0; if (B > 255) B = 255;
	*pR = R; *pG = G; *pB = B;
}

static void RGBtoHSV(int R, int G, int B, float* pH, float* pS, float* pV)
{
	float r = R / 255.0f, g = G / 255.0f, b = B / 255.0f;
	float mx = r; if (g > mx) mx = g; if (b > mx) mx = b;
	float mn = r; if (g < mn) mn = g; if (b < mn) mn = b;
	float d = mx - mn;
	float h = 0.0f;
	if (d > 0.0f) {
		if (mx == r)      h = 60.0f * (float)fmod((double)((g - b) / d), 6.0);
		else if (mx == g) h = 60.0f * (((b - r) / d) + 2.0f);
		else              h = 60.0f * (((r - g) / d) + 4.0f);
		if (h < 0.0f) h += 360.0f;
	}
	*pH = h;
	*pS = (mx > 0.0f) ? (d / mx) : 0.0f;
	*pV = mx;
}

//******************************************************************************
// コンストラクタ／デストラクタ
//******************************************************************************
MTColorPickerDlg::MTColorPickerDlg(void)
{
	m_pThis = this;
	m_hDlg = NULL;
	m_hSV = NULL;
	m_hHue = NULL;
	m_hAlpha = NULL;
	m_H = 0.0f; m_S = 0.0f; m_V = 0.0f;
	m_A = 255;
	m_R = 0; m_G = 0; m_B = 0;
	m_Chosen = false;
	m_Updating = false;
}

MTColorPickerDlg::~MTColorPickerDlg(void)
{
}

//******************************************************************************
// 表示
//******************************************************************************
int MTColorPickerDlg::Show(HWND hParentWnd, D3DXCOLOR inColor, D3DXCOLOR* pOutColor, bool* pChosen)
{
	INT_PTR dresult = 0;
	HINSTANCE hInstance = NULL;
	INITCOMMONCONTROLSEX icc;

	if ((pOutColor == NULL) || (pChosen == NULL)) return -1;
	*pChosen = false;
	m_pThis = this;

	icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icc.dwICC = ICC_BAR_CLASSES;
	InitCommonControlsEx(&icc);

	//初期色（0-255）
	m_R = (int)(inColor.r * 255.0f + 0.5f);
	m_G = (int)(inColor.g * 255.0f + 0.5f);
	m_B = (int)(inColor.b * 255.0f + 0.5f);
	m_A = (int)(inColor.a * 255.0f + 0.5f);
	if (m_R < 0) m_R = 0; if (m_R > 255) m_R = 255;
	if (m_G < 0) m_G = 0; if (m_G > 255) m_G = 255;
	if (m_B < 0) m_B = 0; if (m_B > 255) m_B = 255;
	if (m_A < 0) m_A = 0; if (m_A > 255) m_A = 255;
	RGBtoHSV(m_R, m_G, m_B, &m_H, &m_S, &m_V);
	m_Chosen = false;

	hInstance = (HINSTANCE)(LONG_PTR)GetWindowLongPtr(hParentWnd, GWLP_HINSTANCE);
	dresult = DialogBox(hInstance, MAKEINTRESOURCE(IDD_COLOR_PICKER), hParentWnd, _WndProc);
	if (dresult == -1) return -1;

	*pChosen = m_Chosen;
	if (m_Chosen) {
		*pOutColor = D3DXCOLOR((float)m_R / 255.0f, (float)m_G / 255.0f,
				(float)m_B / 255.0f, (float)m_A / 255.0f);
	}
	return 0;
}

//******************************************************************************
// ウィンドウプロシージャ
//******************************************************************************
INT_PTR CALLBACK MTColorPickerDlg::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return m_pThis->_WndProcImpl(hWnd, message, wParam, lParam);
}

INT_PTR MTColorPickerDlg::_WndProcImpl(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
		case WM_INITDIALOG:
			_OnInitDlg(hDlg);
			return TRUE;

		case WM_HSCROLL:
			//RGBA スライダーが操作された
			m_Updating = true;
			_SyncFromTracks();
			_SyncHSVfromRGB();
			_UpdateEdits();
			_UpdateHex();
			m_Updating = false;
			_RedrawColorAreas();
			return TRUE;

		case WM_DRAWITEM:
			switch ((int)wParam) {
				case IDC_PICK_SV:      _DrawSV((DRAWITEMSTRUCT*)lParam);      return TRUE;
				case IDC_PICK_HUE:     _DrawHue((DRAWITEMSTRUCT*)lParam);     return TRUE;
				case IDC_PICK_ALPHA:   _DrawAlpha((DRAWITEMSTRUCT*)lParam);   return TRUE;
				case IDC_PICK_PREVIEW: _DrawPreview((DRAWITEMSTRUCT*)lParam); return TRUE;
				default: break;
			}
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:     m_Chosen = true;  EndDialog(hDlg, 1); return TRUE;
				case IDCANCEL: m_Chosen = false; EndDialog(hDlg, 2); return TRUE;
				case IDC_PICK_EDIT_R: if (HIWORD(wParam) == EN_CHANGE) _OnEditChanged(IDC_PICK_EDIT_R, false, 0); return TRUE;
				case IDC_PICK_EDIT_G: if (HIWORD(wParam) == EN_CHANGE) _OnEditChanged(IDC_PICK_EDIT_G, false, 1); return TRUE;
				case IDC_PICK_EDIT_B: if (HIWORD(wParam) == EN_CHANGE) _OnEditChanged(IDC_PICK_EDIT_B, false, 2); return TRUE;
				case IDC_PICK_EDIT_A: if (HIWORD(wParam) == EN_CHANGE) _OnEditChanged(IDC_PICK_EDIT_A, true, 3); return TRUE;
				case IDC_PICK_EDIT_HEX: if (HIWORD(wParam) == EN_CHANGE) _OnHexChanged(); return TRUE;
				default: break;
			}
			break;

		default:
			break;
	}
	return FALSE;
}

//******************************************************************************
// 初期化
//******************************************************************************
void MTColorPickerDlg::_OnInitDlg(HWND hDlg)
{
	int ids[4] = { IDC_PICK_TRACK_R, IDC_PICK_TRACK_G, IDC_PICK_TRACK_B, IDC_PICK_TRACK_A };
	int i = 0;

	m_hDlg = hDlg;
	m_hSV    = GetDlgItem(hDlg, IDC_PICK_SV);
	m_hHue   = GetDlgItem(hDlg, IDC_PICK_HUE);
	m_hAlpha = GetDlgItem(hDlg, IDC_PICK_ALPHA);
	m_Updating = true;

	for (i = 0; i < 4; i++) {
		HWND h = GetDlgItem(hDlg, ids[i]);
		SendMessage(h, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0, 255));
		SendMessage(h, TBM_SETPAGESIZE, 0, (LPARAM)16);
	}
	SendMessage(GetDlgItem(hDlg, IDC_PICK_EDIT_R), EM_SETLIMITTEXT, (WPARAM)3, 0);
	SendMessage(GetDlgItem(hDlg, IDC_PICK_EDIT_G), EM_SETLIMITTEXT, (WPARAM)3, 0);
	SendMessage(GetDlgItem(hDlg, IDC_PICK_EDIT_B), EM_SETLIMITTEXT, (WPARAM)3, 0);
	SendMessage(GetDlgItem(hDlg, IDC_PICK_EDIT_A), EM_SETLIMITTEXT, (WPARAM)3, 0);
	SendMessage(GetDlgItem(hDlg, IDC_PICK_EDIT_HEX), EM_SETLIMITTEXT, (WPARAM)8, 0);

	//SV / Hue コントロールをサブクラスしてドラッグを取得
	SetWindowSubclass(m_hSV,    _CtrlSubProc, (UINT_PTR)IDC_PICK_SV,    (DWORD_PTR)this);
	SetWindowSubclass(m_hHue,   _CtrlSubProc, (UINT_PTR)IDC_PICK_HUE,   (DWORD_PTR)this);
	SetWindowSubclass(m_hAlpha, _CtrlSubProc, (UINT_PTR)IDC_PICK_ALPHA, (DWORD_PTR)this);

	_UpdateTracks();
	_UpdateEdits();
	_UpdateHex();

	m_Updating = false;
	_RedrawColorAreas();
}

//******************************************************************************
// SV / Hue コントロールのサブクラス（ドラッグ取得）
//******************************************************************************
LRESULT CALLBACK MTColorPickerDlg::_CtrlSubProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam,
		UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	MTColorPickerDlg* self = (MTColorPickerDlg*)dwRefData;
	switch (msg) {
		case WM_NCHITTEST:
			//SS_OWNERDRAW static は既定で HTTRANSPARENT を返しマウスを受けないため、
			//HTCLIENT を返してドラッグ用のマウスメッセージを受け取れるようにする。
			return HTCLIENT;
		case WM_LBUTTONDOWN:
			SetCapture(hWnd);
			if (self != NULL) self->_OnCtrlDrag((int)uIdSubclass, hWnd, lParam);
			return 0;
		case WM_MOUSEMOVE:
			if ((wParam & MK_LBUTTON) && (self != NULL)) self->_OnCtrlDrag((int)uIdSubclass, hWnd, lParam);
			return 0;
		case WM_LBUTTONUP:
			ReleaseCapture();
			return 0;
		case WM_NCDESTROY:
			RemoveWindowSubclass(hWnd, _CtrlSubProc, uIdSubclass);
			break;
		default:
			break;
	}
	return DefSubclassProc(hWnd, msg, wParam, lParam);
}

void MTColorPickerDlg::_OnCtrlDrag(int ctrlId, HWND hCtrl, LPARAM lParam)
{
	int x = (int)(short)LOWORD(lParam);
	int y = (int)(short)HIWORD(lParam);
	RECT rc;
	GetClientRect(hCtrl, &rc);
	int w = rc.right - rc.left;
	int h = rc.bottom - rc.top;
	if (w <= 1) w = 2;
	if (h <= 1) h = 2;
	if (x < 0) x = 0; if (x > w - 1) x = w - 1;
	if (y < 0) y = 0; if (y > h - 1) y = h - 1;

	if (ctrlId == IDC_PICK_ALPHA) {
		//透明度バー：上=不透明(255)、下=透明(0)。HSV/RGB は変えない。
		m_A = (int)((1.0f - (float)y / (float)(h - 1)) * 255.0f + 0.5f);
		if (m_A < 0) m_A = 0; if (m_A > 255) m_A = 255;
		m_Updating = true;
		_SetTrackPos(IDC_PICK_TRACK_A, m_A);
		_UpdateEdits();
		_UpdateHex();
		m_Updating = false;
		_RedrawColorAreas();
		return;
	}

	if (ctrlId == IDC_PICK_SV) {
		m_S = (float)x / (float)(w - 1);
		m_V = 1.0f - (float)y / (float)(h - 1);
	}
	else if (ctrlId == IDC_PICK_HUE) {
		m_H = ((float)y / (float)(h - 1)) * 360.0f;
		if (m_H >= 360.0f) m_H = 359.99f;
	}

	_RecalcRGB();
	m_Updating = true;
	_UpdateTracks();
	_UpdateEdits();
	_UpdateHex();
	m_Updating = false;
	_RedrawColorAreas();
}

//******************************************************************************
// 値の同期
//******************************************************************************
void MTColorPickerDlg::_RecalcRGB()
{
	HSVtoRGB(m_H, m_S, m_V, &m_R, &m_G, &m_B);
}

void MTColorPickerDlg::_SyncHSVfromRGB()
{
	float h, s, v;
	RGBtoHSV(m_R, m_G, m_B, &h, &s, &v);
	m_V = v;
	if (s > 0.0f) { m_S = s; m_H = h; }   //有彩色：色相と彩度を採用
	else          { m_S = 0.0f;       }   //グレー：色相は維持
}

void MTColorPickerDlg::_SyncFromTracks()
{
	m_R = _GetTrackPos(IDC_PICK_TRACK_R);
	m_G = _GetTrackPos(IDC_PICK_TRACK_G);
	m_B = _GetTrackPos(IDC_PICK_TRACK_B);
	m_A = _GetTrackPos(IDC_PICK_TRACK_A);
}

void MTColorPickerDlg::_UpdateTracks()
{
	_SetTrackPos(IDC_PICK_TRACK_R, m_R);
	_SetTrackPos(IDC_PICK_TRACK_G, m_G);
	_SetTrackPos(IDC_PICK_TRACK_B, m_B);
	_SetTrackPos(IDC_PICK_TRACK_A, m_A);
}

void MTColorPickerDlg::_UpdateEdits()
{
	TCHAR s[16];
	_stprintf_s(s, 16, _T("%d"), m_R); SetWindowText(GetDlgItem(m_hDlg, IDC_PICK_EDIT_R), s);
	_stprintf_s(s, 16, _T("%d"), m_G); SetWindowText(GetDlgItem(m_hDlg, IDC_PICK_EDIT_G), s);
	_stprintf_s(s, 16, _T("%d"), m_B); SetWindowText(GetDlgItem(m_hDlg, IDC_PICK_EDIT_B), s);
	_stprintf_s(s, 16, _T("%d"), m_A); SetWindowText(GetDlgItem(m_hDlg, IDC_PICK_EDIT_A), s);
}

void MTColorPickerDlg::_UpdateHex()
{
	TCHAR s[16];
	_stprintf_s(s, 16, _T("%02X%02X%02X%02X"), m_R, m_G, m_B, m_A);
	SetWindowText(GetDlgItem(m_hDlg, IDC_PICK_EDIT_HEX), s);
}

void MTColorPickerDlg::_UpdateControls(bool updateSliders, bool updateEdits, bool updateHex)
{
	if (updateSliders) _UpdateTracks();
	if (updateEdits)   _UpdateEdits();
	if (updateHex)     _UpdateHex();
}

void MTColorPickerDlg::_RedrawColorAreas()
{
	if (m_hSV != NULL)    InvalidateRect(m_hSV, NULL, FALSE);
	if (m_hHue != NULL)   InvalidateRect(m_hHue, NULL, FALSE);
	if (m_hAlpha != NULL) InvalidateRect(m_hAlpha, NULL, FALSE);
	InvalidateRect(GetDlgItem(m_hDlg, IDC_PICK_PREVIEW), NULL, FALSE);
}

//******************************************************************************
// 数値欄 / hex 欄の入力
//******************************************************************************
void MTColorPickerDlg::_OnEditChanged(int editId, bool isAlpha, int rgbIndex)
{
	TCHAR s[16] = { _T('\0') };
	int v = 0;

	if (m_Updating) return;
	GetWindowText(GetDlgItem(m_hDlg, editId), s, 16);
	if (_tcslen(s) == 0) return;
	v = _ttoi(s);
	if (v < 0) v = 0;
	if (v > 255) v = 255;

	if (isAlpha) {
		m_A = v;
		//アルファは HSV に無関係。トラックバーと hex を更新（編集中の欄は触らない）。
		m_Updating = true;
		_SetTrackPos(IDC_PICK_TRACK_A, m_A);
		_UpdateHex();
		m_Updating = false;
		_RedrawColorAreas();   //プレビュー＋透明度バーを更新
		return;
	}

	if (rgbIndex == 0) m_R = v;
	else if (rgbIndex == 1) m_G = v;
	else m_B = v;
	_SyncHSVfromRGB();

	m_Updating = true;
	_UpdateTracks();
	_UpdateHex();
	m_Updating = false;
	_RedrawColorAreas();
}

void MTColorPickerDlg::_OnHexChanged()
{
	TCHAR s[16] = { _T('\0') };
	size_t len = 0;
	TCHAR b[3] = { 0, 0, 0 };

	if (m_Updating) return;
	GetWindowText(GetDlgItem(m_hDlg, IDC_PICK_EDIT_HEX), s, 16);
	len = _tcslen(s);
	if ((len != 6) && (len != 8)) return;

	b[0] = s[0]; b[1] = s[1]; m_R = (int)_tcstol(b, NULL, 16);
	b[0] = s[2]; b[1] = s[3]; m_G = (int)_tcstol(b, NULL, 16);
	b[0] = s[4]; b[1] = s[5]; m_B = (int)_tcstol(b, NULL, 16);
	if (len == 8) { b[0] = s[6]; b[1] = s[7]; m_A = (int)_tcstol(b, NULL, 16); }
	_SyncHSVfromRGB();

	m_Updating = true;
	_UpdateTracks();
	_UpdateEdits();
	m_Updating = false;
	_RedrawColorAreas();
}

//******************************************************************************
// トラックバー位置
//******************************************************************************
int MTColorPickerDlg::_GetTrackPos(int ctrlId)
{
	return (int)SendMessage(GetDlgItem(m_hDlg, ctrlId), TBM_GETPOS, 0, 0);
}

void MTColorPickerDlg::_SetTrackPos(int ctrlId, int pos)
{
	SendMessage(GetDlgItem(m_hDlg, ctrlId), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)pos);
}

//******************************************************************************
// オーナー描画（DIB 一括転送でちらつき無し）
//******************************************************************************
void MTColorPickerDlg::_DrawSV(DRAWITEMSTRUCT* di)
{
	HDC hdc = di->hDC;
	RECT rc = di->rcItem;
	int w = rc.right - rc.left;
	int h = rc.bottom - rc.top;
	int x = 0, y = 0;
	if ((w <= 0) || (h <= 0)) return;

	DWORD* buf = new DWORD[(size_t)w * h];
	for (y = 0; y < h; y++) {
		float v = 1.0f - (float)y / (float)((h > 1) ? (h - 1) : 1);
		for (x = 0; x < w; x++) {
			float s = (float)x / (float)((w > 1) ? (w - 1) : 1);
			int r, g, b; HSVtoRGB(m_H, s, v, &r, &g, &b);
			buf[(size_t)y * w + x] = ((DWORD)r << 16) | ((DWORD)g << 8) | (DWORD)b;
		}
	}
	BITMAPINFO bmi; ZeroMemory(&bmi, sizeof(bmi));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = w; bmi.bmiHeader.biHeight = -h;   //top-down
	bmi.bmiHeader.biPlanes = 1; bmi.bmiHeader.biBitCount = 32; bmi.bmiHeader.biCompression = BI_RGB;
	StretchDIBits(hdc, rc.left, rc.top, w, h, 0, 0, w, h, buf, &bmi, DIB_RGB_COLORS, SRCCOPY);
	delete[] buf;

	//現在位置マーカー：白リング＋黒リングの二重丸（どんな背景でも見える）
	int mx = rc.left + (int)(m_S * (float)(w - 1));
	int my = rc.top + (int)((1.0f - m_V) * (float)(h - 1));
	int rad = 6;
	HPEN penW = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
	HPEN penB = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	HGDIOBJ ob = SelectObject(hdc, GetStockObject(NULL_BRUSH));
	HGDIOBJ op = SelectObject(hdc, penW);
	Ellipse(hdc, mx - rad, my - rad, mx + rad, my + rad);
	SelectObject(hdc, penB);
	Ellipse(hdc, mx - rad + 1, my - rad + 1, mx + rad - 1, my + rad - 1);
	SelectObject(hdc, op);
	SelectObject(hdc, ob);
	DeleteObject(penW);
	DeleteObject(penB);
}

//バー（色相/透明度）共通のマーカー：白枠＋黒枠の横帯ハンドル
static void _DrawBarMarker(HDC hdc, const RECT& rc, int my)
{
	HBRUSH wbr = (HBRUSH)GetStockObject(WHITE_BRUSH);
	HBRUSH bbr = (HBRUSH)GetStockObject(BLACK_BRUSH);
	RECT r1, r2;
	r1.left = rc.left; r1.right = rc.right; r1.top = my - 3; r1.bottom = my + 3;
	FrameRect(hdc, &r1, wbr);
	r2.left = rc.left + 1; r2.right = rc.right - 1; r2.top = my - 2; r2.bottom = my + 2;
	FrameRect(hdc, &r2, bbr);
}

void MTColorPickerDlg::_DrawHue(DRAWITEMSTRUCT* di)
{
	HDC hdc = di->hDC;
	RECT rc = di->rcItem;
	int w = rc.right - rc.left;
	int h = rc.bottom - rc.top;
	int x = 0, y = 0;
	if ((w <= 0) || (h <= 0)) return;

	DWORD* buf = new DWORD[(size_t)w * h];
	for (y = 0; y < h; y++) {
		float hue = ((float)y / (float)((h > 1) ? (h - 1) : 1)) * 360.0f;
		int r, g, b; HSVtoRGB(hue, 1.0f, 1.0f, &r, &g, &b);
		DWORD c = ((DWORD)r << 16) | ((DWORD)g << 8) | (DWORD)b;
		for (x = 0; x < w; x++) buf[(size_t)y * w + x] = c;
	}
	BITMAPINFO bmi; ZeroMemory(&bmi, sizeof(bmi));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = w; bmi.bmiHeader.biHeight = -h;
	bmi.bmiHeader.biPlanes = 1; bmi.bmiHeader.biBitCount = 32; bmi.bmiHeader.biCompression = BI_RGB;
	StretchDIBits(hdc, rc.left, rc.top, w, h, 0, 0, w, h, buf, &bmi, DIB_RGB_COLORS, SRCCOPY);
	delete[] buf;

	//現在の色相マーカー
	int my = rc.top + (int)((m_H / 360.0f) * (float)(h - 1));
	_DrawBarMarker(hdc, rc, my);
}

void MTColorPickerDlg::_DrawAlpha(DRAWITEMSTRUCT* di)
{
	HDC hdc = di->hDC;
	RECT rc = di->rcItem;
	int w = rc.right - rc.left;
	int h = rc.bottom - rc.top;
	int x = 0, y = 0;
	int cell = 6;
	if ((w <= 0) || (h <= 0)) return;

	//上=不透明(255) → 下=透明(0)。市松模様の上に現在色をアルファ合成。
	DWORD* buf = new DWORD[(size_t)w * h];
	for (y = 0; y < h; y++) {
		float a = 1.0f - (float)y / (float)((h > 1) ? (h - 1) : 1);
		for (x = 0; x < w; x++) {
			int base = (((x / cell) + (y / cell)) & 1) ? 192 : 255;
			int r = (int)(m_R * a + base * (1.0f - a) + 0.5f);
			int g = (int)(m_G * a + base * (1.0f - a) + 0.5f);
			int b = (int)(m_B * a + base * (1.0f - a) + 0.5f);
			buf[(size_t)y * w + x] = ((DWORD)r << 16) | ((DWORD)g << 8) | (DWORD)b;
		}
	}
	BITMAPINFO bmi; ZeroMemory(&bmi, sizeof(bmi));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = w; bmi.bmiHeader.biHeight = -h;
	bmi.bmiHeader.biPlanes = 1; bmi.bmiHeader.biBitCount = 32; bmi.bmiHeader.biCompression = BI_RGB;
	StretchDIBits(hdc, rc.left, rc.top, w, h, 0, 0, w, h, buf, &bmi, DIB_RGB_COLORS, SRCCOPY);
	delete[] buf;

	//現在のアルファ位置マーカー
	int my = rc.top + (int)((1.0f - (float)m_A / 255.0f) * (float)(h - 1));
	_DrawBarMarker(hdc, rc, my);
}

void MTColorPickerDlg::_DrawPreview(DRAWITEMSTRUCT* di)
{
	HDC hdc = di->hDC;
	RECT rc = di->rcItem;
	int w = rc.right - rc.left;
	int h = rc.bottom - rc.top;
	int x = 0, y = 0;
	int cell = 6;
	float a = (float)m_A / 255.0f;
	if ((w <= 0) || (h <= 0)) return;

	//ImGui の AlphaPreviewHalf と同様、左半分＝不透明色 / 右半分＝アルファ合成（市松）
	int half = w / 2;
	DWORD* buf = new DWORD[(size_t)w * h];
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			int r, g, b;
			if (x < half) {
				//左：透明度を無視した不透明色
				r = m_R; g = m_G; b = m_B;
			}
			else {
				//右：市松模様の上にアルファ合成
				int base = (((x / cell) + (y / cell)) & 1) ? 192 : 255;
				r = (int)(m_R * a + base * (1.0f - a) + 0.5f);
				g = (int)(m_G * a + base * (1.0f - a) + 0.5f);
				b = (int)(m_B * a + base * (1.0f - a) + 0.5f);
			}
			buf[(size_t)y * w + x] = ((DWORD)r << 16) | ((DWORD)g << 8) | (DWORD)b;
		}
	}
	BITMAPINFO bmi; ZeroMemory(&bmi, sizeof(bmi));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = w; bmi.bmiHeader.biHeight = -h;
	bmi.bmiHeader.biPlanes = 1; bmi.bmiHeader.biBitCount = 32; bmi.bmiHeader.biCompression = BI_RGB;
	StretchDIBits(hdc, rc.left, rc.top, w, h, 0, 0, w, h, buf, &bmi, DIB_RGB_COLORS, SRCCOPY);
	delete[] buf;
}
