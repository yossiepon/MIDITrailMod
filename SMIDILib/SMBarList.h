//******************************************************************************
//
// Simple MIDI Library / SMBarList
//
// 小節リストクラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

#include "SMSimpleList.h"

namespace SMIDILib {

//******************************************************************************
// 小節リストクラス
//******************************************************************************
class SMIDILIB_API SMBarList
{
public:

	//コンストラクタ／デストラクタ
	SMBarList(void);
	virtual ~SMBarList(void);

	//クリア
	void Clear();

	//小節追加
	int AddBar(unsigned long tickTime);

	//小節取得
	int GetBar(unsigned long index, unsigned long* pTickTime);

	//小節数取得
	unsigned long GetSize();

	//コピー
	int CopyFrom(SMBarList* pSrcList);

private:

	SMSimpleList m_List;

	//代入とコピーコンストラクタの禁止
	void operator=(const SMBarList&);
	SMBarList(const SMBarList&);

};

} // end of namespace

