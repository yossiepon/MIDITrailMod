//******************************************************************************
//
// MIDITrail / MTPianoKeyboardDesignMod
//
// �s�A�m�L�[�{�[�h�f�U�C��Mod�N���X
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "DXColorUtil.h"
#include "MTConfFile.h"
#include "MTPianoKeyboardDesignMod.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;
#include "MTSceneConst.h"


//******************************************************************************
// �p�����[�^��`
//******************************************************************************
//�e�N�X�`�����W�Z�o�F�r�b�g�}�b�v�T�C�Y = 562 x 562
#define TEXTURE_POINT(x, y)  (Vector2((float)x/561.0f, (float)y/561.0f))

//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTPianoKeyboardDesignMod::MTPianoKeyboardDesignMod(void)
{
	//_Initialize()���Ŋ��N���X�̏������������Ă΂�邽�߁A���N���X�̃R���X�g���N�^�͌Ăяo���Ȃ�
	_Initialize();
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTPianoKeyboardDesignMod::~MTPianoKeyboardDesignMod(void)
{
}

//******************************************************************************
// ������
//******************************************************************************
int MTPianoKeyboardDesignMod::Initialize(
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;

	//���N���X�̏������������Ăяo��
	MTPianoKeyboardDesign::Initialize(pSceneName, pSeqData);

	//�ݒ�t�@�C���ǂݍ���
	result = _LoadConfFile(pSceneName);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ������
//******************************************************************************
void MTPianoKeyboardDesignMod::_Initialize()
{
	unsigned long i = 0;

	//���N���X�̏������������Ăяo��
	MTPianoKeyboardDesign::_Initialize();

	for (i = 0; i < 16; i++) {
		m_ActiveKeyColorList[i] = DXColorUtil::MakeColorFromHexRGBA(_T("FF0000FF")); //�ݒ�t�@�C��
	}

	return;
}

//******************************************************************************
// �L�[�{�[�h����W�擾
//******************************************************************************
Vector3 MTPianoKeyboardDesignMod::GetKeyboardBasePos(
		int keyboardIndex,
		float angle
	)
{
	float ox, oy, oz = 0.0f;

	//���[���p�x�ɂ���ĕ`����@��؂�ւ���
	angle += angle < 0.0f ? 360.0f : 0.0f;
	bool flip = !((angle > 120.0f) && (angle < 300.0f));

	//�|�[�g�P�ʂ̌��_���W
	ox = GetPortOriginX();
	oy = GetPortOriginY(keyboardIndex, flip);
	oz = GetPortOriginZ(keyboardIndex, flip);

	return Vector3(ox, oy, oz);
}

//******************************************************************************
// �|�[�g���_X���W�擾
//******************************************************************************
float MTPianoKeyboardDesignMod::GetPortOriginX()
{
	// angle: 120���`300��(rotateX: 90��, rotateZ: 90��)
	//             +z
	//              |
	// -x<----------0---------->+x
	//              | -RippleMargin
	//         +----+----+
	//         |    |    |  @:OriginX
	//         |    |    |
	//         |    |    |
	//         |    |    |
	//         @----+----+
	//    Note #0   |  #127
	//             -z

	// angle: 0���`120��or 300���`360��(rotateX: -90��, rotateZ: 90��)
	//             +z
	//              |
	//    Note #0   |  #127
	//         +----+----+
	//         |    |    |  @:OriginX
	//         |    |    |
	//         |    |    |
	//         |    |    |
	//         @----+----+
	//              | +RippleMargin
	// -x<----------0---------->+x
	//              |
	//             -z

	float originX = -GetPlaybackSectionHeight() / 2.0f;

	//���Ղ�1/2�̕������������Ɉړ�
	originX += GetWhiteKeyStep() * GetKeyboardResizeRatio() / 2.0f;

	return originX;
}

//******************************************************************************
// �|�[�g���_Y���W�擾
//******************************************************************************
float MTPianoKeyboardDesignMod::GetPortOriginY(
		int keyboardIndex,
		bool flip
	)
{
	// angle: 120���`300��(rotateX: 90��, rotateZ: 90��)
	//      +y                                    
	//       |                                    
	//       |                             Ch.15  
	//       |                                    
	//       |            +--------------+        
	//       |      portC +--------------@ Ch.0   
	//       |                             Ch.15  
	//       |                                    
	// +z<---0---------------------------------------------->-z
	//       |            +--------------+        
	//       |      portB +--------------@ Ch.0   
	//       |                             Ch.15  
	//       |                                    
	//       |            +--------------+          @:OriginY(for portA,B,C)
	//       |      portA +--------------@ Ch.0   
	//       |                                    
	//      -y                                    

	// angle: 0���`120��or 300���`360��(rotateX: -90��, rotateZ: 90��)
	//                                           +y
	//                                     Ch.15  |
	//                                            |
	//                    +--------------+        |
	//              portA +--------------@ Ch.0   |
	//                                     Ch.15  |
	//                                            |
	// +z<----------------------------------------0-------------->-z
	//                    +--------------+        |
	//              portB +--------------@ Ch.0   |
	//                                     Ch.15  |
	//                                            |
	//                    +--------------+        |
	//              portC +--------------@ Ch.0   |
	//                                            |
	//                                            | @:OriginY(for portA,B,C)
	//                                           -y

	float portWidth = GetPortWidth();

	// TODO �V���O���L�[�{�[�h�̔�����@���Č���
	int keyboardDispNum = GetKeyboardMaxDispNum() > 1 ? m_PortList.GetSize() : 1;

	float originY;

	if (!flip) {
		originY = -portWidth * (float)(keyboardDispNum - keyboardIndex * 2) / 2.0f;

		//�`���l���Ԋu��62.5%�̍�����������
		originY -= GetChStep() * 0.625f;
	}
	else {
		originY = portWidth * (float)(keyboardDispNum - (keyboardIndex + 1) * 2) / 2.0f;

		//�`���l���Ԋu��37.5%�̍����������
		originY += GetChStep() * 0.375f;
	}

	return originY;
}

//******************************************************************************
// �|�[�g���_Z���W�擾
//******************************************************************************
float MTPianoKeyboardDesignMod::GetPortOriginZ(
		int keyboardIndex,
		bool flip
	)
{
	// angle: 120���`300��(rotateX: 90��, rotateZ: 90��)
	//             +z
	//              |
	// -x<----------0---------->+x
	//              | -RippleMargin
	//         +----+----+
	//         |    |    |  @:OriginZ
	//         |    |    |
	//         |    |    |
	//         |    |    |
	//         @----+----+
	//    Note #0   |  #127
	//             -z

	// angle: 0���`120��or 300���`360��(rotateX: -90��, rotateZ: 90��)
	//             +z
	//              |
	//    Note #0   |  #127
	//         +----+----+
	//         |    |    |  @:OriginZ
	//         |    |    |
	//         |    |    |
	//         |    |    |
	//         @----+----+
	//              | +RippleMargin
	// -x<----------0---------->+x
	//              |
	//             -z

	float originZ;

	if (!flip) {
		originZ = -(GetWhiteKeyLen() * GetKeyboardResizeRatio() + GetRippleMargin());
	}
	else {
		originZ = GetRippleMargin();
	}

	return originZ;
}

//******************************************************************************
// �m�[�g�{�b�N�X�����擾
//******************************************************************************
float MTPianoKeyboardDesignMod::GetNoteBoxHeight()
{
	return m_NoteBoxHeight;
}

//******************************************************************************
// �m�[�g�{�b�N�X���擾
//******************************************************************************
float MTPianoKeyboardDesignMod::GetNoteBoxWidth()
{
	return m_NoteBoxWidth;
}

//******************************************************************************
// �m�[�g�Ԋu�擾
//******************************************************************************
float MTPianoKeyboardDesignMod::GetNoteStep()
{
	return m_NoteStep;
}

//******************************************************************************
// �`�����l���Ԋu�擾
//******************************************************************************
float MTPianoKeyboardDesignMod::GetChStep()
{
	return m_ChStep;
}

//******************************************************************************
// �L�[�{�[�h�����擾
//******************************************************************************
float MTPianoKeyboardDesignMod::GetKeyboardHeight()
{
	return GetBlackKeyHeight();
}

//******************************************************************************
// �L�[�{�[�h���擾
//******************************************************************************
float MTPianoKeyboardDesignMod::GetKeyboardWidth()
{
	return GetWhiteKeyStep() * (float)(SM_MAX_NOTE_NUM - 53);
}

//******************************************************************************
// �O���b�h�����擾
//******************************************************************************
float MTPianoKeyboardDesignMod::GetGridHeight()
{
	return GetNoteStep() * 127.0f;
}

//******************************************************************************
// �O���b�h���擾
//******************************************************************************
float MTPianoKeyboardDesignMod::GetGridWidth()
{
	return GetChStep() * 15.0f;
}

//******************************************************************************
// �|�[�g�����擾
//******************************************************************************
float MTPianoKeyboardDesignMod::GetPortHeight()
{
	return GetGridHeight();
}

//******************************************************************************
// �|�[�g���擾
//******************************************************************************
float MTPianoKeyboardDesignMod::GetPortWidth()
{
	return GetChStep() * 16.0f;
}

//******************************************************************************
// �Đ��ʍ����擾
//******************************************************************************
float MTPianoKeyboardDesignMod::GetPlaybackSectionHeight()
{
	return GetGridHeight() + GetNoteBoxHeight();
}

//******************************************************************************
// �Đ��ʕ��擾
//******************************************************************************
float MTPianoKeyboardDesignMod::GetPlaybackSectionWidth()
{
	return GetGridWidth() + GetNoteBoxWidth();
}

//******************************************************************************
// �g��`��Ԋu�擾
//******************************************************************************
float MTPianoKeyboardDesignMod::GetRippleSpacing()
{
	return m_RippleSpacing;
}

//******************************************************************************
// �g��`��}�[�W���擾
//******************************************************************************
float MTPianoKeyboardDesignMod::GetRippleMargin()
{
	return GetRippleSpacing() * (MTNOTELYRICS_MAX_LYRICS_NUM + MTNOTERIPPLE_MAX_RIPPLE_NUM);
}

//******************************************************************************
// �L�[�{�[�h���T�C�Y��擾
//******************************************************************************
float MTPianoKeyboardDesignMod::GetKeyboardResizeRatio()
{
	//�L�[�{�[�h��̑��΍��W�ɓK�p���郊�T�C�Y��
	return GetPlaybackSectionHeight() / GetKeyboardWidth();
}

//******************************************************************************
// �������L�[�J���[�擾
//******************************************************************************
Color MTPianoKeyboardDesignMod::GetActiveKeyColor(
		unsigned char chNo,
		unsigned char noteNo,
		unsigned long elapsedTime,
		Color* pNoteColor
	)
{
	Color color;
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
		color = m_ActiveKeyColorList[chNo];
	}

	duration = (unsigned long)m_ActiveKeyColorDuration;
	rate     = m_ActiveKeyColorTailRate;

	if (elapsedTime < duration) {
		rate = ((float)elapsedTime / (float)duration) * m_ActiveKeyColorTailRate;
	}

	if (GetKeyType(noteNo) == KeyBlack) {
		r = color.R() - ((color.R()) * rate);
		g = color.G() - ((color.G()) * rate);
		b = color.B() - ((color.B()) * rate);
		a = color.A();
	}
	else {
		r = color.R() + ((1.0f - color.R()) * rate);
		g = color.G() + ((1.0f - color.G()) * rate);
		b = color.B() + ((1.0f - color.B()) * rate);
		a = color.A();
	}
	color = Color(r, g, b, a);

	return color;
}

//******************************************************************************
// �ݒ�t�@�C���ǂݍ���
//******************************************************************************
int MTPianoKeyboardDesignMod::_LoadConfFile(
		const TCHAR* pSceneName
	)
{
	int result = 0;
	TCHAR key[21] = {_T('\0')};
	TCHAR hexColor[16] = {_T('\0')};
	unsigned long i = 0;
	MTConfFile confFile;

	//���N���X�̓ǂݍ��ݏ������Ăяo��
	result = MTPianoKeyboardDesign::_LoadConfFile(pSceneName);
	if (result != 0) goto EXIT;

	//�ݒ�t�@�C�����J��
	result = confFile.Initialize(pSceneName);
	if (result != 0) goto EXIT;

	//----------------------------------
	//�X�P�[�����
	//----------------------------------
	result = confFile.SetCurSection(_T("Scale"));
	if (result != 0) goto EXIT;

	result = confFile.GetFloat(_T("NoteBoxHeight"), &m_NoteBoxHeight, 0.1f);
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("NoteBoxWidth"), &m_NoteBoxWidth, 0.1f);
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("NoteStep"), &m_NoteStep, 0.1f);
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("ChStep"), &m_ChStep, 0.5f);
	if (result != 0) goto EXIT;

	//----------------------------------
	//�F���
	//----------------------------------
	result = confFile.SetCurSection(_T("PianoKeyboard"));
	if (result != 0) goto EXIT;

	//�������̃L�[�F�����擾
	for (i = 0; i < 16; i++) {
		_stprintf_s(key, 21, _T("Ch-%02d-ActiveKeyColor"), i+1);
		result = confFile.GetStr(key, hexColor, 16, _T("FF0000FF"));
		if (result != 0) goto EXIT;

		m_ActiveKeyColorList[i] = DXColorUtil::MakeColorFromHexRGBA(hexColor);
	}

	//----------------------------------
	//�g����
	//----------------------------------
	result = confFile.SetCurSection(_T("Ripple"));
	if (result != 0) goto EXIT;

	//�g��`��Ԋu
	result = confFile.GetFloat(_T("Spacing"), &m_RippleSpacing, 0.002f);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}


