//******************************************************************************
//
// MIDITrail / MTNoteDesign
//
// �m�[�g�f�U�C���N���X
//
// Copyright (C) 2010-2013 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// �m�[�g�{�b�N�X�̐��ʂ�0-1-2-3�̎l�p�Ƃ���B
// ���̎l�p�̒��S���W����_�ł���A�|�[�g�^�`�����l���^�m�[�g�̔ԍ�
// �ɂ���Č��肳���B
//
//           +--+
//          /  /|
//         /  / +
//        /  / /      +x
//       /  / /      /
//     0+--+1/ +y   /
//      |  |/   |  /
//     2+--+3   | /
//              |/
//   +z---------+0
//

#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "SMIDILib.h"

using namespace SMIDILib;


//******************************************************************************
// �m�[�g�f�U�C���N���X
//******************************************************************************
class MTNoteDesign
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTNoteDesign(void);
	virtual ~MTNoteDesign(void);

	//������
	int Initialize(const TCHAR* pSceneName, SMSeqData* pSeqData);

	//���t�ʒu�擾
	float GetPlayPosX(unsigned long curTickTime);

	//���C�u���j�^�p�m�[�g�ʒu�擾
	float GetLivePosX(unsigned long elapsedTime);

	//�m�[�g�{�b�N�X���S���W�擾
	D3DXVECTOR3 GetNoteBoxCenterPosX(
				unsigned long curTickTime,
				unsigned char portNo,
				unsigned char chNo,
				unsigned char noteNo,
				short pitchBendValue = 0,				//�ȗ��F�s�b�`�x���h
				unsigned char pitchBendSensitivity = 0	//�ȗ��F�s�b�`�x���h���x
			);

	//�m�[�g�{�b�N�X�c���T�C�Y�擾
	float GetNoteBoxHeight();
	float GetNoteBoxWidht();

	//�m�[�g�Ԋu�擾
	float GetNoteStep();

	//�`�����l���Ԋu�擾
	float GetChStep();

	//���C�u���j�^�\������
	unsigned long GetLiveMonitorDisplayDuration();

	//�m�[�g�{�b�N�X���_���W�擾
	void GetNoteBoxVirtexPos(
				unsigned long curTickTime,
				unsigned char portNo,
				unsigned char chNo,
				unsigned char noteNo,
				D3DXVECTOR3* pVector0,	//YZ����+X�����������č���
				D3DXVECTOR3* pVector1,	//YZ����+X�����������ĉE��
				D3DXVECTOR3* pVector2,	//YZ����+X�����������č���
				D3DXVECTOR3* pVector3,	//YZ����+X�����������ĉE��
				short pitchBendValue = 0,				//�ȗ��F�s�b�`�x���h
				unsigned char pitchBendSensitivity = 0	//�ȗ��F�s�b�`�x���h���x
			);

	//���C�u���j�^�p�m�[�g�{�b�N�X���_���W�擾
	void GetNoteBoxVirtexPosLive(
				unsigned long elapsedTime,	//�o�ߎ��ԁi�~���b�j
				unsigned char portNo,
				unsigned char chNo,
				unsigned char noteNo,
				D3DXVECTOR3* pVector0,	//YZ����+X�����������č���
				D3DXVECTOR3* pVector1,	//YZ����+X�����������ĉE��
				D3DXVECTOR3* pVector2,	//YZ����+X�����������č���
				D3DXVECTOR3* pVector3,	//YZ����+X�����������ĉE��
				short pitchBendValue = 0,				//�ȗ��F�s�b�`�x���h
				unsigned char pitchBendSensitivity = 0	//�ȗ��F�s�b�`�x���h���x
			);

	//�O���b�h�{�b�N�X���_���W�擾
	void GetGridBoxVirtexPos(
				unsigned long curTickTime,
				unsigned char portNo,
				D3DXVECTOR3* pVector0,	//YZ����+X�����������č���
				D3DXVECTOR3* pVector1,	//YZ����+X�����������ĉE��
				D3DXVECTOR3* pVector2,	//YZ����+X�����������č���
				D3DXVECTOR3* pVector3 	//YZ����+X�����������ĉE��
			);

	//���C�u���j�^�p�O���b�h�{�b�N�X���_���W�擾
	void GetGridBoxVirtexPosLive(
				unsigned long elapsedTime,	//�o�ߎ��ԁi�~���b�j
				unsigned char portNo,	//�|�[�g�ԍ�
				D3DXVECTOR3* pVector0,	//YZ����+X�����������č���
				D3DXVECTOR3* pVector1,	//YZ����+X�����������ĉE��
				D3DXVECTOR3* pVector2,	//YZ����+X�����������č���
				D3DXVECTOR3* pVector3 	//YZ����+X�����������ĉE��
			);

	//�Đ��ʒ��_���W�擾
	void GetPlaybackSectionVirtexPos(
				unsigned long curTickTime,
				D3DXVECTOR3* pVector0,	//YZ����+X�����������č���
				D3DXVECTOR3* pVector1,	//YZ����+X�����������ĉE��
				D3DXVECTOR3* pVector2,	//YZ����+X�����������č���
				D3DXVECTOR3* pVector3 	//YZ����+X�����������ĉE��
			);

	//�g��T�C�Y�擾�F�o�ߎ��ԁi�~���b�j�͏ȗ���
	float GetRippleHeight(unsigned long elapsedTime = 0);
	float GetRippleWidth(unsigned long elapsedTime = 0);
	float GetRippleAlpha(unsigned long elapsedTime = 0);

	//�s�N�`���{�[�h���Έʒu�擾
	float GetPictBoardRelativePos();

	//�|�[�g���_���W�擾
	float GetPortOriginY(unsigned char portNo);
	float GetPortOriginZ(unsigned char portNo);

	//���E���W�z�u�ړ��x�N�g���擾
	D3DXVECTOR3 GetWorldMoveVector();

	//�m�[�g�{�b�N�X�J���[�擾
	D3DXCOLOR GetNoteBoxColor(
				unsigned char portNo,
				unsigned char chNo,
				unsigned char noteNo
			);

	//�������m�[�g�{�b�N�X�J���[�擾
	D3DXCOLOR GetActiveNoteBoxColor(
				unsigned char portNo,
				unsigned char chNo,
				unsigned char noteNo,
				unsigned long elapsedTime
			);

	//�������m�[�g�{�b�N�X�G�~�b�V�u�擾�i�}�e���A���p�j
	D3DXCOLOR GetActiveNoteEmissive();

	//�O���b�h���C���J���[�擾
	D3DXCOLOR GetGridLineColor();

	//�Đ��ʃJ���[�擾
	D3DXCOLOR GetPlaybackSectionColor();

private:

	enum NoteColorType {
		Channel,
		Scale
	};

	unsigned long m_TimeDivision;
	float m_QuarterNoteLength;
	float m_NoteBoxHeight;
	float m_NoteBoxWidth;
	float m_NoteStep;
	float m_ChStep;
	float m_RippleHeight;
	float m_RippleWidth;
	float m_PictBoardRelativePos;
	SMPortList m_PortList;
	unsigned char m_PortIndex[256];

	NoteColorType m_NoteColorType;
	D3DXCOLOR m_NoteColor[16];
	D3DXCOLOR m_NoteColorOfScale[12];
	D3DXCOLOR m_ActiveNoteEmissive;
	D3DXCOLOR m_GridLineColor;
	D3DXCOLOR m_PlaybackSectionColor;

	int m_ActiveNoteDuration;
	float m_ActiveNoteWhiteRate;

	int m_RippleDuration;

	int m_LiveMonitorDisplayDuration;
	float m_LiveNoteLengthPerSecond;

	void _Clear();
	int _LoadConfFile(const TCHAR* pSceneName);

};


