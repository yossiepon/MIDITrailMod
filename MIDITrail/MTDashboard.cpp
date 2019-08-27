//******************************************************************************
//
// MIDITrail / MTDashboard
//
// �_�b�V���{�[�h�`��N���X
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "DXColorUtil.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTDashboard.h"
#include <string>

using namespace YNBaseLib;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTDashboard::MTDashboard(void)
{
	m_hWnd = NULL;
	m_PosCounterX = 0.0f;
	m_PosCounterY = 0.0f;
	m_CounterMag = MTDASHBOARD_DEFAULT_MAGRATE;

	m_PlayTimeSec = 0;
	m_TotalPlayTimeSec = 0;
	m_TempoBPM = 0;
	m_BeatNumerator = 0;
	m_BeatDenominator = 0;
	m_BarNo = 0;
	m_BarNum = 0;
	m_NoteCount = 0;
	m_NoteNum = 0;
	m_PlaySpeedRatio = 100;

	m_TempoBPMOnStart = 0;
	m_BeatNumeratorOnStart = 0;
	m_BeatDenominatorOnStart = 0;

	m_CaptionColor = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

	m_isEnable = true;
	m_isEnableFileName = false;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTDashboard::~MTDashboard(void)
{
	Release();
}

//******************************************************************************
// �_�b�V���{�[�h����
//******************************************************************************
int MTDashboard::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData,
		HWND hWnd
   )
{
	int result = 0;
	std::string title;
	std::string fileName;
	SMTrack track;
	SMNoteList noteList;
	TCHAR counter[100];

	Release();

	if (pSeqData == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	m_hWnd = hWnd;

	//�ݒ�ǂݍ���
	result = _LoadConfFile(pSceneName);
	if (result != 0) goto EXIT;

	//�^�C�g���L���v�V����
	//TODO: UNICODE�Ńr���h�ɂ͑Ή����Ă��Ȃ�
	//title = "TITLE: ";
	title = "";
	title += pSeqData->GetTitle();
	if (title.size() == 0) {
		//�󕶎��ł̓e�N�X�`�������ŃG���[�ƂȂ邽�ߋ󔒕����Ƃ���
		title += " ";
	}
	result = m_Title.Create(
					pD3DDevice,
					MTDASHBOARD_FONTNAME,	//�t�H���g����
					MTDASHBOARD_FONTSIZE,	//�t�H���g�T�C�Y
					(TCHAR*)title.c_str()	//�L���v�V����
				);
	if (result != 0) goto EXIT;
	m_Title.SetColor(m_CaptionColor);

	//�t�@�C�����L���v�V����
	//TODO: UNICODE�Ńr���h�ɂ͑Ή����Ă��Ȃ�
	fileName = "";
	fileName = pSeqData->GetFileName();
	result = m_FileName.Create(
					pD3DDevice,
					MTDASHBOARD_FONTNAME,	//�t�H���g����
					MTDASHBOARD_FONTSIZE,	//�t�H���g�T�C�Y
					(TCHAR*)fileName.c_str()	//�t�@�C����
				);
	if (result != 0) goto EXIT;
	m_FileName.SetColor(m_CaptionColor);

	//�J�E���^�L���v�V����
	result = m_Counter.Create(
					pD3DDevice,
					MTDASHBOARD_FONTNAME,		//�t�H���g����
					MTDASHBOARD_FONTSIZE,		//�t�H���g�T�C�Y
					MTDASHBOARD_COUNTER_CHARS,	//�\������
					MTDASHBOARD_COUNTER_SIZE	//�L���v�V�����T�C�Y
				);
	if (result != 0) goto EXIT;
	m_Counter.SetColor(m_CaptionColor);

	//�S�̉��t����
	SetTotalPlayTimeSec(pSeqData->GetTotalPlayTime()/1000);

	//�e���|(BPM)
	SetTempoBPM(pSeqData->GetTempoBPM());
	m_TempoBPMOnStart = pSeqData->GetTempoBPM();

	//���q�L��
	SetBeat(pSeqData->GetBeatNumerator(), pSeqData->GetBeatDenominator());
	m_BeatNumeratorOnStart = pSeqData->GetBeatNumerator();
	m_BeatDenominatorOnStart = pSeqData->GetBeatDenominator();

	//���ߔԍ�
	SetBarNo(1);

	//���ߐ�
	SetBarNum(pSeqData->GetBarNum());

	result = pSeqData->GetMergedTrack(&track);
	if (result != 0) goto EXIT;

	result = track.GetNoteList(&noteList);
	if (result != 0) goto EXIT;

	m_NoteNum = noteList.GetSize();

	result = _GetCounterStr(counter, 100);
	if (result != 0) goto EXIT;

	result = m_Counter.SetString(counter);
	if (result != 0) goto EXIT;
	
	//�J�E���^�\���ʒu���Z�o
	result = _GetCounterPos(&m_PosCounterX, &m_PosCounterY);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �ړ�
//******************************************************************************
int MTDashboard::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice,
		D3DXVECTOR3 camVector
	)
{
	int result = 0;
	return result;
}

//******************************************************************************
// �`��
//******************************************************************************
int MTDashboard::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	D3DXMATRIX mtxWorld;
	TCHAR counter[100];

	if (pD3DDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	if (!m_isEnable) goto EXIT;

	if (m_isEnableFileName) {
		//�t�@�C�����`��F�J�E���^�Ɠ����g�嗦�ŕ\������
		result = m_FileName.Draw(pD3DDevice, MTDASHBOARD_FRAMESIZE, MTDASHBOARD_FRAMESIZE, m_CounterMag);
		if (result != 0) goto EXIT;
	}
	else {
		//�^�C�g���`��F�J�E���^�Ɠ����g�嗦�ŕ\������
		result = m_Title.Draw(pD3DDevice, MTDASHBOARD_FRAMESIZE, MTDASHBOARD_FRAMESIZE, m_CounterMag);
		if (result != 0) goto EXIT;
	}

	//�J�E���^������`��
	result = _GetCounterStr(counter, 100);
	if (result != 0) goto EXIT;

	result = m_Counter.SetString(counter);
	if (result != 0) goto EXIT;

	result = m_Counter.Draw(pD3DDevice, m_PosCounterX, m_PosCounterY, m_CounterMag);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ���
//******************************************************************************
void MTDashboard::Release()
{
	m_Title.Release();
	m_FileName.Release();
	m_Counter.Release();
}

//******************************************************************************
// �J�E���^�\���ʒu�擾
//******************************************************************************
int MTDashboard::_GetCounterPos(
		float* pX,
		float* pY
	)
{
	int result = 0;
	BOOL bresult = 0;
	RECT rect;
	unsigned long cw = 0;
	unsigned long ch = 0;
	unsigned long tw = 0;
	unsigned long th = 0;
	unsigned long charHeight = 0;
	unsigned long charWidth = 0;
	unsigned long captionWidth = 0;
	float newMag = 0.0f;

	//�N���C�A���g�̈�̃T�C�Y���擾
	bresult = GetClientRect(m_hWnd, &rect);
	if (!bresult) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
		goto EXIT;
	}
	cw = rect.right - rect.left;
	ch = rect.bottom - rect.top;

	//�e�N�X�`���T�C�Y�擾
	m_Counter.GetTextureSize(&th, &tw);

	//�����T�C�Y
	charHeight = th;
	charWidth = tw / (unsigned long)_tcslen(MTDASHBOARD_COUNTER_CHARS);

	//�g�嗦1.0�̃L���v�V�����T�C�Y
	captionWidth = (unsigned long)(charWidth * MTDASHBOARD_COUNTER_SIZE);

	//�J�E���^�����񂪉�ʂ���͂ݏo���ꍇ�͉�ʂɎ��܂�悤�Ɋg�嗦���X�V����
	//  �^�C�g�����͂ݏo���̂͋C�ɂ��Ȃ����Ƃɂ���
	if (((cw - (MTDASHBOARD_FRAMESIZE*2)) < captionWidth) && (tw > 0)) {
		newMag = (float)(cw - (MTDASHBOARD_FRAMESIZE*2)) / (float)captionWidth;
		if (m_CounterMag > newMag) {
			m_CounterMag = newMag;
		}
	}

	//�e�N�X�`���̕\���{�����l�����ĕ\���ʒu���Z�o
	*pX = MTDASHBOARD_FRAMESIZE;
	*pY = (float)ch - ((float)th * m_CounterMag) - MTDASHBOARD_FRAMESIZE;

EXIT:;
	return result;
}

//******************************************************************************
// ���t���ԓo�^�i�b�j
//******************************************************************************
void MTDashboard::SetPlayTimeSec(
		unsigned long playTimeSec
	)
{
	m_PlayTimeSec = playTimeSec;
}

//******************************************************************************
// �S�̉��t���ԓo�^�i�b�j
//******************************************************************************
void MTDashboard::SetTotalPlayTimeSec(
		unsigned long totalPlayTimeSec
	)
{
	m_TotalPlayTimeSec = totalPlayTimeSec;
}

//******************************************************************************
// �e���|�o�^(BPM)
//******************************************************************************
void MTDashboard::SetTempoBPM(
		unsigned long bpm
	)
{
	m_TempoBPM = bpm;
}

//******************************************************************************
// ���q�L���o�^
//******************************************************************************
void MTDashboard::SetBeat(
		unsigned long numerator,
		unsigned long denominator
	)
{
	m_BeatNumerator = numerator;
	m_BeatDenominator = denominator;
}

//******************************************************************************
// ���ߐ��o�^
//******************************************************************************
void MTDashboard::SetBarNum(
		unsigned long barNum
	)
{
	m_BarNum = barNum;
}

//******************************************************************************
// ���ߔԍ��o�^
//******************************************************************************
void MTDashboard::SetBarNo(
		unsigned long barNo
	)
{
	m_BarNo = barNo;
}

//******************************************************************************
// �m�[�gON�o�^
//******************************************************************************
void MTDashboard::SetNoteOn()
{
	m_NoteCount++;
}

//******************************************************************************
// ���t���x�o�^
//******************************************************************************
void MTDashboard::SetPlaySpeedRatio(
		unsigned long ratio
	)
{
	m_PlaySpeedRatio = ratio;
}

//******************************************************************************
// �m�[�g���o�^
//******************************************************************************
void MTDashboard::SetNotesCount(
		unsigned long notesCount
	)
{
	m_NoteCount = notesCount;
}

//******************************************************************************
// �J�E���^������擾
//******************************************************************************
int MTDashboard::_GetCounterStr(
		TCHAR* pStr,
		unsigned long bufSize
	)
{
	int result = 0;
	int eresult = 0;
	TCHAR spdstr[16] = {0};

	eresult = _stprintf_s(
				pStr,
				bufSize,
				_T("TIME:%02d:%02d/%02d:%02d BPM:%03d BEAT:%d/%d BAR:%03d/%03d NOTES:%05d/%05d"),
				m_PlayTimeSec / 60,
				m_PlayTimeSec % 60,
				m_TotalPlayTimeSec / 60,
				m_TotalPlayTimeSec % 60,
				m_TempoBPM,
				m_BeatNumerator,
				m_BeatDenominator,
				m_BarNo,
				m_BarNum,
				m_NoteCount,
				m_NoteNum
			);
	if (eresult < 0) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//���t���x��100%�ȊO�̏ꍇ�Ɍ���J�E���^�ɕ\������
	if (m_PlaySpeedRatio != 100) {
		eresult = _stprintf_s(spdstr, 16, _T(" SPEED:%03lu%%"), m_PlaySpeedRatio);
		if (eresult < 0) {
			result = YN_SET_ERR("Program error.", 0, 0);
			goto EXIT;
		}
		_tcscat_s(pStr, bufSize, spdstr);
	}

EXIT:;
	return result;
}

//******************************************************************************
// ���Z�b�g
//******************************************************************************
void MTDashboard::Reset()
{
	m_PlayTimeSec = 0;
	m_TempoBPM = m_TempoBPMOnStart;
	m_BeatNumerator = m_BeatNumeratorOnStart;
	m_BeatDenominator = m_BeatDenominatorOnStart;
	m_BarNo = 1;
	m_NoteCount = 0;
}

//******************************************************************************
// �ݒ�t�@�C���ǂݍ���
//******************************************************************************
int MTDashboard::_LoadConfFile(
		const TCHAR* pSceneName
	)
{
	int result = 0;
	TCHAR hexColor[16] = {_T('\0')};
	MTConfFile confFile;

	result = confFile.Initialize(pSceneName);
	if (result != 0) goto EXIT;

	//----------------------------------
	//�F���
	//----------------------------------
	result = confFile.SetCurSection(_T("Color"));
	if (result != 0) goto EXIT;

	//�L���v�V�����J���[
	result = confFile.GetStr(_T("CaptionRGBA"), hexColor, 16, _T("FFFFFFFF"));
	if (result != 0) goto EXIT;
	m_CaptionColor = DXColorUtil::MakeColorFromHexRGBA(hexColor);

EXIT:;
	return result;
}

//******************************************************************************
// ���t���Ԏ擾
//******************************************************************************
unsigned long MTDashboard::GetPlayTimeSec()
{
	return m_PlayTimeSec;
}

//******************************************************************************
// �\���ݒ�
//******************************************************************************
void MTDashboard::SetEnable(
		bool isEnable
	)
{
	m_isEnable = isEnable;
}

//******************************************************************************
// �t�@�C�����\���ݒ�
//******************************************************************************
void MTDashboard::SetEnableFileName(
		bool isEnable
	)
{
	m_isEnableFileName = isEnable;
}

