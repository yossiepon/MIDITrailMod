//******************************************************************************
//
// Simple MIDI Library / SMCommon
//
// ���ʒ�`
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once


//�ő�|�[�g��
#define SM_MAX_PORT_NUM  (256)

//�ő�`�����l����
#define SM_MAX_CH_NUM  (16)

//�ő�m�[�g��
#define SM_MAX_NOTE_NUM  (128)

//�ő�R���g���[���`�F���W��
#define SM_MAX_CC_NUM  (128)

//�f�t�H���gBPM (beats per minute)
//  �W��MIDI�t�@�C���d�l�ł͖��w��̏ꍇ��120�Ƃ݂Ȃ�
#define SM_DEFAULT_BPM    (120)

//�f�t�H���g�e���|�i�l�������̎��ԊԊu�^�P�ʁF�}�C�N���b�j
//  BPM=120�i1���ԂŎl������120��j�̏ꍇ = 500msec = 500,000��sec
//  �W��MIDI�t�@�C���d�l�ł̓}�C�N���b�P�ʂŕ\�������
#define SM_DEFAULT_TEMPO  ((60 * 1000 / SM_DEFAULT_BPM) * 1000)

//�f�t�H���g���q�L��
//  �W��MIDI�t�@�C���d�l�ł͖��w��̏ꍇ��4/4�Ƃ݂Ȃ�
#define SM_DEFAULT_TIME_SIGNATURE_NUMERATOR     (4)   //���q
#define SM_DEFAULT_TIME_SIGNATURE_DENOMINATOR   (4)   //����

//�f�t�H���g�s�b�`�x���h���x�F2����
#define SM_DEFAULT_PITCHBEND_SENSITIVITY  (2)

