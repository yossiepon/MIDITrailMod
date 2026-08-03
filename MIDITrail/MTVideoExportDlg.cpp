//******************************************************************************
//
// MIDITrail / MTVideoExportDlg
//
//******************************************************************************

#include "StdAfx.h"
#include "MTVideoExportDlg.h"
#include "Resource.h"
#include <tchar.h>
#include <stdio.h>

MTVideoExportDlg* MTVideoExportDlg::m_pThis = NULL;

// codec combo entries (index == MTVideoCodec value)
static const TCHAR* g_CodecNames[] = {
	_T("H.264  (NVENC, NVIDIA GPU)"),
	_T("H.265 / HEVC  (NVENC, NVIDIA GPU)"),
	_T("H.264  (QSV, Intel GPU)"),
	_T("H.265 / HEVC  (QSV, Intel GPU)"),
	_T("H.264  (libx264, CPU)"),
	_T("H.265 / HEVC  (libx265, CPU)"),
	_T("Transparent .mov  (QuickTime RLE)"),
	_T("Transparent .mov  (ProRes 4444)"),
	_T("Transparent .mkv  (FFV1, lossless)"),
	_T("H.264  (AMF, AMD GPU)"),
	_T("H.265 / HEVC  (AMF, AMD GPU)"),
};
static const int g_CodecCount = (int)(sizeof(g_CodecNames) / sizeof(g_CodecNames[0]));


MTVideoExportDlg::MTVideoExportDlg()
{
	m_pParams = NULL;
	m_isOK = false;
}

bool MTVideoExportDlg::Show(HWND hParentWnd, MTVideoExportParams* pParams)
{
	if (pParams == NULL) return false;
	m_pThis = this;
	m_pParams = pParams;
	m_isOK = false;

	DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_VIDEO_EXPORT),
		hParentWnd, (DLGPROC)_WndProc, (LPARAM)this);

	m_pThis = NULL;
	return m_isOK;
}

INT_PTR CALLBACK MTVideoExportDlg::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (m_pThis == NULL) return (INT_PTR)FALSE;
	return m_pThis->_WndProcImpl(hWnd, message, wParam, lParam);
}

INT_PTR MTVideoExportDlg::_WndProcImpl(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
		case WM_INITDIALOG:
			_OnInitDlg(hWnd);
			return (INT_PTR)TRUE;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					_OnOK(hWnd);
					EndDialog(hWnd, IDOK);
					return (INT_PTR)TRUE;
				case IDCANCEL:
					m_isOK = false;
					EndDialog(hWnd, IDCANCEL);
					return (INT_PTR)TRUE;
			}
			break;
	}
	return (INT_PTR)FALSE;
}

void MTVideoExportDlg::_OnInitDlg(HWND hDlg)
{
	TCHAR buf[32];

	// codec combo
	HWND hCombo = GetDlgItem(hDlg, IDC_COMBO_VE_CODEC);
	for (int i = 0; i < g_CodecCount; i++) {
		SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)g_CodecNames[i]);
	}
	int sel = (int)m_pParams->codec;
	if (sel < 0 || sel >= g_CodecCount) sel = MTVC_H264_CPU;
	SendMessage(hCombo, CB_SETCURSEL, sel, 0);

	// resolution / fps / quality
	_sntprintf_s(buf, _countof(buf), _TRUNCATE, _T("%d"), m_pParams->width);
	SetDlgItemText(hDlg, IDC_EDIT_VE_WIDTH, buf);
	_sntprintf_s(buf, _countof(buf), _TRUNCATE, _T("%d"), m_pParams->height);
	SetDlgItemText(hDlg, IDC_EDIT_VE_HEIGHT, buf);
	_sntprintf_s(buf, _countof(buf), _TRUNCATE, _T("%d"), m_pParams->fps > 0 ? m_pParams->fps : 60);
	SetDlgItemText(hDlg, IDC_EDIT_VE_FPS, buf);
	_sntprintf_s(buf, _countof(buf), _TRUNCATE, _T("%d"), m_pParams->quality);
	SetDlgItemText(hDlg, IDC_EDIT_VE_QUALITY, buf);

	CheckDlgButton(hDlg, IDC_CHECK_VE_360, m_pParams->equirect360 ? BST_CHECKED : BST_UNCHECKED);
}

void MTVideoExportDlg::_OnOK(HWND hDlg)
{
	TCHAR buf[32];

	int sel = (int)SendMessage(GetDlgItem(hDlg, IDC_COMBO_VE_CODEC), CB_GETCURSEL, 0, 0);
	if (sel < 0 || sel >= g_CodecCount) sel = MTVC_H264_CPU;
	m_pParams->codec = (MTVideoCodec)sel;
	m_pParams->transparent = MTVideoCodecIsAlpha(m_pParams->codec);

	GetDlgItemText(hDlg, IDC_EDIT_VE_WIDTH, buf, _countof(buf));
	int w = _ttoi(buf);
	GetDlgItemText(hDlg, IDC_EDIT_VE_HEIGHT, buf, _countof(buf));
	int h = _ttoi(buf);
	GetDlgItemText(hDlg, IDC_EDIT_VE_FPS, buf, _countof(buf));
	int fps = _ttoi(buf);
	GetDlgItemText(hDlg, IDC_EDIT_VE_QUALITY, buf, _countof(buf));
	int q = _ttoi(buf);

	// clamp + force even dimensions (yuv420p needs even W/H)
	if (w < 16) w = 16;
	if (h < 16) h = 16;
	w &= ~1; h &= ~1;
	if (fps < 1) fps = 1;
	if (fps > 240) fps = 240;
	if (q < 0) q = 0;

	m_pParams->equirect360 = (IsDlgButtonChecked(hDlg, IDC_CHECK_VE_360) == BST_CHECKED);
	// equirectangular is always 2:1; force the height (the export also enforces this)
	if (m_pParams->equirect360) { h = w / 2; h &= ~1; if (h < 16) h = 16; }

	m_pParams->width = w;
	m_pParams->height = h;
	m_pParams->fps = fps;
	m_pParams->quality = q;

	m_isOK = true;
}
