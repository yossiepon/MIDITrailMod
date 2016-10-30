//******************************************************************************
//
// MIDITrail / MTNoteRipple
//
// �m�[�g�g��`��N���X
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// �g��́A�ʒu�ƃT�C�Y���ʂɕω�����B
// ���̂��ߏ��񃊃��[�X�O�́ADrawPrimitiveUP�ŕ`�悵�Ă����B
// �܂�`��̂��тɔg��̒��_��GPU���ɗ�������ł����B
// ���������t�J�n��̏���̔g��`�掞�Ɍ���A����������������iFPS��
// ������j���ۂ������������߁A���̕���������߂��B
// ����ɒ��_�o�b�t�@���g�p���A�`��̂��тɃo�b�t�@�����b�N���Ē��_
// ����������������ɐ؂�ւ����B
// �����ƃG���K���g�ȕ��@�����邩������Ȃ��E�E�E�B

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTNoteRipple.h"
#include <new>

using namespace YNBaseLib;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTNoteRipple::MTNoteRipple(void)
{
	m_pTexture = NULL;
	m_pNoteStatus = NULL;
	m_CurTickTime = 0;
	m_ActiveNoteNum = 0;
	m_CamVector = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	ZeroMemory(&m_Material, sizeof(D3DMATERIAL9));
	m_isEnable = true;
	m_isSkipping = false;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTNoteRipple::~MTNoteRipple(void)
{
	Release();
}

//******************************************************************************
// �m�[�g�g�䐶��
//******************************************************************************
int MTNoteRipple::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData,
		MTNotePitchBend* pNotePitchBend
   )
{
	int result = 0;

	Release();

	//�m�[�g�f�U�C���I�u�W�F�N�g������
	result = m_NoteDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	//�e�N�X�`������
	result = _CreateTexture(pD3DDevice, pSceneName);
	if (result != 0) goto EXIT;

	//�m�[�g���z�񐶐�
	result = _CreateNoteStatus();
	if (result != 0) goto EXIT;

	//���_����
	result = _CreateVertex(pD3DDevice);
	if (result != 0) goto EXIT;

	//�}�e���A���쐬
	_MakeMaterial(&m_Material);

	//�s�b�`�x���h���
	m_pNotePitchBend = pNotePitchBend;

EXIT:;
	return result;
}

//******************************************************************************
// �ړ�
//******************************************************************************
int MTNoteRipple::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice,
		D3DXVECTOR3 camVector,
		float rollAngle
	)
{
	int result = 0;
	D3DXVECTOR3 moveVector;
	D3DXMATRIX rotateMatrix;
	D3DXMATRIX moveMatrix;
	D3DXMATRIX worldMatrix;

	m_CamVector = camVector;

	//�g��̒��_�X�V
	result = _TransformRipple(pD3DDevice);
	if (result != 0) goto EXIT;

	//�s�񏉊���
	D3DXMatrixIdentity(&rotateMatrix);
	D3DXMatrixIdentity(&moveMatrix);
	D3DXMatrixIdentity(&worldMatrix);

	//��]�s��
	D3DXMatrixRotationX(&rotateMatrix, D3DXToRadian(rollAngle));

	//�ړ��s��
	moveVector = m_NoteDesign.GetWorldMoveVector();
	D3DXMatrixTranslation(&moveMatrix, moveVector.x, moveVector.y, moveVector.z);

	//�s��̍���
	D3DXMatrixMultiply(&worldMatrix, &rotateMatrix, &moveMatrix);

	//�ϊ��s��ݒ�
	m_Primitive.Transform(worldMatrix);

EXIT:;
	return result;
}

//******************************************************************************
// �g��̒��_�X�V
//******************************************************************************
int MTNoteRipple::_TransformRipple(
		LPDIRECT3DDEVICE9 pD3DDevice
   )
{
	int result = 0;

	//�X�L�b�v���Ȃ牽�����Ȃ�
	if (m_isSkipping) goto EXIT;

	//�g��̒��_�X�V
	result = _UpdateVertexOfRipple(pD3DDevice);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �g��̒��_�X�V
//******************************************************************************
int MTNoteRipple::_UpdateVertexOfRipple(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	MTNOTERIPPLE_VERTEX* pVertex = NULL;
	D3DXMATRIX mtxWorld;
	unsigned long i = 0;
	unsigned long activeNoteNum = 0;
	unsigned long curTime = 0;
	bool isTimeout = false;

	if (pD3DDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	curTime = timeGetTime();

	//�o�b�t�@�̃��b�N
	result = m_Primitive.LockVertex((void**)&pVertex);
	if (result != 0) goto EXIT;

	//�������m�[�g�̔g��ɂ��Ē��_���X�V
	for (i = 0; i < MTNOTERIPPLE_MAX_RIPPLE_NUM; i++) {
		if (m_pNoteStatus[i].isActive) {
			//���_�X�V�F�g��̕`��ʒu�ƃT�C�Y��ς���
			_SetVertexPosition(
					&(pVertex[activeNoteNum*6]),
					&(m_pNoteStatus[i]),
					activeNoteNum,
					curTime,
					&isTimeout
				);
			if (isTimeout) {
				//���Ԑ؂����
				m_pNoteStatus[i].isActive = false;
			}
			else {
			 	activeNoteNum++;
		 	}
		}
	}
	m_ActiveNoteNum = activeNoteNum;

	//�o�b�t�@�̃��b�N����
	result = m_Primitive.UnlockVertex();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �`��
//******************************************************************************
int MTNoteRipple::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
   )
{
	int result = 0;

	if (pD3DDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	if (!m_isEnable) goto EXIT;

	//�e�N�X�`���X�e�[�W�ݒ�
	//  �J���[���Z�F��Z  ����1�F�e�N�X�`��  ����2�F�|���S��
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	// �A���t�@���Z�F����1���g�p  ����1�F�|���S��
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	//�e�N�X�`���t�B���^
	pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	//�v���~�e�B�u�`��
	if (m_ActiveNoteNum > 0) {
		//�o�b�t�@�S�̂łȂ��g��̐��ɍ��킹�ĕ`�悷��v���~�e�B�u�����炷
		result = m_Primitive.Draw(pD3DDevice, m_pTexture, 2 * m_ActiveNoteNum);
		if (result != 0) goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// ���
//******************************************************************************
void MTNoteRipple::Release()
{
	m_Primitive.Release();

	if (m_pTexture != NULL) {
		m_pTexture->Release();
		m_pTexture = NULL;
	}

	if(m_pNoteStatus != NULL) {
		delete [] m_pNoteStatus;
		m_pNoteStatus = NULL;
	}
}

//******************************************************************************
// �e�N�X�`������
//******************************************************************************
int MTNoteRipple::_CreateTexture(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;
	TCHAR imgFilePath[_MAX_PATH] = {_T('\0')};
	TCHAR bmpFileName[_MAX_PATH] = {_T('\0')};
	MTConfFile confFile;

	result = confFile.Initialize(pSceneName);
	if (result != 0) goto EXIT;

	//�r�b�g�}�b�v�t�@�C����
	result = confFile.SetCurSection(_T("Bitmap"));
	if (result != 0) goto EXIT;
	result = confFile.GetStr(_T("Ripple"), bmpFileName, _MAX_PATH, MT_IMGFILE_RIPPLE);
	if (result != 0) goto EXIT;

	//�v���Z�X���s�t�@�C���f�B���N�g���p�X�擾
	result = YNPathUtil::GetModuleDirPath(imgFilePath, _MAX_PATH);
	if (result != 0) goto EXIT;

	//�摜�t�@�C���p�X
	_tcscat_s(imgFilePath, _MAX_PATH, bmpFileName);

	hresult = D3DXCreateTextureFromFileEx(
					pD3DDevice,			//�f�o�C�X
					imgFilePath,		//�t�@�C���p�X
					0,					//���F�t�@�C������擾
					0,					//�����F�t�@�C������擾
					0,					//�~�b�v���x��
					0,					//�g�p���@
					D3DFMT_A8R8G8B8,	//�s�N�Z���t�H�[�}�b�g
					D3DPOOL_MANAGED,	//�e�N�X�`���z�u�惁�����N���X
					D3DX_FILTER_LINEAR,	//�t�B���^�����O�w��
					D3DX_FILTER_LINEAR,	//�t�B���^�����O�w��i�~�b�v�j
					0xFF000000,			//�����F�̎w��F�s������
					NULL,				//�\�[�X�C���[�W���
					NULL,				//256�p���b�g���
					&m_pTexture			//�쐬�����e�N�X�`���I�u�W�F�N�g
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �m�[�g���z�񐶐�
//******************************************************************************
int MTNoteRipple::_CreateNoteStatus()
{
	int result = 0;
	unsigned long i = 0;

	//���_����
	try {
		m_pNoteStatus = new NoteStatus[MTNOTERIPPLE_MAX_RIPPLE_NUM];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

	ZeroMemory(m_pNoteStatus, sizeof(NoteStatus) * MTNOTERIPPLE_MAX_RIPPLE_NUM);

	for (i = 0; i < MTNOTERIPPLE_MAX_RIPPLE_NUM; i++) {
		m_pNoteStatus[i].isActive = false;
	}

EXIT:;
	return result;
}

//******************************************************************************
// ���_����
//******************************************************************************
int MTNoteRipple::_CreateVertex(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	unsigned long vertexNum = 0;
	MTNOTERIPPLE_VERTEX* pVertex = NULL;

	//�v���~�e�B�u������
	result = m_Primitive.Initialize(
					sizeof(MTNOTERIPPLE_VERTEX),//���_�T�C�Y
					_GetFVFFormat(),			//���_FVF�t�H�[�}�b�g
					D3DPT_TRIANGLELIST			//�v���~�e�B�u���
				);
	if (result != 0) goto EXIT;

	//���_�o�b�t�@����
	vertexNum = 6 * MTNOTERIPPLE_MAX_RIPPLE_NUM;
	result = m_Primitive.CreateVertexBuffer(pD3DDevice, vertexNum);
	if (result != 0) goto EXIT;

	//�o�b�t�@�̃��b�N
	result = m_Primitive.LockVertex((void**)&pVertex);
	if (result != 0) goto EXIT;

	ZeroMemory(pVertex, sizeof(MTNOTERIPPLE_VERTEX) * 6 * MTNOTERIPPLE_MAX_RIPPLE_NUM);

	//�o�b�t�@�̃��b�N����
	result = m_Primitive.UnlockVertex();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ���_�̍��W�ݒ�
//******************************************************************************
int MTNoteRipple::_SetVertexPosition(
		MTNOTERIPPLE_VERTEX* pVertex,
		NoteStatus* pNoteStatus,
		unsigned long rippleNo,
		unsigned long curTime,
		bool* pIsTimeout
	)
{
	int result = 0;
	unsigned long i = 0;
	float rh, rw = 0.0f;
	float alpha = 0.0f;
	D3DXVECTOR3 center;
	D3DXCOLOR color;
	unsigned long elapsedTime = 0;
	short pbValue = 0;
	unsigned char pbSensitivity = SM_DEFAULT_PITCHBEND_SENSITIVITY;

	*pIsTimeout = false;

	pbValue =       m_pNotePitchBend->GetValue(pNoteStatus->portNo, pNoteStatus->chNo);
	pbSensitivity = m_pNotePitchBend->GetSensitivity(pNoteStatus->portNo, pNoteStatus->chNo);

	//�m�[�g�{�b�N�X���S���W�擾
	center = m_NoteDesign.GetNoteBoxCenterPosX(
					m_CurTickTime,
					pNoteStatus->portNo,
					pNoteStatus->chNo,
					pNoteStatus->noteNo,
					pbValue,
					pbSensitivity
				);

	//�����J�n����̌o�ߎ���
	elapsedTime = curTime - pNoteStatus->regTime;

	//�g��T�C�Y
	rh = m_NoteDesign.GetRippleHeight(elapsedTime);
	rw = m_NoteDesign.GetRippleWidth(elapsedTime);

	//�`��I���m�F
	if ((rh <= 0.0f) || (rw <= 0.0f)) {
		*pIsTimeout = true;
	}

	//�g����Đ����ʏォ��J�������ɏ��������������ĕ`�悷��
	//�܂��g�䓯�m�����ꕽ�ʏ�ŏd�Ȃ�Ȃ��悤�ɕ`�悷��
	//  Z�t�@�C�e�B���O�ɂ���Ĕ������邿����₩������������
	//  �O���t�B�b�N�J�[�h�ɂ���Č��ۂ��قȂ�
	if (center.x < m_CamVector.x) {
		center.x += (+(float)(rippleNo + 1) * 0.002f);
	}
	else {
		center.x += (-(float)(rippleNo + 1) * 0.002f);
	}

	//���_���W
	pVertex[0].p = D3DXVECTOR3(center.x, center.y+(rh/2.0f), center.z+(rw/2.0f));
	pVertex[1].p = D3DXVECTOR3(center.x, center.y+(rh/2.0f), center.z-(rw/2.0f));
	pVertex[2].p = D3DXVECTOR3(center.x, center.y-(rh/2.0f), center.z+(rw/2.0f));
	pVertex[3].p = pVertex[2].p;
	pVertex[4].p = pVertex[1].p;
	pVertex[5].p = D3DXVECTOR3(center.x, center.y-(rh/2.0f), center.z-(rw/2.0f));

	//�@��
	for (i = 0; i < 6; i++) {
		pVertex[i].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
	}

	//�����x�����X�ɗ��Ƃ�
	alpha = m_NoteDesign.GetRippleAlpha(elapsedTime);

	//�e���_�̃f�B�t���[�Y�F
	for (i = 0; i < 6; i++) {
		color = m_NoteDesign.GetNoteBoxColor(
								pNoteStatus->portNo,
								pNoteStatus->chNo,
								pNoteStatus->noteNo
							);
		pVertex[i].c = D3DXCOLOR(color.r, color.g, color.b, alpha);
	}

	//�e�N�X�`�����W
	pVertex[0].t = D3DXVECTOR2(0.0f, 0.0f);
	pVertex[1].t = D3DXVECTOR2(1.0f, 0.0f);
	pVertex[2].t = D3DXVECTOR2(0.0f, 1.0f);
	pVertex[3].t = pVertex[2].t;
	pVertex[4].t = pVertex[1].t;
	pVertex[5].t = D3DXVECTOR2(1.0f, 1.0f);

	return result;
}

//******************************************************************************
// �}�e���A���쐬
//******************************************************************************
void MTNoteRipple::_MakeMaterial(
		D3DMATERIAL9* pMaterial
	)
{
	ZeroMemory(pMaterial, sizeof(D3DMATERIAL9));
	
	//�g�U��
	pMaterial->Diffuse.r = 1.0f;
	pMaterial->Diffuse.g = 1.0f;
	pMaterial->Diffuse.b = 1.0f;
	pMaterial->Diffuse.a = 1.0f;
	//�����F�e�̐F
	pMaterial->Ambient.r = 0.5f;
	pMaterial->Ambient.g = 0.5f;
	pMaterial->Ambient.b = 0.5f;
	pMaterial->Ambient.a = 1.0f;
	//���ʔ��ˌ�
	pMaterial->Specular.r = 0.2f;
	pMaterial->Specular.g = 0.2f;
	pMaterial->Specular.b = 0.2f;
	pMaterial->Specular.a = 1.0f;
	//���ʔ��ˌ��̑N���x
	pMaterial->Power = 10.0f;
	//�����F
	pMaterial->Emissive.r = 0.0f;
	pMaterial->Emissive.g = 0.0f;
	pMaterial->Emissive.b = 0.0f;
	pMaterial->Emissive.a = 0.0f;
}

//******************************************************************************
// �m�[�gOFF�o�^
//******************************************************************************
void MTNoteRipple::SetNoteOff(
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo
	)
{
	unsigned long i = 0;

	//�Y���̃m�[�g���𖳌���
	for (i = 0; i < MTNOTERIPPLE_MAX_RIPPLE_NUM; i++) {
		if ((m_pNoteStatus[i].isActive)
		 && (m_pNoteStatus[i].portNo == portNo)
		 && (m_pNoteStatus[i].chNo == chNo)
		 && (m_pNoteStatus[i].noteNo == noteNo)) {
			m_pNoteStatus[i].isActive = false;
			break;
		}
	}

	return;
}

//******************************************************************************
// �m�[�gON�o�^
//******************************************************************************
void MTNoteRipple::SetNoteOn(
		unsigned char portNo,
		unsigned char chNo,
		unsigned char noteNo,
		unsigned char velocity
	)
{
	unsigned long i = 0;

	//�󂫃X�y�[�X�Ƀm�[�g����o�^
	//�󂫂�������Ȃ���Δg��̕\���͂�����߂�
	for (i = 0; i < MTNOTERIPPLE_MAX_RIPPLE_NUM; i++) {
		if (!(m_pNoteStatus[i].isActive)) {
			m_pNoteStatus[i].isActive = true;
		 	m_pNoteStatus[i].portNo = portNo;
		 	m_pNoteStatus[i].chNo = chNo;
		 	m_pNoteStatus[i].noteNo = noteNo;
		 	m_pNoteStatus[i].velocity = velocity;
		 	m_pNoteStatus[i].regTime = timeGetTime();
			break;
		}
	}

	return;
}

//******************************************************************************
// �J�����g�`�b�N�^�C���ݒ�
//******************************************************************************
void MTNoteRipple::SetCurTickTime(
		unsigned long curTickTime
	)
{
	m_CurTickTime = curTickTime;
}

//******************************************************************************
// ���Z�b�g
//******************************************************************************
void MTNoteRipple::Reset()
{
	unsigned long i = 0;

	m_CurTickTime = 0;
	m_ActiveNoteNum = 0;

	for (i = 0; i < MTNOTERIPPLE_MAX_RIPPLE_NUM; i++) {
		m_pNoteStatus[i].isActive = false;
	}

	return;
}

//******************************************************************************
// �\���ݒ�
//******************************************************************************
void MTNoteRipple::SetEnable(
		bool isEnable
	)
{
	m_isEnable = isEnable;
}

//******************************************************************************
// �X�L�b�v��Ԑݒ�
//******************************************************************************
void MTNoteRipple::SetSkipStatus(
		bool isSkipping
	)
{
	m_isSkipping = isSkipping;
}


