//******************************************************************************
//
// MIDITrail / MTPianoKeyboardDesign
//
// �s�A�m�L�[�{�[�h�f�U�C���N���X
//
// Copyright (C) 2010-2013 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "DXColorUtil.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTPianoKeyboardDesign.h"

using namespace YNBaseLib;


//******************************************************************************
// �p�����[�^��`
//******************************************************************************
//�e�N�X�`�����W�Z�o�F�r�b�g�}�b�v�T�C�Y = 562 x 562
#define TEXTURE_POINT(x, y)  (D3DXVECTOR2((float)x/561.0f, (float)y/561.0f))

//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTPianoKeyboardDesign::MTPianoKeyboardDesign(void)
{
	_Initialize();
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTPianoKeyboardDesign::~MTPianoKeyboardDesign(void)
{
}

//******************************************************************************
// ������
//******************************************************************************
int MTPianoKeyboardDesign::Initialize(
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;
	unsigned long index = 0;
	unsigned long portIndex = 0;
	unsigned char portNo = 0;

	//���C�u���j�^�����ݒ�
	if (pSeqData == NULL) {
		//�|�[�g���X�g
		m_PortList.Clear();
		m_PortList.AddPort(0);
	}
	//�ʏ�ݒ�
	else {
		//�|�[�g���X�g�擾
		result = pSeqData->GetPortList(&m_PortList);
		if (result != 0) goto EXIT;
	}

	//�ݒ�t�@�C���ǂݍ���
	result = _LoadConfFile(pSceneName);
	if (result != 0) goto EXIT;

	//�|�[�g�ԍ��ɏ����̃C���f�b�N�X��U��
	//�|�[�g 0�� 3�� 5�� �ɏo�͂���ꍇ�̃C���f�b�N�X�͂��ꂼ�� 0, 1, 2
	for (index = 0; index < SM_MAX_PORT_NUM; index++) {
		m_PortIndex[index] = 0;
	}
	for (index = 0; index < m_PortList.GetSize(); index++) {
		m_PortList.GetPort(index, &portNo);
		m_PortIndex[portNo] = (unsigned char)portIndex;
		portIndex++;
	}

	//�L�[��ʏ�����
	_InitKeyType();

	//�L�[���W�ݒ�
	_InitKeyPos();

EXIT:;
	return result;
}

//******************************************************************************
// ������
//******************************************************************************
void MTPianoKeyboardDesign::_Initialize()
{
	unsigned long i = 0;

	ZeroMemory(&(m_KeyInfo[0]), sizeof(MTKeyInfo) * SM_MAX_NOTE_NUM);

	for (i = 0; i < SM_MAX_PORT_NUM; i++) {
		m_PortIndex[i] = 0;
	}

	//�L�[�̃|���S�����W�̓x�^�ɍ�肱��ł��邽��
	//����Ɋւ���p�����[�^�͐ݒ�t�@�C���ɋL�ڂ��Ȃ�

	m_WhiteKeyStep      = 0.236f;
	m_WhiteKeyWidth     = 0.226f;
	m_WhiteKeyHeight    = 0.22f;
	m_WhiteKeyLen       = 1.50f;
	m_BlackKeyWidth     = 0.10f;
	m_BlackKeyHeight    = 0.34f;
	m_BlackKeySlopeLen  = 0.08f;
	m_BlackKeyLen       = 1.00f;
	m_KeySpaceSize      = 0.01f;
	m_KeyRotateAxisXPos = 2.36f;
	m_KeyRotateAngle    = 3.00f;
	m_KeyDownDuration   = 40;         //�ݒ�t�@�C��
	m_KeyUpDuration     = 40;         //�ݒ�t�@�C��
	m_KeyboardStepY     = 0.34f;      //�ݒ�t�@�C��
	m_KeyboardStepZ     = 1.50f;      //�ݒ�t�@�C��
	m_NoteDropPosZ4WhiteKey = 0.25f;
	m_NoteDropPosZ4BlackKey = 0.75f;
	m_BlackKeyShiftCDE  = 0.0216f;    //�e�N�X�`���摜 7�h�b�g����
	m_BlackKeyShiftFGAB = 0.0340f;    //�e�N�X�`���摜11�h�b�g����
	m_KeyboardMaxDispNum = 16;        //�ݒ�t�@�C��
	m_WhiteKeyColor =  DXColorUtil::MakeColorFromHexRGBA(_T("FFFFFFFF")); //�ݒ�t�@�C��
	m_BlackKeyColor =  DXColorUtil::MakeColorFromHexRGBA(_T("FFFFFFFF")); //�ݒ�t�@�C��
	m_ActiveKeyColorType = DefaultColor;  //�ݒ�t�@�C��
	m_ActiveKeyColor = DXColorUtil::MakeColorFromHexRGBA(_T("FF0000FF")); //�ݒ�t�@�C��
	m_ActiveKeyColorDuration = 400;   //�ݒ�t�@�C��
	m_ActiveKeyColorTailRate = 0.5f;  //�ݒ�t�@�C��
	m_KeyDispRangeStart = 0;
	m_KeyDispRangeEnd   = 127;

	return;
}

//******************************************************************************
// �L�[��ʏ�����
//******************************************************************************
void MTPianoKeyboardDesign::_InitKeyType()
{
	unsigned long i = 0;
	unsigned char noteNo = 0;
	KeyType type = KeyWhiteC;

	//���ۂ̌��Ղł͍����������ɂ���Ĕz�u����Ă��邽��
	//�����ɂ�(C,F)(D,G,A)(E,B)�̌`�͂��ׂĈقȂ�

	for (i = 0; i < 10; i++) {
		noteNo = (unsigned char)i * 12;				//  ________ 
		m_KeyInfo[noteNo + 0].keyType = KeyWhiteC;	// |        |C
		m_KeyInfo[noteNo + 1].keyType = KeyBlack;	// |----####|
		m_KeyInfo[noteNo + 2].keyType = KeyWhiteD;	// |        |D
		m_KeyInfo[noteNo + 3].keyType = KeyBlack;	// |----####|
		m_KeyInfo[noteNo + 4].keyType = KeyWhiteE;	// |________|E
		m_KeyInfo[noteNo + 5].keyType = KeyWhiteF;	// |        |F
		m_KeyInfo[noteNo + 6].keyType = KeyBlack;	// |----####|
		m_KeyInfo[noteNo + 7].keyType = KeyWhiteG;	// |        |G
		m_KeyInfo[noteNo + 8].keyType = KeyBlack;	// |----####|
		m_KeyInfo[noteNo + 9].keyType = KeyWhiteA;	// |        |A
		m_KeyInfo[noteNo +10].keyType = KeyBlack;	// |----####|
		m_KeyInfo[noteNo +11].keyType = KeyWhiteB;	// |________|B
	}
	noteNo = 120;									//  ________ 
	m_KeyInfo[noteNo + 0].keyType = KeyWhiteC;		// |        |C
	m_KeyInfo[noteNo + 1].keyType = KeyBlack;		// |----####|
	m_KeyInfo[noteNo + 2].keyType = KeyWhiteD;		// |        |D
	m_KeyInfo[noteNo + 3].keyType = KeyBlack;		// |----####|
	m_KeyInfo[noteNo + 4].keyType = KeyWhiteE;		// |________|E
	m_KeyInfo[noteNo + 5].keyType = KeyWhiteF;		// |        |F
	m_KeyInfo[noteNo + 6].keyType = KeyBlack;		// |----####|
	m_KeyInfo[noteNo + 7].keyType = KeyWhiteB;		// |________|G <= �`���B

	//�L�[�\���͈́F�J�n�L�[�̒���
	type = m_KeyInfo[m_KeyDispRangeStart].keyType;
	switch (type) {
		case KeyWhiteC: type = KeyWhiteC; break;
		case KeyWhiteD: type = KeyWhiteC; break;
		case KeyWhiteE: type = KeyWhiteE; break; //�ύX�ΏۂȂ�
		case KeyWhiteF: type = KeyWhiteF; break;
		case KeyWhiteG: type = KeyWhiteF; break;
		case KeyWhiteA: type = KeyWhiteF; break;
		case KeyWhiteB: type = KeyWhiteB; break; //�ύX�ΏۂȂ�
		default: break;
	}
	m_KeyInfo[m_KeyDispRangeStart].keyType = type;

	//�L�[�\���͈́F�I���L�[�̒���
	type = m_KeyInfo[m_KeyDispRangeEnd].keyType;
	switch (type) {
		case KeyWhiteC: type = KeyWhiteC; break; //�ύX�ΏۂȂ�
		case KeyWhiteD: type = KeyWhiteE; break;
		case KeyWhiteE: type = KeyWhiteE; break;
		case KeyWhiteF: type = KeyWhiteF; break; //�ύX�ΏۂȂ�
		case KeyWhiteG: type = KeyWhiteB; break;
		case KeyWhiteA: type = KeyWhiteB; break;
		case KeyWhiteB: type = KeyWhiteB; break;
		default: break;
	}
	m_KeyInfo[m_KeyDispRangeEnd].keyType = type;

	return;
}

//******************************************************************************
// �L�[���W�ݒ�
//******************************************************************************
void MTPianoKeyboardDesign::_InitKeyPos()
{
	unsigned char noteNo = 0;
	KeyType prevKeyType = KeyWhiteB;
	float posX = 0.0f;
	float shift = 0.0f;

	//�擪�m�[�g�̈ʒu
	//posX = GetWhiteKeyStep() / 2.0f;
	m_KeyInfo[noteNo].keyCenterPosX = posX;
	prevKeyType = m_KeyInfo[noteNo].keyType;

	//���ۂ̌��Ղł͍����������ɂ���Ĕz�u����Ă���
	//�܂������Ɣ����̒��_�ɍ�����z�u���Čォ��␳����

	//2�Ԗڈȍ~�̃m�[�g�̈ʒu
	for (noteNo = 1; noteNo < SM_MAX_NOTE_NUM; noteNo++) {
		//���O�̃L�[������
		if (prevKeyType == KeyBlack) {
			if (m_KeyInfo[noteNo].keyType == KeyBlack) {
				//�����̌�ɍ����͂��肦�Ȃ�
			}
			else {
				//�����Ɣ����̒����ɍ�����z�u����
				//���ۂ̌��ՂƈقȂ邪�H���팸�̂��ߖڂ��Ԃ�
				posX += (GetWhiteKeyStep() / 2.0f);
			}
		}
		//���O�̃L�[������
		else {
			if (m_KeyInfo[noteNo].keyType == KeyBlack) {
				posX += (GetWhiteKeyStep() / 2.0f);
			}
			else {
				posX += GetWhiteKeyStep();
			}
		}
		m_KeyInfo[noteNo].keyCenterPosX = posX;
		prevKeyType = m_KeyInfo[noteNo].keyType;
	}

	//�����̔z�u��␳����
	prevKeyType = KeyWhiteC;
	for (noteNo = 0; noteNo < SM_MAX_NOTE_NUM; noteNo++) {
		if (m_KeyInfo[noteNo].keyType == KeyBlack) {
			//�����̈ʒu�␳�ʂ��擾
			switch (prevKeyType) {
				case KeyWhiteC: shift = -m_BlackKeyShiftCDE;  break;
				case KeyWhiteD: shift = +m_BlackKeyShiftCDE;  break;
				case KeyWhiteF: shift = -m_BlackKeyShiftFGAB; break;
				case KeyWhiteG: shift =  0.00f;               break;
				case KeyWhiteA: shift = +m_BlackKeyShiftFGAB; break;
				default:        shift =  0.00f;               break;
			}
			//�Ō�̍����͒��_�ɔz�u
			if (noteNo == 126) {
				shift = 0.00f;
			}
			
			//�\���͈͂̐擪�����łЂƂ������c����鍕���͒����ɔz�u����
			if ((noteNo - 1) == m_KeyDispRangeStart) {
				if ((m_KeyInfo[noteNo + 1].keyType == KeyWhiteE) 
				 || (m_KeyInfo[noteNo + 1].keyType == KeyWhiteB)) {
					shift =  0.00f;
				}
			}
			if ((noteNo + 1) == m_KeyDispRangeEnd) {
				if ((m_KeyInfo[noteNo - 1].keyType == KeyWhiteD) 
				 || (m_KeyInfo[noteNo - 1].keyType == KeyWhiteF)) {
					shift =  0.00f;
				}
			}
			
			//�ʒu�␳
			m_KeyInfo[noteNo].keyCenterPosX += shift;
		}
		prevKeyType = m_KeyInfo[noteNo].keyType;
	}

	return;
}

//******************************************************************************
// �|�[�g���_X���W�擾
//******************************************************************************
float MTPianoKeyboardDesign::GetPortOriginX(
		unsigned char portNo
	)
{
	float keyboardWidth = 0.0f;
	float originX = 0.0f;

	//             +z
	//              |
	//         +----+----+
	//   Ch.15 |    |    |  @:OriginX(for portA,B,C)
	//         |    |    |
	//         |    |    |
	//         |    |    |
	//   Ch. 0 |    |    | portC
	//         @----+----+
	//   Ch.15 |    |    |
	//         |    |    |
	// -x<-----|----0----|----->+x
	//         |    |    |
	//   Ch. 0 |    |    | portB
	//         @----+----+
	//   Ch.15 |    |    |
	//         |    |    |
	//         |    |    |
	//         |    |    |
	//   Ch. 0 |    |    | portA
	//         @----+----+
	//    Note #0   |  #127
	//             -z

	keyboardWidth = GetWhiteKeyStep() * (float)(SM_MAX_NOTE_NUM - 53);
	originX = (-keyboardWidth) / 2.0f;

	return originX;
}

//******************************************************************************
// �|�[�g���_Y���W�擾
//******************************************************************************
float MTPianoKeyboardDesign::GetPortOriginY(
		unsigned char portNo
	)
{
	float portIndex = 0.0f;
	float portHeight = 0.0f;
	float originY = 0.0f;
	float totalHeight = 0.0f;
	unsigned long chNum = 0;

	//     +--+ Ch.15            +y
	//     |   +--+               |
	//     |       +--+           |
	//     |           +--+       |
	//     +--------------@ Ch.0  |
	//     portC           +--+ Ch.15
	//                     |   +--+
	// +z<------------------------0+--+--------------------->-z
	//                     |      |    +--+
	//                     +------|-------@ Ch.0
	//                     portB  |        +--+ Ch.15
	//                            |        |   +--+
	//                            |        |       +--+
	//                            |        |           +--+
	//                            |        +--------------@ Ch.0
	//                           -y        portA

	portIndex = (float)(m_PortIndex[portNo]);
	portHeight =(m_KeyboardStepY * (float)(SM_MAX_CH_NUM -1)) + GetBlackKeyHeight();

	//�\���`�����l����
	chNum = m_PortList.GetSize() * SM_MAX_CH_NUM;
	if ((unsigned long)m_KeyboardMaxDispNum < chNum) {
		chNum = m_KeyboardMaxDispNum;
	}

	totalHeight = portHeight * ((float)chNum / (float)SM_MAX_CH_NUM);
	originY = (portHeight * portIndex) - (totalHeight / 2.0f);

	return originY;
}

//******************************************************************************
// �|�[�g���_Z���W�擾
//******************************************************************************
float MTPianoKeyboardDesign::GetPortOriginZ(
		unsigned char portNo
	)
{
	float portIndex = 0.0f;
	float portLen = 0.0f;
	float originZ = 0.0f;
	float totalLen = 0.0f;
	unsigned long chNum = 0;

	//             +z
	//              |
	//         +----+----+
	//   Ch.16 |    |    |  @:OriginX(for portA,B,C)
	//         |    |    |
	//         |    |    |
	//         |    |    |
	//   Ch. 0 |    |    | portC
	//         @----+----+
	//   Ch.16 |    |    |
	//         |    |    |
	// -x<-----|----0----|----->+x
	//         |    |    |
	//   Ch. 0 |    |    | portB
	//         @----+----+
	//   Ch.16 |    |    |
	//         |    |    |
	//         |    |    |
	//         |    |    |
	//   Ch. 0 |    |    | portA
	//         @----+----+
	//    Note #0   |  #127
	//             -z

	portIndex = (float)(m_PortIndex[portNo]);
	portLen =(m_KeyboardStepZ * (float)(SM_MAX_CH_NUM -1)) + GetWhiteKeyLen();

	//�\���`�����l����
	chNum = m_PortList.GetSize() * SM_MAX_CH_NUM;
	if ((unsigned long)m_KeyboardMaxDispNum < chNum) {
		chNum = m_KeyboardMaxDispNum;
	}

	totalLen = portLen * ((float)chNum / (float)SM_MAX_CH_NUM);
	originZ = (portLen * portIndex) - (totalLen / 2.0f);

	return originZ;
}

//******************************************************************************
// �L�[��ʎ擾
//******************************************************************************
MTPianoKeyboardDesign::KeyType MTPianoKeyboardDesign::GetKeyType(
		unsigned char noteNo
	)
{
	KeyType keyType = KeyWhiteC;

	if (noteNo < SM_MAX_NOTE_NUM) {
		keyType = m_KeyInfo[noteNo].keyType;
	}

	return keyType;
}

//******************************************************************************
// �L�[���SX���W�擾
//******************************************************************************
float MTPianoKeyboardDesign::GetKeyCenterPosX(
		unsigned char noteNo
	)
{
	float centerPosX = 0.0f;

	if (noteNo < SM_MAX_NOTE_NUM) {
		centerPosX = m_KeyInfo[noteNo].keyCenterPosX;
	}

	return centerPosX;
}

//******************************************************************************
// �����z�u�Ԋu�擾
//******************************************************************************
float MTPianoKeyboardDesign::GetWhiteKeyStep()
{
	return m_WhiteKeyStep;
}

//******************************************************************************
// �������T�C�Y�擾
//******************************************************************************
float MTPianoKeyboardDesign::GetWhiteKeyWidth()
{
	return m_WhiteKeyWidth;
}

//******************************************************************************
// ���������擾
//******************************************************************************
float MTPianoKeyboardDesign::GetWhiteKeyHeight()
{
	return m_WhiteKeyHeight;
}

//******************************************************************************
// ���������擾
//******************************************************************************
float MTPianoKeyboardDesign::GetWhiteKeyLen()
{
	return m_WhiteKeyLen;
}

//******************************************************************************
// �������T�C�Y�擾
//******************************************************************************
float MTPianoKeyboardDesign::GetBlackKeyWidth()
{
	return m_BlackKeyWidth;
}

//******************************************************************************
// ���������擾
//******************************************************************************
float MTPianoKeyboardDesign::GetBlackKeyHeight()
{
	return m_BlackKeyHeight;
}

//******************************************************************************
// �����X�Β����擾
//******************************************************************************
float MTPianoKeyboardDesign::GetBlackKeySlopeLen()
{
	return m_BlackKeySlopeLen;
}

//******************************************************************************
// ���������擾
//******************************************************************************
float MTPianoKeyboardDesign::GetBlackKeyLen()
{
	return m_BlackKeyLen;
}

//******************************************************************************
// �L�[�Ԋu�T�C�Y�擾
//******************************************************************************
float MTPianoKeyboardDesign::GetKeySpaceSize()
{
	return m_KeySpaceSize;
}

//******************************************************************************
// �L�[������]���SY�����W�擾
//******************************************************************************
float MTPianoKeyboardDesign::GetKeyRotateAxisXPos()
{
	return m_KeyRotateAxisXPos;
}

//******************************************************************************
// �L�[������]�p�x
//******************************************************************************
float MTPianoKeyboardDesign::GetKeyRotateAngle()
{
	return m_KeyRotateAngle;
}

//******************************************************************************
// �L�[���~���Ԏ擾(msec)
//******************************************************************************
unsigned long MTPianoKeyboardDesign::GetKeyDownDuration()
{
	return (unsigned long)m_KeyDownDuration;
}

//******************************************************************************
// �L�[�㏸���Ԏ擾(msec)
//******************************************************************************
unsigned long MTPianoKeyboardDesign::GetKeyUpDuration()
{
	return (unsigned long)m_KeyUpDuration;
}

//******************************************************************************
// �m�[�g�h���b�v���W�擾
//******************************************************************************
float MTPianoKeyboardDesign::GetNoteDropPosZ(
		unsigned char noteNo
	)
{
	float dropPosZ = 0.0f;

	if (m_KeyInfo[noteNo].keyType == KeyBlack) {
		dropPosZ = m_NoteDropPosZ4BlackKey;
	}
	else {
		dropPosZ = m_NoteDropPosZ4WhiteKey;
	}

	return dropPosZ;
}

//******************************************************************************
// �s�b�`�x���h�L�[�{�[�h�V�t�g�ʎ擾
//******************************************************************************
float MTPianoKeyboardDesign::GetPitchBendShift(
		short pitchBendValue,				//�s�b�`�x���h
		unsigned char pitchBendSensitivity	//�s�b�`�x���h���x
	)
{
	float shift = 0.0f;
	float noteStep = 0.0f;

	//�����̈ړ���
	//  �L�[�̔z�u�Ԋu�� B->C, E->F �̊Ԃɍ��������݂��Ȃ����ߋψ�ł͂Ȃ�
	//  1�I�N�^�[�u�ł��܂������悤�ɔ����̃V�t�g�ʂ����߂�
	noteStep = GetWhiteKeyStep() * 7.0f / 12.0f;

	//�s�b�`�x���h�ɂ��L�[�{�[�h�ړ���
	if (pitchBendValue < 0) {
		shift = noteStep * pitchBendSensitivity * ((float)pitchBendValue / 8192.0f);
	}
	else {
		shift = noteStep * pitchBendSensitivity * ((float)pitchBendValue / 8191.0f);
	}

	return shift;
}

//******************************************************************************
// �����J���[�擾
//******************************************************************************
D3DXCOLOR MTPianoKeyboardDesign::GetWhiteKeyColor()
{
	return m_WhiteKeyColor;
}

//******************************************************************************
// �����J���[�擾
//******************************************************************************
D3DXCOLOR MTPianoKeyboardDesign::GetBlackKeyColor()
{
	return m_BlackKeyColor;
}

//******************************************************************************
// �������L�[�J���[�擾
//******************************************************************************
D3DXCOLOR MTPianoKeyboardDesign::GetActiveKeyColor(
		unsigned char noteNo,
		unsigned long elapsedTime,
		D3DXCOLOR* pNoteColor
	)
{
	D3DXCOLOR color;
	float r,g,b,a = 0.0f;
	float rate = 0.0f;
	unsigned long duration = 0;

	//          on     off
	//   �� |---+......+---- ��off�ɂȂ����甒���̐F�ɖ߂�
	//      |   :      :
	//      |   :  +---+     ��off�ɂȂ�܂Œ��ԐF�̂܂�
	//      |   : /:   :
	//      |   :/ :   :
	//   �� |   +  :   :     ���L�[��������̐F�i�ԁj
	//      |   :\ :   :
	//      |   : \:   :
	//      |   :  +---+     ��off�ɂȂ�܂Œ��ԐF�̂܂�
	//      |   :  :   :
	//   �� |---+  :   +---- ��off�ɂȂ����獕���̐F�ɖ߂�
	//   ---+---*------*-------> +t
	//      |   on :   off
	//          <-->duration

	if ((pNoteColor != NULL) && (m_ActiveKeyColorType == NoteColor)) {
		//�m�[�g�F���w�肳��Ă���ꍇ
		color = *pNoteColor;
	}
	else {
		//����ȊO�̓f�t�H���g�F�Ƃ���
		color = m_ActiveKeyColor;
	}

	duration = (unsigned long)m_ActiveKeyColorDuration;
	rate     = m_ActiveKeyColorTailRate;

	if (elapsedTime < duration) {
		rate = ((float)elapsedTime / (float)duration) * m_ActiveKeyColorTailRate;
	}

	if (GetKeyType(noteNo) == KeyBlack) {
		r = color.r - ((color.r) * rate);
		g = color.g - ((color.g) * rate);
		b = color.b - ((color.b) * rate);
		a = color.a;
	}
	else {
		r = color.r + ((1.0f - color.r) * rate);
		g = color.g + ((1.0f - color.g) * rate);
		b = color.b + ((1.0f - color.b) * rate);
		a = color.a;
	}
	color = D3DXCOLOR(r, g, b, a);

	return color;
}

//******************************************************************************
// �����e�N�X�`�����W�擾�F���
//******************************************************************************
void MTPianoKeyboardDesign::GetWhiteKeyTexturePosTop(
		unsigned char noteNo,
		D3DXVECTOR2* pTexPos0,
		D3DXVECTOR2* pTexPos1,
		D3DXVECTOR2* pTexPos2,
		D3DXVECTOR2* pTexPos3,
		D3DXVECTOR2* pTexPos4,
		D3DXVECTOR2* pTexPos5,
		D3DXVECTOR2* pTexPos6,
		D3DXVECTOR2* pTexPos7
	)
{
	unsigned long index = 0;
	unsigned long x = 0;
	unsigned long y = 1;

	// 6+-+5       6+-+5       6+-+5  6+-+5       6+-+5     6+-+5       6+-+5
	//  | |         | |         | |    | |         | |       | |         | |
	//  | |         | |         | |    | |         | |       | |         | |
	// 7| |4       7| |4       7| |4  7| |4       7| |4     7| |4       7| |4
	// 3+-+---+2 3+-+-+-+2 3+---+-+2  3+-+---+2 3+-+-+-+2 3+-+-+-+2 3+---+-+2
	//  |     |   |     |   |     |    |     |   |     |   |     |   |     |
	//  |  C  |   |  D  |   |  E  |    |  F  |   |  G  |   |  A  |   |  B  |
	//  |     |   |     |   |     |    |     |   |     |   |     |   |     |
	// 0+-----+1 0+-----+1 0+-----+1  0+-----+1 0+-----+1 0+-----+1 0+-----+1
                                                          
	unsigned long pos[7][8][2] = {
		// 0           1           2           3           4              5              6              7
		{ {  3, 488}, { 77, 488}, { 77, 330}, { 3,  330}, { 56- 7, 330}, { 56- 7,   1}, {  3   ,   1}, {  3   , 330} }, // C
		{ { 79, 488}, {154, 488}, {154, 330}, { 79, 330}, {133+ 7, 330}, {133+ 7,   1}, { 99- 7,   1}, { 99- 7, 330} }, // D
		{ {156, 488}, {230, 488}, {230, 330}, {156, 330}, {230   , 330}, {230   ,   1}, {176+ 7,   1}, {176+ 7, 330} }, // E
		{ {232, 488}, {307, 488}, {307, 330}, {232, 330}, {286-11, 330}, {286-11,   1}, {232   ,   1}, {232   , 330} }, // F
		{ {309, 488}, {384, 488}, {384, 330}, {309, 330}, {363   , 330}, {363   ,   1}, {329-11,   1}, {329-11, 330} }, // G
		{ {386, 488}, {460, 488}, {460, 330}, {386, 330}, {440+11, 330}, {440+11,   1}, {406   ,   1}, {406   , 330} }, // A
		{ {462, 488}, {537, 488}, {537, 330}, {462, 330}, {537   , 330}, {537   ,   1}, {483+11,   1}, {483+11, 330} }  // B
	};

	switch(GetKeyType(noteNo)) {
		case KeyWhiteC: index = 0; break;
		case KeyWhiteD: index = 1; break;
		case KeyWhiteE: index = 2; break;
		case KeyWhiteF: index = 3; break;
		case KeyWhiteG: index = 4; break;
		case KeyWhiteA: index = 5; break;
		case KeyWhiteB: index = 6; break;
	}

	*pTexPos0 = TEXTURE_POINT(pos[index][0][x], pos[index][0][y]);
	*pTexPos1 = TEXTURE_POINT(pos[index][1][x], pos[index][1][y]);
	*pTexPos2 = TEXTURE_POINT(pos[index][2][x], pos[index][2][y]);
	*pTexPos3 = TEXTURE_POINT(pos[index][3][x], pos[index][3][y]);
	*pTexPos4 = TEXTURE_POINT(pos[index][4][x], pos[index][4][y]);
	*pTexPos5 = TEXTURE_POINT(pos[index][5][x], pos[index][5][y]);
	*pTexPos6 = TEXTURE_POINT(pos[index][6][x], pos[index][6][y]);
	*pTexPos7 = TEXTURE_POINT(pos[index][7][x], pos[index][7][y]);

	return;
}

//******************************************************************************
// �����e�N�X�`�����W�擾�F�O��
//******************************************************************************
void MTPianoKeyboardDesign::GetWhiteKeyTexturePosFront(
		unsigned char noteNo,
		D3DXVECTOR2* pTexPos0,
		D3DXVECTOR2* pTexPos1,
		D3DXVECTOR2* pTexPos2,
		D3DXVECTOR2* pTexPos3
	)
{
	unsigned long index = 0;
	unsigned long x = 0;
	unsigned long y = 1;

	//  0+----+1
	//   |    |
	//  2+----+3

	unsigned long pos[7][4][2] = {
		// 0         1         2         3
		{ {  3, 489}, { 77, 489}, {  3, 561}, { 77, 561} }, // C
		{ { 79, 489}, {154, 489}, { 79, 561}, {154, 561} }, // D
		{ {156, 489}, {230, 489}, {156, 561}, {230, 561} }, // E
		{ {232, 489}, {307, 489}, {232, 561}, {307, 561} }, // F
		{ {309, 489}, {384, 489}, {309, 561}, {384, 561} }, // G
		{ {386, 489}, {460, 489}, {386, 561}, {460, 561} }, // A
		{ {462, 489}, {537, 489}, {462, 561}, {537, 561} }  // B
	};

	switch(GetKeyType(noteNo)) {
		case KeyWhiteC: index = 0; break;
		case KeyWhiteD: index = 1; break;
		case KeyWhiteE: index = 2; break;
		case KeyWhiteF: index = 3; break;
		case KeyWhiteG: index = 4; break;
		case KeyWhiteA: index = 5; break;
		case KeyWhiteB: index = 6; break;
	}

	*pTexPos0 = TEXTURE_POINT(pos[index][0][x], pos[index][0][y]);
	*pTexPos1 = TEXTURE_POINT(pos[index][1][x], pos[index][1][y]);
	*pTexPos2 = TEXTURE_POINT(pos[index][2][x], pos[index][2][y]);
	*pTexPos3 = TEXTURE_POINT(pos[index][3][x], pos[index][3][y]);

	return;
}

//******************************************************************************
// �����e�N�X�`�����W�擾�F�P��F
//******************************************************************************
void MTPianoKeyboardDesign::GetWhiteKeyTexturePosSingleColor(
		unsigned char noteNo,
		D3DXVECTOR2* pTexPos
	)
{
	*pTexPos = TEXTURE_POINT(550, 5);
}

//******************************************************************************
// �����e�N�X�`�����W�擾�F��ʁ{����
//******************************************************************************
void MTPianoKeyboardDesign::GetBlackKeyTexturePos(
		unsigned char noteNo,
		D3DXVECTOR2* pTexPos0,
		D3DXVECTOR2* pTexPos1,
		D3DXVECTOR2* pTexPos2,
		D3DXVECTOR2* pTexPos3,
		D3DXVECTOR2* pTexPos4,
		D3DXVECTOR2* pTexPos5,
		D3DXVECTOR2* pTexPos6,
		D3DXVECTOR2* pTexPos7,
		D3DXVECTOR2* pTexPos8,
		D3DXVECTOR2* pTexPos9,
		bool isColored
	)
{
	unsigned long index = 0;
	unsigned long x = 0;
	unsigned long y = 1;

	// 9+--+ 5+-+4 +--+7
	//  |  |  | |  |  |
	//  |  |  | |  |  |
	//  |  + 3+-+2 +  |
	//  | /   | |  \  |
	// 8+-+  0+-+1  +-+6

	unsigned long pos[2][10][2] = {
		// 0              1              2              3              4              5              6              7              8              9
		{ { 63- 7, 324}, { 92- 7, 324}, { 92- 7, 305}, { 63- 7, 305}, { 92- 7,   3}, { 63- 7,   3}, { 97- 7, 324}, { 97- 7,   3}, { 58- 7, 324}, { 58- 7,   3} }, // �ʏ�
		{ {447+11, 324}, {476+11, 324}, {476+11, 305}, {447+11, 305}, {476+11,   3}, {447+11,   3}, {481+11, 324}, {481+11,   3}, {442+11, 324}, {442+11,   3} }  // ���F��
	};

	//�����|���S���ɐF��t����ꍇ��
	//���F�������e�N�X�`����\��t����
	if (isColored) {
		index = 1;
	}

	*pTexPos0 = TEXTURE_POINT(pos[index][0][x], pos[index][0][y]);
	*pTexPos1 = TEXTURE_POINT(pos[index][1][x], pos[index][1][y]);
	*pTexPos2 = TEXTURE_POINT(pos[index][2][x], pos[index][2][y]);
	*pTexPos3 = TEXTURE_POINT(pos[index][3][x], pos[index][3][y]);
	*pTexPos4 = TEXTURE_POINT(pos[index][4][x], pos[index][4][y]);
	*pTexPos5 = TEXTURE_POINT(pos[index][5][x], pos[index][5][y]);
	*pTexPos6 = TEXTURE_POINT(pos[index][6][x], pos[index][6][y]);
	*pTexPos7 = TEXTURE_POINT(pos[index][7][x], pos[index][7][y]);
	*pTexPos8 = TEXTURE_POINT(pos[index][8][x], pos[index][8][y]);
	*pTexPos9 = TEXTURE_POINT(pos[index][9][x], pos[index][9][y]);

	return;
}

//******************************************************************************
// �����e�N�X�`�����W�擾�F�P��F
//******************************************************************************
void MTPianoKeyboardDesign::GetBlackKeyTexturePosSingleColor(
		unsigned char noteNo,
		D3DXVECTOR2* pTexPos,
		bool isColored
	)
{
	if (isColored) {
		*pTexPos = TEXTURE_POINT(550, 5);
	}
	else {
		*pTexPos = TEXTURE_POINT(550, 15);
	}

	return;
}

//******************************************************************************
// �L�[�{�[�h����W�擾
//******************************************************************************
D3DXVECTOR3 MTPianoKeyboardDesign::GetKeyboardBasePos(
		unsigned char portNo,
		unsigned char chNo
	)
{
	float ox, oy, oz = 0.0f;
	D3DXVECTOR3 moveVector;

	//�|�[�g�P�ʂ̌��_���W
	ox = GetPortOriginX(portNo);
	oy = GetPortOriginY(portNo);
	oz = GetPortOriginZ(portNo);

	//�`�����l�����l�������z�u���W
	moveVector.x = ox + 0.0f;
	moveVector.y = oy + ((float)chNo * m_KeyboardStepY);
	moveVector.z = oz + ((float)chNo * m_KeyboardStepZ);

	return moveVector;
}

//******************************************************************************
// �L�[�{�[�h�\�����擾
//******************************************************************************
unsigned long MTPianoKeyboardDesign::GetKeyboardMaxDispNum()
{
	return (unsigned long)m_KeyboardMaxDispNum;
}

// >>> add 20180404 yossiepon begin
//******************************************************************************
// �L�[�{�[�h�\�����ݒ�
//******************************************************************************
void MTPianoKeyboardDesign::SetKeyboardSingle()
{
	m_KeyboardMaxDispNum = 1;
}
// <<< add 20180404 yossiepon end

//******************************************************************************
// �L�[�\���͈́F�J�n
//******************************************************************************
unsigned char MTPianoKeyboardDesign::GetKeyDispRangeStart()
{
	return (unsigned char)m_KeyDispRangeStart;
}

//******************************************************************************
// �L�[�\���͈́F�I��
//******************************************************************************
unsigned char MTPianoKeyboardDesign::GetKeyDispRangeEnd()
{
	return (unsigned char)m_KeyDispRangeEnd;
}

//******************************************************************************
// �L�[�\������
//******************************************************************************
bool MTPianoKeyboardDesign::IsKeyDisp(
		unsigned char noteNo
	)
{
	bool isDisp = false;

	if ((m_KeyDispRangeStart <= noteNo) && (noteNo <= m_KeyDispRangeEnd)) {
		isDisp = true;
	}

	return isDisp;
}

//******************************************************************************
// �ݒ�t�@�C���ǂݍ���
//******************************************************************************
int MTPianoKeyboardDesign::_LoadConfFile(
		const TCHAR* pSceneName
	)
{
	int result = 0;
	TCHAR hexColor[16] = {_T('\0')};
	TCHAR activeKeyColorType[32] = {_T('\0')};
	MTConfFile confFile;

	result = confFile.Initialize(pSceneName);
	if (result != 0) goto EXIT;

	//----------------------------------
	//�s�A�m�L�[�{�[�h���
	//----------------------------------
	result = confFile.SetCurSection(_T("PianoKeyboard"));
	if (result != 0) goto EXIT;
	result = confFile.GetInt(_T("KeyDownDuration"), &m_KeyDownDuration, 40);
	if (result != 0) goto EXIT;
	result = confFile.GetInt(_T("KeyUpDuration"), &m_KeyUpDuration, 40);
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("KeyboardStepY"), &m_KeyboardStepY, 0.34f);
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("KeyboardStepZ"), &m_KeyboardStepZ, 1.50f);
	if (result != 0) goto EXIT;
	result = confFile.GetInt(_T("KeyboardMaxDispNum"), &m_KeyboardMaxDispNum, 16);
	if (result != 0) goto EXIT;

	result = confFile.GetStr(_T("WhiteKeyColor"), hexColor, 16, _T("FFFFFFFF"));
	if (result != 0) goto EXIT;
	m_WhiteKeyColor = DXColorUtil::MakeColorFromHexRGBA(hexColor);

	result = confFile.GetStr(_T("BlackKeyColor"), hexColor, 16, _T("FFFFFFFF"));
	if (result != 0) goto EXIT;
	m_BlackKeyColor = DXColorUtil::MakeColorFromHexRGBA(hexColor);

	result = confFile.GetStr(_T("ActiveKeyColorType"), activeKeyColorType, 32, _T("STANDARD"));
	if (result != 0) goto EXIT;
	if (_tcscmp(activeKeyColorType, _T("NOTE")) == 0) {
		m_ActiveKeyColorType = NoteColor;
	}
	else {
		m_ActiveKeyColorType = DefaultColor;
	}
	result = confFile.GetStr(_T("ActiveKeyColor"), hexColor, 16, _T("FF0000FF"));
	if (result != 0) goto EXIT;
	m_ActiveKeyColor = DXColorUtil::MakeColorFromHexRGBA(hexColor);
	result = confFile.GetInt(_T("ActiveKeyColorDuration"), &m_ActiveKeyColorDuration, 400);
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("ActiveKeyColorTailRate"), &m_ActiveKeyColorTailRate, 0.5f);
	if (result != 0) goto EXIT;

	result = confFile.GetInt(_T("KeyDispRangeStart"), &m_KeyDispRangeStart, 0);
	if (result != 0) goto EXIT;
	result = confFile.GetInt(_T("KeyDispRangeEnd"), &m_KeyDispRangeEnd, 127);
	if (result != 0) goto EXIT;

	//�L�[�{�[�h�ő�\������1�|�[�g���i16ch�j�ɐ�������
	if (m_KeyboardMaxDispNum > SM_MAX_CH_NUM) {
		m_KeyboardMaxDispNum = SM_MAX_CH_NUM;
	}
	if (m_KeyboardMaxDispNum < 0) {
		m_KeyboardMaxDispNum = 0;
	}

	//�L�[�\���͈͂̃N���b�s���O
	if (m_KeyDispRangeStart < 0) {
		m_KeyDispRangeStart = 0;
	}
	if (m_KeyDispRangeStart > 127) {
		m_KeyDispRangeStart = 127;
	}
	if (m_KeyDispRangeEnd < 0) {
		m_KeyDispRangeEnd = 0;
	}
	if (m_KeyDispRangeEnd > 127) {
		m_KeyDispRangeEnd = 127;
	}
	if (m_KeyDispRangeStart > m_KeyDispRangeEnd) {
		m_KeyDispRangeEnd = m_KeyDispRangeStart;
	}

EXIT:;
	return result;
}


