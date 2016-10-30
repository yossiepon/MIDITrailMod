//******************************************************************************
//
// Simple MIDI Library / SMFPUCtrl
//
// �����_�����v���Z�b�T����N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// memo
// ���������_���Z���x�̐�����s���N���X�B
// ���̃N���X���K�v�ɂȂ闝�R���������Ă����B
//
// �����t���Ԃ⎟��C�x���g�������Ԃ̎Z�o�ɂ����āA�`�b�N�^�C����
// �����Ԃɕϊ����邽�߂ɁA���������_(double)��p�������Z���s���B
// ������SMIDILib�𗘗p����X���b�h�ԂŁA���������_���Z���x�̐ݒ�
// ���قȂ�ƁA����ɕs������������ꍇ������B
//
// �Ⴆ�΃X���b�hA���ASMTrack::GetNoteListWithRealTime()�𗘗p���āA
// �m�[�g���Ƃ̔����������`�F�b�N���Ă���ꍇ��z�肷��B
// �V�[�P���T�N���X�́A�}���`���f�B�A�^�C�}�[�X���b�h�ŉ��t������
// �s�����߁A�X���b�hA�ƕ��������_���Z���x����v���Ă��Ȃ���΁A
// �V�[�P���T�N���X�����t���ɒʒm���Ă���m�[�g�����^�C�~���O�ƁA
// �X���b�hA�����҂��Ă��锭�������������B
//
// ���������_���Z�ɂ����āA�ۂߕ��^���Z���x�^��O�Ƃ��������������_
// �v���Z�b�T�̓���́A�X���b�h���Ƃɐ��䂳���B
// ���Z���x�̓f�t�H���g�Ŕ{���x�ł��邪�ADirect3D�𗘗p����ꍇ�A
// IDirect3D9::CreateDevice���Ăяo�������_�ŁA�Ăяo�����X���b�h��
// ���Z���x���P���x�ɐ؂�ւ��B(*1)
// ���̂悤�ȃP�[�X�őO�q�̃Y���̖�肪��������B
//
// (*1) �����}�~����ɂ́AIDirect3D9::CreateDevice�̈�����
//      D3DCREATE_FPU_PRESERVE���w�肷��΂悢���A���\�̒ቺ��
//      �\�����Ȃ�����������\��������B

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

namespace SMIDILib {


//******************************************************************************
// �����_�����v���Z�b�T����N���X
//******************************************************************************
class SMIDILIB_API SMFPUCtrl
{
public:

	//���������_���x
	enum FPUPrecision {
		FPUSingle,		//�P���x(32bit)
		FPUDouble,		//�{���x(64bit)
		FPUExtended		//�g���{���x(80bit)
	};

public:

	//�R���X�g���N�^�^�f�X�g���N�^
	SMFPUCtrl(void);
	virtual ~SMFPUCtrl(void);

	//���x�ݒ�J�n
	int Start(FPUPrecision precision);

	//���x�ݒ�I��
	int End();

	//���x�ݒ��Ԋm�F
	bool IsLocked();

private:

	//�X���b�hID
	unsigned long m_ThreadID;

	//���������_���䃏�[�h
	unsigned int m_FPUCtrl;

	//���x�ݒ���
	bool m_isLock;

	//����ƃR�s�[�R���X�g���N�^�̋֎~
	void operator=(const SMFPUCtrl&);
	SMFPUCtrl(const SMFPUCtrl&);

	//���������_���䃏�[�h�\��
	void _DisplayCurCtrl(TCHAR* pTitle);

};

} // end of namespace


