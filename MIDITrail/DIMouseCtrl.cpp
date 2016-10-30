//******************************************************************************
//
// MIDITrail / DIMouseCtrl
//
// DirectInput �}�E�X����N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "DIMouseCtrl.h"

using namespace YNBaseLib;

//******************************************************************************
// �}�N����`
//******************************************************************************
#define IS_KEYDOWN(btn)  (btn & 0x80)

//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
DIMouseCtrl::DIMouseCtrl(void)
{
	m_pDI = NULL;
	m_pDIDevice = NULL;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
DIMouseCtrl::~DIMouseCtrl(void)
{
	Terminate();
}

//******************************************************************************
// ������
//******************************************************************************
int DIMouseCtrl::Initialize(
		HWND hWnd
	)
{
	int result = 0;
	HRESULT hresult = DI_OK;
	HINSTANCE hInstance = NULL;

	Terminate();

	//�A�v���P�[�V�����C���X�^���X�n���h�����擾
	hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
	if (hInstance == NULL) {
		result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD)hWnd);
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
		result = YN_SET_ERR("DirectInput API error.", hresult, (DWORD)hInstance);
		goto EXIT;
	}

	//�f�o�C�X�I�u�W�F�N�g�̐���
	hresult = m_pDI->CreateDevice(
					GUID_SysMouse,		//���̓f�o�C�X�̃C���X�^���XGUID
					&m_pDIDevice,		//�쐬���ꂽ�C���^�[�t�F�[�X�|�C���^
					NULL				//IUnknown�C���^�[�t�F�C�X�|�C���^
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectInput API error.", hresult, 0);
		goto EXIT;
	}

	//�f�o�C�X�̃f�[�^�t�H�[�}�b�g��ݒ�F��`�ς݃O���[�o���ϐ����w��
	hresult = m_pDIDevice->SetDataFormat(&c_dfDIMouse2);
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
		result = YN_SET_ERR("DirectInput API error.", hresult, (DWORD)hWnd);
		goto EXIT;
	}

	//�f�o�C�X�̃v���p�e�B��ݒ�F�o�b�t�@�T�C�Y
	DIPROPDWORD diprop;
	diprop.diph.dwSize       = sizeof(DIPROPDWORD);
	diprop.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	diprop.diph.dwObj        = 0;			//DIPH_DEVICE�̏ꍇ�̓[��
	diprop.diph.dwHow        = DIPH_DEVICE;	//dwObj�̉��ߕ��@�F�f�o�C�X�S��
	diprop.dwData            = 16;			//�ݒ肷��v���p�e�B�F�o�b�t�@�T�C�Y

	hresult = m_pDIDevice->SetProperty(
					DIPROP_BUFFERSIZE,	//�ݒ�Ώۃv���p�e�B��GUID
					&diprop.diph		//�ݒ肷��DIPROPHEADER�\����
				);
	if (FAILED(hresult) && (hresult != DI_PROPNOEFFECT)) {
		result = YN_SET_ERR("DirectInput API error.", hresult, 0);
		goto EXIT;
	}

	//�f�o�C�X�̃v���p�e�B��ݒ�F�����[�h
	diprop.diph.dwSize       = sizeof(DIPROPDWORD);
	diprop.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	diprop.diph.dwObj        = 0;			//DIPH_DEVICE�̏ꍇ�̓[��
	diprop.diph.dwHow        = DIPH_DEVICE;	//dwObj�̉��ߕ��@�F�f�o�C�X�S��
	diprop.dwData            = DIPROPAXISMODE_REL;	//�ݒ肷��v���p�e�B�F���Βl���[�h

	hresult = m_pDIDevice->SetProperty(
					DIPROP_AXISMODE,	//�ݒ�Ώۃv���p�e�B��GUID
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
void DIMouseCtrl::Terminate()
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
int DIMouseCtrl::Acquire()
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
int DIMouseCtrl::Unacquire()
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
// �}�E�X��Ԏ擾
//******************************************************************************
int DIMouseCtrl::GetMouseStatus()
{
	int result = 0;
	HRESULT hresult = DI_OK;

	if (m_pDIDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	hresult = m_pDIDevice->GetDeviceState(sizeof(DIMOUSESTATE2), &m_MouseState);
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
// �}�E�X�{�^����Ԋm�F
//******************************************************************************
bool DIMouseCtrl::IsBtnDown(
		MouseButton	target
	)
{
	bool isDown = false;
	BYTE btn = 0;

	if (m_pDIDevice == NULL) goto EXIT;

	if (target == LeftButton) {
		btn = m_MouseState.rgbButtons[0];
	}
	if (target == RightButton) {
		btn = m_MouseState.rgbButtons[1];
	}

	if (IS_KEYDOWN(btn)) {
		isDown = true;
	}

EXIT:;
	return isDown;
}

//******************************************************************************
// �}�E�X���Έړ��ʎ擾
//******************************************************************************
int DIMouseCtrl::GetDelta(
		MouseAxis	target
	)
{
	int rel = 0;

	if (m_pDIDevice == NULL) goto EXIT;

	if (target == AxisX) {
		rel = m_MouseState.lX;
	}
	if (target == AxisY) {
		rel = m_MouseState.lY;
	}
	if (target == AxisWheel) {
		rel = m_MouseState.lZ;
	}

EXIT:;
	return rel;
}

//******************************************************************************
// �o�b�t�@�f�[�^�擾
//******************************************************************************
int DIMouseCtrl::GetBuffer(
		bool* pIsExist,
		MouseEvent* pEvent,
		int* pDeltaAxis
	)
{
	int result = 0;
	HRESULT hresult = DI_OK;
	DIDEVICEOBJECTDATA devObjData;
	DWORD inOut = 1;

	if ((m_pDIDevice == NULL) || (pIsExist == NULL) || (pEvent == NULL)) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	*pIsExist = false;

	//�o�b�t�@�f�[�^�擾�i���s��o�b�t�@���ЂƂ���j
	hresult = m_pDIDevice->GetDeviceData(
						sizeof(DIDEVICEOBJECTDATA),	//DIOBJECTDATAFORMAT�\���̃T�C�Y
						&devObjData,				//�o�b�t�@�f�[�^�z��F1����
						&inOut,						//���́F�o�b�t�@�v�f���^�o�́F�f�[�^�擾��
						0							//�t���O
					);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectInput API error.", hresult, 0);
		goto EXIT;
	}

	//�o�b�t�@���Ȃ���ΏI��
	if (inOut == 0) goto EXIT;

	//�o�b�t�@�f�[�^���
	switch (devObjData.dwOfs) {
		case DIMOFS_BUTTON0:
			if (IS_KEYDOWN(devObjData.dwData)) {
				*pEvent = LeftButtonDown;
			}
			else {
				*pEvent = LeftButtonUp;
			}
			break;
		case DIMOFS_BUTTON1:
			if (IS_KEYDOWN(devObjData.dwData)) {
				*pEvent = RightButtonDown;
			}
			else {
				*pEvent = RightButtonUp;
			}
			break;
		case DIMOFS_BUTTON2:
			if (IS_KEYDOWN(devObjData.dwData)) {
				*pEvent = CenterButtonDown;
			}
			else {
				*pEvent = CenterButtonUp;
			}
			break;
		case DIMOFS_X:
			*pEvent = AxisXMove;
			if (pDeltaAxis!= NULL) {
				*pDeltaAxis = (int)devObjData.dwData;
			}
			break;
		case DIMOFS_Y:
			*pEvent = AxisYMove;
			if (pDeltaAxis!= NULL) {
				*pDeltaAxis = (int)devObjData.dwData;
			}
			break;
		case DIMOFS_Z:
			*pEvent = AxisWheelMove;
			if (pDeltaAxis!= NULL) {
				*pDeltaAxis = (int)devObjData.dwData;
			}
			break;
		default:
			break;
	}

	*pIsExist = true;

EXIT:;
	return result;
}

