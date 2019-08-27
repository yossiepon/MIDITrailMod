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
		D3DXVECTOR3 basePosVector,
		D3DXVECTOR3 playbackPosVector,
		float rollAngle
	)
{
	int result = 0;
	D3DXMATRIX scaleMatrix;
	D3DXMATRIX rotateMatrix1;
	D3DXMATRIX rotateMatrix2;
	D3DXMATRIX rotateMatrix3;
	D3DXMATRIX basePosMatrix;
	D3DXMATRIX playbackPosMatrix;
	D3DXMATRIX worldMatrix;

	//�s�񏉊���
	D3DXMatrixIdentity(&scaleMatrix);
	D3DXMatrixIdentity(&rotateMatrix1);
	D3DXMatrixIdentity(&rotateMatrix2);
	D3DXMatrixIdentity(&basePosMatrix);
	D3DXMatrixIdentity(&playbackPosMatrix);
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
	D3DXMatrixTranslation(&basePosMatrix, basePosVector.x, basePosVector.y, basePosVector.z);
	D3DXMatrixTranslation(&playbackPosMatrix, playbackPosVector.x, playbackPosVector.y, playbackPosVector.z);

	//�X�P�[���s��
	float scale = m_KeyboardDesignMod.GetKeyboardResizeRatio();
	D3DXMatrixScaling(&scaleMatrix, scale, scale, scale);

	//�s��̍����F�X�P�[�������_�ړ�����]�P�E�Q�i���Ռ����␳�j����]�R�i�z�C�[���p�x�j���Đ��ʒu�Ǐ]�ړ�
	D3DXMatrixMultiply(&worldMatrix, &worldMatrix, &scaleMatrix);
	D3DXMatrixMultiply(&worldMatrix, &worldMatrix, &basePosMatrix);
	D3DXMatrixMultiply(&worldMatrix, &worldMatrix, &rotateMatrix1);
	D3DXMatrixMultiply(&worldMatrix, &worldMatrix, &rotateMatrix2);
	D3DXMatrixMultiply(&worldMatrix, &worldMatrix, &rotateMatrix3);
	D3DXMatrixMultiply(&worldMatrix, &worldMatrix, &playbackPosMatrix);

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

//******************************************************************************
// �L�[�{�[�h���_�����F����A
//******************************************************************************
int MTPianoKeyboardMod::_CreateVertexOfKeyWhite1(
		unsigned char noteNo,
		MTPIANOKEYBOARD_VERTEX* pVertex,
		unsigned long* pIndex,
		D3DXCOLOR* pColor
	)
{
	//���N���X�̒��_�����������Ăяo��
	int result = MTPianoKeyboard::_CreateVertexOfKeyWhite1(
			noteNo,
			pVertex,
			pIndex,
			pColor
		);

	// �`�揇�̏C��
	int idx = 0;

	//----------------------------------------------------------------
	//���̖�
	//----------------------------------------------------------------
	//  37 6+--+5 36
	//      |  |
	//      |  |
	//      |  |4 35
	//  34 3+--+--+2 33
	//      |     |     +z
	//      |     |      |
	//      |     |      |
	//  31 0+-----+1 32  +---> +x
	//         |
	//        posX

	//���ʃC���f�b�N�X
	unsigned long indexDW[] = { 31, 32, 33, 31, 33, 34, 34, 35, 36, 34, 36, 37 };
	for (int i = 0; i < 12; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + indexDW[i];
	}

	//----------------------------------------------------------------
	//���� 0-1
	//----------------------------------------------------------------
	// 0      1
	// 7+----+8
	//  |    |
	// 9+----+10

	//�C���f�b�N�X
	unsigned long index01[] = { 7, 8, 9, 8, 10, 9 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index01[i];
	}

	//----------------------------------------------------------------
	//���� 1-2
	//----------------------------------------------------------------
	// 2 12+--+14
	//     |  |
	//     |  |
	// 1 11+--+13

	//�C���f�b�N�X
	unsigned long index12[] = { 11, 12, 13, 12, 14, 13 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index12[i];
	}

	//----------------------------------------------------------------
	//���� 2-4
	//----------------------------------------------------------------
	//   18+--+17
	//     |  |
	// 4 16+--+15 2

	//�C���f�b�N�X
	unsigned long index24[] = { 15, 16, 17, 16, 18, 17 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index24[i];
	}

	//----------------------------------------------------------------
	//���� 4-5
	//----------------------------------------------------------------
	// 5 20+--+22
	//     |  |
	//     |  |
	// 4 19+--+21

	//�C���f�b�N�X
	unsigned long index45[] = { 19, 20, 21, 20, 22, 21 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index45[i];
	}

	//----------------------------------------------------------------
	//���� 5-6
	//----------------------------------------------------------------
	//   26+--+25
	//     |  |
	// 6 24+--+23 5

	//�C���f�b�N�X
	unsigned long index56[] = { 23, 24, 25, 24, 26, 25 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index56[i];
	}

	//----------------------------------------------------------------
	//���� 6-0
	//----------------------------------------------------------------
	// 29+--+27 6
	//   |  |
	//   |  |
	// 30+--+28 0

	//�C���f�b�N�X
	unsigned long index60[] = { 27, 28, 29, 28, 30, 29 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index60[i];
	}

	//----------------------------------------------------------------
	//��̖�
	//----------------------------------------------------------------
	// 6+--+5
	//  |  |
	//  |  |
	//  |  |4
	// 3+--+--+2
	//  |     |   +z
	//  |     |    |
	//  |     |    |
	// 0+-----+1   +---> +x
	//     |
	//    posX

	//�C���f�b�N�X
	unsigned long indexUP[] = { 0, 2, 1, 0, 3, 2, 3, 5, 4, 3, 6, 5 };
	for (int i = 0; i < 12; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + indexUP[i];
	}

	return result;
}

//******************************************************************************
// �L�[�{�[�h���_�����F����B
//******************************************************************************
int MTPianoKeyboardMod::_CreateVertexOfKeyWhite2(
		unsigned char noteNo,
		MTPIANOKEYBOARD_VERTEX* pVertex,
		unsigned long* pIndex,
		D3DXCOLOR* pColor
	)
{
	//���N���X�̒��_�����������Ăяo��
	int result = MTPianoKeyboard::_CreateVertexOfKeyWhite2(
			noteNo,
			pVertex,
			pIndex,
			pColor
		);

	D3DXCOLOR keyColor;
	D3DXVECTOR2 tsc;

	//�����J���[
	if (pColor == NULL) {
		keyColor = m_KeyboardDesign.GetWhiteKeyColor();
	}
	else {
		keyColor = *pColor;
	}

	// �`�揇�̏C��
	int idx = 0;

	//----------------------------------------------------------------
	//���̖�
	//----------------------------------------------------------------
	//   42 6+-+5 41
	//       | |
	//       | |
	//   43 7| |4 40
	// 39 3+-+-+-+2 38
	//     |     |     +z
	//     |     |      |
	//     |     |      |
	// 36 0+-----+1 37  +---> +x
	//        |
	//       posX

	//�C���f�b�N�X
	unsigned long indexDW[] = { 36, 37, 38, 36, 38, 39, 43, 40, 41, 43, 41, 42 };
	for (int i = 0; i < 12; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + indexDW[i];
	}
	//----------------------------------------------------------------
	//���� 0-1
	//----------------------------------------------------------------
	//  0      1
	//  8+----+9
	//   |    |
	// 10+----+11

	//�C���f�b�N�X
	unsigned long index01[] = { 8, 9, 10, 9, 11, 10 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index01[i];
	}

	//----------------------------------------------------------------
	//���� 1-2
	//----------------------------------------------------------------
	// 2 13+--+15
	//     |  |
	//     |  |
	// 1 12+--+14

	//�C���f�b�N�X
	unsigned long index12[] = { 12, 13, 14, 13, 15, 14 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index12[i];
	}

	//----------------------------------------------------------------
	//���� 2-4
	//----------------------------------------------------------------
	//   19+--+18
	//     |  |
	// 4 17+--+16 2

	//���N���X�ł�2-3�Ő������Ă��邪�A���߂����4-7�Ԃ��d�؂�Ƃ���
	//�����Ă��܂����߁A2-4�A7-3�ɕ�������

	//���_
	pVertex[17].p = pVertex[4].p;
	pVertex[19].p = D3DXVECTOR3(pVertex[4].p.x, 0.0f, pVertex[4].p.z);  // 4'

	//�C���f�b�N�X
	unsigned long index24[] = { 16, 17, 18, 17, 19, 18 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index24[i];
	}

	//----------------------------------------------------------------
	//���� 4-5
	//----------------------------------------------------------------
	// 5 21+--+23
	//     |  |
	//     |  |
	// 4 20+--+22

	//�C���f�b�N�X
	unsigned long index45[] = { 20, 21, 22, 21, 23, 22 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index45[i];
	}

	//----------------------------------------------------------------
	//���� 5-6
	//----------------------------------------------------------------
	//   27+--+26
	//     |  |
	// 6 25+--+24 5

	//�C���f�b�N�X
	unsigned long index56[] = { 24, 25, 26, 25, 27, 26 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index56[i];
	}


	//----------------------------------------------------------------
	//���� 6-7
	//----------------------------------------------------------------
	// 30+--+28 6
	//   |  |
	//   |  |
	// 31+--+29 7

	//�C���f�b�N�X
	unsigned long index67[] = { 28, 29, 30, 29, 31, 30 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index67[i];
	}

	//----------------------------------------------------------------
	//���� 7-3
	//----------------------------------------------------------------
	//   47+--+46
	//     |  |
	// 3 45+--+44 7

	//���_
	pVertex[44].p = pVertex[7].p;
	pVertex[45].p = pVertex[3].p;
	pVertex[46].p = D3DXVECTOR3(pVertex[7].p.x, 0.0f, pVertex[7].p.z);  // 7'
	pVertex[47].p = D3DXVECTOR3(pVertex[3].p.x, 0.0f, pVertex[3].p.z);  // 3'

	//�@���^�F
	for (int i = 44; i < 48; i++) {
		pVertex[i].n = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
		pVertex[i].c = keyColor;
	}

	//�C���f�b�N�X
	unsigned long index73[] = { 44, 45, 46, 45, 47, 46 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index73[i];
	}

	//�P��F�̃e�N�X�`�����W
	m_KeyboardDesign.GetWhiteKeyTexturePosSingleColor(noteNo, &tsc);
	for (int i = 44; i < 48; i++) {
		pVertex[i].t = tsc;
	}

	//----------------------------------------------------------------
	//���� 3-0
	//----------------------------------------------------------------
	// 34+--+32 3
	//   |  |
	//   |  |
	// 35+--+33 0

	//�C���f�b�N�X
	unsigned long index30[] = { 32, 33, 34, 33, 35, 34 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index30[i];
	}

	//----------------------------------------------------------------
	//��̖�
	//----------------------------------------------------------------
	//   6+-+5
	//    | |
	//    | |
	//   7| |4
	// 3+-+-+-+2
	//  |     |   +z
	//  |     |    |
	//  |     |    |
	// 0+-----+1   +---> +x
	//     |
	//    posX

	//�C���f�b�N�X
	unsigned long indexUP[] = { 0, 2, 1, 0, 3, 2, 7, 5, 4, 7, 6, 5 };
	for (int i = 0; i < 12; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + indexUP[i];
	}

	return result;
}

//******************************************************************************
// �L�[�{�[�h���_�����F����C
//******************************************************************************
int MTPianoKeyboardMod::_CreateVertexOfKeyWhite3(
		unsigned char noteNo,
		MTPIANOKEYBOARD_VERTEX* pVertex,
		unsigned long* pIndex,
		D3DXCOLOR* pColor
	)
{
	//���N���X�̒��_�����������Ăяo��
	int result = MTPianoKeyboard::_CreateVertexOfKeyWhite3(
			noteNo,
			pVertex,
			pIndex,
			pColor
		);

	// �`�揇�̏C��
	int idx = 0;

	//----------------------------------------------------------------
	//���̖�
	//----------------------------------------------------------------
	//    36 5+--+4 35
	//        |  |
	//        |  |
	//    37 6|  |
	// 34 3+--+--+2 33
	//     |     |     +z
	//     |     |      |
	//     |     |      |
	// 31 0+-----+1 32  +---> +x
	//        |
	//       posX

	//�C���f�b�N�X
	unsigned long indexDW[] = { 31, 32, 33, 31, 33, 34, 33, 35, 37, 37, 35, 36 };
	for (int i = 0; i < 12; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + indexDW[i];
	}

	//----------------------------------------------------------------
	//���� 0-1
	//----------------------------------------------------------------
	// 0      1
	// 7+----+8
	//  |    |
	// 9+----+10

	//�C���f�b�N�X
	unsigned long index01[] = { 7, 8, 9, 8, 10, 9 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index01[i];
	}

	//----------------------------------------------------------------
	//���� 1-4
	//----------------------------------------------------------------
	// 4 12+--+14
	//     |  |
	//     |  |
	// 1 11+--+13

	//�C���f�b�N�X
	unsigned long index14[] = { 11, 12, 13, 12, 14, 13 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index14[i];
	}

	//----------------------------------------------------------------
	//���� 4-5
	//----------------------------------------------------------------
	//   18+--+17
	//     |  |
	// 5 16+--+15 4

	//�C���f�b�N�X
	unsigned long index45[] = { 15, 16, 17, 16, 18, 17 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index45[i];
	}

	//----------------------------------------------------------------
	//���� 5-6
	//----------------------------------------------------------------
	// 21+--+19 5
	//   |  |
	//   |  |
	// 22+--+20 6

	//�C���f�b�N�X
	unsigned long index56[] = { 19, 20, 21, 20, 22, 21 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index56[i];
	}

	//----------------------------------------------------------------
	//���� 6-3
	//----------------------------------------------------------------
	//   26+--+25
	//     |  |
	// 3 24+--+23 6

	//�C���f�b�N�X
	unsigned long index63[] = { 23, 24, 25, 24, 26, 25 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index63[i];
	}

	//----------------------------------------------------------------
	//���� 3-0
	//----------------------------------------------------------------
	// 29+--+27 3
	//   |  |
	//   |  |
	// 30+--+28 0

	//�C���f�b�N�X
	unsigned long index30[] = { 27, 28, 29, 28, 30, 29 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index30[i];
	}

	//----------------------------------------------------------------
	//��̖�
	//----------------------------------------------------------------
	//    5+--+4
	//     |  |
	//     |  |
	//    6|  |
	// 3+--+--+2
	//  |     |   +z
	//  |     |    |
	//  |     |    |
	// 0+-----+1   +---> +x
	//     |
	//    posX

	//�C���f�b�N�X
	unsigned long indexUP[] = { 0, 2, 1, 0, 3, 2, 2, 6, 4, 6, 5, 4 };
	for (int i = 0; i < 12; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + indexUP[i];
	}

	return result;
}

//******************************************************************************
// �L�[�{�[�h���_�����F����
//******************************************************************************
int MTPianoKeyboardMod::_CreateVertexOfKeyBlack(
		unsigned char noteNo,
		MTPIANOKEYBOARD_VERTEX* pVertex,
		unsigned long* pIndex,
		D3DXCOLOR* pColor
	)
{
	//���N���X�̒��_�����������Ăяo��
	int result = MTPianoKeyboard::_CreateVertexOfKeyBlack(
			noteNo,
			pVertex,
			pIndex,
			pColor
		);

	// �`�揇�̏C��
	int idx = 0;

	//----------------------------------------------------------------
	//���̖�
	//----------------------------------------------------------------
	//   29 6+-+5 28
	//       | |
	//       | |
	//       | |
	//   26 0+-+1 27
	//        |      +z
	//        |       |
	//        |       |
	//      --+--     +---> +x
	//        |
	//       posX

	//�C���f�b�N�X
	unsigned long indexDW[] = { 26, 27, 28, 26, 28, 29 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + indexDW[i];
	}

	//----------------------------------------------------------------
	//���� 0-1
	//----------------------------------------------------------------
	//  0      1
	//  8+----+9
	//   |    |
	// 10+----+11

	//�C���f�b�N�X
	unsigned long index01[] = { 8, 9, 10, 9, 11, 10 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index01[i];
	}

	//----------------------------------------------------------------
	//���� 1-2-5
	//----------------------------------------------------------------
	// 5 14+--+16
	//     |  |
	//     |  |
	// 2 13+  |
	//      \ |
	// 1 12 +-+15

	//�C���f�b�N�X
	unsigned long index125[] = { 12, 13, 15, 13, 16, 15, 13, 14, 16 };
	for (int i = 0; i < 9; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index125[i];
	}

	//----------------------------------------------------------------
	//���� 5-6
	//----------------------------------------------------------------
	//   20+--+19
	//     |  |
	// 6 18+--+17 5

	//�C���f�b�N�X
	unsigned long index56[] = { 17, 18, 19, 18, 20, 19 };
	for (int i = 0; i < 6; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index56[i];
	}

	//----------------------------------------------------------------
	//���� 6-3-0
	//----------------------------------------------------------------
	// 24+--+21 6
	//   |  |
	//   |  |
	//   |  +22 3
	//   | /
	// 25+-+23  0

	//�C���f�b�N�X
	unsigned long index630[] = { 21, 22, 24, 22, 25, 24, 22, 23, 25 };
	for (int i = 0; i < 9; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + index630[i];
	}

	//----------------------------------------------------------------
	//��̖�
	//----------------------------------------------------------------
	//   6+-+5
	//    | |
	//    | |
	// 7 3+-+2 4
	//   0+-+1
	//     |   +z
	//     |    |
	//     |    |
	//   --+--  +---> +x
	//     |
	//    posX

	//�C���f�b�N�X
	unsigned long indexUP[] = { 0, 2, 1, 0, 3, 2, 4, 7, 5, 7, 6, 5 };
	for (int i = 0; i < 12; i++) {
		pIndex[idx++] = m_BufInfo[noteNo].vertexPos + indexUP[i];
	}

	return result;
}

