//******************************************************************************
//
// MIDITrail / MTFirstPersonCam
//
// ��l�̃J�����N���X
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "mmsystem.h"
#include "YNBaseLib.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTFirstPersonCam.h"

using namespace YNBaseLib;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTFirstPersonCam::MTFirstPersonCam(void)
{
	m_CamVector = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_CamDirPhi = 0.0f;
	m_CamDirTheta = 0.0f;
	m_IsMouseCamMode = false;
	m_IsAutoRollMode = false;
	m_hWnd = NULL;

	m_VelocityFB = 15.0f; // m/sec.
	m_VelocityLR = 15.0f; // m/sec.
	m_VelocityUD = 10.0f; // m/sec.
	m_VelocityPT =  6.0f; // degrees/sec.
	m_AcceleRate =  2.0f; // �����{��
	m_PrevTime = 0;
	m_DeltaTime = 0;

	m_RollAngle = 0.0f;
	m_VelocityAutoRoll = 6.0f;
	m_VelocityManualRoll = 1.0f;

	m_PrevTickTime = 0;
	m_CurTickTime = 0;
	m_ProgressDirection = DirX;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTFirstPersonCam::~MTFirstPersonCam(void)
{
	m_DIKeyCtrl.Terminate();
	m_DIMouseCtrl.Terminate();
	_ClipCursor(false);
}

//******************************************************************************
// ����������
//******************************************************************************
int MTFirstPersonCam::Initialize(
		HWND hWnd,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;

	m_hWnd = hWnd;

	//�p�����[�^�ݒ�t�@�C���ǂݍ���
	result = _LoadConfFile(pSceneName);
	if (result != 0) goto EXIT;

	//�m�[�g�f�U�C���I�u�W�F�N�g������
	result = m_NoteDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	//�L�[�{�[�h�f�o�C�X���䏉����
	result = m_DIKeyCtrl.Initialize(hWnd);
	if (result != 0) goto EXIT;

	//�}�E�X�f�o�C�X���䏉����
	result = m_DIMouseCtrl.Initialize(hWnd);
	if (result != 0) goto EXIT;

	//�f�o�C�X�A�N�Z�X���擾
	m_DIKeyCtrl.Acquire();
	m_DIMouseCtrl.Acquire();

	//�Q�[���p�b�h�������F���[�U�C���f�b�N�X0�Œ�
	result = m_GamePadCtrl.Initialize(0);
	if (result != 0) goto EXIT;
	
	//�J����������
	result = m_Camera.Initialize();
	if (result != 0) goto EXIT;

	//��{�p�����[�^�ݒ�
	m_Camera.SetBaseParam(
			45.0f,		//��p
			1.0f,		//Near�v���[���F0����Z�������䂪���������Ȃ�
			1000.0f		//Far�v���[��
		);

	//�J�����ʒu�ݒ�
	m_Camera.SetPosition(
			D3DXVECTOR3(0.0f, 0.0f, 0.0f),	//�J�����ʒu
			D3DXVECTOR3(0.0f, 0.0f, 1.0f), 	//���ړ_
			D3DXVECTOR3(0.0f, 1.0f, 0.0f)	//�J���������
		);

EXIT:;
	return result;
}

//******************************************************************************
// �J�����ʒu�ݒ�
//******************************************************************************
void MTFirstPersonCam::SetPosition(
		D3DXVECTOR3 camVector
	)
{
	m_CamVector = camVector;
}

//******************************************************************************
// �J���������ݒ�
//******************************************************************************
void MTFirstPersonCam::SetDirection(
		float phi,
		float theta
	)
{
	m_CamDirPhi = phi;
	m_CamDirTheta = theta;
}
//******************************************************************************
// �J�����ʒu�擾
//******************************************************************************
void MTFirstPersonCam::GetPosition(
		D3DXVECTOR3* pCamVector
	)
{
	*pCamVector = m_CamVector;
}

//******************************************************************************
// �J���������擾
//******************************************************************************
void MTFirstPersonCam::GetDirection(
		float* pPhi,
		float* pTheta
	)
{
	*pPhi = m_CamDirPhi;
	*pTheta = m_CamDirTheta;
}

//******************************************************************************
// �}�E�X�����ړ����[�h�o�^
//******************************************************************************
void MTFirstPersonCam::SetMouseCamMode(
		bool isEnable
	)
{
	m_IsMouseCamMode = isEnable;

	if (m_IsMouseCamMode) {
		ShowCursor(FALSE);
		_ClipCursor(true);
	}
	else {
		ShowCursor(TRUE);
		_ClipCursor(false);
	}
}

//******************************************************************************
// ������]���[�h�o�^
//******************************************************************************
void MTFirstPersonCam::SetAutoRollMode(
		bool isEnable
	)
{
	m_IsAutoRollMode = isEnable;
}

//******************************************************************************
// ������]�����؂�ւ�
//******************************************************************************
void MTFirstPersonCam::SwitchAutoRllDirecton()
{
	//��]�������t�ɂ���
	m_VelocityAutoRoll *= -1.0f;
}

//******************************************************************************
// �ϊ�����
//******************************************************************************
int MTFirstPersonCam::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	float dt = 0.0f;
	int dX = 0;
	int dY = 0;
	int dW = 0;
	
	//�f���^�^�C��
	dt = (float)m_DeltaTime / 1000.0f;
	
	//TODO: ��������Ȃ��ǂ����ֈڂ�
	m_DIKeyCtrl.Acquire();
	m_DIMouseCtrl.Acquire();

	//�E�B���h�E����A�N�e�B�u��Ԃ̂Ƃ���Ԏ擾���G���[�ɂȂ�
	//�Ƃ肠�����������邯�ǁE�E�E

	//���݂̃L�[�{�[�h��Ԃ��擾
	result = m_DIKeyCtrl.GetKeyStatus();
	//if (result != 0) goto EXIT;
	result = 0;

	//�}�E�X��Ԏ擾
	result = m_DIMouseCtrl.GetMouseStatus();
	//if (result != 0) goto EXIT;
	result = 0;

	//�Q�[���p�b�h��ԍX�V
	result = m_GamePadCtrl.UpdateState();
	if (result != 0) goto EXIT;

	//_RPTN(_CRT_WARN, "GamePad: %f %f\n", m_GamePadCtrl.GetState_ThumbRX(), m_GamePadCtrl.GetState_ThumbRY());
	
	//�}�E�X�^�z�C�[���ړ���
	dX = m_DIMouseCtrl.GetDelta(DIMouseCtrl::AxisX);
	dY = m_DIMouseCtrl.GetDelta(DIMouseCtrl::AxisY);
	dW = m_DIMouseCtrl.GetDelta(DIMouseCtrl::AxisWheel);

	//�}�E�X�����ړ����[�hOFF�Ȃ�ړ��ʂ𖳎�����
	if (!m_IsMouseCamMode) {
		dX = 0;
		dY = 0;
	}

	//�Q�[���p�b�h����F�E�X�e�B�b�N
	//�X�e�B�b�N�l��-1.0����1.0�͈̔�
	dX += (int)(m_VelocityPT * dt * m_GamePadCtrl.GetState_ThumbRX() * (100.0f));
	dY += (int)(m_VelocityPT * dt * m_GamePadCtrl.GetState_ThumbRY() * (-100.0f));
	
	//CTRL+�ړ��L�[�Ŏ���������ω�������
	if (m_DIKeyCtrl.IsKeyDown(DIK_LCONTROL) || m_DIKeyCtrl.IsKeyDown(DIK_RCONTROL)) {
		if (m_DIKeyCtrl.IsKeyDown(DIK_W) || m_DIKeyCtrl.IsKeyDown(DIK_UP)) {
			dY -= (int)m_VelocityPT;
		}
		if (m_DIKeyCtrl.IsKeyDown(DIK_S) || m_DIKeyCtrl.IsKeyDown(DIK_DOWN)) {
			dY += (int)m_VelocityPT;
		}
		if (m_DIKeyCtrl.IsKeyDown(DIK_A) || m_DIKeyCtrl.IsKeyDown(DIK_LEFT)) {
			dX -= (int)m_VelocityPT;
		}
		if (m_DIKeyCtrl.IsKeyDown(DIK_D) || m_DIKeyCtrl.IsKeyDown(DIK_RIGHT)) {
			dX += (int)m_VelocityPT;
		}
	}

	//�f���^�^�C���Z�o
	_CalcDeltaTime();

	//���������̍X�V
	result = _TransformEyeDirection(dX, dY);
	if (result != 0) goto EXIT;

	//�J�����ʒu�̍X�V
	result = _TransformCamPosition();
	if (result != 0) goto EXIT;

	//�J�����ʒu�ݒ�
	result = _SetCamPosition();
	if (result != 0) goto EXIT;

	//�J�����X�V
	result = m_Camera.Transform(pD3DDevice);
	if (result != 0) goto EXIT;

	//��]�Ή�
	result = _TransformRolling(dW);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ���������X�V
//******************************************************************************
int MTFirstPersonCam::_TransformEyeDirection(
		int dX,
		int dY
	)
{
	int result = 0;
	float dt = 0.0f;
	float dPhi = 0.0f;
	float dTheta = 0.0f;

	//�f���^�^�C��
	dt = (float)m_DeltaTime / 1000.0f;

	//�}�E�X�ړ��ʂ�����ʊp�ƓV���p�̑����ʂ��Z�o
	dPhi   = (float)-dX * m_VelocityPT * dt;
	dTheta = (float) dY * m_VelocityPT * dt;

	//�ɒ[�Ȋp�x�̕ω���}�~����
	//  ��ʕ`�悪�������������ꍇ�Ƀ}�E�X�ړ��ʂ��~�ς���
	//  �ˑR����ʕ����������Ă��܂����Ƃ��������
	if (abs(dPhi) > 45.0f) {
		dPhi = 0.0f;
	}
	if (abs(dTheta) > 45.0f) {
		dTheta = 0.0f;
	}

	//�}�E�X�ړ��ʂ���ʊp�ƓV���p�ɔ��f����
	m_CamDirPhi += dPhi;
	m_CamDirTheta += dTheta;

	//�N���b�s���O����
	if (m_CamDirPhi >= 360.0f) {
		m_CamDirPhi -= 360.0f;
	}
	else if (m_CamDirPhi <= -360.0f) {
		m_CamDirPhi += 360.0f;
	}
	if (m_CamDirTheta <= 1.0f) {
		m_CamDirTheta = 1.0f;
	}
	else if (m_CamDirTheta >= 179.0f) {
		m_CamDirTheta = 179.0f;
	}
	//���V���p��0�x�܂���180�x�ɂȂ�ƕ`�悪���������Ȃ�E�E�E

//EXIT:;
	return result;
}

//******************************************************************************
// �J�����ʒu�X�V
//******************************************************************************
int MTFirstPersonCam::_TransformCamPosition()
{
	int result = 0;
	float dFB = 0.0f;
	float dLR = 0.0f;
	float phi = 0.0f;
	float phiRad = 0.0f;
	float distance = 0.0f;
	float dt = 0.0f;
	float rate = 0.0f;
	float progress = 0.0f;
	D3DXVECTOR3 moveVector;

	//�f���^�^�C��
	dt = (float)m_DeltaTime / 1000.0f;

	//�ړ������̕��ʊp
	phi = m_CamDirPhi;

	if (m_DIKeyCtrl.IsKeyDown(DIK_LCONTROL) || m_DIKeyCtrl.IsKeyDown(DIK_RCONTROL)) {
		//��CTRL�܂��͉ECTRL�L�[��������Ă���ꍇ�̓L�[���͂𖳎�����
	}
	else {
		//�ړ����x�̉����{��
		rate = 1.0f;
		if (m_DIKeyCtrl.IsKeyDown(DIK_LSHIFT) || m_DIKeyCtrl.IsKeyDown(DIK_RSHIFT)) {
			rate = m_AcceleRate;
		}
		
		//�O�ړ�
		if (m_DIKeyCtrl.IsKeyDown(DIK_W) || m_DIKeyCtrl.IsKeyDown(DIK_UP)) {
			distance = m_VelocityFB * dt * rate;
			phi += 0.0f;
		}
		//���ړ��F�����͑O���������܂�
		if (m_DIKeyCtrl.IsKeyDown(DIK_S) || m_DIKeyCtrl.IsKeyDown(DIK_DOWN)) {
			distance = m_VelocityFB * dt * rate;
			phi += 180.0f;
		}
		//���ړ��F�����͑O���������܂�
		if (m_DIKeyCtrl.IsKeyDown(DIK_A) || m_DIKeyCtrl.IsKeyDown(DIK_LEFT)) {
			distance = m_VelocityLR * dt * rate;
			phi += 90.0f;
		}
		//�E�ړ��F�����͑O���������܂�
		if (m_DIKeyCtrl.IsKeyDown(DIK_D) || m_DIKeyCtrl.IsKeyDown(DIK_RIGHT)) {
			distance = m_VelocityLR * dt * rate;
			phi += -90.0f;
		}
		//�㏸�F�����ύX�Ȃ�
		if (m_DIKeyCtrl.IsKeyDown(DIK_Q) || m_DIKeyCtrl.IsKeyDown(DIK_PRIOR)) {
			m_CamVector.y += +(m_VelocityUD * dt * rate);
		}
		//���~�F�����ύX�Ȃ�
		if (m_DIKeyCtrl.IsKeyDown(DIK_E) ||  m_DIKeyCtrl.IsKeyDown(DIK_NEXT)) {
			m_CamVector.y += -(m_VelocityUD * dt * rate);
		}
		//-X�������i�ȍĐ��t�����j�Ɉړ��F�����ύX�Ȃ�
		if (m_DIKeyCtrl.IsKeyDown(DIK_Z) || m_DIKeyCtrl.IsKeyDown(DIK_COMMA)) {
			m_CamVector.x +=  -(m_VelocityFB * dt * rate);
		}
		//+X�������i�ȍĐ������j�Ɉړ��F�����ύX�Ȃ�
		if (m_DIKeyCtrl.IsKeyDown(DIK_C) || m_DIKeyCtrl.IsKeyDown(DIK_PERIOD)) {
			m_CamVector.x +=  +(m_VelocityFB * dt * rate);
		}
	}
	
	//�Q�[���p�b�h����F�\���L�[���O�㍶�E�ړ�
	if (distance == 0.0f) {
		if (m_GamePadCtrl.GetState_DPadUp()) {
			dFB = m_VelocityFB * dt * (1.0f);
		}
		if (m_GamePadCtrl.GetState_DPadDown()) {
			dFB = m_VelocityFB * dt * (-1.0f);
		}
		if (m_GamePadCtrl.GetState_DPadRight()) {
			dLR = m_VelocityLR * dt * (-1.0f);
		}
		if (m_GamePadCtrl.GetState_DPadLeft()) {
			dLR = m_VelocityLR * dt * (1.0f);
		}
		distance = sqrt((dFB * dFB) + (dLR * dLR));
		phi += D3DXToDegree(atan2(dLR, dFB));
	}
	//�Q�[���p�b�h����F���X�e�B�b�N���O�㍶�E�ړ�
	if (distance == 0.0f) {
		//�X�e�B�b�N�l��-1.0����1.0�͈̔�
		dFB += m_VelocityFB * dt * m_GamePadCtrl.GetState_ThumbLX() * -1.0f;
		dLR += m_VelocityLR * dt * m_GamePadCtrl.GetState_ThumbLY();
		distance = sqrt((dFB * dFB) + (dLR * dLR));
		phi += D3DXToDegree(atan2(dFB, dLR));
	}
	//�Q�[���p�b�h����FX,Y�{�^�������~,�㏸�ړ�
	if (m_GamePadCtrl.GetState_X()) {
		m_CamVector.y += -(m_VelocityUD * dt);
	}
	if (m_GamePadCtrl.GetState_Y()) {
		m_CamVector.y += +(m_VelocityUD * dt);
	}
	
	//�N���b�s���O
	if (phi >= 360.0f) {
		phi -= 360.0f;
	}
	else if (phi <= -360.0f) {
		phi += 360.0f;
	}

	//�ړ��x�N�g���쐬�i�ɍ��W���璼�s���W�֕ϊ��j
	phiRad = D3DXToRadian(phi);
	moveVector.x = distance * cos(phiRad);  // r * sin(90) * cos(phi)
	moveVector.y = 0.0f;                    // r * cos(90)
	moveVector.z = distance * sin(phiRad);  // r * sin(90) * cos(phi)

	//�J�����ʒu���ړ�
	m_CamVector.x += moveVector.x;
	m_CamVector.y += moveVector.y;
	m_CamVector.z += moveVector.z;

	//���t�ǐ�
	progress = m_NoteDesign.GetPlayPosX(m_CurTickTime) - m_NoteDesign.GetPlayPosX(m_PrevTickTime);
	switch (m_ProgressDirection) {
		case DirX:
			m_CamVector.x += progress;
			break;
		case DirY:
			m_CamVector.y += progress;
			break;
		case DirZ:
			m_CamVector.z += progress;
			break;
	}

	//�J�����ʒu�N���b�s���O
	_ClipCamVector(&m_CamVector);

	m_PrevTickTime = m_CurTickTime;

//EXIT:;
	return result;
}

//******************************************************************************
// ��]�Ή�
//******************************************************************************
int MTFirstPersonCam::_TransformRolling(
		int dW
	)
{
	int result = 0;
	float dt = 0.0f;
	float domega = 0.0f;

	//�f���^�^�C��
	dt = (float)m_DeltaTime / 1000.0f;

	//�z�C�[���ړ��ʂ���p�x���Z�o
	domega = (float)dW * m_VelocityManualRoll * dt;

	//�ɒ[�Ȋp�x�̕ω���}�~����
	//  ��ʕ`�悪�������������ꍇ�Ƀ}�E�X�ړ��ʂ��~�ς���
	//  �ˑR����ʕ����������Ă��܂����Ƃ��������
	if (abs(domega) > 45.0f) {
		domega = 0.0f;
	}

	//������]
	if (m_IsAutoRollMode) {
		domega += m_VelocityAutoRoll * dt;
	}

	//��]�p�x�X�V
	m_RollAngle += domega;

	//��]�p�x�̃N���b�v
	if (m_RollAngle >= 360.0f) {
		m_RollAngle -= 360.0f;
	}
	else if (m_RollAngle <= -360.0f) {
		m_RollAngle += 360.0f;
	}

//EXIT:;
	return result;
}

//******************************************************************************
// �蓮��]�p�x�擾
//******************************************************************************
float MTFirstPersonCam::GetManualRollAngle()
{
	return m_RollAngle;
}

//******************************************************************************
// �蓮��]�p�x�ݒ�
//******************************************************************************
void MTFirstPersonCam::SetManualRollAngle(
		float rollAngle
	)
{
	m_RollAngle = rollAngle;
}

//******************************************************************************
// ������]���x�擾
//******************************************************************************
float MTFirstPersonCam::GetAutoRollVelocity()
{
	return m_VelocityAutoRoll;
}

//******************************************************************************
// ������]���x�ݒ�
//******************************************************************************
void MTFirstPersonCam::SetAutoRollVelocity(
		float rollVelocity
	)
{
	m_VelocityAutoRoll = rollVelocity;
}

//******************************************************************************
// �J�����ʒu�ݒ�
//******************************************************************************
int MTFirstPersonCam::_SetCamPosition()
{
	int result = 0;
	float phiRad = 0.0f;
	float thetaRad = 0.0f;
	D3DXVECTOR3 lookVector;
	D3DXVECTOR3 camLookAtVector;
	D3DXVECTOR3 camUpVector;

	//�����x�N�g���i�ɍ��W���璼�����W�֕ϊ��j
	phiRad    = D3DXToRadian(m_CamDirPhi);
	thetaRad  = D3DXToRadian(m_CamDirTheta);
	lookVector.x = 10.0f * sin(thetaRad) * cos(phiRad);
	lookVector.y = 10.0f * cos(thetaRad);
	lookVector.z = 10.0f * sin(thetaRad) * sin(phiRad);

	//�J�����ʒu�Ɏ����x�N�g���𑫂��Ē��ړ_���Z�o
	camLookAtVector = m_CamVector;
	camLookAtVector.x += lookVector.x;
	camLookAtVector.y += lookVector.y;
	camLookAtVector.z += lookVector.z;

	//�J���������
	camUpVector = D3DXVECTOR3(0.0f, 1.0f, 0.0f);

	//�J�����ʒu�o�^
	m_Camera.SetPosition(
			m_CamVector,		//�J�����ʒu
			camLookAtVector, 	//���ړ_
			camUpVector			//�J���������
		);

	return result;
}

//******************************************************************************
// �J�[�\���ړ��͈͐���
//******************************************************************************
int MTFirstPersonCam::_ClipCursor(
		bool isClip
	)
{
	int result = 0;
	BOOL bresult = FALSE;
	RECT wrect;
	RECT crect;
	RECT clip;
	int wh = 0;
	int ww = 0;
	int ch = 0;
	int cw = 0;

	if (isClip) {
		bresult = GetWindowRect(m_hWnd, &wrect);
		if (!bresult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)m_hWnd);
			goto EXIT;
		}
		bresult = GetClientRect(m_hWnd, &crect);
		if (!bresult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), (DWORD64)m_hWnd);
			goto EXIT;
		}
		wh = wrect.bottom - wrect.top;
		ww = wrect.right  - wrect.left;
		ch = crect.bottom - crect.top;
		cw = crect.right  - crect.left;
		clip = wrect;
		clip.left   += +(ww - cw);
		clip.right  += -(ww - cw);
		clip.top    += +(wh - ch);
		clip.bottom += -(wh - ch);
		bresult = ClipCursor(&clip);
		if (!bresult) {
			result = YN_SET_ERR("Windows API error.", GetLastError(), 0);
			goto EXIT;
		}
	}
	else {
		ClipCursor(NULL);
	}

EXIT:;
	return result;
}

//******************************************************************************
// �f���^�^�C���擾
//******************************************************************************
void MTFirstPersonCam::_CalcDeltaTime()
{
	unsigned long curTime = 0;
	
	curTime = timeGetTime();
	
	if (m_PrevTime == 0) {
		//���񑪒莞�͕ω��Ȃ��Ƃ���
		m_DeltaTime = 0;
	}
	else {
		//�f���^�^�C��
		//49.71�����܂����ꍇ�����̌v�Z�Ŗ��Ȃ��͂�
		m_DeltaTime = curTime - m_PrevTime;
	}
	
	m_PrevTime = curTime;
	
	return;
}

//******************************************************************************
// �`�b�N�^�C���ݒ�
//******************************************************************************
void MTFirstPersonCam::SetCurTickTime(
		unsigned long curTickTime
	)
{
	m_CurTickTime = curTickTime;
}

//******************************************************************************
// ���Z�b�g
//******************************************************************************
void MTFirstPersonCam::Reset()
{
	m_PrevTime = 0;
	m_DeltaTime = 0;
	m_PrevTickTime = 0;
	m_CurTickTime = 0;
}

//******************************************************************************
// �ݒ�t�@�C���ǂݍ���
//******************************************************************************
int MTFirstPersonCam::_LoadConfFile(
		const TCHAR* pSceneName
	)
{
	int result = 0;
	MTConfFile confFile;

	result = confFile.Initialize(pSceneName);
	if (result != 0) goto EXIT;

	//�J�����ړ����x���擾
	result = confFile.SetCurSection(_T("FirstPersonCam"));
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("VelocityFB"), &m_VelocityFB, 15.0f);
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("VelocityLR"), &m_VelocityLR, 15.0f);
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("VelocityUD"), &m_VelocityUD, 10.0f);
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("VelocityPT"), &m_VelocityPT, 6.0f);
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("AcceleRate"), &m_AcceleRate, 2.0f);
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("VelocityAutoRoll"), &m_VelocityAutoRoll, 6.0f);
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("VelocityManualRoll"), &m_VelocityManualRoll, 1.0f);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �J�����ʒu�N���b�s���O
//******************************************************************************
void MTFirstPersonCam::_ClipCamVector(
		D3DXVECTOR3* pVector
	)
{
	if (pVector->x < -(MTFIRSTPERSONCAM_CAMVECTOR_LIMIT)) {
		pVector->x = -(MTFIRSTPERSONCAM_CAMVECTOR_LIMIT);
	}
	if (pVector->x > MTFIRSTPERSONCAM_CAMVECTOR_LIMIT) {
		pVector->x = MTFIRSTPERSONCAM_CAMVECTOR_LIMIT;
	}
	if (pVector->y < -(MTFIRSTPERSONCAM_CAMVECTOR_LIMIT)) {
		pVector->y = -(MTFIRSTPERSONCAM_CAMVECTOR_LIMIT);
	}
	if (pVector->y > MTFIRSTPERSONCAM_CAMVECTOR_LIMIT) {
		pVector->y = MTFIRSTPERSONCAM_CAMVECTOR_LIMIT;
	}
	if (pVector->z < -(MTFIRSTPERSONCAM_CAMVECTOR_LIMIT)) {
		pVector->z = -(MTFIRSTPERSONCAM_CAMVECTOR_LIMIT);
	}
	if (pVector->z > MTFIRSTPERSONCAM_CAMVECTOR_LIMIT) {
		pVector->z = MTFIRSTPERSONCAM_CAMVECTOR_LIMIT;
	}
}

//******************************************************************************
// �i�s�����ݒ�
//******************************************************************************
void MTFirstPersonCam::SetProgressDirection(
		MTProgressDirection dir
	)
{
	m_ProgressDirection = dir;
}


