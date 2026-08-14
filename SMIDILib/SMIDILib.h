//******************************************************************************
//
// Simple MIDI Library / SMIDILib
//
// Simple MIDI Library public header.
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#ifdef SMIDILIB_EXPORTS
#define SMIDILIB_API __declspec(dllexport)
#else
#define SMIDILIB_API __declspec(dllimport)
#endif

//Common definitions
#include "SMCommon.h"

//Standard MIDI file reader class
#include "SMFileReader.h"

//Event classes
#include "SMEvent.h"
#include "SMEventMIDI.h"
#include "SMEventSysEx.h"
#include "SMEventSysMsg.h"
#include "SMEventMeta.h"

//List classes
#include "SMTrack.h"
#include "SMNoteList.h"
#include "SMBarList.h"
#include "SMPortList.h"

//Device control
#include "SMOutDevCtrl.h"
#include "SMInDevCtrl.h"

//Sequence processing
#include "SMSeqData.h"
#include "SMSequencer.h"
#include "SMMsgParser.h"

//Monitor
#include "SMLiveMonitor.h"

//Other
#include "SMRcpConv.h"

