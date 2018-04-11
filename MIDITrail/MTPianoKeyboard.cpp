//******************************************************************************
//
// MIDITrail / MTPianoKeyboard
//
// �s�A�m�L�[�{�[�h�`��N���X
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "shlwapi.h"
#include "YNBaseLib.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTPianoKeyboard.h"

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
MTPianoKeyboard::MTPianoKeyboard(void)
{
	m_pTexture = NULL;
	ZeroMemory(&m_ImgInfo, sizeof(D3DXIMAGE_INFO));
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTPianoKeyboard::~MTPianoKeyboard(void)
{
	Release();
}

//******************************************************************************
// ��������
//******************************************************************************
int MTPianoKeyboard::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData,
		LPDIRECT3DTEXTURE9 pTexture
	)
{
	int result = 0;
	SMTrack track;

	Release();

	if (pD3DDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//�L�[�{�[�h�f�U�C��������
	result = m_KeyboardDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	//�e�N�X�`���ǂݍ���
	if (pTexture == NULL) {
		result = _LoadTexture(pD3DDevice, pSceneName);
		if (result != 0) goto EXIT;
	}
	else {
		m_pTexture = pTexture;
	}

	//�L�[�{�[�h����
	result = _CreateKeyboard(pD3DDevice);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �L�[�{�[�h����
//******************************************************************************
int MTPianoKeyboard::_CreateKeyboard(
		LPDIRECT3DDEVICE9 pD3DDevice
   )
{
	int result = 0;

	//�o�b�t�@��񐶐�
	_CreateBufInfo();

	//�L�[�{�[�h���_����
	result = _CreateVertexOfKeyboard(pD3DDevice);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �o�b�t�@��񐶐�
//******************************************************************************
void MTPianoKeyboard::_CreateBufInfo()
{
	unsigned char noteNo = 0;
	unsigned long vertexNum = 0;
	unsigned long indexNum = 0;
	unsigned long vertexPos = 0;
	unsigned long indexPos = 0;

	ZeroMemory(&(m_BufInfo[0]), sizeof(MTBufInfo) * SM_MAX_NOTE_NUM);

	vertexPos = 0;
	indexPos = 0;

	//�e�L�[�̒��_���^�C���f�b�N�X���^�o�^�ʒu�𐶐�����
	for (noteNo = 0; noteNo < SM_MAX_NOTE_NUM; noteNo++) {
		switch (m_KeyboardDesign.GetKeyType(noteNo)) {
			case (MTPianoKeyboardDesign::KeyWhiteC):
			case (MTPianoKeyboardDesign::KeyWhiteF):
				vertexNum = MTPIANOKEYBOARD_KEY_WHITE_1_VERTEX_NUM;
				indexNum  = MTPIANOKEYBOARD_KEY_WHITE_1_INDEX_NUM;
				break;
			case (MTPianoKeyboardDesign::KeyWhiteD):
			case (MTPianoKeyboardDesign::KeyWhiteG):
			case (MTPianoKeyboardDesign::KeyWhiteA):
				vertexNum = MTPIANOKEYBOARD_KEY_WHITE_2_VERTEX_NUM;
				indexNum  = MTPIANOKEYBOARD_KEY_WHITE_2_INDEX_NUM;
				break;
			case (MTPianoKeyboardDesign::KeyWhiteE):
			case (MTPianoKeyboardDesign::KeyWhiteB):
				vertexNum = MTPIANOKEYBOARD_KEY_WHITE_3_VERTEX_NUM;
				indexNum  = MTPIANOKEYBOARD_KEY_WHITE_3_INDEX_NUM;
				break;
			case (MTPianoKeyboardDesign::KeyBlack):
				vertexNum = MTPIANOKEYBOARD_KEY_BLACK_VERTEX_NUM;
				indexNum  = MTPIANOKEYBOARD_KEY_BLACK_INDEX_NUM;
				break;
		}
		m_BufInfo[noteNo].vertexNum = vertexNum;
		m_BufInfo[noteNo].indexNum  = indexNum;
		m_BufInfo[noteNo].vertexPos = vertexPos;
		m_BufInfo[noteNo].indexPos  = indexPos;
		vertexPos += vertexNum;
		indexPos  += indexNum;
	}

	return;
}

//******************************************************************************
// �L�[�{�[�h���_����
//******************************************************************************
int MTPianoKeyboard::_CreateVertexOfKeyboard(
		LPDIRECT3DDEVICE9 pD3DDevice
   )
{
	int result = 0;
	unsigned long vertexNum = 0;
	unsigned long indexNum = 0;
	unsigned char noteNo = 0;
	D3DMATERIAL9 material;

	//�v���~�e�B�u������
	result = m_PrimitiveKeyboard.Initialize(
					sizeof(MTPIANOKEYBOARD_VERTEX),	//���_�T�C�Y
					_GetFVFFormat(),				//���_FVF�t�H�[�}�b�g
					D3DPT_TRIANGLELIST				//�v���~�e�B�u���
				);
	if (result != 0) goto EXIT;

	//���_�ƃC���f�b�N�X�̑���
	vertexNum = 0;
	indexNum = 0;
	for (noteNo = 0; noteNo < SM_MAX_NOTE_NUM; noteNo++) {
		vertexNum += m_BufInfo[noteNo].vertexNum;
		indexNum  += m_BufInfo[noteNo].indexNum;
	}

	//���_�o�b�t�@����
	result = m_PrimitiveKeyboard.CreateVertexBuffer(pD3DDevice, vertexNum);
	if (result != 0) goto EXIT;

	//�C���f�b�N�X�o�b�t�@����
	result = m_PrimitiveKeyboard.CreateIndexBuffer(pD3DDevice, indexNum);
	if (result != 0) goto EXIT;

	//�o�b�t�@�ɒ��_�ƃC���f�b�N�X����������
	for (noteNo = 0; noteNo < SM_MAX_NOTE_NUM; noteNo++) {
		result = _CreateVertexOfKey(noteNo);
		if (result != 0) goto EXIT;
	}

	//�}�e���A���쐬
	_MakeMaterial(&material);
	m_PrimitiveKeyboard.SetMaterial(material);

EXIT:;
	return result;
}

//******************************************************************************
// �L�[�{�[�h���_����
//******************************************************************************
int MTPianoKeyboard::_CreateVertexOfKey(
		unsigned char noteNo
   )
{
	int result = 0;
	MTPIANOKEYBOARD_VERTEX* pVertex = NULL;
	unsigned long* pIndex = NULL;
	unsigned long offset = 0;
	unsigned long size = 0;

	if (noteNo >= SM_MAX_NOTE_NUM) {
		result = YN_SET_ERR("Program error.", noteNo, 0);
		goto EXIT;
	}

	//���_�o�b�t�@�̃��b�N
	offset = m_BufInfo[noteNo].vertexPos * sizeof(MTPIANOKEYBOARD_VERTEX);
	size   = m_BufInfo[noteNo].vertexNum * sizeof(MTPIANOKEYBOARD_VERTEX);
	result = m_PrimitiveKeyboard.LockVertex((void**)&pVertex, offset, size);
	if (result != 0) goto EXIT;

	ZeroMemory(pVertex, size);

	//�C���f�b�N�X�o�b�t�@�̃��b�N
	offset = m_BufInfo[noteNo].indexPos * sizeof(unsigned long);
	size   = m_BufInfo[noteNo].indexNum * sizeof(unsigned long);
	result = m_PrimitiveKeyboard.LockIndex(&pIndex, offset, size);
	if (result != 0) goto EXIT;

	ZeroMemory(pIndex, size);

	//���_����
	switch (m_KeyboardDesign.GetKeyType(noteNo)) {
		case (MTPianoKeyboardDesign::KeyWhiteC):
		case (MTPianoKeyboardDesign::KeyWhiteF):
			result = _CreateVertexOfKeyWhite1(noteNo, pVertex, pIndex);
			if (result != 0) goto EXIT;
			break;
		case (MTPianoKeyboardDesign::KeyWhiteD):
		case (MTPianoKeyboardDesign::KeyWhiteG):
		case (MTPianoKeyboardDesign::KeyWhiteA):
			result = _CreateVertexOfKeyWhite2(noteNo, pVertex, pIndex);
			if (result != 0) goto EXIT;
			break;
		case (MTPianoKeyboardDesign::KeyWhiteE):
		case (MTPianoKeyboardDesign::KeyWhiteB):
			result = _CreateVertexOfKeyWhite3(noteNo, pVertex, pIndex);
			if (result != 0) goto EXIT;
			break;
		case (MTPianoKeyboardDesign::KeyBlack):
			result = _CreateVertexOfKeyBlack(noteNo, pVertex, pIndex);
			if (result != 0) goto EXIT;
			break;
	}

	//���_�o�b�t�@�̃��b�N����
	result = m_PrimitiveKeyboard.UnlockVertex();
	if (result != 0) goto EXIT;

	//�C���f�b�N�X�o�b�t�@�̃��b�N����
	result = m_PrimitiveKeyboard.UnlockIndex();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �L�[�{�[�h���_�����F����A
//******************************************************************************
int MTPianoKeyboard::_CreateVertexOfKeyWhite1(
		unsigned char noteNo,
		MTPIANOKEYBOARD_VERTEX* pVertex,
		unsigned long* pIndex,
		D3DXCOLOR* pColor
	)
{
	int result = 0;
	unsigned long i = 0;
	float centerX       = m_KeyboardDesign.GetKeyCenterPosX(noteNo);
	float heightY       = m_KeyboardDesign.GetWhiteKeyHeight();
	float whiteKeyWidth = m_KeyboardDesign.GetWhiteKeyWidth();
	float whiteKeyLen   = m_KeyboardDesign.GetWhiteKeyLen();
	float blackKeyWidth = m_KeyboardDesign.GetBlackKeyWidth();
	float blackKeyLen   = m_KeyboardDesign.GetBlackKeyLen();
	float deltaKeyLen   = whiteKeyLen - blackKeyLen;
	float spc           = m_KeyboardDesign.GetKeySpaceSize();
	float nextCenterX   = m_KeyboardDesign.GetKeyCenterPosX(noteNo+1);
	D3DXCOLOR keyColor;
	D3DXVECTOR2 t0, t1, t2, t3, t4, t5, t6, t7, tsc;

	//�����J���[
	if (pColor == NULL) {
		keyColor = m_KeyboardDesign.GetWhiteKeyColor();
	}
	else {
		keyColor = *pColor;
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

	//���_
	pVertex[0].p = D3DXVECTOR3(centerX - (whiteKeyWidth/2.0f),           heightY, 0.0f);
	pVertex[1].p = D3DXVECTOR3(centerX + (whiteKeyWidth/2.0f),           heightY, 0.0f);
	pVertex[2].p = D3DXVECTOR3(centerX + (whiteKeyWidth/2.0f),           heightY, deltaKeyLen - spc);
	pVertex[3].p = D3DXVECTOR3(centerX - (whiteKeyWidth/2.0f),           heightY, deltaKeyLen - spc);
	pVertex[4].p = D3DXVECTOR3(nextCenterX - (blackKeyWidth/2.0f) - spc, heightY, deltaKeyLen - spc);
	pVertex[5].p = D3DXVECTOR3(nextCenterX - (blackKeyWidth/2.0f) - spc, heightY, whiteKeyLen);
	pVertex[6].p = D3DXVECTOR3(centerX - (whiteKeyWidth/2.0f),           heightY, whiteKeyLen);

	//�@���^�F
	for (i = 0; i < 7; i++) {
		pVertex[i].n = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		pVertex[i].c = keyColor;
	}

	//�C���f�b�N�X
	unsigned long indexUP[] = { 0, 2, 1, 0, 3, 2, 3, 5, 4, 3, 6, 5 };
	for (i = 0; i < 12; i++) {
		pIndex[i] = m_BufInfo[noteNo].vertexPos + indexUP[i];
	}

	//�e���_�̃e�N�X�`�����W
	m_KeyboardDesign.GetWhiteKeyTexturePosTop(noteNo, &t0, &t1, &t2, &t3, &t4, &t5, &t6, &t7);
	pVertex[0].t = t0;
	pVertex[1].t = t1;
	pVertex[2].t = t2;
	pVertex[3].t = t3;
	pVertex[4].t = t4;
	pVertex[5].t = t5;
	pVertex[6].t = t6;

	//----------------------------------------------------------------
	//���� 0-1
	//----------------------------------------------------------------
	// 0      1
	// 7+----+8
	//  |    |
	// 9+----+10

	//���_
	pVertex[7].p  = pVertex[0].p;
	pVertex[8].p  = pVertex[1].p;
	pVertex[9].p  = D3DXVECTOR3(pVertex[0].p.x, 0.0f, pVertex[0].p.z); // 0'
	pVertex[10].p = D3DXVECTOR3(pVertex[1].p.x, 0.0f, pVertex[1].p.z); // 1'

	//�@���^�F
	for (i = 7; i < 11; i++) {
		pVertex[i].n = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
		pVertex[i].c = keyColor;
	}

	//�C���f�b�N�X
	unsigned long index01[] = { 7, 8, 9, 8, 10, 9 };
	for (i = 0; i < 6; i++) {
		pIndex[12 + i] = m_BufInfo[noteNo].vertexPos + index01[i];
	}

	//�e���_�̃e�N�X�`�����W
	m_KeyboardDesign.GetWhiteKeyTexturePosFront(noteNo, &t0, &t1, &t2, &t3);
	pVertex[7].t  = t0;
	pVertex[8].t  = t1;
	pVertex[9].t  = t2;
	pVertex[10].t = t3;

	//----------------------------------------------------------------
	//���� 1-2
	//----------------------------------------------------------------
	// 2 12+--+14
	//     |  |
	//     |  |
	// 1 11+--+13

	//���_
	pVertex[11].p = pVertex[1].p;
	pVertex[12].p = pVertex[2].p;
	pVertex[13].p = D3DXVECTOR3(pVertex[1].p.x, 0.0f, pVertex[1].p.z);  // 1'
	pVertex[14].p = D3DXVECTOR3(pVertex[2].p.x, 0.0f, pVertex[2].p.z);  // 2'

	//�@���^�F
	for (i = 11; i < 15; i++) {
		pVertex[i].n = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
		pVertex[i].c = keyColor;
	}

	//�C���f�b�N�X
	unsigned long index12[] = { 11, 12, 13, 12, 14, 13 };
	for (i = 0; i < 6; i++) {
		pIndex[18 + i] = m_BufInfo[noteNo].vertexPos + index12[i];
	}

	//----------------------------------------------------------------
	//���� 2-4
	//----------------------------------------------------------------
	//   18+--+17
	//     |  |
	// 4 16+--+15 2

	//���_
	pVertex[15].p = pVertex[2].p;
	pVertex[16].p = pVertex[4].p;
	pVertex[17].p = D3DXVECTOR3(pVertex[2].p.x, 0.0f, pVertex[2].p.z);  // 2'
	pVertex[18].p = D3DXVECTOR3(pVertex[4].p.x, 0.0f, pVertex[4].p.z);  // 4'

	//�@���^�F
	for (i = 15; i < 19; i++) {
		pVertex[i].n = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
		pVertex[i].c = keyColor;
	}

	//�C���f�b�N�X
	unsigned long index24[] = { 15, 16, 17, 16, 18, 17 };
	for (i = 0; i < 6; i++) {
		pIndex[24 + i] = m_BufInfo[noteNo].vertexPos + index24[i];
	}

	//----------------------------------------------------------------
	//���� 4-5
	//----------------------------------------------------------------
	// 5 20+--+22
	//     |  |
	//     |  |
	// 4 19+--+21

	//���_
	pVertex[19].p = pVertex[4].p;
	pVertex[20].p = pVertex[5].p;
	pVertex[21].p = D3DXVECTOR3(pVertex[4].p.x, 0.0f, pVertex[4].p.z);  // 4'
	pVertex[22].p = D3DXVECTOR3(pVertex[5].p.x, 0.0f, pVertex[5].p.z);  // 5'

	//�@���^�F
	for (i = 19; i < 23; i++) {
		pVertex[i].n = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
		pVertex[i].c = keyColor;
	}

	//�C���f�b�N�X
	unsigned long index45[] = { 19, 20, 21, 20, 22, 21 };
	for (i = 0; i < 6; i++) {
		pIndex[30 + i] = m_BufInfo[noteNo].vertexPos + index45[i];
	}

	//----------------------------------------------------------------
	//���� 5-6
	//----------------------------------------------------------------
	//   26+--+25
	//     |  |
	// 6 24+--+23 5

	//���_
	pVertex[23].p = pVertex[5].p;
	pVertex[24].p = pVertex[6].p;
	pVertex[25].p = D3DXVECTOR3(pVertex[5].p.x, 0.0f, pVertex[5].p.z);  // 5'
	pVertex[26].p = D3DXVECTOR3(pVertex[6].p.x, 0.0f, pVertex[6].p.z);  // 6'

	//�@���^�F
	for (i = 23; i < 27; i++) {
		pVertex[i].n = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
		pVertex[i].c = keyColor;
	}

	//�C���f�b�N�X
	unsigned long index56[] = { 23, 24, 25, 24, 26, 25 };
	for (i = 0; i < 6; i++) {
		pIndex[36 + i] = m_BufInfo[noteNo].vertexPos + index56[i];
	}

	//----------------------------------------------------------------
	//���� 6-0
	//----------------------------------------------------------------
	// 29+--+27 6
	//   |  |
	//   |  |
	// 30+--+28 0

	//���_
	pVertex[27].p = pVertex[6].p;
	pVertex[28].p = pVertex[0].p;
	pVertex[29].p = D3DXVECTOR3(pVertex[6].p.x, 0.0f, pVertex[6].p.z);  // 6'
	pVertex[30].p = D3DXVECTOR3(pVertex[0].p.x, 0.0f, pVertex[0].p.z);  // 0'

	//�@���^�F
	for (i = 27; i < 31; i++) {
		pVertex[i].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
		pVertex[i].c = keyColor;
	}

	//�C���f�b�N�X
	unsigned long index60[] = { 27, 28, 29, 28, 30, 29 };
	for (i = 0; i < 6; i++) {
		pIndex[42 + i] = m_BufInfo[noteNo].vertexPos + index60[i];
	}

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

	//���_
	pVertex[31].p = D3DXVECTOR3(pVertex[0].p.x, 0.0f, pVertex[0].p.z);  // 0'
	pVertex[32].p = D3DXVECTOR3(pVertex[1].p.x, 0.0f, pVertex[1].p.z);  // 1'
	pVertex[33].p = D3DXVECTOR3(pVertex[2].p.x, 0.0f, pVertex[2].p.z);  // 2'
	pVertex[34].p = D3DXVECTOR3(pVertex[3].p.x, 0.0f, pVertex[3].p.z);  // 3'
	pVertex[35].p = D3DXVECTOR3(pVertex[4].p.x, 0.0f, pVertex[4].p.z);  // 4'
	pVertex[36].p = D3DXVECTOR3(pVertex[5].p.x, 0.0f, pVertex[5].p.z);  // 5'
	pVertex[37].p = D3DXVECTOR3(pVertex[6].p.x, 0.0f, pVertex[6].p.z);  // 6'

	//�@���^�F
	for (i = 31; i < 38; i++) {
		pVertex[i].n = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
		pVertex[i].c = keyColor;
	}

	//�C���f�b�N�X
	unsigned long indexDW[] = { 31, 32, 33, 31, 33, 34, 34, 35, 36, 34, 36, 37 };
	for (i = 0; i < 12; i++) {
		pIndex[48 + i] = m_BufInfo[noteNo].vertexPos + indexDW[i];
	}

	//----------------------------------------------------------------
	//�P��F�̃e�N�X�`�����W
	//----------------------------------------------------------------
	m_KeyboardDesign.GetWhiteKeyTexturePosSingleColor(noteNo, &tsc);
	for (i = 11; i < 38; i++) {
		pVertex[i].t = tsc;
	}

	return result;
}

//******************************************************************************
// �L�[�{�[�h���_�����F����B
//******************************************************************************
int MTPianoKeyboard::_CreateVertexOfKeyWhite2(
		unsigned char noteNo,
		MTPIANOKEYBOARD_VERTEX* pVertex,
		unsigned long* pIndex,
		D3DXCOLOR* pColor
	)
{
	int result = 0;
	unsigned long i = 0;
	float centerX       = m_KeyboardDesign.GetKeyCenterPosX(noteNo);
	float heightY       = m_KeyboardDesign.GetWhiteKeyHeight();
	float whiteKeyWidth = m_KeyboardDesign.GetWhiteKeyWidth();
	float whiteKeyLen   = m_KeyboardDesign.GetWhiteKeyLen();
	float blackKeyWidth = m_KeyboardDesign.GetBlackKeyWidth();
	float blackKeyLen   = m_KeyboardDesign.GetBlackKeyLen();
	float deltaKeyLen   = whiteKeyLen - blackKeyLen;
	float spc           = m_KeyboardDesign.GetKeySpaceSize();
	float prevCenterX   = m_KeyboardDesign.GetKeyCenterPosX(noteNo-1);
	float nextCenterX   = m_KeyboardDesign.GetKeyCenterPosX(noteNo+1);
	D3DXCOLOR keyColor;
	D3DXVECTOR2 t0, t1, t2, t3, t4, t5, t6, t7, tsc;

	//�����J���[
	if (pColor == NULL) {
		keyColor = m_KeyboardDesign.GetWhiteKeyColor();
	}
	else {
		keyColor = *pColor;
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

	//���_
	pVertex[0].p = D3DXVECTOR3(centerX - (whiteKeyWidth/2.0f),           heightY, 0.0f);
	pVertex[1].p = D3DXVECTOR3(centerX + (whiteKeyWidth/2.0f),           heightY, 0.0f);
	pVertex[2].p = D3DXVECTOR3(centerX + (whiteKeyWidth/2.0f),           heightY, deltaKeyLen - spc);
	pVertex[3].p = D3DXVECTOR3(centerX - (whiteKeyWidth/2.0f),           heightY, deltaKeyLen - spc);
	pVertex[4].p = D3DXVECTOR3(nextCenterX - (blackKeyWidth/2.0f) - spc, heightY, deltaKeyLen - spc);
	pVertex[5].p = D3DXVECTOR3(nextCenterX - (blackKeyWidth/2.0f) - spc, heightY, whiteKeyLen);
	pVertex[6].p = D3DXVECTOR3(prevCenterX + (blackKeyWidth/2.0f) + spc, heightY, whiteKeyLen);
	pVertex[7].p = D3DXVECTOR3(prevCenterX + (blackKeyWidth/2.0f) + spc, heightY, deltaKeyLen - spc);

	//�@���^�F
	for (i = 0; i < 8; i++) {
		pVertex[i].n = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		pVertex[i].c = keyColor;
	}

	//�C���f�b�N�X
	unsigned long indexUP[] = { 0, 2, 1, 0, 3, 2, 7, 5, 4, 7, 6, 5 };
	for (i = 0; i < 12; i++) {
		pIndex[i] = m_BufInfo[noteNo].vertexPos + indexUP[i];
	}

	//�e���_�̃e�N�X�`�����W
	m_KeyboardDesign.GetWhiteKeyTexturePosTop(noteNo, &t0, &t1, &t2, &t3, &t4, &t5, &t6, &t7);
	pVertex[0].t = t0;
	pVertex[1].t = t1;
	pVertex[2].t = t2;
	pVertex[3].t = t3;
	pVertex[4].t = t4;
	pVertex[5].t = t5;
	pVertex[6].t = t6;
	pVertex[7].t = t7;

	//----------------------------------------------------------------
	//���� 0-1
	//----------------------------------------------------------------
	//  0      1
	//  8+----+9
	//   |    |
	// 10+----+11

	//���_
	pVertex[8].p  = pVertex[0].p;
	pVertex[9].p  = pVertex[1].p;
	pVertex[10].p = D3DXVECTOR3(pVertex[0].p.x, 0.0f, pVertex[0].p.z); // 0'
	pVertex[11].p = D3DXVECTOR3(pVertex[1].p.x, 0.0f, pVertex[1].p.z); // 1'

	//�@���^�F
	for (i = 8; i < 12; i++) {
		pVertex[i].n = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
		pVertex[i].c = keyColor;
	}

	//�C���f�b�N�X
	unsigned long index01[] = { 8, 9, 10, 9, 11, 10 };
	for (i = 0; i < 6; i++) {
		pIndex[12 + i] = m_BufInfo[noteNo].vertexPos + index01[i];
	}

	//�e���_�̃e�N�X�`�����W
	m_KeyboardDesign.GetWhiteKeyTexturePosFront(noteNo, &t0, &t1, &t2, &t3);
	pVertex[8].t  = t0;
	pVertex[9].t  = t1;
	pVertex[10].t = t2;
	pVertex[11].t = t3;

	//----------------------------------------------------------------
	//���� 1-2
	//----------------------------------------------------------------
	// 2 13+--+15
	//     |  |
	//     |  |
	// 1 12+--+14

	//���_
	pVertex[12].p = pVertex[1].p;
	pVertex[13].p = pVertex[2].p;
	pVertex[14].p = D3DXVECTOR3(pVertex[1].p.x, 0.0f, pVertex[1].p.z);  // 1'
	pVertex[15].p = D3DXVECTOR3(pVertex[2].p.x, 0.0f, pVertex[2].p.z);  // 2'

	//�@���^�F
	for (i = 12; i < 16; i++) {
		pVertex[i].n = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
		pVertex[i].c = keyColor;
	}

	//�C���f�b�N�X
	unsigned long index12[] = { 12, 13, 14, 13, 15, 14 };
	for (i = 0; i < 6; i++) {
		pIndex[18 + i] = m_BufInfo[noteNo].vertexPos + index12[i];
	}

	//----------------------------------------------------------------
	//���� 2-3
	//----------------------------------------------------------------
	//   19+--+18
	//     |  |
	// 3 17+--+16 2

	//���_
	pVertex[16].p = pVertex[2].p;
	pVertex[17].p = pVertex[3].p;
	pVertex[18].p = D3DXVECTOR3(pVertex[2].p.x, 0.0f, pVertex[2].p.z);  // 2'
	pVertex[19].p = D3DXVECTOR3(pVertex[3].p.x, 0.0f, pVertex[3].p.z);  // 3'

	//�@���^�F
	for (i = 16; i < 20; i++) {
		pVertex[i].n = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
		pVertex[i].c = keyColor;
	}

	//�C���f�b�N�X
	unsigned long index24[] = { 16, 17, 18, 17, 19, 18 };
	for (i = 0; i < 6; i++) {
		pIndex[24 + i] = m_BufInfo[noteNo].vertexPos + index24[i];
	}

	//----------------------------------------------------------------
	//���� 4-5
	//----------------------------------------------------------------
	// 5 21+--+23
	//     |  |
	//     |  |
	// 4 20+--+22

	//���_
	pVertex[20].p = pVertex[4].p;
	pVertex[21].p = pVertex[5].p;
	pVertex[22].p = D3DXVECTOR3(pVertex[4].p.x, 0.0f, pVertex[4].p.z);  // 4'
	pVertex[23].p = D3DXVECTOR3(pVertex[5].p.x, 0.0f, pVertex[5].p.z);  // 5'

	//�@���^�F
	for (i = 20; i < 24; i++) {
		pVertex[i].n = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
		pVertex[i].c = keyColor;
	}

	//�C���f�b�N�X
	unsigned long index45[] = { 20, 21, 22, 21, 23, 22 };
	for (i = 0; i < 6; i++) {
		pIndex[30 + i] = m_BufInfo[noteNo].vertexPos + index45[i];
	}

	//----------------------------------------------------------------
	//���� 5-6
	//----------------------------------------------------------------
	//   27+--+26
	//     |  |
	// 6 25+--+24 5

	//���_
	pVertex[24].p = pVertex[5].p;
	pVertex[25].p = pVertex[6].p;
	pVertex[26].p = D3DXVECTOR3(pVertex[5].p.x, 0.0f, pVertex[5].p.z);  // 5'
	pVertex[27].p = D3DXVECTOR3(pVertex[6].p.x, 0.0f, pVertex[6].p.z);  // 6'

	//�@���^�F
	for (i = 24; i < 28; i++) {
		pVertex[i].n = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
		pVertex[i].c = keyColor;
	}

	//�C���f�b�N�X
	unsigned long index56[] = { 24, 25, 26, 25, 27, 26 };
	for (i = 0; i < 6; i++) {
		pIndex[36 + i] = m_BufInfo[noteNo].vertexPos + index56[i];
	}

	//----------------------------------------------------------------
	//���� 6-7
	//----------------------------------------------------------------
	// 30+--+28 6
	//   |  |
	//   |  |
	// 31+--+29 7

	//���_
	pVertex[28].p = pVertex[6].p;
	pVertex[29].p = pVertex[7].p;
	pVertex[30].p = D3DXVECTOR3(pVertex[6].p.x, 0.0f, pVertex[6].p.z);  // 6'
	pVertex[31].p = D3DXVECTOR3(pVertex[7].p.x, 0.0f, pVertex[7].p.z);  // 7'

	//�@���^�F
	for (i = 28; i < 32; i++) {
		pVertex[i].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
		pVertex[i].c = keyColor;
	}

	//�C���f�b�N�X
	unsigned long index67[] = { 28, 29, 30, 29, 31, 30 };
	for (i = 0; i < 6; i++) {
		pIndex[42 + i] = m_BufInfo[noteNo].vertexPos + index67[i];
	}

	//----------------------------------------------------------------
	//���� 3-0
	//----------------------------------------------------------------
	// 34+--+32 3
	//   |  |
	//   |  |
	// 35+--+33 0

	//���_
	pVertex[32].p = pVertex[3].p;
	pVertex[33].p = pVertex[0].p;
	pVertex[34].p = D3DXVECTOR3(pVertex[3].p.x, 0.0f, pVertex[3].p.z);  // 3'
	pVertex[35].p = D3DXVECTOR3(pVertex[0].p.x, 0.0f, pVertex[0].p.z);  // 0'

	//�@���^�F
	for (i = 32; i < 36; i++) {
		pVertex[i].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
		pVertex[i].c = keyColor;
	}

	//�C���f�b�N�X
	unsigned long index30[] = { 32, 33, 34, 33, 35, 34 };
	for (i = 0; i < 6; i++) {
		pIndex[48 + i] = m_BufInfo[noteNo].vertexPos + index30[i];
	}

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

	//���_
	pVertex[36].p = D3DXVECTOR3(pVertex[0].p.x, 0.0f, pVertex[0].p.z);  // 0'
	pVertex[37].p = D3DXVECTOR3(pVertex[1].p.x, 0.0f, pVertex[1].p.z);  // 1'
	pVertex[38].p = D3DXVECTOR3(pVertex[2].p.x, 0.0f, pVertex[2].p.z);  // 2'
	pVertex[39].p = D3DXVECTOR3(pVertex[3].p.x, 0.0f, pVertex[3].p.z);  // 3'
	pVertex[40].p = D3DXVECTOR3(pVertex[4].p.x, 0.0f, pVertex[4].p.z);  // 4'
	pVertex[41].p = D3DXVECTOR3(pVertex[5].p.x, 0.0f, pVertex[5].p.z);  // 5'
	pVertex[42].p = D3DXVECTOR3(pVertex[6].p.x, 0.0f, pVertex[6].p.z);  // 6'
	pVertex[43].p = D3DXVECTOR3(pVertex[7].p.x, 0.0f, pVertex[7].p.z);  // 7'

	//�@���^�F
	for (i = 36; i < 44; i++) {
		pVertex[i].n = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
		pVertex[i].c = keyColor;
	}

	//�C���f�b�N�X
	unsigned long indexDW[] = { 36, 37, 38, 36, 38, 39, 43, 40, 41, 43, 41, 42 };
	for (i = 0; i < 12; i++) {
		pIndex[54 + i] = m_BufInfo[noteNo].vertexPos + indexDW[i];
	}

	//----------------------------------------------------------------
	//�P��F�̃e�N�X�`�����W
	//----------------------------------------------------------------
	m_KeyboardDesign.GetWhiteKeyTexturePosSingleColor(noteNo, &tsc);
	for (i = 12; i < 44; i++) {
		pVertex[i].t = tsc;
	}

	return result;
}

//******************************************************************************
// �L�[�{�[�h���_�����F����C
//******************************************************************************
int MTPianoKeyboard::_CreateVertexOfKeyWhite3(
		unsigned char noteNo,
		MTPIANOKEYBOARD_VERTEX* pVertex,
		unsigned long* pIndex,
		D3DXCOLOR* pColor
	)
{
	int result = 0;
	unsigned long i = 0;
	float centerX       = m_KeyboardDesign.GetKeyCenterPosX(noteNo);
	float heightY       = m_KeyboardDesign.GetWhiteKeyHeight();
	float whiteKeyWidth = m_KeyboardDesign.GetWhiteKeyWidth();
	float whiteKeyLen   = m_KeyboardDesign.GetWhiteKeyLen();
	float blackKeyWidth = m_KeyboardDesign.GetBlackKeyWidth();
	float blackKeyLen   = m_KeyboardDesign.GetBlackKeyLen();
	float deltaKeyLen   = whiteKeyLen - blackKeyLen;
	float spc           = m_KeyboardDesign.GetKeySpaceSize();
	float prevCenterX   = m_KeyboardDesign.GetKeyCenterPosX(noteNo-1);
	D3DXCOLOR keyColor;
	D3DXVECTOR2 t0, t1, t2, t3, t4, t5, t6, t7, tsc;

	//�����J���[
	if (pColor == NULL) {
		keyColor = m_KeyboardDesign.GetWhiteKeyColor();
	}
	else {
		keyColor = *pColor;
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

	//���_
	pVertex[0].p = D3DXVECTOR3(centerX - (whiteKeyWidth/2.0f),           heightY, 0.0f);
	pVertex[1].p = D3DXVECTOR3(centerX + (whiteKeyWidth/2.0f),           heightY, 0.0f);
	pVertex[2].p = D3DXVECTOR3(centerX + (whiteKeyWidth/2.0f),           heightY, deltaKeyLen - spc);
	pVertex[3].p = D3DXVECTOR3(centerX - (whiteKeyWidth/2.0f),           heightY, deltaKeyLen - spc);
	pVertex[4].p = D3DXVECTOR3(centerX + (whiteKeyWidth/2.0f),           heightY, whiteKeyLen);
	pVertex[5].p = D3DXVECTOR3(prevCenterX + (blackKeyWidth/2.0f) + spc, heightY, whiteKeyLen);
	pVertex[6].p = D3DXVECTOR3(prevCenterX + (blackKeyWidth/2.0f) + spc, heightY, deltaKeyLen - spc);

	//�@���^�F
	for (i = 0; i < 7; i++) {
		pVertex[i].n = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		pVertex[i].c = keyColor;
	}

	//�C���f�b�N�X
	unsigned long indexUP[] = { 0, 2, 1, 0, 3, 2, 2, 6, 4, 6, 5, 4 };
	for (i = 0; i < 12; i++) {
		pIndex[i] = m_BufInfo[noteNo].vertexPos + indexUP[i];
	}

	//�e���_�̃e�N�X�`�����W
	m_KeyboardDesign.GetWhiteKeyTexturePosTop(noteNo, &t0, &t1, &t2, &t3, &t4, &t5, &t6, &t7);
	pVertex[0].t = t0;
	pVertex[1].t = t1;
	pVertex[2].t = t2;
	pVertex[3].t = t3;
	pVertex[4].t = t5;
	pVertex[5].t = t6;
	pVertex[6].t = t7;

	//----------------------------------------------------------------
	//���� 0-1
	//----------------------------------------------------------------
	// 0      1
	// 7+----+8
	//  |    |
	// 9+----+10

	//���_
	pVertex[7].p  = pVertex[0].p;
	pVertex[8].p  = pVertex[1].p;
	pVertex[9].p  = D3DXVECTOR3(pVertex[0].p.x, 0.0f, pVertex[0].p.z); // 0'
	pVertex[10].p = D3DXVECTOR3(pVertex[1].p.x, 0.0f, pVertex[1].p.z); // 1'

	//�@���^�F
	for (i = 7; i < 11; i++) {
		pVertex[i].n = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
		pVertex[i].c = keyColor;
	}

	//�C���f�b�N�X
	unsigned long index01[] = { 7, 8, 9, 8, 10, 9 };
	for (i = 0; i < 6; i++) {
		pIndex[12 + i] = m_BufInfo[noteNo].vertexPos + index01[i];
	}

	//�e���_�̃e�N�X�`�����W
	m_KeyboardDesign.GetWhiteKeyTexturePosFront(noteNo, &t0, &t1, &t2, &t3);
	pVertex[7].t  = t0;
	pVertex[8].t  = t1;
	pVertex[9].t  = t2;
	pVertex[10].t = t3;

	//----------------------------------------------------------------
	//���� 1-4
	//----------------------------------------------------------------
	// 4 12+--+14
	//     |  |
	//     |  |
	// 1 11+--+13

	//���_
	pVertex[11].p = pVertex[1].p;
	pVertex[12].p = pVertex[4].p;
	pVertex[13].p = D3DXVECTOR3(pVertex[1].p.x, 0.0f, pVertex[1].p.z);  // 1'
	pVertex[14].p = D3DXVECTOR3(pVertex[4].p.x, 0.0f, pVertex[4].p.z);  // 4'

	//�@���^�F
	for (i = 11; i < 15; i++) {
		pVertex[i].n = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
		pVertex[i].c = keyColor;
	}

	//�C���f�b�N�X
	unsigned long index14[] = { 11, 12, 13, 12, 14, 13 };
	for (i = 0; i < 6; i++) {
		pIndex[18 + i] = m_BufInfo[noteNo].vertexPos + index14[i];
	}

	//----------------------------------------------------------------
	//���� 4-5
	//----------------------------------------------------------------
	//   18+--+17
	//     |  |
	// 5 16+--+15 4

	//���_
	pVertex[15].p = pVertex[4].p;
	pVertex[16].p = pVertex[5].p;
	pVertex[17].p = D3DXVECTOR3(pVertex[4].p.x, 0.0f, pVertex[4].p.z);  // 4'
	pVertex[18].p = D3DXVECTOR3(pVertex[5].p.x, 0.0f, pVertex[5].p.z);  // 5'

	//�@���^�F
	for (i = 15; i < 19; i++) {
		pVertex[i].n = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
		pVertex[i].c = keyColor;
	}

	//�C���f�b�N�X
	unsigned long index45[] = { 15, 16, 17, 16, 18, 17 };
	for (i = 0; i < 6; i++) {
		pIndex[24 + i] = m_BufInfo[noteNo].vertexPos + index45[i];
	}

	//----------------------------------------------------------------
	//���� 5-6
	//----------------------------------------------------------------
	// 21+--+19 5
	//   |  |
	//   |  |
	// 22+--+20 6

	//���_
	pVertex[19].p = pVertex[5].p;
	pVertex[20].p = pVertex[6].p;
	pVertex[21].p = D3DXVECTOR3(pVertex[5].p.x, 0.0f, pVertex[5].p.z);  // 5'
	pVertex[22].p = D3DXVECTOR3(pVertex[6].p.x, 0.0f, pVertex[6].p.z);  // 6'

	//�@���^�F
	for (i = 19; i < 23; i++) {
		pVertex[i].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
		pVertex[i].c = keyColor;
	}

	//�C���f�b�N�X
	unsigned long index56[] = { 19, 20, 21, 20, 22, 21 };
	for (i = 0; i < 6; i++) {
		pIndex[30 + i] = m_BufInfo[noteNo].vertexPos + index56[i];
	}

	//----------------------------------------------------------------
	//���� 6-3
	//----------------------------------------------------------------
	//   26+--+25
	//     |  |
	// 3 24+--+23 6

	//���_
	pVertex[23].p = pVertex[6].p;
	pVertex[24].p = pVertex[3].p;
	pVertex[25].p = D3DXVECTOR3(pVertex[6].p.x, 0.0f, pVertex[6].p.z);  // 6'
	pVertex[26].p = D3DXVECTOR3(pVertex[3].p.x, 0.0f, pVertex[3].p.z);  // 3'

	//�@���^�F
	for (i = 23; i < 27; i++) {
		pVertex[i].n = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
		pVertex[i].c = keyColor;
	}

	//�C���f�b�N�X
	unsigned long index63[] = { 23, 24, 25, 24, 26, 25 };
	for (i = 0; i < 6; i++) {
		pIndex[36 + i] = m_BufInfo[noteNo].vertexPos + index63[i];
	}

	//----------------------------------------------------------------
	//���� 3-0
	//----------------------------------------------------------------
	// 29+--+27 3
	//   |  |
	//   |  |
	// 30+--+28 0

	//���_
	pVertex[27].p = pVertex[3].p;
	pVertex[28].p = pVertex[0].p;
	pVertex[29].p = D3DXVECTOR3(pVertex[3].p.x, 0.0f, pVertex[3].p.z);  // 3'
	pVertex[30].p = D3DXVECTOR3(pVertex[0].p.x, 0.0f, pVertex[0].p.z);  // 0'

	//�@���^�F
	for (i = 27; i < 31; i++) {
		pVertex[i].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
		pVertex[i].c = keyColor;
	}

	//�C���f�b�N�X
	unsigned long index30[] = { 27, 28, 29, 28, 30, 29 };
	for (i = 0; i < 6; i++) {
		pIndex[42 + i] = m_BufInfo[noteNo].vertexPos + index30[i];
	}

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

	//���_
	pVertex[31].p = D3DXVECTOR3(pVertex[0].p.x, 0.0f, pVertex[0].p.z);  // 0'
	pVertex[32].p = D3DXVECTOR3(pVertex[1].p.x, 0.0f, pVertex[1].p.z);  // 1'
	pVertex[33].p = D3DXVECTOR3(pVertex[2].p.x, 0.0f, pVertex[2].p.z);  // 2'
	pVertex[34].p = D3DXVECTOR3(pVertex[3].p.x, 0.0f, pVertex[3].p.z);  // 3'
	pVertex[35].p = D3DXVECTOR3(pVertex[4].p.x, 0.0f, pVertex[4].p.z);  // 4'
	pVertex[36].p = D3DXVECTOR3(pVertex[5].p.x, 0.0f, pVertex[5].p.z);  // 5'
	pVertex[37].p = D3DXVECTOR3(pVertex[6].p.x, 0.0f, pVertex[6].p.z);  // 6'

	//�@���^�F
	for (i = 31; i < 38; i++) {
		pVertex[i].n = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
		pVertex[i].c = keyColor;
	}

	//�C���f�b�N�X
	unsigned long indexDW[] = { 31, 32, 33, 31, 33, 34, 33, 35, 37, 37, 35, 36 };
	for (i = 0; i < 12; i++) {
		pIndex[48 + i] = m_BufInfo[noteNo].vertexPos + indexDW[i];
	}

	//----------------------------------------------------------------
	//�P��F�̃e�N�X�`�����W
	//----------------------------------------------------------------
	m_KeyboardDesign.GetWhiteKeyTexturePosSingleColor(noteNo, &tsc);
	for (i = 11; i < 38; i++) {
		pVertex[i].t = tsc;
	}

	return result;
}

//******************************************************************************
// �L�[�{�[�h���_�����F����
//******************************************************************************
int MTPianoKeyboard::_CreateVertexOfKeyBlack(
		unsigned char noteNo,
		MTPIANOKEYBOARD_VERTEX* pVertex,
		unsigned long* pIndex,
		D3DXCOLOR* pColor
	)
{
	int result = 0;
	unsigned long i = 0;
	float centerX        = m_KeyboardDesign.GetKeyCenterPosX(noteNo);
	float heightY        = m_KeyboardDesign.GetWhiteKeyHeight();
	float whiteKeyLen    = m_KeyboardDesign.GetWhiteKeyLen();
	float blackKeyWidth  = m_KeyboardDesign.GetBlackKeyWidth();
	float blackKeyHeight = m_KeyboardDesign.GetBlackKeyHeight();
	float blackKeyLen    = m_KeyboardDesign.GetBlackKeyLen();
	float deltaKeyLen    = whiteKeyLen - blackKeyLen;
	float blackKeySlope  = m_KeyboardDesign.GetBlackKeySlopeLen();
	D3DXVECTOR3 nVector;
	D3DXVECTOR3 normalizedVector;
	D3DXCOLOR keyColor;
	D3DXVECTOR2 t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, tsc;
	bool isColored = false;

	//�����J���[�擾
	if (pColor == NULL) {
		keyColor = m_KeyboardDesign.GetBlackKeyColor();
	}
	else {
		keyColor = *pColor;
		isColored = true;
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

	//���_
	pVertex[0].p = D3DXVECTOR3(centerX - (blackKeyWidth/2.0f), heightY,        deltaKeyLen);
	pVertex[1].p = D3DXVECTOR3(centerX + (blackKeyWidth/2.0f), heightY,        deltaKeyLen);
	pVertex[2].p = D3DXVECTOR3(centerX + (blackKeyWidth/2.0f), blackKeyHeight, deltaKeyLen + blackKeySlope);
	pVertex[3].p = D3DXVECTOR3(centerX - (blackKeyWidth/2.0f), blackKeyHeight, deltaKeyLen + blackKeySlope);
	pVertex[4].p = pVertex[2].p;
	pVertex[5].p = D3DXVECTOR3(centerX + (blackKeyWidth/2.0f), blackKeyHeight, whiteKeyLen);
	pVertex[6].p = D3DXVECTOR3(centerX - (blackKeyWidth/2.0f), blackKeyHeight, whiteKeyLen);
	pVertex[7].p = pVertex[3].p;

	//�@���^�F�F0-1-2-3��
	nVector = D3DXVECTOR3(0.0f, 0.12f, -0.08f);
	D3DXVec3Normalize(&normalizedVector, &nVector);
	for (i = 0; i < 4; i++) {
		pVertex[i].n = normalizedVector;
		pVertex[i].c = keyColor;
	}
	//�@���^�F�F4-5-6-7��
	for (i = 4; i < 8; i++) {
		pVertex[i].n = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		pVertex[i].c = keyColor;
	}

	//�C���f�b�N�X
	unsigned long indexUP[] = { 0, 2, 1, 0, 3, 2, 4, 7, 5, 7, 6, 5 };
	for (i = 0; i < 12; i++) {
		pIndex[i] = m_BufInfo[noteNo].vertexPos + indexUP[i];
	}

	//�e���_�̃e�N�X�`�����W
	m_KeyboardDesign.GetBlackKeyTexturePos(
			noteNo, &t0, &t1, &t2, &t3, &t4, &t5, &t6, &t7, &t8, &t9, isColored
		);
	pVertex[0].t = t0;
	pVertex[1].t = t1;
	pVertex[2].t = t2;
	pVertex[3].t = t3;
	pVertex[4].t = t2;
	pVertex[5].t = t4;
	pVertex[6].t = t5;
	pVertex[7].t = t3;

	//----------------------------------------------------------------
	//���� 0-1
	//----------------------------------------------------------------
	//  0      1
	//  8+----+9
	//   |    |
	// 10+----+11

	//���_
	pVertex[8].p  = pVertex[0].p;
	pVertex[9].p  = pVertex[1].p;
	pVertex[10].p = D3DXVECTOR3(pVertex[0].p.x, 0.0f, pVertex[0].p.z); // 0'
	pVertex[11].p = D3DXVECTOR3(pVertex[1].p.x, 0.0f, pVertex[1].p.z); // 1'

	//�@���^�F
	for (i = 8; i < 12; i++) {
		pVertex[i].n = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
		pVertex[i].c = keyColor;
	}

	//�C���f�b�N�X
	unsigned long index01[] = { 8, 9, 10, 9, 11, 10 };
	for (i = 0; i < 6; i++) {
		pIndex[12 + i] = m_BufInfo[noteNo].vertexPos + index01[i];
	}

	//�e���_�̃e�N�X�`�����W
	m_KeyboardDesign.GetBlackKeyTexturePosSingleColor(noteNo, &tsc, isColored);
	for (i = 8; i < 12; i++) {
		pVertex[i].t = tsc;
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

	//���_
	pVertex[12].p  = pVertex[1].p;
	pVertex[13].p  = pVertex[2].p;
	pVertex[14].p  = pVertex[5].p;
	pVertex[15].p = D3DXVECTOR3(pVertex[1].p.x, 0.0f, pVertex[1].p.z); // 1'
	pVertex[16].p = D3DXVECTOR3(pVertex[5].p.x, 0.0f, pVertex[5].p.z); // 5'

	//�@���^�F
	for (i = 12; i < 17; i++) {
		pVertex[i].n = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
		pVertex[i].c = keyColor;
	}

	//�C���f�b�N�X
	unsigned long index125[] = { 12, 13, 15, 13, 16, 15, 13, 14, 16 };
	for (i = 0; i < 9; i++) {
		pIndex[18 + i] = m_BufInfo[noteNo].vertexPos + index125[i];
	}

	//�e���_�̃e�N�X�`�����W
	pVertex[12].t = t1;
	pVertex[13].t = t2;
	pVertex[14].t = t4;
	pVertex[15].t = t6;
	pVertex[16].t = t7;

	//----------------------------------------------------------------
	//���� 5-6
	//----------------------------------------------------------------
	//   20+--+19
	//     |  |
	// 6 18+--+17 5

	//���_
	pVertex[17].p = pVertex[5].p;
	pVertex[18].p = pVertex[6].p;
	pVertex[19].p = D3DXVECTOR3(pVertex[5].p.x, 0.0f, pVertex[5].p.z);  // 5'
	pVertex[20].p = D3DXVECTOR3(pVertex[6].p.x, 0.0f, pVertex[6].p.z);  // 6'

	//�@���^�F
	for (i = 17; i < 21; i++) {
		pVertex[i].n = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
		pVertex[i].c = keyColor;
	}

	//�C���f�b�N�X
	unsigned long index56[] = { 17, 18, 19, 18, 20, 19 };
	for (i = 0; i < 6; i++) {
		pIndex[27 + i] = m_BufInfo[noteNo].vertexPos + index56[i];
	}

	//�e���_�̃e�N�X�`�����W
	for (i = 17; i < 21; i++) {
		pVertex[i].t = tsc;
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

	//���_
	pVertex[21].p  = pVertex[6].p;
	pVertex[22].p  = pVertex[3].p;
	pVertex[23].p  = pVertex[0].p;
	pVertex[24].p = D3DXVECTOR3(pVertex[6].p.x, 0.0f, pVertex[6].p.z); // 6'
	pVertex[25].p = D3DXVECTOR3(pVertex[0].p.x, 0.0f, pVertex[0].p.z); // 0'

	//�@���^�F
	for (i = 21; i < 26; i++) {
		pVertex[i].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
		pVertex[i].c = keyColor;
	}

	//�C���f�b�N�X
	unsigned long index630[] = { 21, 22, 24, 22, 25, 24, 22, 23, 25 };
	for (i = 0; i < 9; i++) {
		pIndex[33 + i] = m_BufInfo[noteNo].vertexPos + index630[i];
	}

	//�e���_�̃e�N�X�`�����W
	pVertex[21].t = t5;
	pVertex[22].t = t3;
	pVertex[23].t = t0;
	pVertex[24].t = t9;
	pVertex[25].t = t8;

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

	//���_
	pVertex[26].p = D3DXVECTOR3(pVertex[0].p.x, 0.0f, pVertex[0].p.z); // 0'
	pVertex[27].p = D3DXVECTOR3(pVertex[1].p.x, 0.0f, pVertex[1].p.z); // 1'
	pVertex[28].p = D3DXVECTOR3(pVertex[5].p.x, 0.0f, pVertex[5].p.z); // 5'
	pVertex[29].p = D3DXVECTOR3(pVertex[6].p.x, 0.0f, pVertex[6].p.z); // 6'

	//�@���^�F
	for (i = 26; i < 30; i++) {
		pVertex[i].n = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
		pVertex[i].c = keyColor;
	}

	//�C���f�b�N�X
	unsigned long indexDW[] = { 26, 27, 28, 26, 28, 29 };
	for (i = 0; i < 6; i++) {
		pIndex[42 + i] = m_BufInfo[noteNo].vertexPos + indexDW[i];
	}

	//�e���_�̃e�N�X�`�����W
	for (i = 26; i < 30; i++) {
		pVertex[i].t = tsc;
	}

	return result;
}

//******************************************************************************
// �e�N�X�`���摜�ǂݍ���
//******************************************************************************
int MTPianoKeyboard::_LoadTexture(
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
	result = confFile.GetStr(_T("Keyboard"), bmpFileName, _MAX_PATH, MT_IMGFILE_KEYBOARD);
	if (result != 0) goto EXIT;

	//�v���Z�X���s�t�@�C���f�B���N�g���p�X�擾
	result = YNPathUtil::GetModuleDirPath(imgFilePath, _MAX_PATH);
	if (result != 0) goto EXIT;

	//�摜�t�@�C���p�X�쐬
	_tcscat_s(imgFilePath, _MAX_PATH, bmpFileName);

	//�摜�t�@�C�������݂��Ȃ��ꍇ�͓ǂݍ��݂𒆎~����
	if (!PathFileExists(imgFilePath)) {
		m_pTexture = NULL;
		goto EXIT;
	}

	//�ǂݍ��މ摜�̏c���T�C�Y���擾���Ă���
	hresult = D3DXGetImageInfoFromFile(imgFilePath, &m_ImgInfo);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

	//�e�N�X�`���摜�Ƃ��ēǂݍ���
	hresult = D3DXCreateTextureFromFile(
					pD3DDevice,		//�e�N�X�`���Ɋ֘A�t����f�o�C�X
					imgFilePath,	//�t�@�C����
					&m_pTexture		//�쐬���ꂽ�e�N�X�`���I�u�W�F�N�g
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

EXIT:;
	if (result != 0) {
		ZeroMemory(&m_ImgInfo, sizeof(D3DXIMAGE_INFO));
	}
	return result;
}

//******************************************************************************
// �}�e���A���쐬
//******************************************************************************
void MTPianoKeyboard::_MakeMaterial(
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
	pMaterial->Power = 40.0f;
	//�����F
	pMaterial->Emissive.r = 0.0f;
	pMaterial->Emissive.g = 0.0f;
	pMaterial->Emissive.b = 0.0f;
	pMaterial->Emissive.a = 0.0f;
}

//******************************************************************************
// �ړ�
//******************************************************************************
int MTPianoKeyboard::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice,
		D3DXVECTOR3 moveVector,
		float rollAngle
	)
{
	int result = 0;
	D3DXMATRIX rotateMatrix;
	D3DXMATRIX moveMatrix;
	D3DXMATRIX worldMatrix;

	//�s�񏉊���
	D3DXMatrixIdentity(&rotateMatrix);
	D3DXMatrixIdentity(&moveMatrix);
	D3DXMatrixIdentity(&worldMatrix);

	//��]�s��
	D3DXMatrixRotationY(&rotateMatrix, D3DXToRadian(rollAngle));

	//�ړ��s��
	D3DXMatrixTranslation(&moveMatrix, moveVector.x, moveVector.y, moveVector.z);

	//�s��̍����F�ړ�����]
	//�s�b�`�x���h�ɂ��V�t�g���ɓK�p���Ă����]����
	D3DXMatrixMultiply(&worldMatrix, &moveMatrix, &rotateMatrix);

	//�ϊ��s��ݒ�
	m_PrimitiveKeyboard.Transform(worldMatrix);

//EXIT:;
	return result;
}

//******************************************************************************
// �ړ�
//******************************************************************************
int MTPianoKeyboard::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice,
		D3DXVECTOR3 moveVector1,
		D3DXVECTOR3 moveVector2,
		float scale,
		float z,
		float rollAngle
	)
{
	return YN_SET_ERR("Program error.", 0, 0);
}

//******************************************************************************
// �L�[�̃��Z�b�g
//******************************************************************************
int MTPianoKeyboard::ResetKey(
		unsigned char noteNo
	)
{
	int result = 0;
	float angle = 0.0f;

	if (noteNo >= SM_MAX_NOTE_NUM) {
		result = YN_SET_ERR("Program error.", noteNo, 0);
		goto EXIT;
	}

	_RotateKey(noteNo, angle);

EXIT:;
	return result;
}

//******************************************************************************
// �L�[�̉�������
//******************************************************************************
int MTPianoKeyboard::PushKey(
		unsigned char noteNo,
		float keyDownRate,
		unsigned long elapsedTime
	)
{
	int result = 0;
	float angle = 0.0f;
	D3DXCOLOR color;

	if (noteNo >= SM_MAX_NOTE_NUM) {
		result = YN_SET_ERR("Program error.", noteNo, 0);
		goto EXIT;
	}

	angle = m_KeyboardDesign.GetKeyRotateAngle() * keyDownRate;

	if (keyDownRate < 1.0f) {
		//�L�[�����~���^�㏸���̏ꍇ�͐F��ύX������]������
		_RotateKey(noteNo, angle);
	}
	else {
		//�L�[��������Ԃ̏ꍇ�͐F��ύX���ĉ�]������
		color = m_KeyboardDesign.GetActiveKeyColor(noteNo, elapsedTime);
		_RotateKey(noteNo, angle, &color);
	}

EXIT:;
	return result;
}

//******************************************************************************
// �L�[�̉�������
//******************************************************************************
int MTPianoKeyboard::PushKey(
		unsigned char chNo,
		unsigned char noteNo,
		float keyDownRate,
		unsigned long elapsedTime
	)
{
	return YN_SET_ERR("Program error.", 0, 0);
}

//******************************************************************************
// �`��
//******************************************************************************
int MTPianoKeyboard::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
   )
{
	int result = 0;

	//�e�N�X�`���X�e�[�W�ݒ�
	//  �J���[���Z�F��Z  ����1�F�e�N�X�`��  ����2�F�|���S��
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	// �A���t�@���Z�F��Z  ����1�F�e�N�X�`��  ����2�F�|���S��
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	//�e�N�X�`���t�B���^
	pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	//�L�[�{�[�h�̕`��
	result = m_PrimitiveKeyboard.Draw(pD3DDevice, m_pTexture);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ���
//******************************************************************************
void MTPianoKeyboard::Release()
{
	m_PrimitiveKeyboard.Release();

	if ((m_ImgInfo.Width != 0) && (m_pTexture != NULL)) {
		m_pTexture->Release();
		m_pTexture = NULL;
		ZeroMemory(&m_ImgInfo, sizeof(D3DXIMAGE_INFO));
	}
}

//******************************************************************************
// �L�[��]
//******************************************************************************
int MTPianoKeyboard::_RotateKey(
		unsigned char noteNo,
		float angle,
		D3DXCOLOR* pColor
	)
{
	int result = 0;
	unsigned long i = 0;
	MTPIANOKEYBOARD_VERTEX* pVertex = NULL;
	unsigned long offset = 0;
	unsigned long size = 0;
	MTPIANOKEYBOARD_VERTEX tempVertex[MTPIANOKEYBOARD_KEY_VERTEX_NUM_MAX];
	unsigned long tempIndex[MTPIANOKEYBOARD_KEY_INDEX_NUM_MAX];
	float centerY, centerZ = 0.0f;
	D3DXVECTOR2 ts;

	if (noteNo >= SM_MAX_NOTE_NUM) {
		result = YN_SET_ERR("Program error.", noteNo, 0);
		goto EXIT;
	}

	//��]�Ȃ��̒��_���擾
	switch (m_KeyboardDesign.GetKeyType(noteNo)) {
		case (MTPianoKeyboardDesign::KeyWhiteC):
		case (MTPianoKeyboardDesign::KeyWhiteF):
			result = _CreateVertexOfKeyWhite1(noteNo, tempVertex, tempIndex, pColor);
			if (result != 0) goto EXIT;
			break;
		case (MTPianoKeyboardDesign::KeyWhiteD):
		case (MTPianoKeyboardDesign::KeyWhiteG):
		case (MTPianoKeyboardDesign::KeyWhiteA):
			result = _CreateVertexOfKeyWhite2(noteNo, tempVertex, tempIndex, pColor);
			if (result != 0) goto EXIT;
			break;
		case (MTPianoKeyboardDesign::KeyWhiteE):
		case (MTPianoKeyboardDesign::KeyWhiteB):
			result = _CreateVertexOfKeyWhite3(noteNo, tempVertex, tempIndex, pColor);
			if (result != 0) goto EXIT;
			break;
		case (MTPianoKeyboardDesign::KeyBlack):
			result = _CreateVertexOfKeyBlack(noteNo, tempVertex, tempIndex, pColor);
			if (result != 0) goto EXIT;
			break;
	}

	//���_�Ɩ@���̍��W��]
	//
	//    |<=�L���v�X�^���{�^��   +---------+ ������
	//  +-+--------------+--------+---------+-----+
	//  |                |        |               | ������
	//  +----------------@--------+-------------+-+
	//       @:�x�_      |<=�o�����X�s��        |<=�t�����g�s��
	//                   :        :               :
	//            +z<----|--------+---------------* ���_
	//                  2.36     1.50            0.00
	//
	centerY = 0.00f;
	centerZ = m_KeyboardDesign.GetKeyRotateAxisXPos();
	for (i = 0; i < m_BufInfo[noteNo].vertexNum; i++) {
		tempVertex[i].p = _RotateYZ(centerY, centerZ, tempVertex[i].p, angle);
		tempVertex[i].n = _RotateYZ(   0.0f,   0.00f, tempVertex[i].n, angle);
	}

	//���_�o�b�t�@�̃��b�N
	offset = m_BufInfo[noteNo].vertexPos * sizeof(MTPIANOKEYBOARD_VERTEX);
	size   = m_BufInfo[noteNo].vertexNum * sizeof(MTPIANOKEYBOARD_VERTEX);
	result = m_PrimitiveKeyboard.LockVertex((void**)&pVertex, offset, size);
	if (result != 0) goto EXIT;

	//��]��̒��_���R�s�[
	memcpy(pVertex, tempVertex, size);

	//���_�o�b�t�@�̃��b�N����
	result = m_PrimitiveKeyboard.UnlockVertex();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ���W��]�FYZ����
//******************************************************************************
D3DXVECTOR3 MTPianoKeyboard::_RotateYZ(
		float centerY,
		float centerZ,
		D3DXVECTOR3 p1,
		float angle
	)
{
	D3DXVECTOR3 p2;
	float rad = 0.0f;

	rad = D3DXToRadian(angle);
	p2.x = p1.x;
	p2.y = centerY + (sin(rad) * (p1.z - centerZ)) + (cos(rad) * (p1.y - centerY));
	p2.z = centerZ + (cos(rad) * (p1.z - centerZ)) - (sin(rad) * (p1.y - centerY));

	return p2;
}

//******************************************************************************
// ���L�p�e�N�X�`���擾
//******************************************************************************
LPDIRECT3DTEXTURE9 MTPianoKeyboard::GetTexture()
{
	return m_pTexture;
}


