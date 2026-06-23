//******************************************************************************
//
// MIDITrail / MTVideoExportDlg
//
// M6: video export settings dialog (codec / resolution / fps / quality).
//
//******************************************************************************

#pragma once

#include <windows.h>
#include "MTVideoExporter.h"


//******************************************************************************
// video export settings dialog
//******************************************************************************
class MTVideoExportDlg
{
public:

	MTVideoExportDlg();

	// Modal. pParams is in/out: width/height/fps/codec are pre-filled as defaults
	// and, on OK, overwritten with the user's choices (transparent is derived from
	// the chosen codec). Returns true on OK, false on Cancel.
	bool Show(HWND hParentWnd, MTVideoExportParams* pParams);

private:

	static MTVideoExportDlg* m_pThis;
	MTVideoExportParams* m_pParams;
	bool m_isOK;

	static INT_PTR CALLBACK _WndProc(HWND, UINT, WPARAM, LPARAM);
	INT_PTR _WndProcImpl(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void _OnInitDlg(HWND hDlg);
	void _OnOK(HWND hDlg);
};
