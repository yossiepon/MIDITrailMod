//******************************************************************************
//
// Simple MIDI Library / SMSimpleList
//
// 単純リストクラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// MEMO:
// 固定サイズのアイテムを追加／参照するだけの単純リストクラス。
// メモリをブロック単位で確保することにより、newの実施回数を抑止して、
// 性能を優先する。トレードオフでメモリを無駄遣いする。

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
// 単純リストクラス
//******************************************************************************
class SMIDILIB_API SMSimpleList
{
public:

	//コンストラクタ／デストラクタ
	SMSimpleList(unsigned long itemSize, unsigned long unitNum);
	virtual ~SMSimpleList(void);

	//クリア
	virtual void Clear();

	//項目追加
	virtual int AddItem(void* pItem);

	//項目取得
	virtual int GetItem(unsigned long index, void* pItem);

	//項目登録（上書き）
	virtual int SetItem(unsigned long index, void* pItem);

	//項目数取得
	virtual unsigned long GetSize();

	//コピー
	virtual int CopyFrom(SMSimpleList* pSrcList);

	// 32-bit item-count cap (one below the wrap point of the unsigned-long index)
	static const unsigned long SMSIMPLELIST_MAX_ITEMS = 0xFFFFFFFFUL;

	// Global "an AddItem was dropped because a list hit the 32-bit item cap" flag.
	// The app resets it before loading a file and checks it afterwards to ask whether
	// to display the truncated portion. (Static: the cap is a process-wide data limit.)
	static void ResetTruncatedFlag();
	static bool WasTruncated();

private:

	typedef std::map<unsigned long, unsigned char*> SMMemBlockMap;
	typedef std::pair<unsigned long, unsigned char*> SMMemBlockMapPair;

	static bool s_Truncated;

private:

	unsigned long m_ItemSize;
	unsigned long m_UnitNum;
	unsigned long m_DataNum;

	SMMemBlockMap m_MemBlockMap;

	// cache of the most recently used block (blocks are never freed/moved until
	// Clear, so this is valid for the common sequential-access pattern and avoids
	// a std::map lookup per item - a big win when iterating millions of notes).
	unsigned long  m_CacheBlockNo;
	unsigned char* m_pCacheBlock;

	unsigned long _GetBlockNo(unsigned long index);
	unsigned long _GetBlockIndex(unsigned long index);

	//代入とコピーコンストラクタの禁止
	void operator=(const SMSimpleList&);
	SMSimpleList(const SMSimpleList&);

};

} // end of namespace

#pragma warning(default:4251)


