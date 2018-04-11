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


//******************************************************************************
// �p�����[�^��`
//******************************************************************************
//�e�N�X�`�����W�Z�o�F�r�b�g�}�b�v�T�C�Y = 562 x 562
#define TEXTURE_POINT(x, y)  (D3DXVECTOR2((float)x/561.0f, (float)y/561.0f))

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
// �|�[�g���_Y���W�擾
//******************************************************************************
float MTPianoKeyboardDesignMod::GetPortOriginY(
		unsigned char portNo
	)
{
	return 0;
}

//******************************************************************************
// �|�[�g���_Z���W�擾
//******************************************************************************
float MTPianoKeyboardDesignMod::GetPortOriginZ(
		unsigned char portNo
	)
{

	return 0;
}

//******************************************************************************
// �`�����l���Ԋu�擾
//******************************************************************************
float MTPianoKeyboardDesignMod::GetChStep()
{
	return m_ChStep;
}

//******************************************************************************
// �������L�[�J���[�擾
//******************************************************************************
D3DXCOLOR MTPianoKeyboardDesignMod::GetActiveKeyColor(
		unsigned char chNo,
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
		color = m_ActiveKeyColorList[chNo];
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
// �L�[�{�[�h����W�擾
//******************************************************************************
D3DXVECTOR3 MTPianoKeyboardDesignMod::GetKeyboardBasePos(
		unsigned char portNo,
		unsigned char chNo,
		float scale
	)
{
	float ox, oy, oz = 0.0f;

	//�|�[�g�P�ʂ̌��_���W
	ox = GetPortOriginX(portNo);
	oy = GetPortOriginY(portNo);
	oz = GetPortOriginZ(portNo);

	return D3DXVECTOR3(ox, oy, oz);
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

EXIT:;
	return result;
}


