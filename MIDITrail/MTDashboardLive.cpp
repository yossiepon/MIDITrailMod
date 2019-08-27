//******************************************************************************
//
// MIDITrail / MTDashboardLive
//
// ���C�u���j�^�p�_�b�V���{�[�h�`��N���X
//
// Copyright (C) 2012-2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTDashboardLive.h"

using namespace YNBaseLib;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTDashboardLive::MTDashboardLive(void)
{
	m_hWnd = NULL;
	m_PosCounterX = 0.0f;
	m_PosCounterY = 0.0f;
	m_CounterMag = MTDASHBOARDLIVE_DEFAULT_MAGRATE;
	m_isMonitoring = false;
	m_NoteCount = 0;
	m_CaptionColor = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	m_isEnable = true;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTDashboardLive::~MTDashboardLive(void)
{
	Release();
}

//******************************************************************************
// �_�b�V���{�[�h����
//******************************************************************************
int MTDashboardLive::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName,
		HWND hWnd
   )
{
	int result = 0;
	TCHAR counter[100];
	
	Release();
	
	m_hWnd = hWnd;
	
	//�ݒ�ǂݍ���
	result = _LoadConfFile(pSceneName);
	if (result != 0) goto EXIT;
	
	//�^�C�g���L���v�V����
	result = SetMIDIINDeviceName(pD3DDevice, _T(""));
	if (result != 0) goto EXIT;
	
	//�J�E���^�L���v�V����
	result = m_Counter.Create(
					pD3DDevice,
					MTDASHBOARDLIVE_FONTNAME,		//�t�H���g����
					MTDASHBOARDLIVE_FONTSIZE,		//�t�H���g�T�C�Y
					MTDASHBOARDLIVE_COUNTER_CHARS,	//�\������
					MTDASHBOARDLIVE_COUNTER_SIZE	//�L���v�V�����T�C�Y
				);
	if (result != 0) goto EXIT;
	m_Counter.SetColor(m_CaptionColor);
	
	//�J�E���^�\�������񐶐�
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
int MTDashboardLive::Transform(
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
int MTDashboardLive::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	TCHAR counter[100];
	
	if (pD3DDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	
	if (!m_isEnable) goto EXIT;
	
	//�^�C�g���`��F�J�E���^�Ɠ����g�嗦�ŕ\������
	result = m_Title.Draw(pD3DDevice, MTDASHBOARDLIVE_FRAMESIZE, MTDASHBOARDLIVE_FRAMESIZE, m_CounterMag);
	if (result != 0) goto EXIT;
	
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
void MTDashboardLive::Release()
{
	m_Title.Release();
	m_Counter.Release();
}

//******************************************************************************
// �J�E���^�\���ʒu�擾
//******************************************************************************
int MTDashboardLive::_GetCounterPos(
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
	charWidth = tw / (unsigned long)_tcslen(MTDASHBOARDLIVE_COUNTER_CHARS);
	
	//�g�嗦1.0�̃L���v�V�����T�C�Y
	captionWidth = (unsigned long)(charWidth * MTDASHBOARDLIVE_COUNTER_SIZE);
	
	//�J�E���^�����񂪉�ʂ���͂ݏo���ꍇ�͉�ʂɎ��܂�悤�Ɋg�嗦���X�V����
	//  �^�C�g�����͂ݏo���̂͋C�ɂ��Ȃ����Ƃɂ���
	if (((cw - (MTDASHBOARDLIVE_FRAMESIZE*2)) < captionWidth) && (tw > 0)) {
		newMag = (float)(cw - (MTDASHBOARDLIVE_FRAMESIZE*2)) / (float)captionWidth;
		if (m_CounterMag > newMag) {
			m_CounterMag = newMag;
		}
	}
	
	//�e�N�X�`���̕\���{�����l�����ĕ\���ʒu���Z�o
	*pX = MTDASHBOARDLIVE_FRAMESIZE;
	*pY = (float)ch - ((float)th * m_CounterMag) - MTDASHBOARDLIVE_FRAMESIZE;

EXIT:;
	return result;
}

//******************************************************************************
// ���j�^��ԓo�^
//******************************************************************************
void MTDashboardLive::SetMonitoringStatus(
		bool isMonitoring
	)
{
	m_isMonitoring = isMonitoring;
}

//******************************************************************************
// �m�[�gON�o�^
//******************************************************************************
void MTDashboardLive::SetNoteOn()
{
	m_NoteCount++;
}

//******************************************************************************
// �J�E���^������擾
//******************************************************************************
int MTDashboardLive::_GetCounterStr(
		TCHAR* pStr,
		unsigned long bufSize
	)
{
	int result = 0;
	int eresult = 0;
	const TCHAR* pMonitorStatus = _T("");
	
	if (m_isMonitoring) {
		pMonitorStatus = _T("");
	}
	else {
		pMonitorStatus = _T("[MONITERING OFF]");
	}
	
	eresult = _stprintf_s(
				pStr,
				bufSize,
				_T("NOTES:%08lu %s"),
				m_NoteCount,
				pMonitorStatus
			);
	if (eresult < 0) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// ���Z�b�g
//******************************************************************************
void MTDashboardLive::Reset()
{
	m_isMonitoring = false;
	m_NoteCount = 0;
}

//******************************************************************************
// �ݒ�t�@�C���ǂݍ���
//******************************************************************************
int MTDashboardLive::_LoadConfFile(
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
// �\���ݒ�
//******************************************************************************
void MTDashboardLive::SetEnable(
		bool isEnable
	)
{
	m_isEnable = isEnable;
}

//******************************************************************************
//MIDI IN �f�o�C�X���o�^
//******************************************************************************
int MTDashboardLive::SetMIDIINDeviceName(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pName
	)
{
	int result = 0;
	int eresult = 0;
	TCHAR title[256] = {0}; //MAXPNAMELEN 32 ���傫���T�C�Y�ɂ���
	const TCHAR* pDisplayName = NULL;
	
	m_Title.Release();
	
	if (pName == NULL) {
		pDisplayName = _T("(none)");
	}
	else if (_tcslen(pName) == 0) {
		pDisplayName = _T("(none)");
	}
	else {
		pDisplayName = pName;
	}
	
	//�^�C�g���L���v�V����
	eresult = _stprintf_s(
				title,
				256,
				_T("MIDI IN: %s"),
				pDisplayName
			);

	result = m_Title.Create(
					pD3DDevice,					//�f�o�C�X
					MTDASHBOARDLIVE_FONTNAME,	//�t�H���g����
					MTDASHBOARDLIVE_FONTSIZE,	//�t�H���g�T�C�Y
					title						//�L���v�V����
				);
	if (result != 0) goto EXIT;
	m_Title.SetColor(m_CaptionColor);

EXIT:;
	return result;
}


