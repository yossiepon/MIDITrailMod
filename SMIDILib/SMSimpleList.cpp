//******************************************************************************
//
// Simple MIDI Library / SMSimpleList
//
// �P�����X�g�N���X
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMSimpleList.h"

using namespace YNBaseLib;

namespace SMIDILib {


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
SMSimpleList::SMSimpleList(
		unsigned long itemSize,
		unsigned long unitNum
	)
{
	m_ItemSize = itemSize;
	m_UnitNum = unitNum;
	m_DataNum = 0;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
SMSimpleList::~SMSimpleList(void)
{
	Clear();
}

//******************************************************************************
// �N���A
//******************************************************************************
void SMSimpleList::Clear()
{
	SMMemBlockMap::iterator blockitr;

	for (blockitr = m_MemBlockMap.begin(); blockitr != m_MemBlockMap.end(); blockitr++) {
		delete [] (blockitr->second);
	}
	m_MemBlockMap.clear();

	m_DataNum = 0;

	return;
}

//******************************************************************************
// ���ڒǉ�
//******************************************************************************
int SMSimpleList::AddItem(
		void* pItem
	)
{
	int result = 0;
	unsigned long index = 0;
	unsigned long blockNo = 0;
	unsigned long blockIndex = 0;
	unsigned char* pBlock = NULL;
	SMMemBlockMap::iterator blockitr;

	if (pItem == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	index = m_DataNum;

	//�f�[�^�Z�b�g���i�[���郁�����u���b�N�̈ʒu���Z�o
	blockNo = _GetBlockNo(index);
	blockIndex = _GetBlockIndex(index);

	//�������u���b�N���Ȃ���΍쐬����
	blockitr = m_MemBlockMap.find(blockNo);
	if (blockitr == m_MemBlockMap.end()) {
		try {
			pBlock = new unsigned char[m_ItemSize * m_UnitNum];
		}
		catch (std::bad_alloc) {
			result = YN_SET_ERR("Could not allocate memory.", m_ItemSize, m_UnitNum);
			goto EXIT;
		}
		ZeroMemory(pBlock, m_ItemSize * m_UnitNum);
		m_MemBlockMap.insert(SMMemBlockMapPair(blockNo, pBlock));
	}
	else {
		pBlock = blockitr->second;
	}

	//�������u���b�N��ɃA�C�e�����R�s�[����
	try {
		memcpy(pBlock + (m_ItemSize * blockIndex), pItem, m_ItemSize);
	}
	catch(...) {
		result = YN_SET_ERR("Memory access error.", blockNo, blockIndex);
		goto EXIT;
	}

	//�C���f�b�N�X���X�V
	m_DataNum += 1;

EXIT:;
	return result;
}

//******************************************************************************
// ���ڎ擾
//******************************************************************************
int SMSimpleList::GetItem(
		unsigned long index,
		void* pItem
	)
{
	int result = 0;
	unsigned long blockNo = 0;
	unsigned long blockIndex = 0;
	unsigned char* pBlock = NULL;
	SMMemBlockMap::iterator blockitr;

	if (index >= m_DataNum) {
		result = YN_SET_ERR("Program error.", index, m_DataNum);
		goto EXIT;
	}
	if (pItem == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//�f�[�^�Z�b�g���i�[���郁�����u���b�N�̈ʒu���Z�o
	blockNo = _GetBlockNo(index);
	blockIndex = _GetBlockIndex(index);

	//�������u���b�N������
	blockitr = m_MemBlockMap.find(blockNo);
	if (blockitr == m_MemBlockMap.end()) {
		result = YN_SET_ERR("Program error.", index, blockIndex);
		goto EXIT;
	}
	pBlock = blockitr->second;

	//�������u���b�N��̃A�C�e�����Q�Ƃ���
	try {
		memcpy(pItem, pBlock + (m_ItemSize * blockIndex), m_ItemSize);
	}
	catch(...) {
		result = YN_SET_ERR("Memory access error.", blockNo, blockIndex);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// ���ړo�^�i�㏑���j
//******************************************************************************
int SMSimpleList::SetItem(
		unsigned long index,
		void* pItem
	)
{
	int result = 0;
	unsigned long blockNo = 0;
	unsigned long blockIndex = 0;
	unsigned char* pBlock = NULL;
	SMMemBlockMap::iterator blockitr;

	if (index >= m_DataNum) {
		result = YN_SET_ERR("Program error.", index, m_DataNum);
		goto EXIT;
	}
	if (pItem == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//�f�[�^�Z�b�g���i�[���郁�����u���b�N�̈ʒu���Z�o
	blockNo = _GetBlockNo(index);
	blockIndex = _GetBlockIndex(index);

	//�������u���b�N������
	blockitr = m_MemBlockMap.find(blockNo);
	if (blockitr == m_MemBlockMap.end()) {
		result = YN_SET_ERR("Program error.", index, blockIndex);
		goto EXIT;
	}
	pBlock = blockitr->second;

	//�������u���b�N��ɃA�C�e�����R�s�[����
	try {
		memcpy(pBlock + (m_ItemSize * blockIndex), pItem, m_ItemSize);
	}
	catch(...) {
		result = YN_SET_ERR("Memory access error.", blockNo, blockIndex);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �A�C�e�����擾
//******************************************************************************
unsigned long SMSimpleList::GetSize()
{
	return m_DataNum;
}

//******************************************************************************
// �u���b�N�ԍ��擾
//******************************************************************************
unsigned long SMSimpleList::_GetBlockNo(
		unsigned long index
	)
{
	return (index / m_UnitNum);
}

//******************************************************************************
// �u���b�N���C���f�b�N�X�擾
//******************************************************************************
unsigned long SMSimpleList::_GetBlockIndex(
		unsigned long index
	)
{
	return (index % m_UnitNum);
}

//******************************************************************************
// �R�s�[
//******************************************************************************
int SMSimpleList::CopyFrom(
		SMSimpleList* pSrcList
	)
{
	int result = 0;
	unsigned long index = 0;
	unsigned char* pData = NULL;

	//TODO: ���������C���e���W�F���g�ȃR�s�[�ɂ���

	if (pSrcList == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	if (m_ItemSize != pSrcList->m_ItemSize) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	Clear();

	try {
		pData = new unsigned char[m_ItemSize];
	}
	catch(std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", m_ItemSize, 0);
		goto EXIT;
	}

	for (index = 0; index < pSrcList->GetSize(); index++) {
		result = pSrcList->GetItem(index, pData);
		if (result != 0) goto EXIT;

		result = AddItem(pData);
		if (result != 0) goto EXIT;
	}

EXIT:;
	delete [] pData;
	return result;
}

} // end of namespace

