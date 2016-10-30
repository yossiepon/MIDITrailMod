//******************************************************************************
//
// Simple MIDI Library / SMNoteList
//
// ノートリストクラス
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
// ノート情報構造体
//******************************************************************************
//ノート情報
typedef struct {
	unsigned char portNo;
	unsigned char chNo;
	unsigned char noteNo;
	unsigned char velocity;
	unsigned long startTime;
	unsigned long endTime;
} SMNote;

//******************************************************************************
// ノートリストクラス
//******************************************************************************
class SMIDILIB_API SMNoteList
{
public:

	//コンストラクタ／デストラクタ
	SMNoteList(void);
	virtual ~SMNoteList(void);

	//クリア
	void Clear();

	//ノート情報追加
	int AddNote(SMNote note);

	//ノート情報取得
	int GetNote(unsigned long index, SMNote* pNote);

	//ノート情報登録（上書き）
	int SetNote(unsigned long index, SMNote* pNote);

	//ノート数取得
	unsigned long GetSize();

	//コピー
	int CopyFrom(SMNoteList* pSrcList);

private:

	SMSimpleList m_List;

	//代入とコピーコンストラクタの禁止
	void operator=(const SMNoteList&);
	SMNoteList(const SMNoteList&);

};

} // end of namespace

