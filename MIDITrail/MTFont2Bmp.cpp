//******************************************************************************
//
// MIDITrail / MTFont2Bmp
//
// �t�H���g���r�b�g�}�b�v�ϊ��N���X
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
// �R���X�g���N�^
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
// �f�X�g���N�^
//******************************************************************************
MTFont2Bmp::~MTFont2Bmp(void)
{
	Clear();
}

//******************************************************************************
// �N���A
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
// �t�H���g�ݒ�
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
// BMP�쐬
//******************************************************************************
int MTFont2Bmp::CreateBmp(
		const TCHAR* pStr
	)
{
	int result = 0;

	Clear();

	//�_���t�H���g�쐬
	result = _CreateLogFont();
	if (result != 0) goto EXIT;

	//�O���tBMP���X�g���쐬
	result = _CreateGlyphBmpList(pStr);
	if (result != 0) goto EXIT;

	//������S�̂̃o�b�t�@���쐬
	result = _CreateBmpBuf();
	if (result != 0) goto EXIT;

	//�o�b�t�@�ɃO���tBMP����������
	result = _WriteGlyphToBmpBuf();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// BMP�T�C�Y�擾
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
// BMP�s�N�Z���l�擾
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
// �_���t�H���g�쐬
//******************************************************************************
int MTFont2Bmp::_CreateLogFont()
{
	int result = 0;
	LOGFONT logfont;

	//�_���t�H���g���𐶐�
	ZeroMemory(&logfont, sizeof(LOGFONT));
	logfont.lfHeight         = m_FontSize;			//����
	logfont.lfWidth          = 0;					//��
	logfont.lfEscapement     = 0;					//�p�x
	logfont.lfOrientation    = 0;					//�p�x
	logfont.lfWeight         = 0;					//�E�F�C�g
	logfont.lfItalic         = FALSE;				//�C�^���b�N
	logfont.lfUnderline      = FALSE;				//�A���_�[���C��
	logfont.lfStrikeOut      = FALSE;				//�X�g���C�N�A�E�g
	logfont.lfCharSet        = DEFAULT_CHARSET;		//�L�����N�^�Z�b�g
	logfont.lfOutPrecision   = OUT_TT_ONLY_PRECIS;	//�o�͐��x�FTrueType�t�H���g���g�p�i���݂��Ȃ���΃f�t�H���g�̓���j
	logfont.lfClipPrecision  = CLIP_DEFAULT_PRECIS;	//�N���b�s���O���x�F�f�t�H���g�w��
	logfont.lfQuality        = PROOF_QUALITY;		//�i���F�t�H���g�������`��i����D��
	logfont.lfPitchAndFamily = DEFAULT_PITCH		//�s�b�`�F�f�t�H���g
								| FF_DONTCARE;		//�t�@�~���F��ʓI�ȃt�@�~��
	_tcscpy_s(logfont.lfFaceName, LF_FACESIZE, m_FontName);

	if (m_isForceFixedPitch) {
		logfont.lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;
	}

	//�_���t�H���g����
	m_hFont = CreateFontIndirect(&logfont);
	if (m_hFont == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �O���tBMP�쐬
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

	//�f�o�C�X�R���e�L�X�g�擾
	hDC = GetDC(NULL);
	if (hDC == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}

	//�f�o�C�X�R���e�L�X�g�ɘ_���t�H���g��ݒ�
	hOldFont = (HFONT)SelectObject(hDC, m_hFont);
	if (hOldFont == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hDC);
		goto EXIT;
	}

	//�e�L�X�g���g���N�X�擾
	bresult = GetTextMetrics(hDC, &m_TextMetric);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hDC);
		goto EXIT;
	}

	//�r�b�g�}�b�v�쐬�ɕK�v�ȃo�b�t�@�T�C�Y���擾
	size = GetGlyphOutline(
					hDC,				//�f�o�C�X�R���e�L�X�g
					code,				//�����FTODO:�T���Q�[�g�͈����Ȃ��H�H
					GGO_GRAY4_BITMAP,	//�t�H�[�}�b�g�F�r�b�g�}�b�v�i�O���C���x��17�i�j
					&glyphMetric,		//�O���t���g���N�X�F�쐬���ꂽ�����Z���̏�񂪓���
					0,					//�o�b�t�@�T�C�Y�F�[�����w�肵�ĕK�v�ȃT�C�Y�𓾂�
					NULL,				//�o�b�t�@�ʒu
					&mat				//�ϊ��s��
				);
	if (size < 0) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hDC);
		goto EXIT;
	}
	
	//�󔒕����̏ꍇ�̓r�b�g�}�b�v���쐬���Ȃ�
	if (size == 0) {
		//�O���tBMP���
		pGlyphBmp->glyphMetric = glyphMetric;
		pGlyphBmp->bmpHeight   = 0;
		pGlyphBmp->bmpWidth    = 0;
		pGlyphBmp->pBmp        = NULL;
	}
	//�󔒕����ȊO�̓r�b�g�}�b�v���쐬����
	else {

		//�r�b�g�}�b�v�쐬�ɕK�v�ȃ������̈���m�ۂ���
		try {
			pBuf = new BYTE[size];
		}
		catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", size, 0);
			goto EXIT;
		}

		//TrueType�t�H���g�r�b�g�}�b�v�쐬
		size = GetGlyphOutline(
						hDC,				//�f�o�C�X�R���e�L�X�g
						code,				//�����FTODO:�T���Q�[�g�͈����Ȃ��H�H
						GGO_GRAY4_BITMAP,	//�t�H�[�}�b�g�F�r�b�g�}�b�v�i�O���C���x��17�i�j
						&glyphMetric,		//�O���t���g���N�X�F�쐬���ꂽ�����Z���̏�񂪓���
						size,				//�o�b�t�@�T�C�Y
						pBuf,				//�o�b�t�@�ʒu
						&mat				//�ϊ��s��
					);
		if (size <= 0) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
		//�O���tBMP���FBMP����4�̔{���ł��邱�Ƃ��ӎ�����
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
// �O���tBMP���X�g�쐬
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
	
	//1�������ƂɃO���tBMP���쐬
	while (ptr[0] != _T('\0')) {
		code = ptr[0];
		ptr += 1;

		//�O���tBMP�쐬
		result = _CreateGlyphBmp(code, &glyphBmp);
		if (result != 0) goto EXIT;

		//�����񃊃X�g�ɓo�^
		m_GlyphBmpList.push_back(glyphBmp);
	}

#else

	unsigned char* ptr = (unsigned char*)pStr;
	
	//1�������ƂɃO���tBMP���쐬
	while (ptr[0] != '\0') {
		if (IsDBCSLeadByte(ptr[0]) && (ptr[1] != '\0')) {
			code = (ptr[0] << 8) | ptr[1];
			ptr += 2;
		}
		else {
			code = ptr[0];
			ptr += 1;
		}

		//�O���tBMP�쐬
		result = _CreateGlyphBmp(code, &glyphBmp);
		if (result != 0) goto EXIT;

		//�����񃊃X�g�ɓo�^
		m_GlyphBmpList.push_back(glyphBmp);
	}

#endif

EXIT:;
	return result;
}

//******************************************************************************
// ������o�b�t�@�쐬
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

	//����
	m_BmpHeight = m_TextMetric.tmHeight;

	//��
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

	//����4�̔{���ɂ���
	m_BmpWidth = m_BmpWidth + ((4 - (m_BmpWidth % 4)) % 4);

	//�e�N�X�`���Ƃ��ċ��e������ʓI�ȃT�C�Y�𒴂���ꍇ�̓N���b�v����
	if (m_BmpWidth > MTFONT2BMP_MAX_BMP_WIDTH) {
		m_BmpWidth = MTFONT2BMP_MAX_BMP_WIDTH;
	}

	//BMP�o�b�t�@����
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
// �O���tBMP��BMP�o�b�t�@�ɏ�������
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

			//�󕶎��̓X�L�b�v
			if (itr->pBmp == NULL) {
				offsetX += (itr->glyphMetric.gmCellIncX);
				continue;
			}

			//�R�s�[���O���tBMP�̍��W��4�̔{������������BMP�T�C�Y���ӎ�����
			//���f�[�^�͈̔͂ŃX�L��������
			for (y = 0; y < (itr->glyphMetric.gmBlackBoxY); y++) {
				//Ticket #33695 �΍�
				//�R�s�[��̗̈�O�ɂȂ�ꍇ�̓X�L�b�v����
				if (y >= m_BmpHeight) continue;

				for (x = 0; x < (itr->glyphMetric.gmBlackBoxX); x++) {

					//�R�s�[��̗̈�O�ɂȂ�ꍇ�̓X�L�b�v����
					destX = offsetX + (itr->glyphMetric.gmptGlyphOrigin.x) + x;
					if (destX >= (m_BmpWidth-1)) continue;

					//�R�s�[���s�N�Z���|�C���^�FBMP�T�C�Y��4�̔{���������ӎ����ĎZ�o����
					pSrc = itr->pBmp + (itr->bmpWidth * y) + x;

					//�R�s�[��s�N�Z���|�C���^
					pDest = m_pBmpBuf
								+ (m_TextMetric.tmAscent - (itr->glyphMetric.gmptGlyphOrigin.y) + y) * m_BmpWidth
								+ (offsetX + (itr->glyphMetric.gmptGlyphOrigin.x) + x);

					//�m�ۂ����o�b�t�@���z���ď����������Ƃ��Ă��Ȃ����`�F�b�N����
					if (pDest > (m_pBmpBuf + (m_BmpHeight * m_BmpWidth) - 1)) {
						//result = YN_SET_ERR("Program error.", itr->glyphMetric.gmBlackBoxY, itr->glyphMetric.gmBlackBoxX);
						//goto EXIT;
						//Ticket #33695 �΍�
						//�G���[�Ƃ����X�L�b�v����
						continue;
					}

					//�s�N�Z���R�s�[
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

