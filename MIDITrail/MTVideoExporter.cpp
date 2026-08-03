//******************************************************************************
//
// MIDITrail / MTVideoExporter
//
//******************************************************************************

#include "StdAfx.h"
#include "MTVideoExporter.h"
#include <stdio.h>
#include <map>

//==============================================================================
// codec helpers
//==============================================================================
bool MTVideoCodecIsAlpha(MTVideoCodec codec)
{
	return (codec == MTVC_QTRLE_ALPHA)
	    || (codec == MTVC_PRORES4444_ALPHA)
	    || (codec == MTVC_FFV1_ALPHA);
}

const TCHAR* MTVideoCodecExt(MTVideoCodec codec)
{
	switch (codec) {
		case MTVC_QTRLE_ALPHA:      return _T(".mov");
		case MTVC_PRORES4444_ALPHA: return _T(".mov");
		case MTVC_FFV1_ALPHA:       return _T(".mkv");
		default:                    return _T(".mp4");
	}
}


//==============================================================================
// MTTempoMap
//==============================================================================
MTTempoMap::MTTempoMap()
{
}

int MTTempoMap::Build(SMSeqData* pSeqData)
{
	m_Segs.clear();
	m_TempoSegs.clear();
	m_TimeSigSegs.clear();
	if (pSeqData == NULL) return -1;

	unsigned long timeDiv = pSeqData->GetTimeDivision();
	if (timeDiv == 0) timeDiv = 480;

	SMTrack track;
	if (pSeqData->GetMergedTrack(&track) != 0) return -1;

	double curTempoUs = 500000.0;                            // 120 BPM default
	double msPerTick  = curTempoUs / ((double)timeDiv * 1000.0);
	unsigned long absTick = 0;
	double absMs = 0.0;

	Seg s0; s0.tick = 0; s0.ms = 0.0; s0.msPerTick = msPerTick;
	m_Segs.push_back(s0);

	// initial tempo (120 BPM) + time signature (4/4 at bar 1)
	TempoSeg t0; t0.tick = 0; t0.bpm = 120;
	m_TempoSegs.push_back(t0);
	TimeSigSeg ts0;
	ts0.tick = 0; ts0.num = 4; ts0.denom = 4;
	ts0.ticksPerBar = ts0.num * timeDiv * 4 / ts0.denom;
	ts0.barAtTick = 1;
	m_TimeSigSegs.push_back(ts0);

	unsigned long n = track.GetSize();
	SMEvent event;
	SMEventMeta meta;
	for (unsigned long i = 0; i < n; i++) {
		unsigned long dt = 0;
		if (track.GetDataSet(i, &dt, &event, NULL) != 0) continue;
		absTick += dt;
		absMs   += (double)dt * msPerTick;
		if (event.GetType() == SMEvent::EventMeta) {
			meta.Attach(&event);
			unsigned char mtype = meta.GetType();
			if (mtype == 0x51) {                             // set tempo
				curTempoUs = (double)meta.GetTempo();
				if (curTempoUs <= 0.0) curTempoUs = 500000.0;
				msPerTick = curTempoUs / ((double)timeDiv * 1000.0);
				Seg s; s.tick = absTick; s.ms = absMs; s.msPerTick = msPerTick;
				m_Segs.push_back(s);
				TempoSeg t; t.tick = absTick; t.bpm = meta.GetTempoBPM();
				m_TempoSegs.push_back(t);
			}
			else if (mtype == 0x58) {                        // time signature
				unsigned long num = 4, denom = 4;
				meta.GetTimeSignature(&num, &denom);
				if (num == 0) num = 4;
				if (denom == 0) denom = 4;
				const TimeSigSeg& prev = m_TimeSigSegs.back();
				unsigned long barsElapsed = (prev.ticksPerBar > 0)
					? (absTick - prev.tick) / prev.ticksPerBar : 0;
				TimeSigSeg ts;
				ts.tick = absTick; ts.num = num; ts.denom = denom;
				ts.ticksPerBar = num * timeDiv * 4 / denom;
				if (ts.ticksPerBar == 0) ts.ticksPerBar = timeDiv * 4;
				ts.barAtTick = prev.barAtTick + barsElapsed;
				m_TimeSigSegs.push_back(ts);
			}
		}
	}
	return 0;
}

void MTTempoMap::GetStateAtTick(unsigned long tick, MTPlaybackState* pOut) const
{
	if (pOut == NULL) return;
	pOut->tempoBPM = 120;
	pOut->barNo = 1;
	pOut->beatNum = 4;
	pOut->beatDenom = 4;

	// tempo: last tempo segment at or before tick
	for (size_t i = 0; i < m_TempoSegs.size(); i++) {
		if (m_TempoSegs[i].tick <= tick) pOut->tempoBPM = m_TempoSegs[i].bpm;
		else break;
	}
	// time signature + bar: last time-sig segment at or before tick
	for (size_t i = 0; i < m_TimeSigSegs.size(); i++) {
		const TimeSigSeg& s = m_TimeSigSegs[i];
		if (s.tick <= tick) {
			pOut->beatNum = s.num;
			pOut->beatDenom = s.denom;
			unsigned long bars = (s.ticksPerBar > 0) ? (tick - s.tick) / s.ticksPerBar : 0;
			pOut->barNo = s.barAtTick + bars;
		}
		else break;
	}
}

unsigned long MTTempoMap::MsToTick(double ms) const
{
	if (m_Segs.empty()) return 0;
	size_t idx = 0;
	for (size_t i = 0; i < m_Segs.size(); i++) {
		if (m_Segs[i].ms <= ms) idx = i;
		else break;
	}
	const Seg& s = m_Segs[idx];
	double tick = (double)s.tick + (ms - s.ms) / s.msPerTick;
	if (tick < 0.0) tick = 0.0;
	return (unsigned long)(tick + 0.5);
}

double MTTempoMap::TickToMs(unsigned long tick) const
{
	if (m_Segs.empty()) return 0.0;
	size_t idx = 0;
	for (size_t i = 0; i < m_Segs.size(); i++) {
		if (m_Segs[i].tick <= tick) idx = i;
		else break;
	}
	const Seg& s = m_Segs[idx];
	double ms = s.ms + (double)((long)tick - (long)s.tick) * s.msPerTick;
	if (ms < 0.0) ms = 0.0;
	return ms;
}


//==============================================================================
// MTPitchBendTimeline
//==============================================================================
int MTPitchBendTimeline::Build(SMSeqData* pSeqData)
{
	m_Chans.clear();
	if (pSeqData == NULL) return -1;

	SMTrack track;
	if (pSeqData->GetMergedTrack(&track) != 0) return -1;

	std::map<unsigned int, size_t> idx;        // (port<<8|ch) -> m_Chans index
	std::map<unsigned int, unsigned char> rpnMSB, rpnLSB;

	unsigned long absTick = 0;
	unsigned long n = track.GetSize();
	SMEvent event;
	SMEventMIDI midi;
	unsigned char port = 0;

	for (unsigned long i = 0; i < n; i++) {
		unsigned long dt = 0;
		if (track.GetDataSet(i, &dt, &event, &port) != 0) continue;
		absTick += dt;
		if (event.GetType() != SMEvent::EventMIDI) continue;

		midi.Attach(&event);
		SMEventMIDI::ChMsg msg = midi.GetChMsg();
		unsigned char ch = midi.GetChNo();
		unsigned int key = ((unsigned int)port << 8) | ch;

		if (msg == SMEventMIDI::PitchBend) {
			size_t ci;
			std::map<unsigned int, size_t>::iterator it = idx.find(key);
			if (it != idx.end()) ci = it->second;
			else {
				Chan c; c.port = port; c.ch = ch; c.bi = 0; c.si = 0;
				c.curValue = 0; c.curSens = SM_DEFAULT_PITCHBEND_SENSITIVITY;
				m_Chans.push_back(c); ci = m_Chans.size() - 1; idx[key] = ci;
			}
			BendEvt e; e.tick = absTick; e.value = midi.GetPitchBendValue();
			m_Chans[ci].bends.push_back(e);
		}
		else if (msg == SMEventMIDI::ControlChange) {
			unsigned char cc = midi.GetCCNo();
			unsigned char val = midi.GetCCValue();
			if (cc == 101) rpnMSB[key] = val;            // RPN MSB
			else if (cc == 100) rpnLSB[key] = val;       // RPN LSB
			else if (cc == 6) {                          // Data Entry MSB
				unsigned char m = (rpnMSB.find(key) != rpnMSB.end()) ? rpnMSB[key] : 0x7F;
				unsigned char l = (rpnLSB.find(key) != rpnLSB.end()) ? rpnLSB[key] : 0x7F;
				if ((m == 0) && (l == 0)) {              // RPN 0,0 = pitch-bend range
					size_t ci;
					std::map<unsigned int, size_t>::iterator it = idx.find(key);
					if (it != idx.end()) ci = it->second;
					else {
						Chan c; c.port = port; c.ch = ch; c.bi = 0; c.si = 0;
						c.curValue = 0; c.curSens = SM_DEFAULT_PITCHBEND_SENSITIVITY;
						m_Chans.push_back(c); ci = m_Chans.size() - 1; idx[key] = ci;
					}
					SensEvt e; e.tick = absTick; e.sens = val;
					m_Chans[ci].senss.push_back(e);
				}
			}
		}
	}

	Reset();
	return 0;
}

void MTPitchBendTimeline::Reset()
{
	for (size_t c = 0; c < m_Chans.size(); c++) {
		m_Chans[c].bi = 0;
		m_Chans[c].si = 0;
		m_Chans[c].curValue = 0;
		m_Chans[c].curSens = SM_DEFAULT_PITCHBEND_SENSITIVITY;
	}
}

void MTPitchBendTimeline::Apply(unsigned long tick, MTNotePitchBend* pPB)
{
	if (pPB == NULL) return;
	for (size_t c = 0; c < m_Chans.size(); c++) {
		Chan& ch = m_Chans[c];
		while ((ch.bi < ch.bends.size()) && (ch.bends[ch.bi].tick <= tick)) {
			ch.curValue = ch.bends[ch.bi].value; ch.bi++;
		}
		while ((ch.si < ch.senss.size()) && (ch.senss[ch.si].tick <= tick)) {
			ch.curSens = ch.senss[ch.si].sens; ch.si++;
		}
		pPB->SetPitchBend(ch.port, ch.ch, ch.curValue, ch.curSens);
	}
}


//==============================================================================
// MTFFmpegPipe
//==============================================================================
// frame buffer pool depth (more = more render/encode overlap, more memory)
#define MTFFMPEG_POOL_FRAMES  4

MTFFmpegPipe::MTFFmpegPipe()
{
	m_hProc = NULL;
	m_hStdinWr = NULL;
	m_FrameBytes = 0;
	m_Stop = false;
	m_Error = false;
	m_LogPath[0] = _T('\0');
}

MTFFmpegPipe::~MTFFmpegPipe()
{
	Close();
	if (m_LogPath[0] != _T('\0')) { DeleteFile(m_LogPath); m_LogPath[0] = _T('\0'); }
}

int MTFFmpegPipe::Open(const MTVideoExportParams& params, const TCHAR* pFFmpegExe)
{
	if (m_hProc != NULL) return -1;

	const TCHAR* exe = (pFFmpegExe != NULL && pFFmpegExe[0] != _T('\0')) ? pFFmpegExe : _T("ffmpeg");
	int q = params.quality;

	// per-codec output args
	TCHAR codecArgs[512] = { _T('\0') };
	switch (params.codec) {
		case MTVC_H264_NVENC:
			_sntprintf_s(codecArgs, _countof(codecArgs), _TRUNCATE,
				_T("-c:v h264_nvenc -preset p5 -rc vbr -cq %d -pix_fmt yuv420p"), q ? q : 19);
			break;
		case MTVC_HEVC_NVENC:
			_sntprintf_s(codecArgs, _countof(codecArgs), _TRUNCATE,
				_T("-c:v hevc_nvenc -preset p5 -rc vbr -cq %d -pix_fmt yuv420p"), q ? q : 22);
			break;
		case MTVC_H264_QSV:
			_sntprintf_s(codecArgs, _countof(codecArgs), _TRUNCATE,
				_T("-c:v h264_qsv -global_quality %d -pix_fmt nv12"), q ? q : 22);
			break;
		case MTVC_HEVC_QSV:
			_sntprintf_s(codecArgs, _countof(codecArgs), _TRUNCATE,
				_T("-c:v hevc_qsv -global_quality %d -pix_fmt nv12"), q ? q : 24);
			break;
		case MTVC_HEVC_CPU:
			_sntprintf_s(codecArgs, _countof(codecArgs), _TRUNCATE,
				_T("-c:v libx265 -preset medium -crf %d -pix_fmt yuv420p"), q ? q : 20);
			break;
		case MTVC_QTRLE_ALPHA:
			_sntprintf_s(codecArgs, _countof(codecArgs), _TRUNCATE,
				_T("-c:v qtrle -pix_fmt argb"));
			break;
		case MTVC_PRORES4444_ALPHA:
			_sntprintf_s(codecArgs, _countof(codecArgs), _TRUNCATE,
				_T("-c:v prores_ks -profile:v 4444 -pix_fmt yuva444p10le"));
			break;
		case MTVC_FFV1_ALPHA:
			_sntprintf_s(codecArgs, _countof(codecArgs), _TRUNCATE,
				_T("-c:v ffv1 -pix_fmt bgra"));
			break;
		case MTVC_H264_AMF:
			_sntprintf_s(codecArgs, _countof(codecArgs), _TRUNCATE,
				_T("-c:v h264_amf -quality quality -rc cqp -qp_i %d -qp_p %d -pix_fmt yuv420p"),
				q ? q : 22, q ? q : 22);
			break;
		case MTVC_HEVC_AMF:
			_sntprintf_s(codecArgs, _countof(codecArgs), _TRUNCATE,
				_T("-c:v hevc_amf -quality quality -rc cqp -qp_i %d -qp_p %d -pix_fmt yuv420p"),
				q ? q : 24, q ? q : 24);
			break;
		case MTVC_H264_CPU:
		default:
			_sntprintf_s(codecArgs, _countof(codecArgs), _TRUNCATE,
				_T("-c:v libx264 -preset medium -crf %d -pix_fmt yuv420p"), q ? q : 18);
			break;
	}

	// supersampling: stdin frames arrive at (ss x) the output size; ffmpeg downscales
	// with lanczos for clean (anti-aliased) edges before encoding.
	int ss = (params.superSample >= 2) ? params.superSample : 1;
	int inW = params.width * ss;
	int inH = params.height * ss;
	TCHAR scaleArg[80] = { _T('\0') };
	if (ss > 1) {
		_sntprintf_s(scaleArg, _countof(scaleArg), _TRUNCATE,
			_T("-vf scale=%d:%d:flags=lanczos "), params.width, params.height);
	}

	// full command line: raw BGRA frames on stdin -> encoded file
	TCHAR cmd[1024] = { _T('\0') };
	_sntprintf_s(cmd, _countof(cmd), _TRUNCATE,
		_T("\"%s\" -y -hide_banner -loglevel error ")
		_T("-f rawvideo -pixel_format bgra -video_size %dx%d -framerate %d -i - ")
		_T("%s%s \"%s\""),
		exe, inW, inH, params.fps, scaleArg, codecArgs, params.outPath);

	// stdin pipe (child reads frames); discard child stdout/stderr
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;

	HANDLE hRd = NULL, hWr = NULL;
	if (!CreatePipe(&hRd, &hWr, &sa, 0)) return -1;
	SetHandleInformation(hWr, HANDLE_FLAG_INHERIT, 0);   // keep our write end private

	// capture ffmpeg's stderr/stdout to a temp file so a failure (missing libx264,
	// odd resolution, ...) can be shown to the user instead of a generic message.
	m_LogPath[0] = _T('\0');
	{
		TCHAR tmpDir[MAX_PATH] = { _T('\0') };
		if (GetTempPath(MAX_PATH, tmpDir) > 0) {
			GetTempFileName(tmpDir, _T("mtv"), 0, m_LogPath);
		}
	}
	HANDLE hLog = INVALID_HANDLE_VALUE;
	if (m_LogPath[0] != _T('\0')) {
		hLog = CreateFile(m_LogPath, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
			&sa, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);
	}
	if (hLog == INVALID_HANDLE_VALUE) {
		// fall back to discarding the output if the temp file could not be created
		hLog = CreateFile(_T("NUL"), GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, OPEN_EXISTING, 0, NULL);
		m_LogPath[0] = _T('\0');
	}

	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdInput  = hRd;
	si.hStdOutput = hLog;
	si.hStdError  = hLog;

	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));

	BOOL ok = CreateProcess(NULL, cmd, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);

	CloseHandle(hRd);                                    // child owns the read end now
	if (hLog != INVALID_HANDLE_VALUE) CloseHandle(hLog); // child keeps its own copy

	if (!ok) {
		CloseHandle(hWr);
		return -1;
	}
	CloseHandle(pi.hThread);
	m_hProc = pi.hProcess;
	m_hStdinWr = hWr;

	// frame-buffer pool + background writer thread (overlaps ffmpeg encoding with
	// the caller's render + GPU readback)
	m_FrameBytes = (size_t)inW * (size_t)inH * 4;
	m_Stop = false;
	m_Error = false;
	m_Buffers.resize(MTFFMPEG_POOL_FRAMES);
	while (!m_Free.empty()) m_Free.pop();
	while (!m_Ready.empty()) m_Ready.pop();
	for (int i = 0; i < MTFFMPEG_POOL_FRAMES; i++) {
		m_Buffers[i].resize(m_FrameBytes);
		m_Free.push(i);
	}
	m_Writer = std::thread(&MTFFmpegPipe::_WriterLoop, this);
	return 0;
}

bool MTFFmpegPipe::_WriteAll(const unsigned char* p, size_t bytes)
{
	if (m_hStdinWr == NULL) return false;
	size_t off = 0;
	while (off < bytes) {
		size_t remain = bytes - off;
		DWORD chunk = (remain > (1u << 20)) ? (1u << 20) : (DWORD)remain;
		DWORD wrote = 0;
		if (!WriteFile(m_hStdinWr, p + off, chunk, &wrote, NULL)) return false;
		if (wrote == 0) return false;
		off += wrote;
	}
	return true;
}

void MTFFmpegPipe::_WriterLoop()
{
	for (;;) {
		int slot;
		{
			std::unique_lock<std::mutex> lk(m_Mtx);
			m_CvReady.wait(lk, [this]{ return !m_Ready.empty() || m_Stop; });
			if (m_Ready.empty()) break;          // stopped and drained
			slot = m_Ready.front();
			m_Ready.pop();
		}
		bool ok = _WriteAll(&m_Buffers[slot][0], m_FrameBytes);
		{
			std::lock_guard<std::mutex> lk(m_Mtx);
			if (!ok) m_Error = true;
			m_Free.push(slot);
		}
		m_CvFree.notify_one();
	}
}

int MTFFmpegPipe::AcquireBuffer()
{
	std::unique_lock<std::mutex> lk(m_Mtx);
	m_CvFree.wait(lk, [this]{ return !m_Free.empty() || m_Error; });
	if (m_Error) return -1;
	int slot = m_Free.front();
	m_Free.pop();
	return slot;
}

unsigned char* MTFFmpegPipe::BufferPtr(int slot)
{
	if (slot < 0 || slot >= (int)m_Buffers.size()) return NULL;
	return &m_Buffers[slot][0];
}

void MTFFmpegPipe::SubmitBuffer(int slot)
{
	{
		std::lock_guard<std::mutex> lk(m_Mtx);
		m_Ready.push(slot);
	}
	m_CvReady.notify_one();
}

int MTFFmpegPipe::Close()
{
	int exitCode = -1;

	// signal the writer to drain + exit, then join it
	if (m_Writer.joinable()) {
		{
			std::lock_guard<std::mutex> lk(m_Mtx);
			m_Stop = true;
		}
		m_CvReady.notify_all();
		m_Writer.join();
	}

	if (m_hStdinWr != NULL) { CloseHandle(m_hStdinWr); m_hStdinWr = NULL; }
	if (m_hProc != NULL) {
		WaitForSingleObject(m_hProc, INFINITE);
		DWORD code = 0;
		if (GetExitCodeProcess(m_hProc, &code)) exitCode = (int)code;
		CloseHandle(m_hProc);
		m_hProc = NULL;
	}
	return exitCode;
}

bool MTFFmpegPipe::GetErrorTail(TCHAR* pBuf, int bufChars)
{
	if ((pBuf == NULL) || (bufChars <= 0)) return false;
	pBuf[0] = _T('\0');
	if (m_LogPath[0] == _T('\0')) return false;

	HANDLE h = CreateFile(m_LogPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, 0, NULL);
	if (h == INVALID_HANDLE_VALUE) return false;

	LARGE_INTEGER sz; sz.QuadPart = 0;
	GetFileSizeEx(h, &sz);
	const DWORD MAXR = 1500;
	DWORD toRead = (sz.QuadPart > (LONGLONG)MAXR) ? MAXR : (DWORD)sz.QuadPart;
	if (toRead == 0) { CloseHandle(h); return false; }

	// read only the tail (the last error lines are what matters)
	LARGE_INTEGER ofs; ofs.QuadPart = sz.QuadPart - (LONGLONG)toRead;
	SetFilePointerEx(h, ofs, NULL, FILE_BEGIN);

	char raw[1501];
	DWORD got = 0;
	BOOL rok = ReadFile(h, raw, toRead, &got, NULL);
	CloseHandle(h);
	if (!rok || (got == 0)) return false;
	raw[got] = '\0';

#ifdef UNICODE
	MultiByteToWideChar(CP_UTF8, 0, raw, -1, pBuf, bufChars);
	pBuf[bufChars - 1] = _T('\0');
#else
	strncpy_s(pBuf, bufChars, raw, _TRUNCATE);
#endif
	return (pBuf[0] != _T('\0'));
}
