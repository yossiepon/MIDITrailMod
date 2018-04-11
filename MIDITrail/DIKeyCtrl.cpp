//******************************************************************************
//
// MIDITrail / DIKeyCtrl
//
// DirectInput �L�[���͐���N���X
//
// Copyright (C) 2010-2014 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "DIKeyCtrl.h"

using namespace YNBaseLib;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
DIKeyCtrl::DIKeyCtrl(void)
{
	m_pDI = NULL;
	m_pDIDevice = NULL;
	ZeroMemory(m_KeyStatus, sizeof(unsigned char) * 256);
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
DIKeyCtrl::~DIKeyCtrl(void)
{
	Terminate();
}

//******************************************************************************
// ������
//******************************************************************************
int DIKeyCtrl::Initialize(
		HWND hWnd
	)
{
	int result = 0;
	HRESULT hresult = DI_OK;
	HINSTANCE hInstance = NULL;

	Terminate();

	//�A�v���P�[�V�����C���X�^���X�n���h�����擾
	hInstance = (HINSTANCE)(LONG_PTR)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
	if (hInstance == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)hWnd);
		goto EXIT;
	}

	//DirectInput�I�u�W�F�N�g�̐���
	hresult = DirectInput8Create(
				hInstance,				//�A�v���P�[�V�����C���X�^���X�n���h��
				DIRECTINPUT_VERSION,	//DirectInput�o�[�W�����ԍ�
				IID_IDirectInput8,		//�C���^�[�t�F�[�X���ʎq
				(void**)&m_pDI,			//�쐬���ꂽ�C���^�[�t�F�[�X�|�C���^
				NULL					//IUnknown�C���^�[�t�F�C�X�|�C���^
			);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectInput API error.", hresult, (DWORD64)hInstance);
		goto EXIT;
	}

	//�f�o�C�X�I�u�W�F�N�g�̐���
	hresult = m_pDI->CreateDevice(
					GUID_SysKeyboard,	//���̓f�o�C�X�̃C���X�^���XGUID
					&m_pDIDevice,		//�쐬���ꂽ�C���^�[�t�F�[�X�|�C���^
					NULL				//IUnknown�C���^�[�t�F�C�X�|�C���^
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectInput API error.", hresult, 0);
		goto EXIT;
	}

	//�f�o�C�X�̃f�[�^�t�H�[�}�b�g��ݒ�F��`�ς݃O���[�o���ϐ����w��
	hresult = m_pDIDevice->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectInput API error.", hresult, 0);
		goto EXIT;
	}

	//�f�o�C�X�̋������x����ݒ�
	hresult = m_pDIDevice->SetCooperativeLevel(
					hWnd,					//�f�o�C�X�Ɋ֘A�t�����Ă���E�B���h�E�n���h��
					DISCL_FOREGROUND		//�������x���F�t�H�A�O�����h�A�N�Z�X��
					| DISCL_NONEXCLUSIVE	//�������x���F��r���I�A�N�Z�X��
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectInput API error.", hresult, (DWORD64)hWnd);
		goto EXIT;
	}

	//�f�o�C�X�̃v���p�e�B��ݒ�
	DIPROPDWORD diprop;
	diprop.diph.dwSize       = sizeof(DIPROPDWORD);
	diprop.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	diprop.diph.dwObj        = 0;			//DIPH_DEVICE�̏ꍇ�̓[��
	diprop.diph.dwHow        = DIPH_DEVICE;	//dwObj�̉��ߕ��@�F�f�o�C�X�S��
	diprop.dwData            = 8;			//�ݒ肷��v���p�e�B�F�o�b�t�@�T�C�Y

	hresult = m_pDIDevice->SetProperty(
					DIPROP_BUFFERSIZE,	//�ݒ�Ώۃv���p�e�B��GUID
					&diprop.diph		//�ݒ肷��DIPROPHEADER�\����
				);
	if (FAILED(hresult) && (hresult != DI_PROPNOEFFECT)) {
		result = YN_SET_ERR("DirectInput API error.", hresult, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �I������
//******************************************************************************
void DIKeyCtrl::Terminate()
{
	if (m_pDIDevice != NULL) {
		m_pDIDevice->Unacquire();
		m_pDIDevice->Release();
		m_pDIDevice = NULL;
	}

	if (m_pDI != NULL) {
		m_pDI->Release();
		m_pDI = NULL;
	}

	return;
}

//******************************************************************************
// �f�o�C�X�A�N�Z�X���擾
//******************************************************************************
int DIKeyCtrl::Acquire()
{
	int result = 0;
	HRESULT hresult = DI_OK;

	if (m_pDIDevice == NULL) goto EXIT;

	//�A�N�Z�X���擾�F//�f�o�C�X�擾�ς�(S_FALSE)�͐���Ƃ݂Ȃ�
	hresult = m_pDIDevice->Acquire();
	if (FAILED(hresult) && (hresult != S_FALSE)) {
		result = YN_SET_ERR("DirectInput API error.", hresult, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �f�o�C�X�A�N�Z�X�����
//******************************************************************************
int DIKeyCtrl::Unacquire()
{
	int result = 0;
	HRESULT hresult = DI_OK;

	if (m_pDIDevice == NULL) goto EXIT;

	//�A�N�Z�X�����
	hresult = m_pDIDevice->Unacquire();
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectInput API error.", hresult, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �L�[��Ԏ擾
//******************************************************************************
int DIKeyCtrl::GetKeyStatus()
{
	int result = 0;
	HRESULT hresult = DI_OK;

	hresult = m_pDIDevice->GetDeviceState(256, m_KeyStatus);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectInput API error.", hresult, 0);
		goto EXIT;
	}

	//�E�B���h�E����A�N�e�B�u��Ԃł����GetDeviceState()�̓G���[�ɂȂ�(0x8007000c)
	//�ǂ����悤�E�E�E

EXIT:;
	return result;
}

//******************************************************************************
// �L�[��Ԋm�F
//******************************************************************************
bool DIKeyCtrl::IsKeyDown(
		unsigned char key
	)
{
	bool isDown = false;

	if ((m_KeyStatus[key] & 0x80) != 0) {
		isDown = true;
	}

	return isDown;
}

