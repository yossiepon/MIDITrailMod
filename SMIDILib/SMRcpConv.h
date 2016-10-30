//******************************************************************************
//
// Simple MIDI Library / SMRcpConv
//
// RCP�t�@�C���ϊ��N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// �ӂ݂��������J���Ă��� RCPCV.DLL(*1) ��p���āA���R���|�[�U��
// �f�[�^�t�@�C��(*.rcp,*.r36,*.g36)��W��MIDI�t�@�C���ɕϊ�����B
// RCPCV.DLL ���A�v���P�[�V�����Ɠ����t�H���_�ɑ��݂���Ƃ��ɗ��p�\�Ƃ���B
// RCPCV.DLL �����݂��Ȃ���Ζ{�N���X�̋@�\�͗��p�ł��Ȃ��B
// ���O��IsAvailable()���\�b�h��p���ė��p�ۂ��m�F���邱�ƁB
//
// (*1) RCPCV.DLL
// http://www.vector.co.jp/soft/win95/art/se114143.html

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

#include <windows.h>

namespace SMIDILib {

//******************************************************************************
// SysEx�C�x���g�N���X
//******************************************************************************
class SMIDILIB_API SMRcpConv
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	SMRcpConv();
	virtual ~SMRcpConv(void);

	//������
	int Initialize();

	//���p�۔���
	bool IsAvailable();

	//�W��MID�t�@�C���ϊ�
	int Convert(const TCHAR* pRCPPath, const TCHAR* pSMFPath);

	//�g���q�ɂ��T�|�[�g�Ώۃt�@�C������
	bool IsSupportFileExt(const TCHAR* pFilePath);

	//GetOpenFileName�p�t�@�C���t�B���^�擾
	const TCHAR* GetOpenFileNameFilter();

private:

	HMODULE m_hModule;

	//RCPCV.DLL API��`
	typedef DWORD (WINAPI *RCPCV_ConvertFile)(LPCSTR, UINT, DWORD, UINT, DWORD);
	typedef int   (WINAPI *RCPCV_SaveSMF)(DWORD, LPCSTR);
	typedef void  (WINAPI *RCPCV_DeleteObject)(DWORD);

	//�֐��|�C���^
	RCPCV_ConvertFile  m_pFuncConvertFile;
	RCPCV_SaveSMF      m_pFuncSaveSMF;
	RCPCV_DeleteObject m_pFuncDeleteObject;

	void _Release();

};

} // end of namespace

