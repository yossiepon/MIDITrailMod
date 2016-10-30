//******************************************************************************
//
// MIDITrail / MTFont2Bmp
//
// フォント＞ビットマップ変換クラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// 指定文字列が書き込まれたビットマップを作成する。
// 1行の文字列にのみ対応する。複数行は対応していない。
// ビットマップのサイズは横幅が4の倍数になるように調整する。

#pragma once

#include <list>

//******************************************************************************
// パラメータ定義
//******************************************************************************
//最大ビットマップサイズ：テクスチャ画像で許容される一般的な最大サイズ
#define MTFONT2BMP_MAX_BMP_WIDTH  (2048)

//******************************************************************************
// フォント＞ビットマップ変換クラス
//******************************************************************************
class MTFont2Bmp
{
public:

	//コンストラクタ／デストラクタ
	MTFont2Bmp(void);
	virtual ~MTFont2Bmp(void);

	//クリア
	void Clear();
	
	//フォント設定
	//  強制的に固定ピッチにする場合はisForceFixedPitchにtrueを指定する
	int SetFont(const TCHAR* pFontName, unsigned long fontSize, bool isForceFixedPitch = false);

	//ビットマップ作成
	int CreateBmp(const TCHAR* pStr);
	
	//ビットマップサイズ取得
	void GetBmpSize(unsigned long* pHeight, unsigned long* pWidth);
	
	//ビットマップピクセル取得
	//  階調数17段(0x00～0x10)のピクセル値を返す
	//  範囲外を指定すると0xFFを返す
	BYTE GetBmpPixcel(unsigned long x, unsigned long y);

private:

	typedef struct {
		GLYPHMETRICS glyphMetric;
		unsigned long bmpHeight;
		unsigned long bmpWidth;
		unsigned char* pBmp;
	} MTGlyphBmp;

	typedef std::list<MTGlyphBmp> MTGlyphBmpList;

private:

	TCHAR m_FontName[LF_FACESIZE];
	unsigned long m_FontSize;
	bool m_isForceFixedPitch;

	HFONT m_hFont;
	TEXTMETRIC m_TextMetric;
	MTGlyphBmpList m_GlyphBmpList;

	BYTE* m_pBmpBuf;
	unsigned long m_BmpHeight;
	unsigned long m_BmpWidth;
	
	int _CreateLogFont();
	int _CreateGlyphBmp(unsigned long code, MTGlyphBmp* pGB);
	int _CreateGlyphBmpList(const TCHAR* pStr);
	int _CreateBmpBuf();
	int _WriteGlyphToBmpBuf();

};

