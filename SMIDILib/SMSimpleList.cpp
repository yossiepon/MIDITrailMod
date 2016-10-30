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

	//メモリブロックを検索
	blockitr = m_MemBlockMap.find(blockNo);
	if (blockitr == m_MemBlockMap.end()) {
		result = YN_SET_ERR("Program error.", index, blockIndex);
		goto EXIT;
	}
	pBlock = blockitr->second;

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

	//メモリブロックを検索
	blockitr = m_MemBlockMap.find(blockNo);
	if (blockitr == m_MemBlockMap.end()) {
		result = YN_SET_ERR("Program error.", index, blockIndex);
		goto EXIT;
	}
	pBlock = blockitr->second;

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

