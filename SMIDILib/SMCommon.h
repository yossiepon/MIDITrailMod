//******************************************************************************
//
// Simple MIDI Library / SMCommon
//
// Common definitions for the Simple MIDI Library.
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once


//Maximum number of ports
#define SM_MAX_PORT_NUM  (256)

//Maximum number of channels
#define SM_MAX_CH_NUM  (16)

//Maximum number of notes
#define SM_MAX_NOTE_NUM  (128)

//Maximum number of control changes
#define SM_MAX_CC_NUM  (128)

//Default BPM (beats per minute)
//  The Standard MIDI File spec assumes 120 when unspecified
#define SM_DEFAULT_BPM    (120)

//Default tempo (quarter note interval, in microseconds)
//  For BPM=120 (120 quarter notes per minute) = 500msec = 500,000usec
//  The Standard MIDI File spec expresses this in microseconds
#define SM_DEFAULT_TEMPO  ((60 * 1000 / SM_DEFAULT_BPM) * 1000)

//Default time signature
//  The Standard MIDI File spec assumes 4/4 when unspecified
#define SM_DEFAULT_TIME_SIGNATURE_NUMERATOR     (4)   //numerator
#define SM_DEFAULT_TIME_SIGNATURE_DENOMINATOR   (4)   //denominator

//Default pitch bend sensitivity: 2 semitones
#define SM_DEFAULT_PITCHBEND_SENSITIVITY  (2)

namespace SMIDILib {

//Progress callback type for file loading operations
typedef void (*SMLoadProgressFunc)(unsigned long current, unsigned long total, void* userData);

} // end of namespace

