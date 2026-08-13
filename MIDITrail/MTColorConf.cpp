//******************************************************************************
//
// MIDITrail / MTColorConf
//
// Color configuration class.
//
// Copyright (C) 2022 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025-2026 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTColorConf.h"
#include "DXColorUtil.h"

using namespace YNBaseLib;
using namespace DirectX::SimpleMath;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTColorConf::MTColorConf(void)
{
	unsigned long i = 0;

	m_SelectedColorPaletteNo = 0;
	
	for (i = 0; i < MT_COLOR_PALETTE_NUM_MAX; i++) {
		m_pColorPalette[i] = NULL;
	}

	return;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTColorConf::~MTColorConf(void)
{
	unsigned long i = 0;
	
	for (i = 0; i < MT_COLOR_PALETTE_NUM_MAX; i++) {
		if (m_pColorPalette[i] != NULL) {
			delete m_pColorPalette[i];
		}
		m_pColorPalette[i] = NULL;
	}
	
	return;
}

//******************************************************************************
// ������
//******************************************************************************
int MTColorConf::Initialize(const TCHAR* pDefaultSceneName)
{
	int result = 0;
	int i = 0;
		
	//�F�p���b�g�����Ə�����
	for (i = 0; i < MT_COLOR_PALETTE_NUM_MAX; i++) {
		try {
			m_pColorPalette[i] = new MTColorPalette();
		}
		catch (std::bad_alloc) {
			result = YN_SET_ERR("Could not allocate memory.", i, 0);
			goto EXIT;
		}
		result = m_pColorPalette[i]->Initialize();
		if (result != 0) goto EXIT;
	}
	
	//�ݒ�t�@�C��������
	result = _InitConfFile();
	if (result != 0) goto EXIT;
	
	//���[�U�ݒ�ǂݍ���
	result = _LoadColorConf(pDefaultSceneName);
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// �I���J���[�p���b�g�ԍ��擾�F0 �f�t�H���g�A1-6 �p���b�g�ԍ�
//******************************************************************************
unsigned long MTColorConf::GetSelectedColorPaletteNo()
{
	return m_SelectedColorPaletteNo;
}

//******************************************************************************
// �I���J���[�p���b�g�ԍ��o�^�F0 �f�t�H���g�A1-6 �p���b�g�ԍ�
//******************************************************************************
int MTColorConf::SetSelectedColorPaletteNo(unsigned long paletteNo)
{
	int result = 0;
	
	if (paletteNo >= MT_COLOR_PALETTE_NUM_MAX) {
		result = YN_SET_ERR("Program error.", paletteNo, 0);
		goto EXIT;
	}
	
	m_SelectedColorPaletteNo = paletteNo;
	
EXIT:;
	return result;
}

//******************************************************************************
// �J���[�p���b�g�擾�F0 �f�t�H���g�A1-6 �p���b�g�ԍ�
//******************************************************************************
int MTColorConf::GetColorPalette(
		unsigned long paletteNo,
		MTColorPalette* pColorPalette
	)
{
	int result = 0;
	
	if (paletteNo >= MT_COLOR_PALETTE_NUM_MAX) {
		result = YN_SET_ERR("Program error.", paletteNo, 0);
		goto EXIT;
	}
	
	pColorPalette->CopyFrom(m_pColorPalette[paletteNo]);
	
EXIT:;
	return result;
}

//******************************************************************************
// �I���J���[�p���b�g�擾
//******************************************************************************
void MTColorConf::GetSelectedColorPalette(MTColorPalette* pColorPalette)
{
	pColorPalette->CopyFrom(m_pColorPalette[m_SelectedColorPaletteNo]);
}

//******************************************************************************
// �J���[�p���b�g�o�^�F1-6 �p���b�g�ԍ��A0 �f�t�H���g�͓o�^�s��
//******************************************************************************
int MTColorConf::SetColorPalette(
		unsigned long paletteNo,
		MTColorPalette* pColorPalette
	)
{
	int result = 0;
	
	//�f�t�H���g0�̃p���b�g�͏��������s��
	if ((paletteNo == 0) || (paletteNo >= MT_COLOR_PALETTE_NUM_MAX)) {
		result = YN_SET_ERR("Program error.", paletteNo, 0);
		goto EXIT;
	}
	
	m_pColorPalette[paletteNo]->CopyFrom(pColorPalette);
	
EXIT:;
	return result;
}

//******************************************************************************
// �ݒ�t�@�C��������
//******************************************************************************
int MTColorConf::_InitConfFile()
{
	int result = 0;
	TCHAR userConfFilePath[_MAX_PATH] = {_T('\0')};

	result = YNPathUtil::GetAppDataDirPath(userConfFilePath, _MAX_PATH);
	if (result != 0) goto EXIT;

	_tcscat_s(userConfFilePath, _MAX_PATH, MT_USER_CONFFILE_DIR);
	_tcscat_s(userConfFilePath, _MAX_PATH, MT_USER_CONFFILE_COLOR);

	result = m_ConfFile.Initialize(userConfFilePath);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ���[�U�ݒ�ǂݍ���
//******************************************************************************
int MTColorConf::_LoadColorConf(const TCHAR* pDefaultSceneName)
{
	int result = 0;
	unsigned long paletteNo = 0;
	
	//�Z�N�V�����ݒ�
	result = m_ConfFile.SetCurSection(_T("ColorSelect"));
	if (result != 0) goto EXIT;
	
	//���[�U�ݒ�l�擾�F�I���J���[�p���b�g�ԍ�
	result = m_ConfFile.GetInt(_T("SelectedColorPaletteNo"), &m_SelectedColorPaletteNo, 0);
	if (result != 0) goto EXIT;
	if ((m_SelectedColorPaletteNo < 0) || (m_SelectedColorPaletteNo >= MT_COLOR_PALETTE_NUM_MAX)) {
		m_SelectedColorPaletteNo = 0;
	}
	
	//�f�t�H���g�J���[�p���b�g�ǂݍ���
	result = _LoadColorPaletteDefault(pDefaultSceneName, m_pColorPalette[0]);
	if (result != 0) goto EXIT;

	//�J���[�p���b�g�ݒ�ǂݍ���
	for (paletteNo = 1; paletteNo < MT_COLOR_PALETTE_NUM_MAX; paletteNo++) {
		result = _LoadColorPalettes(paletteNo, m_pColorPalette[paletteNo]);
		if (result != 0) goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// �f�t�H���g�J���[�p���b�g�ǂݍ���
//******************************************************************************
int MTColorConf::_LoadColorPaletteDefault(
		const TCHAR* pDefaultSceneName,
		MTColorPalette* pColorPalette
	)
{
	int result = 0;
	unsigned long chNo = 0;
	TCHAR key[32] = {_T('\0')};
	TCHAR hexColor[16] = {_T('\0')};
	MTConfFile confFile;
	
	//�ݒ�t�@�C���ǂݍ���
	result = confFile.Initialize(pDefaultSceneName);
	if (result != 0) goto EXIT;
	
	//�Z�N�V�����w��
	result = confFile.SetCurSection(_T("Color"));
	if (result != 0) goto EXIT;
	
	//�`�����l���F�擾
	for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
		_stprintf_s(key, 32, _T("Ch-%02d-NoteRGBA"), chNo+1);
		result = confFile.GetStr(key, hexColor, 16, _T("FFFFFFFF"));
		if (result != 0) goto EXIT;
		pColorPalette->SetChColor(chNo, DXColorUtil::MakeColorFromHexRGBA(hexColor));
	}
	//�w�i�F�擾（RGB 6桁 → RGBA 8桁に変換して読み込み）
	result = confFile.GetStr(_T("BackGroundRGB"), hexColor, 16, _T("000000"));
	if (result != 0) goto EXIT;
	{
		TCHAR hexRGBA[16] = {0};
		_stprintf_s(hexRGBA, 16, _T("%sFF"), hexColor);
		pColorPalette->SetBackgroundColor(DXColorUtil::MakeColorFromHexRGBA(hexRGBA));
	}
	
	//�O���b�h���C���F�擾
	result = confFile.GetStr(_T("GridLineRGBA"), hexColor, 16, "444444FF");
	if (result != 0) goto EXIT;
	pColorPalette->SetGridLineColor(DXColorUtil::MakeColorFromHexRGBA(hexColor));
	
	//�J�E���^�[�F�擾
	result = confFile.GetStr(_T("CaptionRGBA"), hexColor, 16, "FFFFFFFF");
	if (result != 0) goto EXIT;
	pColorPalette->SetCounterColor(DXColorUtil::MakeColorFromHexRGBA(hexColor));

EXIT:;
	return result;
}

//******************************************************************************
// �J���[�p���b�g�ǂݍ���
//******************************************************************************
int MTColorConf::_LoadColorPalettes(
		unsigned long paletteNo,
		MTColorPalette* pColorPalette
	)
{
	int result = 0;
	unsigned long chNo = 0;
	TCHAR section[32] = {_T('\0')};
	TCHAR key[32] = {_T('\0')};
	TCHAR hexColor[16] = {_T('\0')};
	
	//�Z�N�V�����ݒ�
	_stprintf_s(section, 32, _T("ColorPalette-%u"), paletteNo);
	result = m_ConfFile.SetCurSection(section);
	if (result != 0) goto EXIT;
	
	//�`�����l���F�擾
	for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
		_stprintf_s(key, 32, _T("Ch-%02u-NoteRGBA"), chNo+1);
		result = m_ConfFile.GetStr(key, hexColor, 16, _T("FFFFFFFF"));
		if (result != 0) goto EXIT;
		pColorPalette->SetChColor(chNo, DXColorUtil::MakeColorFromHexRGBA(hexColor));
	}
	
	//�w�i�F�擾
	result = m_ConfFile.GetStr(_T("BackGroundRGBA"), hexColor, 16, _T("000000FF"));
	if (result != 0) goto EXIT;
	pColorPalette->SetBackgroundColor(DXColorUtil::MakeColorFromHexRGBA(hexColor));
	
	//�O���b�h���C���F�擾
	result = m_ConfFile.GetStr(_T("GridLineRGBA"), hexColor, 16, _T("444444FF"));
	if (result != 0) goto EXIT;
	pColorPalette->SetGridLineColor(DXColorUtil::MakeColorFromHexRGBA(hexColor));
	
	//�J�E���^�[�F�擾
	result = m_ConfFile.GetStr(_T("CaptionRGBA"), hexColor, 16, _T("FFFFFFFF"));
	if (result != 0) goto EXIT;
	pColorPalette->SetCounterColor(DXColorUtil::MakeColorFromHexRGBA(hexColor));
	
EXIT:;
	return result;
}

//******************************************************************************
// �ݒ�ۑ�
//******************************************************************************
int MTColorConf::Save()
{
	int result = 0;
	unsigned long paletteNo = 0;
	
	//�I���J���[�p���b�g�ԍ��ۑ�
	result = m_ConfFile.SetCurSection(_T("ColorSelect"));
	if (result != 0) goto EXIT;
	result = m_ConfFile.SetInt(_T("SelectedColorPaletteNo"), m_SelectedColorPaletteNo);
	if (result != 0) goto EXIT;
	
	//�J���[�p���b�g 1-6 �ۑ�
	for (paletteNo = 1; paletteNo < MT_COLOR_PALETTE_NUM_MAX; paletteNo++) {
		result = _SaveColorPalette(paletteNo, m_pColorPalette[paletteNo]);
		if (result != 0) goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// �J���[�p���b�g�ۑ�
//******************************************************************************
int MTColorConf::_SaveColorPalette(
		unsigned long paletteNo,
		MTColorPalette* pColorPalette
	)
{
	int result = 0;
	unsigned long chNo = 0;
	TCHAR section[32] = {_T('\0')};
	TCHAR key[32] = {_T('\0')};
	TCHAR hexColor[16] = {_T('\0')};
	Color color;
	
	//�Z�N�V�����ݒ�
	_stprintf_s(section, 32, _T("ColorPalette-%u"), paletteNo);
	result = m_ConfFile.SetCurSection(section);
	if (result != 0) goto EXIT;
	
	//�`�����l���F�o�^
	for (chNo = 0; chNo < SM_MAX_CH_NUM; chNo++) {
		_stprintf_s(key, 32, _T("Ch-%02u-NoteRGBA"), chNo+1);
		result = pColorPalette->GetChColor(chNo, &color);
		if (result != 0) goto EXIT;
		DXColorUtil::MakeHexRGBAFromColor(color, hexColor, 16);
		result = m_ConfFile.SetStr(key, hexColor);
		if (result != 0) goto EXIT;
	}
	
	//�w�i�F�o�^
	pColorPalette->GetBackgroundColor(&color);
	DXColorUtil::MakeHexRGBAFromColor(color, hexColor, 16);
	result = m_ConfFile.SetStr(_T("BackGroundRGBA"), hexColor);
	if (result != 0) goto EXIT;
	
	//�O���b�h���C���F�o�^
	pColorPalette->GetGridLineColor(&color);
	DXColorUtil::MakeHexRGBAFromColor(color, hexColor, 16);
	result = m_ConfFile.SetStr(_T("GridLineRGBA"), hexColor);
	if (result != 0) goto EXIT;
	
	//�J�E���^�[�F�o�^
	pColorPalette->GetCounterColor(&color);
	DXColorUtil::MakeHexRGBAFromColor(color, hexColor, 16);
	result = m_ConfFile.SetStr(_T("CaptionRGBA"), hexColor);
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}


