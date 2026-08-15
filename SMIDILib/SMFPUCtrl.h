//******************************************************************************
//
// Simple MIDI Library / SMFPUCtrl
//
// Floating-point unit precision control class.
//
// Copyright (C) 2010 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

// memo
// Class that controls floating-point arithmetic precision.
// Notes on why this class is needed.
//
// When calculating total playback time or the next event processing time,
// floating-point (double) arithmetic is used to convert tick time to real
// time. However, if the floating-point precision setting differs between
// threads using SMIDILib, inconsistent behavior can occur.
//
// For example, consider thread A checking each note's sounding time using
// SMTrack::GetNoteListWithRealTime(). The sequencer class performs playback
// processing on the multimedia timer thread, so if its floating-point
// precision does not match thread A's, the note-on timing notified by the
// sequencer class during playback will drift from the sounding time thread
// A expects.
//
// In floating-point arithmetic, rounding mode / precision / exceptions -
// i.e. the FPU's behavior - are controlled per thread.
// Precision defaults to double precision, but when using Direct3D, calling
// IDirect3D9::CreateDevice switches the calling thread's precision to
// single precision. (*1)
// This causes the aforementioned drift problem.
//
// (*1) To prevent this, pass D3DCREATE_FPU_PRESERVE as an argument to
//      IDirect3D9::CreateDevice, but this may reduce performance or cause
//      unexpected behavior.

#pragma once

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

namespace SMIDILib {


//******************************************************************************
// FPU control class
//******************************************************************************
class SMIDILIB_API SMFPUCtrl
{
public:

	//Floating-point precision
	enum FPUPrecision {
		FPUSingle,		//Single precision (32bit)
		FPUDouble,		//Double precision (64bit)
		FPUExtended		//Extended precision (80bit)
	};

public:

	//Constructor / Destructor
	SMFPUCtrl(void);
	virtual ~SMFPUCtrl(void);

	//Start precision setting
	int Start(FPUPrecision precision);

	//End precision setting
	int End();

	//Check precision setting state
	bool IsLocked();

private:

	//Thread ID
	unsigned long m_ThreadID;

	//Floating-point control word
	unsigned int m_FPUCtrl;

	//Precision setting state
	bool m_isLock;

	//Prohibit assignment and copy constructor
	void operator=(const SMFPUCtrl&);
	SMFPUCtrl(const SMFPUCtrl&);

	//Display current control word
	void _DisplayCurCtrl(const TCHAR* pTitle);

};

} // end of namespace


