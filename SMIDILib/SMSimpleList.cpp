//******************************************************************************
//
// Simple MIDI Library / SMSimpleList
//
// 単純リストクラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMSimpleList.h"

using namespace YNBaseLib;

namespace SMIDILib {

// process-wide "a list dropped an item at the 32-bit cap" flag (see header)
bool SMSimpleList::s_Truncated = false;

void SMSimpleList::ResetTruncatedFlag() { s_Truncated = false; }
bool SMSimpleList::WasTruncated()       { return s_Truncated; }


//******************************************************************************
// コンストラクタ
//******************************************************************************
SMSimpleList::SMSimpleList(
		unsigned long itemSize,
		unsigned long unitNum
	)
{
	m_ItemSize = itemSize;
	m_UnitNum = unitNum;
	m_DataNum = 0;
	m_CacheBlockNo = 0;
	m_pCacheBlock = NULL;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
SMSimpleList::~SMSimpleList(void)
{
	Clear();
}

//******************************************************************************
// クリア
//******************************************************************************
void SMSimpleList::Clear()
{
	SMMemBlockMap::iterator blockitr;

	for (blockitr = m_MemBlockMap.begin(); blockitr != m_MemBlockMap.end(); blockitr++) {
		delete [] (blockitr->second);
	}
	m_MemBlockMap.clear();

	m_DataNum = 0;
	m_pCacheBlock = NULL;   // invalidate the block cache (blocks were freed)

	return;
}

//******************************************************************************
// 項目追加
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

	// 32-bit item-count limit. m_DataNum / the block indices are unsigned long (32-bit
	// on Windows), so one more item past 0xFFFFFFFF would wrap m_DataNum to 0 and make
	// _GetBlockNo() collide with the first blocks - silently OVERWRITING earlier data
	// and miscounting. Extreme Black MIDI (> ~4.29 billion events, i.e. > ~2.1 billion
	// notes) hits this. Instead of corrupting, drop the item and raise a global flag so
	// the app can ask whether to show the (truncated) portion that did load.
	if (m_DataNum >= SMSIMPLELIST_MAX_ITEMS) {
		s_Truncated = true;
		goto EXIT;   // drop silently (no store, no count); result stays 0
	}

	index = m_DataNum;

	//データセットを格納するメモリブロックの位置を算出
	blockNo = _GetBlockNo(index);
	blockIndex = _GetBlockIndex(index);

	//メモリブロックがなければ作成する
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

	//メモリブロック上にアイテムをコピーする
	try {
		memcpy(pBlock + (m_ItemSize * blockIndex), pItem, m_ItemSize);
	}
	catch(...) {
		result = YN_SET_ERR("Memory access error.", blockNo, blockIndex);
		goto EXIT;
	}

	//�u���b�N�L���b�V���X�V
	m_CacheBlockNo = blockNo;
	m_pCacheBlock = pBlock;

	//インデックスを更新
	m_DataNum += 1;

EXIT:;
	return result;
}

//******************************************************************************
// 項目取得
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

	//データセットを格納するメモリブロックの位置を算出
	blockNo = _GetBlockNo(index);
	blockIndex = _GetBlockIndex(index);

	//�������u���b�N�����i���O�̃u���b�N�̓L���b�V������map�����������j
	if ((m_pCacheBlock != NULL) && (blockNo == m_CacheBlockNo)) {
		pBlock = m_pCacheBlock;
	}
	else {
		blockitr = m_MemBlockMap.find(blockNo);
		if (blockitr == m_MemBlockMap.end()) {
			result = YN_SET_ERR("Program error.", index, blockIndex);
			goto EXIT;
		}
		pBlock = blockitr->second;
		m_CacheBlockNo = blockNo;
		m_pCacheBlock = pBlock;
	}

	//メモリブロック上のアイテムを参照する
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
// 項目登録（上書き）
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

	//データセットを格納するメモリブロックの位置を算出
	blockNo = _GetBlockNo(index);
	blockIndex = _GetBlockIndex(index);

	//�������u���b�N�����i�L���b�V������map�����������j
	if ((m_pCacheBlock != NULL) && (blockNo == m_CacheBlockNo)) {
		pBlock = m_pCacheBlock;
	}
	else {
		blockitr = m_MemBlockMap.find(blockNo);
		if (blockitr == m_MemBlockMap.end()) {
			result = YN_SET_ERR("Program error.", index, blockIndex);
			goto EXIT;
		}
		pBlock = blockitr->second;
		m_CacheBlockNo = blockNo;
		m_pCacheBlock = pBlock;
	}

	//メモリブロック上にアイテムをコピーする
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
// アイテム数取得
//******************************************************************************
unsigned long SMSimpleList::GetSize()
{
	return m_DataNum;
}

//******************************************************************************
// ブロック番号取得
//******************************************************************************
unsigned long SMSimpleList::_GetBlockNo(
		unsigned long index
	)
{
	return (index / m_UnitNum);
}

//******************************************************************************
// ブロック内インデックス取得
//******************************************************************************
unsigned long SMSimpleList::_GetBlockIndex(
		unsigned long index
	)
{
	return (index % m_UnitNum);
}

//******************************************************************************
// コピー
//******************************************************************************
int SMSimpleList::CopyFrom(
		SMSimpleList* pSrcList
	)
{
	int result = 0;
	unsigned long index = 0;
	unsigned char* pData = NULL;

	//TODO: もう少しインテリジェントなコピーにする

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

