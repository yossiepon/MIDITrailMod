//******************************************************************************
//
// MIDITrail / MTFont2Bmp
//
// フォント＞ビットマップ変換クラス
//
// Copyright (C) 2010-2014 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTFont2Bmp.h"
#include <new>

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTFont2Bmp::MTFont2Bmp(void)
{
	m_FontName[0] = _T('\0');
	m_FontSize = 0;
	m_isForceFixedPitch = false;
	m_hFont = NULL;
	m_pBmpBuf = NULL;
	ZeroMemory(&m_TextMetric, sizeof(TEXTMETRIC));
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTFont2Bmp::~MTFont2Bmp(void)
{
	Clear();
}

//******************************************************************************
// クリア
//******************************************************************************
void MTFont2Bmp::Clear()
{
	MTGlyphBmpList::iterator itr;

	if (m_hFont != NULL) {
		DeleteObject(m_hFont);
		m_hFont = NULL;
	}

// >>> add 20120728 yossiepon begin
	if(!m_GlyphBmpList.empty()) {
// <<< add 20120728 yossiepon end

		for (itr = m_GlyphBmpList.begin(); itr != m_GlyphBmpList.end(); itr++) {

// >>> add 20120728 yossiepon begin
			if(itr->pBmp != NULL) {
// <<< add 20120728 yossiepon end

				delete [] (itr->pBmp);

// >>> add 20120728 yossiepon begin
			}
// <<< add 20120728 yossiepon end

		}
		m_GlyphBmpList.clear();

// >>> add 20120728 yossiepon begin
	}
// <<< add 20120728 yossiepon end

// >>> add 20120728 yossiepon begin
	if(m_pBmpBuf != NULL) {
// <<< add 20120728 yossiepon end

		delete [] m_pBmpBuf;
		m_pBmpBuf = NULL;

// >>> add 20120728 yossiepon begin
	}
// <<< add 20120728 yossiepon end
}

//******************************************************************************
// フォント設定
//******************************************************************************
int MTFont2Bmp::SetFont(
		const TCHAR* pFontName,
		unsigned long fontSize,
		bool isForceFixedPitch
	)
{
	int result = 0;
	errno_t eresult = 0;

	eresult = _tcscpy_s(m_FontName, LF_FACESIZE, pFontName);
	if (eresult != 0) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	m_FontSize = fontSize;
	m_isForceFixedPitch = isForceFixedPitch;

EXIT:;
	return result;
}

//******************************************************************************
// BMP作成
//******************************************************************************
int MTFont2Bmp::CreateBmp(
		const TCHAR* pStr
	)
{
	int result = 0;

	Clear();

	//論理フォント作成
	result = _CreateLogFont();
	if (result != 0) goto EXIT;

	//グリフBMPリストを作成
	result = _CreateGlyphBmpList(pStr);
	if (result != 0) goto EXIT;

	//文字列全体のバッファを作成
	result = _CreateBmpBuf();
	if (result != 0) goto EXIT;

	//バッファにグリフBMPを書き込む
	result = _WriteGlyphToBmpBuf();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// BMPサイズ取得
//******************************************************************************
void MTFont2Bmp::GetBmpSize(
		unsigned long* pHeight,
		unsigned long* pWidth
	)
{
	*pHeight = m_BmpHeight;
	*pWidth = m_BmpWidth;
}

//******************************************************************************
// BMPピクセル値取得
//******************************************************************************
BYTE MTFont2Bmp::GetBmpPixcel(
		unsigned long x,
		unsigned long y
	)
{
	BYTE pixcel = 0xFF;

	if (m_pBmpBuf == NULL) goto EXIT;

	if ((x >= m_BmpWidth) || (y >= m_BmpHeight)) goto EXIT;

	pixcel = m_pBmpBuf[y*m_BmpWidth + x];

EXIT:;
	return pixcel;
}

//******************************************************************************
// 論理フォント作成
//******************************************************************************
int MTFont2Bmp::_CreateLogFont()
{
	int result = 0;
	LOGFONT logfont;

	//論理フォント情報を生成
	ZeroMemory(&logfont, sizeof(LOGFONT));
	logfont.lfHeight         = m_FontSize;			//高さ
	logfont.lfWidth          = 0;					//幅
	logfont.lfEscapement     = 0;					//角度
	logfont.lfOrientation    = 0;					//角度
	logfont.lfWeight         = 0;					//ウェイト
	logfont.lfItalic         = FALSE;				//イタリック
	logfont.lfUnderline      = FALSE;				//アンダーライン
	logfont.lfStrikeOut      = FALSE;				//ストライクアウト
	logfont.lfCharSet        = DEFAULT_CHARSET;		//キャラクタセット
	logfont.lfOutPrecision   = OUT_TT_ONLY_PRECIS;	//出力精度：TrueTypeフォントを使用（存在しなければデフォルトの動作）
	logfont.lfClipPrecision  = CLIP_DEFAULT_PRECIS;	//クリッピング精度：デフォルト指定
	logfont.lfQuality        = PROOF_QUALITY;		//品質：フォント属性より描画品質を優先
	logfont.lfPitchAndFamily = DEFAULT_PITCH		//ピッチ：デフォルト
								| FF_DONTCARE;		//ファミリ：一般的なファミリ
	_tcscpy_s(logfont.lfFaceName, LF_FACESIZE, m_FontName);

	if (m_isForceFixedPitch) {
		logfont.lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;
	}

	//論理フォント生成
	m_hFont = CreateFontIndirect(&logfont);
	if (m_hFont == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// グリフBMP作成
//******************************************************************************
int MTFont2Bmp::_CreateGlyphBmp(
		unsigned long code,
		MTGlyphBmp* pGlyphBmp
	)
{
	int result = 0;
	HDC hDC = NULL;
	BOOL bresult = FALSE;
	HFONT hOldFont = NULL;
	unsigned long size = 0;
	unsigned char* pBuf = NULL;
	GLYPHMETRICS glyphMetric;
	CONST MAT2 mat = {{0,1},{0,0},{0,0},{0,1}};

	//デバイスコンテキスト取得
	hDC = GetDC(NULL);
	if (hDC == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//デバイスコンテキストに論理フォントを設定
	hOldFont = (HFONT)SelectObject(hDC, m_hFont);
	if (hOldFont == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hDC);
		goto EXIT;
	}

	//テキストメトリクス取得
	bresult = GetTextMetrics(hDC, &m_TextMetric);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hDC);
		goto EXIT;
	}

	//ビットマップ作成に必要なバッファサイズを取得
	size = GetGlyphOutline(
					hDC,				//デバイスコンテキスト
					code,				//文字：TODO:サロゲートは扱えない？？
					GGO_GRAY4_BITMAP,	//フォーマット：ビットマップ（グレイレベル17段）
					&glyphMetric,		//グラフメトリクス：作成された文字セルの情報が入る
					0,					//バッファサイズ：ゼロを指定して必要なサイズを得る
					NULL,				//バッファ位置
					&mat				//変換行列
				);
	if (size < 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hDC);
		goto EXIT;
	}
	
	//空白文字の場合はビットマップを作成しない
	if (size == 0) {
		//グリフBMP情報
		pGlyphBmp->glyphMetric = glyphMetric;
		pGlyphBmp->bmpHeight   = 0;
		pGlyphBmp->bmpWidth    = 0;
		pGlyphBmp->pBmp        = NULL;
	}
	//空白文字以外はビットマップを作成する
	else {

		//ビットマップ作成に必要なメモリ領域を確保する
		try {
			pBuf = new BYTE[size];
		}
		catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", size, 0);
			goto EXIT;
		}

		//TrueTypeフォントビットマップ作成
		size = GetGlyphOutline(
						hDC,				//デバイスコンテキスト
						code,				//文字：TODO:サロゲートは扱えない？？
						GGO_GRAY4_BITMAP,	//フォーマット：ビットマップ（グレイレベル17段）
						&glyphMetric,		//グラフメトリクス：作成された文字セルの情報が入る
						size,				//バッファサイズ
						pBuf,				//バッファ位置
						&mat				//変換行列
					);
		if (size <= 0) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
		//グリフBMP情報：BMP幅は4の倍数であることを意識する
		pGlyphBmp->glyphMetric = glyphMetric;
		pGlyphBmp->bmpHeight   = glyphMetric.gmBlackBoxY;
		pGlyphBmp->bmpWidth    = glyphMetric.gmBlackBoxX + (4 - (glyphMetric.gmBlackBoxX % 4)) % 4;
		pGlyphBmp->pBmp        = pBuf;
	}
	pBuf = NULL;

EXIT:;
	delete [] pBuf;
	if (hDC != NULL) {
		if (hOldFont != NULL) {
			SelectObject(hDC, hOldFont);
		}
		ReleaseDC(NULL, hDC);
	}
	return result;
}

//******************************************************************************
// グリフBMPリスト作成
//******************************************************************************
int MTFont2Bmp::_CreateGlyphBmpList(
		const TCHAR* pStr
	)
{
	int result = 0;
	unsigned long code = 0;
	MTGlyphBmp glyphBmp;

#ifdef _UNICODE

	TCHAR* ptr = pStr;
	
	//1文字ごとにグリフBMPを作成
	while (ptr[0] != _T('\0')) {
		code = ptr[0];
		ptr += 1;

		//グリフBMP作成
		result = _CreateGlyphBmp(code, &glyphBmp);
		if (result != 0) goto EXIT;

		//文字列リストに登録
		m_GlyphBmpList.push_back(glyphBmp);
	}

#else

	unsigned char* ptr = (unsigned char*)pStr;
	
	//1文字ごとにグリフBMPを作成
	while (ptr[0] != '\0') {
		if (IsDBCSLeadByte(ptr[0]) && (ptr[1] != '\0')) {
			code = (ptr[0] << 8) | ptr[1];
			ptr += 2;
		}
		else {
			code = ptr[0];
			ptr += 1;
		}

		//グリフBMP作成
		result = _CreateGlyphBmp(code, &glyphBmp);
		if (result != 0) goto EXIT;

		//文字列リストに登録
		m_GlyphBmpList.push_back(glyphBmp);
	}

#endif

EXIT:;
	return result;
}

//******************************************************************************
// 文字列バッファ作成
//******************************************************************************
int MTFont2Bmp::_CreateBmpBuf()
{
	int result = 0;
	MTGlyphBmpList::iterator itr;
	
	//            |<- gmCellIncX ->|
	//  ----------0----------------+----->x
	//   ^  ^     |  |<---bx--->|  |
	//   |  | ----|--@----------+--|----  @ = (gmptGlyphOrigin.x, ta-gy)
	//   |  |  ^  |  | *       *|  | ^
	//   |  |  |  |  |  *     * |  | |    th: tmHeight
	//   th ta gy |  |   *   *  |  | by   ta: tmAscent
	//   |  |  |  |  |    * *   |  | |    gy: gmptGlyphOrigin.y
	//   |  v  v  |  |     *    |  | |    bx: gmBlackBoxX
	//   | =======|==|=== * ====|==| |    by: gmBlackBoxY
	//   |        |  |   *      |  | v    ==: base line
	//   |        |--+----------+--|----
	//   v        |  |          |  |
	//  ----------+----------------+
	//            |
	//            v
	//            y

	//高さ
	m_BmpHeight = m_TextMetric.tmHeight;

	//幅
	m_BmpWidth = 0;

// >>> add 20120728 yossiepon begin
	if(!m_GlyphBmpList.empty()) {
// <<< add 20120728 yossiepon end

		for (itr = m_GlyphBmpList.begin(); itr != m_GlyphBmpList.end(); itr++) {
			m_BmpWidth += (itr->glyphMetric.gmCellIncX);
		}

// >>> add 20120728 yossiepon begin
	}
// <<< add 20120728 yossiepon end

	//幅を4の倍数にする
	m_BmpWidth = m_BmpWidth + ((4 - (m_BmpWidth % 4)) % 4);

	//テクスチャとして許容される一般的なサイズを超える場合はクリップする
	if (m_BmpWidth > MTFONT2BMP_MAX_BMP_WIDTH) {
		m_BmpWidth = MTFONT2BMP_MAX_BMP_WIDTH;
	}

	//BMPバッファ生成
	try {
		m_pBmpBuf = new BYTE[(m_BmpHeight * m_BmpWidth)];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", m_BmpHeight, m_BmpWidth);
		goto EXIT;
	}
	ZeroMemory(m_pBmpBuf, (m_BmpHeight * m_BmpWidth));

EXIT:;
	return result;
}

//******************************************************************************
// グリフBMPをBMPバッファに書き込む
//******************************************************************************
int MTFont2Bmp::_WriteGlyphToBmpBuf()
{
	int result = 0;
	MTGlyphBmpList::iterator itr;
	unsigned long offsetX = 0;
	unsigned long x = 0;
	unsigned long y = 0;
	unsigned long destX = 0;
	BYTE* pSrc = NULL;
	BYTE* pDest = NULL;

// >>> add 20120728 yossiepon begin
	if(!m_GlyphBmpList.empty()) {
// <<< add 20120728 yossiepon end

		for (itr = m_GlyphBmpList.begin(); itr != m_GlyphBmpList.end(); itr++) {

			//空文字はスキップ
			if (itr->pBmp == NULL) {
				offsetX += (itr->glyphMetric.gmCellIncX);
				continue;
			}

			//コピー元グリフBMPの座標は4の倍数制限があるBMPサイズを意識せず
			//実データの範囲でスキャンする
			for (y = 0; y < (itr->glyphMetric.gmBlackBoxY); y++) {
				//Ticket #33695 対策
				//コピー先の領域外になる場合はスキップする
				if (y >= m_BmpHeight) continue;

				for (x = 0; x < (itr->glyphMetric.gmBlackBoxX); x++) {

					//コピー先の領域外になる場合はスキップする
					destX = offsetX + (itr->glyphMetric.gmptGlyphOrigin.x) + x;
					if (destX >= (m_BmpWidth-1)) continue;

					//コピー元ピクセルポインタ：BMPサイズの4の倍数制限を意識して算出する
					pSrc = itr->pBmp + (itr->bmpWidth * y) + x;

					//コピー先ピクセルポインタ
					pDest = m_pBmpBuf
								+ (m_TextMetric.tmAscent - (itr->glyphMetric.gmptGlyphOrigin.y) + y) * m_BmpWidth
								+ (offsetX + (itr->glyphMetric.gmptGlyphOrigin.x) + x);

					//確保したバッファを越えて書き込もうとしていないかチェックする
					if (pDest > (m_pBmpBuf + (m_BmpHeight * m_BmpWidth) - 1)) {
						//result = YN_SET_ERR("Program error.", itr->glyphMetric.gmBlackBoxY, itr->glyphMetric.gmBlackBoxX);
						//goto EXIT;
						//Ticket #33695 対策
						//エラーとせずスキップする
						continue;
					}

					//ピクセルコピー
					*pDest = *pSrc;
				}
			}
			offsetX += (itr->glyphMetric.gmCellIncX);
		}

// >>> add 20120728 yossiepon begin
	}
// <<< add 20120728 yossiepon end

//EXIT:;
	return result;
}

