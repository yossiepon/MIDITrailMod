//******************************************************************************
//
// MIDITrail / MTPianoKeyboardMod
//
// �s�A�m�L�[�{�[�h�`��Mod�N���X
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTPianoKeyboardMod.h"

using namespace YNBaseLib;


//******************************************************************************
// �p�����[�^��`
//******************************************************************************
//�e�L�[�̒��_��
#define MTPIANOKEYBOARD_KEY_WHITE_1_VERTEX_NUM  (38)
#define MTPIANOKEYBOARD_KEY_WHITE_2_VERTEX_NUM  (44)
#define MTPIANOKEYBOARD_KEY_WHITE_3_VERTEX_NUM  (38)
#define MTPIANOKEYBOARD_KEY_BLACK_VERTEX_NUM    (30)
#define MTPIANOKEYBOARD_KEY_VERTEX_NUM_MAX      MTPIANOKEYBOARD_KEY_WHITE_2_VERTEX_NUM

//�e�L�[�̃C���f�b�N�X��
#define MTPIANOKEYBOARD_KEY_WHITE_1_INDEX_NUM   (60)
#define MTPIANOKEYBOARD_KEY_WHITE_2_INDEX_NUM   (66)
#define MTPIANOKEYBOARD_KEY_WHITE_3_INDEX_NUM   (60)
#define MTPIANOKEYBOARD_KEY_BLACK_INDEX_NUM     (48)
#define MTPIANOKEYBOARD_KEY_INDEX_NUM_MAX       MTPIANOKEYBOARD_KEY_WHITE_2_INDEX_NUM


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTPianoKeyboardMod::MTPianoKeyboardMod(void)
{
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTPianoKeyboardMod::~MTPianoKeyboardMod(void)
{
}

//******************************************************************************
// ��������
//******************************************************************************
int MTPianoKeyboardMod::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData,
		LPDIRECT3DTEXTURE9 pTexture
	)
{
	int result = 0;
	SMTrack track;

	//���N���X�̐����������Ăяo��
	result = MTPianoKeyboard::Create(pD3DDevice, pSceneName, pSeqData,  pTexture);
	if (result != 0) goto EXIT;
	
	//�L�[�{�[�h�f�U�C��������
	result = m_KeyboardDesignMod.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �ړ�
//******************************************************************************
int MTPianoKeyboardMod::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice,
		D3DXVECTOR3 moveVector1,
		D3DXVECTOR3 moveVector2,
		float scale,
		float z,
		float rollAngle
	)
{
	int result = 0;
	D3DXMATRIX scaleMatrix;
	D3DXMATRIX rotateMatrix1;
	D3DXMATRIX rotateMatrix2;
	D3DXMATRIX rotateMatrix3;
	D3DXMATRIX moveMatrix1;
	D3DXMATRIX moveMatrix2;
	D3DXMATRIX moveMatrix3;
	D3DXMATRIX worldMatrix;

	//�s�񏉊���
	D3DXMatrixIdentity(&scaleMatrix);
	D3DXMatrixIdentity(&rotateMatrix1);
	D3DXMatrixIdentity(&rotateMatrix2);
	D3DXMatrixIdentity(&rotateMatrix3);
	D3DXMatrixIdentity(&moveMatrix1);
	D3DXMatrixIdentity(&moveMatrix2);
	D3DXMatrixIdentity(&worldMatrix);

	//��]�s��

	if(rollAngle < 0.0f) {
		rollAngle += 360.0f;
	}

	if((rollAngle > 120.0f) && (rollAngle < 300.0f)) {
		D3DXMatrixRotationX(&rotateMatrix1, D3DX_PI / 2.0f);
		D3DXMatrixRotationZ(&rotateMatrix2, D3DX_PI / 2.0f);
	} else {
		D3DXMatrixRotationX(&rotateMatrix1, -D3DX_PI / 2.0f);
		D3DXMatrixRotationZ(&rotateMatrix2, D3DX_PI / 2.0f);
	}

	D3DXMatrixRotationX(&rotateMatrix3, D3DXToRadian(rollAngle));

	//�ړ��s��
	D3DXMatrixTranslation(&moveMatrix1, moveVector1.x, moveVector1.y, moveVector1.z);
	D3DXMatrixTranslation(&moveMatrix2, moveVector2.x, moveVector2.y, moveVector2.z);
	D3DXMatrixTranslation(&moveMatrix3, 0.0f, 0.0f, z / scale);

	//�X�P�[���s��
	D3DXMatrixScaling(&scaleMatrix, scale, scale, scale);

	//�s��̍����F�s�b�`�x���h�ړ��P�����Ռ����␳��]�P�E�Q���O���b�h�ʂ܂ňړ��R���z�C�[����]�R���X�P�[�����Đ��ʒǏ]�ړ��Q
	D3DXMatrixMultiply(&worldMatrix, &worldMatrix, &moveMatrix1);
	D3DXMatrixMultiply(&worldMatrix, &worldMatrix, &rotateMatrix1);
	D3DXMatrixMultiply(&worldMatrix, &worldMatrix, &rotateMatrix2);
	D3DXMatrixMultiply(&worldMatrix, &worldMatrix, &moveMatrix3);
	D3DXMatrixMultiply(&worldMatrix, &worldMatrix, &rotateMatrix3);
	D3DXMatrixMultiply(&worldMatrix, &worldMatrix, &scaleMatrix);
	D3DXMatrixMultiply(&worldMatrix, &worldMatrix, &moveMatrix2);

	//�ϊ��s��ݒ�
	m_PrimitiveKeyboard.Transform(worldMatrix);

//EXIT:;
	return result;
}

//******************************************************************************
// �L�[�̉�������
//******************************************************************************
int MTPianoKeyboardMod::PushKey(
		unsigned char chNo,
		unsigned char noteNo,
		float keyDownRate,
		unsigned long elapsedTime,
		D3DXCOLOR* pNoteColor
	)
{
	int result = 0;
	float angle = 0.0f;
	D3DXCOLOR color;

	if (noteNo >= SM_MAX_NOTE_NUM) {
		result = YN_SET_ERR("Program error.", noteNo, 0);
		goto EXIT;
	}

	angle = m_KeyboardDesignMod.GetKeyRotateAngle() * keyDownRate;

	if (keyDownRate < 1.0f) {
		//�L�[�����~���^�㏸���̏ꍇ�͐F��ύX������]������
		_RotateKey(noteNo, angle);
	}
	else {
		//�L�[��������Ԃ̏ꍇ�͐F��ύX���ĉ�]������
		color = m_KeyboardDesignMod.GetActiveKeyColor(chNo, noteNo, elapsedTime, pNoteColor);
		_RotateKey(noteNo, angle, &color);
	}

EXIT:;
	return result;
}

