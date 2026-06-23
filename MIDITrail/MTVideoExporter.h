//******************************************************************************
//
// MIDITrail / MTVideoExporter
//
// M6: offline video export helpers.
//   MTTempoMap   - frame time (ms) -> sequencer tick, honoring tempo changes,
//                  built once from the merged track. Lets the export loop drive
//                  the tick-based *11 components at a fixed output FPS.
//   MTFFmpegPipe - spawns ffmpeg with a stdin pipe and streams raw BGRA frames
//                  to it (no temp files). Encoder/format is chosen per export.
//
//******************************************************************************

#pragma once

#include <windows.h>
#include <tchar.h>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include "SMIDILib.h"
#include "MTNotePitchBend.h"

using namespace SMIDILib;


//------------------------------------------------------------------------------
// encoder presets (ffmpeg)
//------------------------------------------------------------------------------
enum MTVideoCodec {
	MTVC_H264_NVENC = 0,   // GPU (NVIDIA)   -> .mp4
	MTVC_HEVC_NVENC,       // GPU (NVIDIA)   -> .mp4
	MTVC_H264_QSV,         // GPU (Intel)    -> .mp4
	MTVC_HEVC_QSV,         // GPU (Intel)    -> .mp4
	MTVC_H264_CPU,         // libx264        -> .mp4
	MTVC_HEVC_CPU,         // libx265        -> .mp4
	MTVC_QTRLE_ALPHA,      // transparent    -> .mov (QuickTime RLE)
	MTVC_PRORES4444_ALPHA, // transparent    -> .mov (ProRes 4444)
	MTVC_FFV1_ALPHA,       // transparent    -> .mkv (lossless)
	// ced 20260629: AMD GPU hardware encoder（末尾に追加。enum 値=コンボ index を維持）
	MTVC_H264_AMF,         // GPU (AMD)      -> .mp4
	MTVC_HEVC_AMF,         // GPU (AMD)      -> .mp4
};

struct MTVideoExportParams {
	MTVideoCodec codec;
	int          width;
	int          height;
	int          fps;
	int          quality;          // codec-specific knob (CRF/CQ/global_quality); 0 = preset default
	bool         transparent;      // render with a clear alpha (matches an alpha codec)
	int          superSample;      // render at superSample x output res, then ffmpeg
	                               // downscales (lanczos) for clean edges. 1 = off.
	bool         equirect360;      // render a 360 equirectangular (2:1) panorama
	TCHAR        outPath[MAX_PATH];
};

// true if the codec carries an alpha channel (transparent output)
bool MTVideoCodecIsAlpha(MTVideoCodec codec);
// preferred file extension for the codec (".mp4" / ".mov" / ".mkv"), incl. dot
const TCHAR* MTVideoCodecExt(MTVideoCodec codec);


//------------------------------------------------------------------------------
// playback state at a given tick (dashboard counters that are normally fed by
// the live sequencer message queue, which does not run during offline export)
//------------------------------------------------------------------------------
struct MTPlaybackState {
	unsigned long tempoBPM;
	unsigned long barNo;       // 1-based current bar
	unsigned long beatNum;     // time-signature numerator
	unsigned long beatDenom;   // time-signature denominator
};

//------------------------------------------------------------------------------
// tempo / time-signature map built from the merged track.
//   MsToTick      - output-frame time (ms) -> tick (drives the components)
//   GetStateAtTick - tempo / bar / beat at a tick (drives the dashboard counters)
//------------------------------------------------------------------------------
class MTTempoMap
{
public:
	MTTempoMap();
	int Build(SMSeqData* pSeqData);
	unsigned long MsToTick(double ms) const;
	double TickToMs(unsigned long tick) const;
	void GetStateAtTick(unsigned long tick, MTPlaybackState* pOut) const;

private:
	struct Seg {
		unsigned long tick;     // absolute tick at the segment start
		double        ms;       // absolute ms at the segment start
		double        msPerTick;
	};
	struct TempoSeg {
		unsigned long tick;
		unsigned long bpm;
	};
	struct TimeSigSeg {
		unsigned long tick;
		unsigned long num;
		unsigned long denom;
		unsigned long ticksPerBar;
		unsigned long barAtTick;   // 1-based bar number at this tick
	};
	std::vector<Seg>        m_Segs;
	std::vector<TempoSeg>   m_TempoSegs;
	std::vector<TimeSigSeg> m_TimeSigSegs;
};


//------------------------------------------------------------------------------
// pitch-bend timeline: per (port,ch) bend value + RPN sensitivity over the song.
// Normally fed live from the sequencer message queue; offline export replays it
// per frame so notes/keyboards bend the same way they do during playback.
//------------------------------------------------------------------------------
class MTPitchBendTimeline
{
public:
	int Build(SMSeqData* pSeqData);
	void Reset();                                       // rewind the per-channel cursors
	void Apply(unsigned long tick, MTNotePitchBend* pPB); // set every channel's value at tick
	bool HasData() const { return !m_Chans.empty(); }

private:
	struct BendEvt { unsigned long tick; short value; };
	struct SensEvt { unsigned long tick; unsigned char sens; };
	struct Chan {
		unsigned char port;
		unsigned char ch;
		std::vector<BendEvt> bends;
		std::vector<SensEvt> senss;
		size_t bi;            // bend cursor
		size_t si;            // sensitivity cursor
		short  curValue;
		unsigned char curSens;
	};
	std::vector<Chan> m_Chans;
};


//------------------------------------------------------------------------------
// ffmpeg stdin pipe
//------------------------------------------------------------------------------
class MTFFmpegPipe
{
public:
	MTFFmpegPipe();
	~MTFFmpegPipe();

	// build the ffmpeg command line for the params + spawn it with a stdin pipe.
	// pFFmpegExe = NULL uses "ffmpeg" (resolved from PATH). Also starts a background
	// writer thread + a small frame-buffer pool so encoding (ffmpeg) overlaps with
	// the caller's render + GPU readback.
	int Open(const MTVideoExportParams& params, const TCHAR* pFFmpegExe = NULL);

	// pipelined frame submission:
	//   AcquireBuffer - get a free frame slot (blocks until one frees; -1 on error)
	//   BufferPtr     - the slot's pixel buffer (BGRA, width*height*4 bytes)
	//   SubmitBuffer  - hand the filled slot to the writer thread (non-blocking)
	int AcquireBuffer();
	unsigned char* BufferPtr(int slot);
	void SubmitBuffer(int slot);

	int Close();   // drain the queue, close stdin, wait for ffmpeg (returns its exit code)
	bool IsOpen() const { return (m_hProc != NULL); }

	// copy the tail of ffmpeg's captured stderr (the actual failure reason) into
	// pBuf. Valid after Close(). Returns false if there is nothing to report.
	bool GetErrorTail(TCHAR* pBuf, int bufChars);

private:
	void _WriterLoop();
	bool _WriteAll(const unsigned char* p, size_t bytes);

	HANDLE m_hProc;
	HANDLE m_hStdinWr;
	size_t m_FrameBytes;
	TCHAR  m_LogPath[MAX_PATH];   // ffmpeg stderr capture file ('\0' if none)

	std::vector<std::vector<unsigned char> > m_Buffers;
	std::queue<int> m_Free;
	std::queue<int> m_Ready;
	std::mutex m_Mtx;
	std::condition_variable m_CvFree;
	std::condition_variable m_CvReady;
	std::thread m_Writer;
	bool m_Stop;
	bool m_Error;
};
