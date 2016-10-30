//******************************************************************************
//
// MIDITrail / MTFont2Bmp
//
// �t�H���g���r�b�g�}�b�v�ϊ��N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// �w�蕶���񂪏������܂ꂽ�r�b�g�}�b�v���쐬����B
// 1�s�̕�����ɂ̂ݑΉ�����B�����s�͑Ή����Ă��Ȃ��B
// �r�b�g�}�b�v�̃T�C�Y�͉�����4�̔{���ɂȂ�悤�ɒ�������B

#pragma once

#include <list>

//******************************************************************************
// �p�����[�^��`
//******************************************************************************
//�ő�r�b�g�}�b�v�T�C�Y�F�e�N�X�`���摜�ŋ��e������ʓI�ȍő�T�C�Y
#define MTFONT2BMP_MAX_BMP_WIDTH  (2048)

//******************************************************************************
// �t�H���g���r�b�g�}�b�v�ϊ��N���X
//******************************************************************************
class MTFont2Bmp
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTFont2Bmp(void);
	virtual ~MTFont2Bmp(void);

	//�N���A
	void Clear();
	
	//�t�H���g�ݒ�
	//  �����I�ɌŒ�s�b�`�ɂ���ꍇ��isForceFixedPitch��true���w�肷��
	int SetFont(const TCHAR* pFontName, unsigned long fontSize, bool isForceFixedPitch = false);

	//�r�b�g�}�b�v�쐬
	int CreateBmp(const TCHAR* pStr);
	
	//�r�b�g�}�b�v�T�C�Y�擾
	void GetBmpSize(unsigned long* pHeight, unsigned long* pWidth);
	
	//�r�b�g�}�b�v�s�N�Z���擾
	//  �K����17�i(0x00�`0x10)�̃s�N�Z���l��Ԃ�
	//  �͈͊O���w�肷���0xFF��Ԃ�
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

