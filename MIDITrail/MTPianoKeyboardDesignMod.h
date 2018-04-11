//******************************************************************************
//
// MIDITrail / MTPianoKeyboardDesignMod
//
// �s�A�m�L�[�{�[�h�f�U�C��Mod�N���X
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "MTPianoKeyboardDesign.h"


//******************************************************************************
// �s�A�m�L�[�{�[�h�f�U�C��Mod�N���X
//******************************************************************************
class MTPianoKeyboardDesignMod : public MTPianoKeyboardDesign
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTPianoKeyboardDesignMod(void);
	virtual ~MTPianoKeyboardDesignMod(void);

	//������
	virtual int Initialize(const TCHAR* pSceneName, SMSeqData* pSeqData);

	//�L�[�{�[�h����W�擾
	D3DXVECTOR3 GetKeyboardBasePos(int keyboardIndex, float angle);

	//�|�[�g���_���W�擾
	float GetPortOriginX();
	float GetPortOriginY(int keyboardIndex, float antiResizeScale, bool flip);
	float GetPortOriginZ(int keyboardIndex, float antiResizeScale, bool flip);

	//�m�[�g�{�b�N�X�����E���擾
	float GetNoteBoxHeight();
	float GetNoteBoxWidth();

	//�m�[�g�Ԋu�擾
	float GetNoteStep();

	//�`�����l���Ԋu�擾
	float GetChStep();

	//�L�[�{�[�h�����E���擾
	float GetKeyboardHeight();
	float GetKeyboardWidth();

	//�O���b�h�����E���擾
	float GetGridHeight();
	float GetGridWidth();

	//�Đ��ʍ����E���擾
	float GetPlaybackSectionHeight();
	float GetPlaybackSectionWidth();

	//�g��`��Ԋu�擾
	float GetRippleSpacing();

	//�g��`��}�[�W���擾
	float GetRippleMargin();

	//�������L�[�J���[�擾
	D3DXCOLOR GetActiveKeyColor(
			unsigned char chNo,
			unsigned char noteNo,
			unsigned long elapsedTime,
			D3DXCOLOR* pNoteColor = NULL
		);

protected:

	virtual void _Initialize();
	virtual int _LoadConfFile(const TCHAR* pSceneName);

private:

	//�m�[�g�{�b�N�X����
	float m_NoteBoxHeight;
	//�m�[�g�{�b�N�X��
	float m_NoteBoxWidth;
	//�m�[�g�Ԋu
	float m_NoteStep;
	//�`�����l���Ԋu
	float m_ChStep;

	//�g��`��Ԋu
	float m_RippleSpacing;

	//�������L�[�F���
	D3DXCOLOR m_ActiveKeyColorList[16];

};


