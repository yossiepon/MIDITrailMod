//******************************************************************************
//
// Simple MIDI Library / SMSimpleList
//
// �P�����X�g�N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// �Œ�T�C�Y�̃A�C�e����ǉ��^�Q�Ƃ��邾���̒P�����X�g�N���X�B
// ���������u���b�N�P�ʂŊm�ۂ��邱�Ƃɂ��Anew�̎��{�񐔂�}�~���āA
// ���\��D�悷��B�g���[�h�I�t�Ń������𖳑ʌ�������B

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

#include <map>

#pragma warning(disable:4251)

namespace SMIDILib {


//******************************************************************************
// �P�����X�g�N���X
//******************************************************************************
class SMIDILIB_API SMSimpleList
{
public:

	//�R���X�g���N�^�^�f�X�g���N�^
	SMSimpleList(unsigned long itemSize, unsigned long unitNum);
	virtual ~SMSimpleList(void);

	//�N���A
	virtual void Clear();

	//���ڒǉ�
	virtual int AddItem(void* pItem);

	//���ڎ擾
	virtual int GetItem(unsigned long index, void* pItem);

	//���ړo�^�i�㏑���j
	virtual int SetItem(unsigned long index, void* pItem);

	//���ڐ��擾
	virtual unsigned long GetSize();

	//�R�s�[
	virtual int CopyFrom(SMSimpleList* pSrcList);

private:

	typedef std::map<unsigned long, unsigned char*> SMMemBlockMap;
	typedef std::pair<unsigned long, unsigned char*> SMMemBlockMapPair;

private:

	unsigned long m_ItemSize;
	unsigned long m_UnitNum;
	unsigned long m_DataNum;

	SMMemBlockMap m_MemBlockMap;

	unsigned long _GetBlockNo(unsigned long index);
	unsigned long _GetBlockIndex(unsigned long index);

	//����ƃR�s�[�R���X�g���N�^�̋֎~
	void operator=(const SMSimpleList&);
	SMSimpleList(const SMSimpleList&);

};

} // end of namespace

#pragma warning(default:4251)


