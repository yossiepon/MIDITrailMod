//******************************************************************************
//
// Simple MIDI Library / SMFPUCtrl
//
// 浮動点小数プロセッサ制御クラス
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// memo
// 浮動小数点演算精度の制御を行うクラス。
// このクラスが必要になる理由をメモしておく。
//
// 総演奏時間や次回イベント処理時間の算出において、チックタイムを
// 実時間に変換するために、浮動小数点(double)を用いた演算を行う。
// しかしSMIDILibを利用するスレッド間で、浮動小数点演算精度の設定
// が異なると、動作に不整合が生じる場合がある。
//
// 例えばスレッドAが、SMTrack::GetNoteListWithRealTime()を利用して、
// ノートごとの発音時刻をチェックしている場合を想定する。
// シーケンサクラスは、マルチメディアタイマースレッドで演奏処理を
// 行うため、スレッドAと浮動小数点演算精度が一致していなければ、
// シーケンサクラスが演奏時に通知してくるノート発音タイミングと、
// スレッドAが期待している発音時刻がずれる。
//
// 浮動小数点演算において、丸め方／演算精度／例外といった浮動小数点
// プロセッサの動作は、スレッドごとに制御される。
// 演算精度はデフォルトで倍精度であるが、Direct3Dを利用する場合、
// IDirect3D9::CreateDeviceを呼び出した時点で、呼び出したスレッドの
// 演算精度が単精度に切り替わる。(*1)
// このようなケースで前述のズレの問題が発生する。
//
// (*1) これを抑止するには、IDirect3D9::CreateDeviceの引数で
//      D3DCREATE_FPU_PRESERVEを指定すればよいが、性能の低下や
//      予期しない動作を招く可能性がある。

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

namespace SMIDILib {


//******************************************************************************
// 浮動点小数プロセッサ制御クラス
//******************************************************************************
class SMIDILIB_API SMFPUCtrl
{
public:

	//浮動小数点精度
	enum FPUPrecision {
		FPUSingle,		//単精度(32bit)
		FPUDouble,		//倍精度(64bit)
		FPUExtended		//拡張倍精度(80bit)
	};

public:

	//コンストラクタ／デストラクタ
	SMFPUCtrl(void);
	virtual ~SMFPUCtrl(void);

	//精度設定開始
	int Start(FPUPrecision precision);

	//精度設定終了
	int End();

	//精度設定状態確認
	bool IsLocked();

private:

	//スレッドID
	unsigned long m_ThreadID;

	//浮動小数点制御ワード
	unsigned int m_FPUCtrl;

	//精度設定状態
	bool m_isLock;

	//代入とコピーコンストラクタの禁止
	void operator=(const SMFPUCtrl&);
	SMFPUCtrl(const SMFPUCtrl&);

	//浮動小数点制御ワード表示
	void _DisplayCurCtrl(TCHAR* pTitle);

};

} // end of namespace


