//******************************************************************************
//
// MIDITrail / DXRenderer
//
// �����_���N���X
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "stdafx.h"
#include "YNBaseLib.h"
#include "DXScene.h"
#include "DXRenderer.h"

using namespace YNBaseLib;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
DXRenderer::DXRenderer()
{
	m_hWnd = NULL;
	m_pD3D = NULL;
	m_pD3DDevice = NULL;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
DXRenderer::~DXRenderer()
{
	Terminate();
}

//******************************************************************************
// ������
//******************************************************************************
int DXRenderer::Initialize(
		HWND hWnd,
		unsigned long multiSampleType	//�ȗ����̓[���F�A���`�G�C���A�V���O����
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;
	D3DDISPLAYMODE dispMode;
	bool isSupport = false;
	D3DMULTISAMPLE_TYPE type = D3DMULTISAMPLE_NONE;
	unsigned long qualityLevels = 0;

	m_hWnd = hWnd;

	if (DX_MULTI_SAMPLE_TYPE_MAX < multiSampleType) {
		result = YN_SET_ERR("Program error.", multiSampleType, 0);
		goto EXIT;
	}

	//Direct3D9�I�u�W�F�N�g�쐬
	m_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (m_pD3D == NULL) {
		result = YN_SET_ERR("DirectX API error.", 0, 0);
		goto EXIT;
	}

	//�f�B�X�v���C���[�h�擾
	hresult = m_pD3D->GetAdapterDisplayMode(
					D3DADAPTER_DEFAULT,	//�₢���킹�Ώۃf�B�X�v���C�A�_�v�^
					&dispMode			//�f�B�X�v���C���[�h���
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

	//�v���[���e�[�V�����p�����[�^������
	ZeroMemory(&m_D3DPP, sizeof(D3DPRESENT_PARAMETERS));
	m_D3DPP.BackBufferCount = 1;					//�o�b�t�@��
	m_D3DPP.Windowed = TRUE;						//�E�C���h�E���\���̎w��
	m_D3DPP.BackBufferFormat = dispMode.Format;		//�o�b�t�@�T�[�t�F�X�t�H�[�}�b�g
	m_D3DPP.SwapEffect = D3DSWAPEFFECT_DISCARD;		//�_�u���o�b�t�@�����O�X���b�v�w��
	m_D3DPP.EnableAutoDepthStencil = TRUE;			//�[�x�X�e���V���o�b�t�@�쐬
	m_D3DPP.AutoDepthStencilFormat = D3DFMT_D16;	//�����[�x�X�e���V���T�[�t�F�X�t�H�[�}�b�g

	//�A���`�G�C���A�V���O�L����
	if (multiSampleType >= DX_MULTI_SAMPLE_TYPE_MIN) {
		//�A���`�G�C���A�V���O�w�肠��
		//�n�[�h�E�F�A�̃A���`�G�C���A�V���O�T�|�[�g�󋵂��m�F����
		type = _EnumMultiSampleType(multiSampleType);
		result = _CheckAntialiasSupport(
						m_D3DPP,			//�v���[���e�[�V�����p�����[�^
						type,				//�}���`�T���v�����
						&isSupport,			//�T�|�[�g�m�F����
						&qualityLevels		//�i�����x����
					);
		if (result != 0) goto EXIT;
		
		//�w�肳�ꂽ�}���`�T���v�������T�|�[�g���Ă���ꍇ��
		//�v���[���e�[�V�����p�����[�^�ɔ��f����
		if (isSupport) {
			m_D3DPP.MultiSampleType = type;
			m_D3DPP.MultiSampleQuality = qualityLevels - 1;
		}
	}

	//�f�B�X�v���C�A�_�v�^�f�o�C�X�쐬�i�`��F�n�[�h�^���_�����F�n�[�h�j
	hresult = m_pD3D->CreateDevice(
					D3DADAPTER_DEFAULT,	//�f�B�X�v���C�A�_�v�^
					D3DDEVTYPE_HAL,		//�f�o�C�X�^�C�v�F�n�[�h�E�F�A���X�^��
					hWnd,				//�t�H�[�J�X�ΏۃE�B���h�E�n���h��
					D3DCREATE_HARDWARE_VERTEXPROCESSING,
										//�t���O�F�n�[�h�E�F�A���_����
					&m_D3DPP,			//�v���[���e�[�V�����p�����[�^
					&m_pD3DDevice		//�쐬���ꂽ�f�o�C�X
				);
	if (FAILED(hresult)) {
		//���s���Ă����̕��@������
	}
	else {
		//����������I��
		goto EXIT;
	}

	//�f�B�X�v���C�A�_�v�^�f�o�C�X�쐬�i�`��F�n�[�h�^���_�����FCPU�j
	hresult = m_pD3D->CreateDevice(
					D3DADAPTER_DEFAULT,	//�f�B�X�v���C�A�_�v�^
					D3DDEVTYPE_HAL,		//�f�o�C�X�^�C�v�F�n�[�h�E�F�A���X�^��
					hWnd,				//�t�H�[�J�X�ΏۃE�B���h�E�n���h��
					D3DCREATE_SOFTWARE_VERTEXPROCESSING,
										//�t���O�F�\�t�g�E�F�A���_����
					&m_D3DPP,			//�v���[���e�[�V�����p�����[�^
					&m_pD3DDevice		//�쐬���ꂽ�f�o�C�X
				);
	if (FAILED(hresult)) {
		//���s���Ă����̕��@������
	}
	else {
		//����������I��
		goto EXIT;
	}

	//HAL�ȊO�ł̓A���`�G�C���A�V���O�͖����ɂ���
	m_D3DPP.MultiSampleType = D3DMULTISAMPLE_NONE;
	m_D3DPP.MultiSampleQuality = 0;

	//�f�B�X�v���C�A�_�v�^�f�o�C�X�쐬�i�`��FCPU�^���_�����FCPU�j
	hresult = m_pD3D->CreateDevice(
					D3DADAPTER_DEFAULT,	//�f�B�X�v���C�A�_�v�^
					D3DDEVTYPE_REF,		//�f�o�C�X�^�C�v�F�\�t�g�E�F�A
					hWnd,				//�t�H�[�J�X�ΏۃE�B���h�E�n���h��
					D3DCREATE_SOFTWARE_VERTEXPROCESSING,
										//�t���O�F�\�t�g�E�F�A���_����
					&m_D3DPP,			//�v���[���e�[�V�����p�����[�^
					&m_pD3DDevice		//�쐬���ꂽ�f�o�C�X
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �f�o�C�X�擾
//******************************************************************************
LPDIRECT3DDEVICE9 DXRenderer::GetDevice()
{
	return m_pD3DDevice;
}

//*****************************************************************************
// �V�[���`��
//******************************************************************************
int DXRenderer::RenderScene(
		DXScene* pScene
	)
{
	int result = 0;
	HRESULT hresult = 0;
	D3DCOLOR bgColor;

	if (pScene == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//�o�b�t�@�������F�̎w��FARGB
	bgColor = pScene->GetBGColor();

	//�r���[�|�[�g�N���A�{�[�x�o�b�t�@�N���A�{�X�e���V���o�b�t�@�폜
	hresult = m_pD3DDevice->Clear(
					0,						//��`���F�S��
					NULL, 					//�N���A�Ώۋ�`�F�S��
					D3DCLEAR_TARGET			//�����_�����O�^�[�Q�b�g�N���A
					| D3DCLEAR_ZBUFFER, 	//�[�x�o�b�t�@�N���A
					bgColor,				//�ݒ�F(ARGB)
					//D3DCOLOR_XRGB(0,0,0), 			//�ݒ�F(ARGB)�F��
					//D3DCOLOR_XRGB(255,255,255), 		//�ݒ�F(ARGB)�F��
					1.0f, 					//�[�x�o�b�t�@�ݒ�l
					0						//�X�e���V���o�b�t�@�ݒ�l
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

	//�`��J�n
	hresult = m_pD3DDevice->BeginScene();
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

	//�`��
	result = pScene->Draw(m_pD3DDevice);
	if (result != 0) {
		//���s���Ă��`���^�����𑱍s����
		YN_SHOW_ERR(m_hWnd);
	}

	//�`��I��
	hresult = m_pD3DDevice->EndScene();
	if (FAILED(hresult)) {
		//�`���s���Ă��`���^�����𑱍s����
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		YN_SHOW_ERR(m_hWnd);
	}

	//�`�挋�ʓ]��
	hresult = m_pD3DDevice->Present(
						NULL,	//�]������`�F�]�����T�[�t�F�C�X�S�̂�\��
						NULL,	//�]�����`�F�N���C�A���g�̈�S��
						NULL,	//�]����E�B���h�E�n���h���F�v���[���e�[�V�����p�����[�^�ɏ]��
						NULL	//�X�V�Ώۋ�`�i�œK���x���p�j
					);
	if (FAILED(hresult)) {
		//�f�o�C�X���X�g
		if (hresult == D3DERR_DEVICELOST) {
			result = DXRENDERER_ERR_DEVICE_LOST;
			goto EXIT;

			//���J�o��
			//result = _RecoverDevice();
			//if (result != 0) goto EXIT;
		}
		else {
			result = YN_SET_ERR("DirectX API error.", hresult, 0);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

//*****************************************************************************
// �I������
//******************************************************************************
void DXRenderer::Terminate()
{
	if (m_pD3DDevice != NULL) {
		m_pD3DDevice->Release();
		m_pD3DDevice = NULL;
	}
	if (m_pD3D != NULL) {
		m_pD3D->Release();
		m_pD3D = NULL;
	}
	m_hWnd = NULL;
}

//*****************************************************************************
// �f�o�C�X���X�g���J�o������
//******************************************************************************
int DXRenderer::_RecoverDevice()
{
	int result = 0;
	HRESULT hresult = D3D_OK;

	//�f�o�C�X���X�g�ɑΏ�����ɂ�
	//  (1) �f�o�C�X�����Z�b�g�\�ɂȂ�܂ő҂�
	//      �`�揈����API�Ăяo���͐������邪���ۂ͉�����������Ȃ�
	//  (2) �f�o�C�X�����Z�b�g�\�ɂȂ����烊�Z�b�g����
	//  (3) ���Z�b�g�����烊�\�[�X��j�����čč\�z����
	//      ���_�o�b�t�@�^�C���f�b�N�X�o�b�t�@�^�e�N�X�`���Ȃ�
	//
	//(3)�̎������炢�̂Ō����_�͖��Ή��Ƃ���

	//�f�o�C�X�̓����Ԃ��擾
	hresult = m_pD3DDevice->TestCooperativeLevel();
	if (hresult == D3D_OK) {
		//���퓮�쒆�̂��߃��J�o���s�v
	}
	else {
		//�f�o�C�X���X�g
		if (hresult == D3DERR_DEVICELOST) {
			//�܂����Z�b�g�ł��Ȃ��̂ŕ��u����
		}
		//���Z�b�g�\
		else if (hresult == D3DERR_DEVICENOTRESET) {
			//�f�o�C�X���Z�b�g
			hresult = m_pD3DDevice->Reset(&m_D3DPP);
			if (FAILED(hresult)) {
				result = YN_SET_ERR("DirectX API error.", hresult, 0);
				goto EXIT;
			}
			//�S���\�[�X��j��
			// TODO: �������E�E�E�ǂ����܂���
			//�S���\�[�X���Đ���
			// TODO: �������E�E�E�ǂ����܂���
		}
		//�����G���[
		else if (hresult == D3DERR_DRIVERINTERNALERROR) {
			result = YN_SET_ERR("DirectX API error.", hresult, 0);
			goto EXIT;
		}
		//���m�̃G���[
		else {
			result = YN_SET_ERR("DirectX API error.", hresult, 0);
			goto EXIT;
		}
	}

EXIT:;
	return result;
}

//*****************************************************************************
// �A���`�G�C���A�V���O�T�|�[�g�`�F�b�N
//******************************************************************************
int DXRenderer::_CheckAntialiasSupport(
		D3DPRESENT_PARAMETERS d3dpp,
		D3DMULTISAMPLE_TYPE multiSampleType,
		bool* pIsSupport,
		unsigned long* pQualityLevels
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;
	unsigned long qualityLevelsBackBuffer = 0;
	unsigned long qualityLevelsZBuffer = 0;
	bool isSupportBackBuffer = false;
	bool isSupportZbuffer = false;

	//�n�[�h�E�F�A�̃A���`�G�C���A�V���O�T�|�[�g�󋵂̂݊m�F����

	//�o�b�N�o�b�t�@�t�H�[�}�b�g�̃A���`�G�C���A�V���O�T�|�[�g�m�F
	hresult = m_pD3D->CheckDeviceMultiSampleType(
					D3DADAPTER_DEFAULT,			//�₢���킹�Ώۃf�B�X�v���C�A�_�v�^
					D3DDEVTYPE_HAL,				//�f�o�C�X�^�C�v�F�n�[�h�E�F�A
					d3dpp.BackBufferFormat,		//�T�[�t�F�C�X�t�H�[�}�b�g
					d3dpp.Windowed,				//�E�B���h�E���
					multiSampleType,			//�}���`�T���v�����O�e�N�j�b�N
					&qualityLevelsBackBuffer	//���p�\�ȕi�����x���̐��F�s�v
				);
	if (SUCCEEDED(hresult)) {
		//�T�|�[�g���Ă���
		isSupportBackBuffer = true;
	}

	//�[�x�o�b�t�@�t�H�[�}�b�g�̃A���`�G�C���A�V���O�T�|�[�g�m�F
	hresult = m_pD3D->CheckDeviceMultiSampleType(
					D3DADAPTER_DEFAULT,			//�₢���킹�Ώۃf�B�X�v���C�A�_�v�^
					D3DDEVTYPE_HAL,				//�f�o�C�X�^�C�v�F�n�[�h�E�F�A
					d3dpp.BackBufferFormat,		//�T�[�t�F�C�X�t�H�[�}�b�g
					d3dpp.Windowed,				//�E�B���h�E���
					multiSampleType,			//�}���`�T���v�����O�e�N�j�b�N
					&qualityLevelsZBuffer		//���p�\�ȕi�����x���̐��F�s�v
				);
	if (SUCCEEDED(hresult)) {
		//�T�|�[�g���Ă���
		isSupportZbuffer = true;
	}

	//�A���`�G�C���A�V���O�`�F�b�N����
	*pIsSupport = false;
	*pQualityLevels = 0;
	if (isSupportBackBuffer && isSupportZbuffer) {
		*pIsSupport = true;
		if (qualityLevelsBackBuffer < qualityLevelsZBuffer) {
			*pQualityLevels = qualityLevelsBackBuffer;
		}
		else {
			*pQualityLevels = qualityLevelsZBuffer;
		}
	}

	return result;
}

//*****************************************************************************
// �}���`�T���v����ʎ擾
//******************************************************************************
D3DMULTISAMPLE_TYPE DXRenderer::_EnumMultiSampleType(
		unsigned long multiSampleNum
	)
{
	D3DMULTISAMPLE_TYPE type = D3DMULTISAMPLE_NONE;
	D3DMULTISAMPLE_TYPE types[] = {
			D3DMULTISAMPLE_2_SAMPLES,  D3DMULTISAMPLE_3_SAMPLES,  D3DMULTISAMPLE_4_SAMPLES,
			D3DMULTISAMPLE_5_SAMPLES,  D3DMULTISAMPLE_6_SAMPLES,  D3DMULTISAMPLE_7_SAMPLES,
			D3DMULTISAMPLE_8_SAMPLES,  D3DMULTISAMPLE_9_SAMPLES,  D3DMULTISAMPLE_10_SAMPLES,
			D3DMULTISAMPLE_11_SAMPLES, D3DMULTISAMPLE_12_SAMPLES, D3DMULTISAMPLE_13_SAMPLES,
			D3DMULTISAMPLE_14_SAMPLES, D3DMULTISAMPLE_15_SAMPLES, D3DMULTISAMPLE_16_SAMPLES
		};

	if ((DX_MULTI_SAMPLE_TYPE_MIN <= multiSampleNum)
	 && (multiSampleNum <= DX_MULTI_SAMPLE_TYPE_MAX)) {
		type = types[multiSampleNum - 2];
	}

	return type;
}

//*****************************************************************************
// �A���`�G�C���A�V���O�T�|�[�g�`�F�b�N
//******************************************************************************
int DXRenderer::IsSupportAntialias(
		unsigned long multiSampleType,
		bool* pIsSupport
	)
{
	int result = 0;
	unsigned long qualityLevels = 0;

	if (m_pD3D == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	if (pIsSupport == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	if ((multiSampleType < DX_MULTI_SAMPLE_TYPE_MIN)
	 || (DX_MULTI_SAMPLE_TYPE_MAX < multiSampleType)) {
		result = YN_SET_ERR("Program error.", multiSampleType, 0);
		goto EXIT;
	}

	result = _CheckAntialiasSupport(
					m_D3DPP,
					_EnumMultiSampleType(multiSampleType),
					pIsSupport,
					&qualityLevels
				);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//*****************************************************************************
// �C���f�b�N�X�o�b�t�@�T�|�[�g�`�F�b�N
//******************************************************************************
int DXRenderer::IsSupportIndexBuffer(
		bool* pIsSupport,
		unsigned long* pMaxVertexIndex
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;
	D3DCAPS9 caps;

	if (m_pD3D == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	if ((pIsSupport == NULL) || (pMaxVertexIndex == NULL)) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	*pIsSupport = false;

	//�f�o�C�X���擾
	hresult = m_pD3D->GetDeviceCaps(
					D3DADAPTER_DEFAULT,	//�₢���킹�Ώۃf�B�X�v���C�A�_�v�^
					D3DDEVTYPE_HAL,		//�f�o�C�X�^�C�v�F�n�[�h�E�F�A
					&caps				//�f�o�C�X�\�͏��
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

	*pMaxVertexIndex = caps.MaxVertexIndex;

	if (caps.MaxVertexIndex > 0x0000FFFF) {
		//�C���f�b�N�X�o�b�t�@���T�|�[�g���Ă���
		*pIsSupport = true;
	}

EXIT:;
	return result;
}


