//******************************************************************************
//
// MIDITrail / MTKeyboard11
//
// DX11 piano keyboard - faithful port of the DX9 MTPianoKeyboardMod +
// MTPianoKeyboardCtrlMod (M3 / M4.6 multi-keyboard).
//
//******************************************************************************

#include "stdafx.h"
#include "YNBaseLib.h"
#include "MTKeyboard11.h"
#include "DXTexture11.h"
#include <stdlib.h>

using namespace YNBaseLib;
using namespace DirectX;


MTKeyboard11::MTKeyboard11()
{
	m_pSRV = NULL;
	m_pPitchBend = NULL;
	m_SingleKbd = true;
	m_LiveMode = false;
	m_TickPerMs = 0.96;   // ~120bpm @ 480tpqn until the app feeds the real tempo scale
	m_Ready = false;
	m_CurTickTime = 0;
	m_WorldMove = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_pBaseVerts = NULL;
	m_VertexNum = 0;
	m_NumKbd = 0;
	m_InfiniteKbd = false;          //ced 20260629
	m_OctaveWidthX = 0.0f;
	m_pExtVerts = NULL;             //ced 20260703
	m_pExtIdx = NULL;
	m_ExtVertNum = 0;
	m_ExtIdxNum = 0;
	m_ExtBottomIdxNum = 0;
	m_LastAnimMs = 0;
	m_KeyDownDurMs = 40;   // DX9 default; overridden from the design in Create
	m_KeyUpDurMs = 40;
	for (unsigned long i = 0; i < MTKBD11_MAX_KEYBOARDS; i++) {
		m_Subs[i].pWorkVerts = NULL;
		m_Subs[i].keyboardIndex = 0;
		m_Subs[i].portNo = 0;
		m_Subs[i].dirty = false;
		m_Subs[i].pNotes = NULL;
		m_Subs[i].noteCount = 0;
		m_Subs[i].lastTick = 0;
		ZeroMemory(m_Subs[i].keyOffset, sizeof(m_Subs[i].keyOffset));
		ZeroMemory(m_Subs[i].keyCursor, sizeof(m_Subs[i].keyCursor));
		ZeroMemory(m_Subs[i].keyDown, sizeof(m_Subs[i].keyDown));
		ZeroMemory(m_Subs[i].keyRate, sizeof(m_Subs[i].keyRate));
		ZeroMemory(m_Subs[i].keyColor, sizeof(m_Subs[i].keyColor));
		ZeroMemory(m_Subs[i].keyRenderedColor, sizeof(m_Subs[i].keyRenderedColor));
		ZeroMemory(m_Subs[i].keyMaxEndTick, sizeof(m_Subs[i].keyMaxEndTick));
	}
}

MTKeyboard11::~MTKeyboard11()
{
	Release();
}

void MTKeyboard11::_ReleaseSub(SubKbd* pSub)
{
	pSub->prim.Release();
	if (pSub->pWorkVerts != NULL) { free(pSub->pWorkVerts); pSub->pWorkVerts = NULL; }
	if (pSub->pNotes != NULL) { free(pSub->pNotes); pSub->pNotes = NULL; }
	pSub->noteCount = 0;
	pSub->lastTick = 0;
	pSub->dirty = false;
	ZeroMemory(pSub->keyOffset, sizeof(pSub->keyOffset));
	ZeroMemory(pSub->keyCursor, sizeof(pSub->keyCursor));
	ZeroMemory(pSub->keyDown, sizeof(pSub->keyDown));
	ZeroMemory(pSub->keyRate, sizeof(pSub->keyRate));
	ZeroMemory(pSub->keyColor, sizeof(pSub->keyColor));
	ZeroMemory(pSub->keyRenderedColor, sizeof(pSub->keyRenderedColor));
	ZeroMemory(pSub->keyMaxEndTick, sizeof(pSub->keyMaxEndTick));
}

void MTKeyboard11::Release()
{
	for (unsigned long i = 0; i < MTKBD11_MAX_KEYBOARDS; i++) _ReleaseSub(&m_Subs[i]);
	m_NumKbd = 0;
	if (m_pSRV != NULL) { m_pSRV->Release(); m_pSRV = NULL; }
	if (m_pBaseVerts != NULL) { free(m_pBaseVerts); m_pBaseVerts = NULL; }
	if (m_pExtVerts != NULL) { free(m_pExtVerts); m_pExtVerts = NULL; }   //ced 20260703
	if (m_pExtIdx != NULL) { free(m_pExtIdx); m_pExtIdx = NULL; }
	m_ExtVertNum = 0;
	m_ExtIdxNum = 0;
	m_ExtBottomIdxNum = 0;
	m_InfiniteKbd = false;          //ced 20260629
	m_VertexNum = 0;
	m_CurTickTime = 0;
	m_Ready = false;
}

void MTKeyboard11::Reset()
{
	m_CurTickTime = 0;
	for (unsigned long i = 0; i < m_NumKbd; i++) {
		m_Subs[i].lastTick = 0;
		// rewind every key's note cursor to the start of its block (a seek can go back)
		for (unsigned long k = 0; k < SM_MAX_NOTE_NUM; k++) m_Subs[i].keyCursor[k] = m_Subs[i].keyOffset[k];
		ZeroMemory(m_Subs[i].keyMaxEndTick, sizeof(m_Subs[i].keyMaxEndTick));
		// Force every key to be rebuilt on the next draw (a seek/stop must not leave a
		// key visually stuck). The work vertices still hold the pre-seek pressed
		// geometry, so we set keyRate to an impossible sentinel: _ApplyKeyStates then
		// sees rate != keyRate for every key and rebuilds it (to base if released, or
		// to the new pressed state) instead of skipping it as "unchanged".
		for (unsigned char k = 0; k < SM_MAX_NOTE_NUM; k++) m_Subs[i].keyRate[k] = -1.0f;
		m_Subs[i].dirty = true;   // release all pressed keys on the next draw
	}
}

//******************************************************************************
// Live monitor key presses (single keyboard = sub 0)
//******************************************************************************
void MTKeyboard11::SetNoteOnLive(unsigned char portNo, unsigned char chNo, unsigned char noteNo)
{
	if ((m_NumKbd == 0) || (noteNo >= SM_MAX_NOTE_NUM)) return;
	// key stays down until note-off (m_CurTickTime is 0 in live, so any non-zero
	// keyMaxEndTick reads as "down" in _ApplyKeyStates).
	m_Subs[0].keyMaxEndTick[noteNo] = 0xFFFFFFFF;
	m_Subs[0].keyColor[noteNo] = (unsigned long)m_NoteDesign.GetNoteBoxColor(portNo, chNo, noteNo);
	m_Subs[0].dirty = true;
}

void MTKeyboard11::SetNoteOffLive(unsigned char noteNo)
{
	if ((m_NumKbd == 0) || (noteNo >= SM_MAX_NOTE_NUM)) return;
	m_Subs[0].keyMaxEndTick[noteNo] = 0;
	m_Subs[0].dirty = true;
}

void MTKeyboard11::AllNoteOffLive()
{
	if (m_NumKbd == 0) return;
	ZeroMemory(m_Subs[0].keyMaxEndTick, sizeof(m_Subs[0].keyMaxEndTick));
	m_Subs[0].dirty = true;
}

//******************************************************************************
// Current playback tick: advance every keyboard's active-note window
//******************************************************************************
void MTKeyboard11::SetCurTickTime(unsigned long curTickTime)
{
	m_CurTickTime = curTickTime;
	for (unsigned long i = 0; i < m_NumKbd; i++) {
		_AdvanceWindow(&m_Subs[i], curTickTime);
		m_Subs[i].dirty = true;
	}
}

//******************************************************************************
// Track the playback position. Per-key note scanning (the envelope) is done in
// _ApplyKeyStates straight from the per-key blocks + cursor; here we only rewind
// the cursors on a genuine backward seek so the scan can re-find the notes.
//******************************************************************************
void MTKeyboard11::_AdvanceWindow(SubKbd* pSub, unsigned long tick)
{
	// Only a genuine backward seek rewinds the cursors (live tick jitters back a few
	// ticks; rewinding every frame in a dense section would stall the keyboard).
	#define MTKBD11_SEEK_BACK_TICKS  (1920)   // ~a few beats
	if (tick + MTKBD11_SEEK_BACK_TICKS < pSub->lastTick) {
		for (unsigned long k = 0; k < SM_MAX_NOTE_NUM; k++) pSub->keyCursor[k] = pSub->keyOffset[k];
		ZeroMemory(pSub->keyMaxEndTick, sizeof(pSub->keyMaxEndTick));
		pSub->lastTick = tick;
	}
	else if (tick > pSub->lastTick) {
		pSub->lastTick = tick;
	}
}

//******************************************************************************
// Apply pending key-press changes to one keyboard's vertex buffer
//******************************************************************************
int MTKeyboard11::_ApplyKeyStates(ID3D11DeviceContext* pContext, SubKbd* pSub, unsigned long elapsedMs)
{
	int result = 0;
	bool changed = false;
	bool animating = false;   // any key still easing toward its target this frame
	unsigned char note;
	DXP11_VERTEX* pBase = (DXP11_VERTEX*)m_pBaseVerts;
	DXP11_VERTEX* pWork = (DXP11_VERTEX*)pSub->pWorkVerts;

	if (!pSub->dirty) return 0;
	if ((pBase == NULL) || (pWork == NULL)) { pSub->dirty = false; return 0; }

	// LIVE ease steps (wall-clock); PLAYBACK envelope durations (ticks at song tempo)
	float downStep = (m_KeyDownDurMs > 0) ? ((float)elapsedMs / (float)m_KeyDownDurMs) : 1.0f;
	float upStep   = (m_KeyUpDurMs   > 0) ? ((float)elapsedMs / (float)m_KeyUpDurMs)   : 1.0f;
	double downTicks = (double)m_KeyDownDurMs * m_TickPerMs;
	double upTicks   = (double)m_KeyUpDurMs   * m_TickPerMs;
	bool trackMode = m_NoteDesign.IsTrackColorMode();   // CHANNELTRACK: keep track colours

	for (note = 0; note < SM_MAX_NOTE_NUM; note++) {
		float rate = 0.0f;
		unsigned long useColor = pSub->keyColor[note];

		if (m_LiveMode) {
			// real-time: ease toward down/up (note-on sets keyMaxEndTick, m_CurTickTime=0)
			bool wantDown = (pSub->keyMaxEndTick[note] > m_CurTickTime);
			float target = wantDown ? 1.0f : 0.0f;
			rate = pSub->keyRate[note];
			if (rate < target)      { rate += downStep; if (rate > target) rate = target; }
			else if (rate > target) { rate -= upStep;   if (rate < target) rate = target; }
			if (rate != target) animating = true;
		}
		else {
			// PLAYBACK: DX9 anticipatory envelope evaluated over this key's note block.
			// The cursor skips notes whose release tail has fully passed; we then scan
			// forward over every note in the active window (start <= cur + downTicks) and
			// take the strongest (max-rate). No per-key cap -> a hammered key keeps its
			// sounding note (rate=1) however dense the stream.
			double cur = (double)m_CurTickTime;
			unsigned long lo = pSub->keyOffset[note];
			unsigned long hi = pSub->keyOffset[note + 1];
			unsigned long c = pSub->keyCursor[note];
			if (c < lo) c = lo;
			while (c < hi && (double)pSub->pNotes[c].endTime + upTicks < cur) c++;
			pSub->keyCursor[note] = c;
			unsigned char bestCh = 0;
			for (unsigned long j = c; j < hi; j++) {
				double s = (double)pSub->pNotes[j].startTime;
				if (s > cur + downTicks) break;   // beyond the anticipatory horizon (sorted by start)
				double e = (double)pSub->pNotes[j].endTime;
				float er;
				if (cur < s) {            // anticipatory press-down toward the note onset
					double d = s - cur;
					er = (d >= downTicks) ? 0.0f : (float)(1.0 - d / downTicks);
				}
				else if (cur <= e) er = 1.0f;                                  // held down
				else {                    // release ramp after note-off
					double d = cur - e;
					er = (d >= upTicks) ? 0.0f : (float)(1.0 - d / upTicks);
				}
				// >= so that among equally-pressed (overlapping) notes the LAST one in
				// the block - i.e. the most recent note-on - wins, layering the newest
				// note's colour on top (matches the previous behaviour). rate is the max
				// either way, so the press animation is unaffected.
				if (er >= rate) {
					rate = er;
					useColor = pSub->pNotes[j].color;
					bestCh = pSub->pNotes[j].chNo;
				}
			}
			// pressed-key colour: only the fully-pressed key is coloured. In track
			// colour mode (CHANNELTRACK) keep the per-track note colour; otherwise use
			// the [PianoKeyboard] ActiveKeyColor palette/type. elapsedTime = 0 so the
			// colour appears at full immediately on note-on (no Duration/TailRate
			// fade-in) and stays constant while held.
			if ((rate >= 1.0f) && !trackMode) {
				D3DXCOLOR noteCol((D3DCOLOR)useColor);
				useColor = (unsigned long)(D3DCOLOR)m_DesignMod.GetActiveKeyColor(bestCh, note, 0, &noteCol);
			}
		}

		// rebuild only when the rendered press depth or colour actually changes
		bool colorChanged = (rate > 0.0f) && (useColor != pSub->keyRenderedColor[note]);
		bool rateChanged  = (rate != pSub->keyRate[note]);
		if (!rateChanged && !colorChanged) continue;

		unsigned long pos = 0, num = 0;
		m_Geom.GetKeyVertexRange(note, &pos, &num);
		if (num == 0) { pSub->keyRate[note] = rate; continue; }

		if (rate >= 1.0f) {
			// fully pressed (note sounding): tilt + tint with the note colour
			D3DXCOLOR col((D3DCOLOR)useColor);
			m_Geom.BuildKeyCPU(note, rate, &col, &pWork[pos]);
			pSub->keyRenderedColor[note] = useColor;
			pSub->keyColor[note] = useColor;
		}
		else if (rate > 0.0f) {
			// DX9 only colours a fully-pressed key; during the down/up ramps the key
			// just rotates and stays the neutral key colour (NULL). So the note colour
			// snaps on exactly at note onset and off at note-off, not during the ramp.
			m_Geom.BuildKeyCPU(note, rate, NULL, &pWork[pos]);
			pSub->keyRenderedColor[note] = 0xFFFFFFFF;   // neutral; recolour when it reaches full press
		}
		else {
			memcpy(&pWork[pos], &pBase[pos], (size_t)num * sizeof(DXP11_VERTEX));
			pSub->keyRenderedColor[note] = 0xFFFFFFFF;   // force a colour rebuild on the next press
		}
		pSub->keyRate[note] = rate;
		pSub->keyDown[note] = (rate > 0.0f);
		changed = true;
	}

	if (changed) {
		DXP11_VERTEX* pv = NULL;
		result = pSub->prim.LockVertex(pContext, &pv);
		if (result == 0) {
			// whole-buffer DISCARD map: copy main + the static extension tail (infinite kbd)
			unsigned long total = m_VertexNum + (m_InfiniteKbd ? m_ExtVertNum : 0);
			memcpy(pv, pWork, (size_t)total * sizeof(DXP11_VERTEX));
			pSub->prim.UnlockVertex(pContext);
		}
	}

	// keep the keyboard "dirty" while any key is still easing so DrawDX11 keeps
	// advancing the animation on the following frames (playback also re-dirties it
	// every tick; live mode relies on this to finish the ease between note events).
	pSub->dirty = animating;
	return result;
}

//******************************************************************************
// Create: generate the shared geometry/texture, then one keyboard per port
//******************************************************************************
int MTKeyboard11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData,
		bool isSingleKeyboard
	)
{
	int result = 0;
	unsigned long vn = 0;
	unsigned long in = 0;
	void* pCpuVB = NULL;
	unsigned long* pCpuIB = NULL;
	D3DXVECTOR3 mv;
	TCHAR texPath[_MAX_PATH] = { _T('\0') };
	unsigned int tw = 0, th = 0;
	SMPortList portList;
	int keyboardIndexForPort[SM_MAX_PORT_NUM];
	unsigned long i = 0;
	unsigned long maxDisp = 0;

	Release();

	result = DXPrimitive11::InitPipeline(pDevice);
	if (result != 0) goto EXIT;

	// designs (device-free)
	result = m_Geom.InitForDX11(pSceneName, pSeqData);
	if (result != 0) goto EXIT;
	result = m_DesignMod.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;
	result = m_NoteDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	// key-press animation durations (DX9 [Keyboard] KeyDownDuration/KeyUpDuration, ms)
	m_KeyDownDurMs = m_DesignMod.GetKeyDownDuration();
	m_KeyUpDurMs   = m_DesignMod.GetKeyUpDuration();
	m_LastAnimMs   = timeGetTime();
	m_LiveMode     = (pSeqData == NULL);   // live monitor: no song -> wall-clock ease

	mv = m_NoteDesign.GetWorldMoveVector();
	m_WorldMove = XMFLOAT3(mv.x, mv.y, mv.z);

	//----------------------------------
	// keyboard count + port -> keyboard index map.
	// single: all ports/channels share ONE centered keyboard (SetKeyboardSingle
	// makes GetKeyboardBasePos(0) center it). multi: one keyboard per active port,
	// stacked (the original DX9 Mod behavior). Toggled from the View menu.
	//----------------------------------
	m_SingleKbd = isSingleKeyboard;
	for (i = 0; i < SM_MAX_PORT_NUM; i++) keyboardIndexForPort[i] = -1;

	if (isSingleKeyboard) {
		m_DesignMod.SetKeyboardSingle();
		for (i = 0; i < SM_MAX_PORT_NUM; i++) keyboardIndexForPort[i] = 0;  //all ports -> keyboard 0
		m_Subs[0].keyboardIndex = 0;
		m_Subs[0].portNo = 0;
		m_NumKbd = 1;
	}
	else {
		result = pSeqData->GetPortList(&portList);
		if (result != 0) goto EXIT;
		maxDisp = m_DesignMod.GetKeyboardMaxDispNum();
		m_NumKbd = 0;
		for (i = 0; i < portList.GetSize(); i++) {
			unsigned char portNo = 0;
			portList.GetPort(i, &portNo);
			if (portNo >= SM_MAX_PORT_NUM) continue;
			keyboardIndexForPort[portNo] = (int)m_NumKbd;
			m_Subs[m_NumKbd].keyboardIndex = (int)m_NumKbd;
			m_Subs[m_NumKbd].portNo = (int)portNo;
			m_NumKbd++;
			if (m_NumKbd >= maxDisp) break;
			if (m_NumKbd >= MTKBD11_MAX_KEYBOARDS) break;
		}
		if (m_NumKbd == 0) {   // no ports listed: fall back to a single keyboard
			keyboardIndexForPort[0] = 0;
			m_Subs[0].keyboardIndex = 0;
			m_Subs[0].portNo = 0;
			m_NumKbd = 1;
		}
	}

	//----------------------------------
	// shared geometry (one master CPU copy + the index buffer data)
	//----------------------------------
	m_Geom.GetGeometrySize(&vn, &in);
	if ((vn == 0) || (in == 0)) { result = 0; goto EXIT; }

	pCpuVB = malloc((size_t)vn * sizeof(DXP11_VERTEX));
	pCpuIB = (unsigned long*)malloc((size_t)in * sizeof(unsigned long));
	if ((pCpuVB == NULL) || (pCpuIB == NULL)) { result = YN_SET_ERR("Could not allocate memory.", 0, 0); goto EXIT; }

	result = m_Geom.BuildGeometryCPU(pCpuVB, pCpuIB);
	if (result != 0) goto EXIT;

	m_VertexNum = vn;
	m_pBaseVerts = malloc((size_t)vn * sizeof(DXP11_VERTEX));
	if (m_pBaseVerts == NULL) { result = YN_SET_ERR("Could not allocate memory.", 0, 0); goto EXIT; }
	memcpy(m_pBaseVerts, pCpuVB, (size_t)vn * sizeof(DXP11_VERTEX));

	//ced 20260703: 無限鍵盤（NotLive box 2D/3D）。拡張オクターブを CPU 上に用意しておき、下の
	//ループで各キーボードの頂点/インデックスバッファの「末尾に連結」する。別バッファ・別 Draw を
	//作らず、通常の 0-127 と全く同じ 1 回の Draw・同じワールド行列・同じ送信順で描かれるため、
	//どの GPU でも通常鍵盤と同一の見た目になる。拡張部は静的（_ApplyKeyStates は 0-127 のみ触る）。
	m_InfiniteKbd = (m_DesignMod.IsInfiniteKeyboard() && !m_LiveMode);
	if (m_InfiniteKbd) {
		if (_BuildExtCPU(pCpuVB, pCpuIB) != 0) {
			if (m_pExtVerts != NULL) { free(m_pExtVerts); m_pExtVerts = NULL; }
			if (m_pExtIdx != NULL) { free(m_pExtIdx); m_pExtIdx = NULL; }
			m_ExtVertNum = 0;
			m_ExtIdxNum = 0;
			m_InfiniteKbd = false;
		}
	}

	// per-keyboard GPU buffers + CPU work mirror. When the infinite keyboard is on, each
	// buffer is (main 0-127) followed by (extension octaves) so a single Draw covers both.
	{
		unsigned long extV = m_InfiniteKbd ? m_ExtVertNum : 0;
		unsigned long extI = m_InfiniteKbd ? m_ExtIdxNum : 0;
		for (i = 0; i < m_NumKbd; i++) {
			DXP11_VERTEX* pv = NULL;
			unsigned long* pi = NULL;
			result = m_Subs[i].prim.CreateVertexBuffer(pDevice, vn + extV);
			if (result != 0) goto EXIT;
			result = m_Subs[i].prim.CreateIndexBuffer(pDevice, in + extI);
			if (result != 0) goto EXIT;
			result = m_Subs[i].prim.LockVertex(pContext, &pv);
			if (result != 0) goto EXIT;
			memcpy(pv, pCpuVB, (size_t)vn * sizeof(DXP11_VERTEX));
			if (extV > 0) memcpy(pv + vn, m_pExtVerts, (size_t)extV * sizeof(DXP11_VERTEX));
			m_Subs[i].prim.UnlockVertex(pContext);
			result = m_Subs[i].prim.LockIndex(pContext, &pi);
			if (result != 0) goto EXIT;
			if (extI > 0) {
				// Submit the whole keyboard in TRUE note order so the semi-transparent keys blend
				// (and any coplanar depth tie resolves) exactly the same at the seams as in the
				// interior: [below-note-0 extension] -> [main 0-127] -> [note-128+ extension].
				// Extension indices are 0-based within the extension vertex block -> shift by vn.
				unsigned long botI = m_ExtBottomIdxNum;   // below-0 index count
				unsigned long pos = 0;
				for (unsigned long e = 0; e < botI; e++) pi[pos++] = m_pExtIdx[e] + vn;      // below 0
				memcpy(pi + pos, pCpuIB, (size_t)in * sizeof(unsigned long)); pos += in;      // main 0-127
				for (unsigned long e = botI; e < extI; e++) pi[pos++] = m_pExtIdx[e] + vn;    // note 128+
			}
			else {
				memcpy(pi, pCpuIB, (size_t)in * sizeof(unsigned long));
			}
			m_Subs[i].prim.UnlockIndex(pContext);
			m_Subs[i].prim.SetMaterialAmbient(0.55f, 0.55f, 0.55f);

			// CPU work mirror holds the FULL buffer (main + static extension tail); the
			// extension part is never rewritten, so a whole-buffer DISCARD upload keeps it.
			m_Subs[i].pWorkVerts = malloc((size_t)(vn + extV) * sizeof(DXP11_VERTEX));
			if (m_Subs[i].pWorkVerts == NULL) { result = YN_SET_ERR("Could not allocate memory.", 0, 0); goto EXIT; }
			memcpy(m_Subs[i].pWorkVerts, pCpuVB, (size_t)vn * sizeof(DXP11_VERTEX));
			if (extV > 0) memcpy((DXP11_VERTEX*)m_Subs[i].pWorkVerts + vn, m_pExtVerts, (size_t)extV * sizeof(DXP11_VERTEX));
		}
	}

	//----------------------------------
	// notes -> per-keyboard compact arrays (routed by port), color precomputed.
	// Live monitor (pSeqData == NULL): no song -> skip; keys are driven directly
	// by SetNoteOnLive/OffLive, so the per-keyboard note arrays stay empty.
	//----------------------------------
	if (pSeqData != NULL) {
		SMNote note;
		unsigned long counts[MTKBD11_MAX_KEYBOARDS];
		unsigned long total = 0;
		// track color mode: keep each note's source track so the pressed-key color
		// (ActiveKeyColorType=NOTE) matches the track-channel note color.
		bool trackMode = m_NoteDesign.IsTrackColorMode();
		SMNoteList* pNotes = NULL;
		const unsigned char* pTrackNo = NULL;

		for (i = 0; i < MTKBD11_MAX_KEYBOARDS; i++) counts[i] = 0;

		if (trackMode) {
			// shared, cached note list + per-note source track (built once)
			result = pSeqData->GetMergedNoteListWithTrack(&pNotes, &pTrackNo);
			if (result != 0) goto EXIT;
		}
		else {
			// shared, cached merged note list (built once across all components)
			result = pSeqData->GetMergedNoteList(&pNotes);
			if (result != 0) goto EXIT;
		}
		total = pNotes->GetSize();

		// pass 1: count notes per (keyboard, noteNo). keyOffset[n+1] becomes the per-key
		// block size, then a prefix sum turns it into the block start offset.
		for (i = 0; i < total; i++) {
			if (pNotes->GetNote(i, &note) != 0) { result = YN_SET_ERR("Program error.", i, 0); goto EXIT; }
			if (note.portNo >= SM_MAX_PORT_NUM) continue;
			if (note.noteNo >= SM_MAX_NOTE_NUM) continue;
			int kbd = keyboardIndexForPort[note.portNo];
			if (kbd < 0) continue;
			counts[kbd]++;
			m_Subs[kbd].keyOffset[note.noteNo + 1]++;
		}
		// alloc + prefix-sum the per-key block offsets; keyCursor doubles as the fill pos
		for (i = 0; i < m_NumKbd; i++) {
			m_Subs[i].noteCount = counts[i];
			if (counts[i] > 0) {
				m_Subs[i].pNotes = (KbdNote*)malloc((size_t)counts[i] * sizeof(KbdNote));
				if (m_Subs[i].pNotes == NULL) { result = YN_SET_ERR("Could not allocate memory.", 0, 0); goto EXIT; }
			}
			m_Subs[i].keyOffset[0] = 0;
			for (unsigned long k = 0; k < SM_MAX_NOTE_NUM; k++)
				m_Subs[i].keyOffset[k + 1] += m_Subs[i].keyOffset[k];
			for (unsigned long k = 0; k < SM_MAX_NOTE_NUM; k++)
				m_Subs[i].keyCursor[k] = m_Subs[i].keyOffset[k];   // running fill position
		}
		// pass 2: place each note into its key's block (the list is start-sorted, so each
		// block also comes out start-sorted)
		for (i = 0; i < total; i++) {
			if (pNotes->GetNote(i, &note) != 0) { result = YN_SET_ERR("Program error.", i, 0); goto EXIT; }
			if (note.portNo >= SM_MAX_PORT_NUM) continue;
			if (note.noteNo >= SM_MAX_NOTE_NUM) continue;
			int kbd = keyboardIndexForPort[note.portNo];
			if (kbd < 0) continue;
			KbdNote* pn = &m_Subs[kbd].pNotes[m_Subs[kbd].keyCursor[note.noteNo]++];
			pn->startTime = note.startTime;
			pn->endTime   = note.endTime;
			pn->color     = trackMode
				? (D3DCOLOR)m_NoteDesign.GetTrackChannelColor(pTrackNo[i], note.chNo)
				: (D3DCOLOR)m_NoteDesign.GetNoteBoxColor(note.portNo, note.chNo, note.noteNo);
			pn->noteNo    = note.noteNo;
			pn->chNo      = note.chNo;
		}
		// rewind the fill cursors back to each block's start for playback scanning
		for (i = 0; i < m_NumKbd; i++)
			for (unsigned long k = 0; k < SM_MAX_NOTE_NUM; k++)
				m_Subs[i].keyCursor[k] = m_Subs[i].keyOffset[k];
	}

	// keyboard texture (HDKeyboard.png) via WIC, shared across keyboards
	if (m_Geom.GetTexturePath(pSceneName, texPath, _MAX_PATH) == 0) {
		DXTexture11::LoadFromFile(pDevice, texPath, &m_pSRV, &tw, &th);
	}

	m_Ready = true;

EXIT:;
	if (pCpuVB != NULL) free(pCpuVB);
	if (pCpuIB != NULL) free(pCpuIB);
	return result;
}

//******************************************************************************
// ced 20260703: build the infinite-keyboard extension octaves into CPU arrays (m_pExtVerts /
// m_pExtIdx), each octave's X-offset BAKED into its vertex positions. Create() then APPENDS
// these to every keyboard's own vertex/index buffer, so the extension is drawn by the same
// single Draw call, in the same primitive submission order, as the original 0-127 keys ->
// there is no separate buffer and no second draw, hence no cross-draw z-fight and it looks
// identical to the main keyboard on every GPU.
//
// Keys are emitted in NOTE ORDER (per octave: pitch class 0..11 = C,C#,D,...,B), exactly the
// order BuildGeometryCPU emits the main keyboard, so the black keys (which are physically
// raised) resolve in front the same way they do for the original keys. Indices are 0-based
// within the extension block; Create() shifts them by the main vertex count on upload.
//
// Octave k renders notes 12k..12k+11 from the interior octave source (notes 12-23) with
// pos.x += (k-1)*octaveWidth so note 12 lands on posX(12k). The top gap (notes 128-131) is
// the interior keys G#,A,A#,B (pitch classes 8-11) copied with +9*octaveWidth.
//******************************************************************************
int MTKeyboard11::_BuildExtCPU(const void* pCpuVB, const unsigned long* pCpuIB)
{
	const int EXT_OCTAVES = 24;                 // octaves extended each direction (>= any zoom)
	const DXP11_VERTEX* srcV = (const DXP11_VERTEX*)pCpuVB;
	int p = 0, j = 0;
	unsigned long i = 0;

	// per-key source ranges for the interior octave (notes 12-23, indexed by pitch class 0-11)
	unsigned long kv0[12], kvn[12], ki0[12], kin[12];
	for (p = 0; p < 12; p++) {
		m_Geom.GetKeyVertexRange((unsigned char)(12 + p), &kv0[p], &kvn[p]);
		m_Geom.GetKeyIndexRange((unsigned char)(12 + p), &ki0[p], &kin[p]);
	}
	unsigned long vp24 = 0, vn24 = 0;
	m_Geom.GetKeyVertexRange(24, &vp24, &vn24);
	m_OctaveWidthX = srcV[vp24].pos[0] - srcV[kv0[0]].pos[0];   // note 24 (C) - note 12 (C)
	if ((m_OctaveWidthX <= 0.0f) || (kvn[0] == 0)) return -1;

	// per-key instance list in NOTE ORDER: lowest extension octave up to the highest. Full
	// octaves below note 0, then note 0-127 belong to the main buffer (skipped here), then the
	// note 128-131 gap-fill (interior pitch classes 8-11 at +9 octaves), then full octaves above.
	struct Inst { int p; float ox; };
	Inst inst[(2 * EXT_OCTAVES) * 12 + 4];
	int ni = 0;
	for (int k = -EXT_OCTAVES; k <= -1; k++)
		for (p = 0; p < 12; p++) { inst[ni].p = p; inst[ni].ox = (float)(k - 1) * m_OctaveWidthX; ni++; }
	for (p = 8; p <= 11; p++) { inst[ni].p = p; inst[ni].ox = 9.0f * m_OctaveWidthX; ni++; }   // 128-131
	for (int k = 11; k <= 10 + EXT_OCTAVES; k++)
		for (p = 0; p < 12; p++) { inst[ni].p = p; inst[ni].ox = (float)(k - 1) * m_OctaveWidthX; ni++; }

	unsigned long totalV = 0, totalI = 0;
	for (j = 0; j < ni; j++) { totalV += kvn[inst[j].p]; totalI += kin[inst[j].p]; }
	if ((totalV == 0) || (totalI == 0)) return -1;

	m_pExtVerts = (DXP11_VERTEX*)malloc((size_t)totalV * sizeof(DXP11_VERTEX));
	m_pExtIdx = (unsigned long*)malloc((size_t)totalI * sizeof(unsigned long));
	if ((m_pExtVerts == NULL) || (m_pExtIdx == NULL)) return -1;

	// note order: emit each key's vertices/indices in sequence (identical to the main path). The
	// first (EXT_OCTAVES*12) instances are the below-note-0 keys; record their index count so
	// Create() can submit below-0 -> main -> note128+ (= one continuous note-ordered keyboard).
	const int bottomKeyCount = EXT_OCTAVES * 12;
	unsigned long vbase = 0, ibase = 0;
	m_ExtBottomIdxNum = 0;
	for (j = 0; j < ni; j++) {
		if (j == bottomKeyCount) m_ExtBottomIdxNum = ibase;   // end of the below-0 block
		int pc = inst[j].p;
		for (i = 0; i < kvn[pc]; i++) {
			m_pExtVerts[vbase + i] = srcV[kv0[pc] + i];
			m_pExtVerts[vbase + i].pos[0] += inst[j].ox;
		}
		for (i = 0; i < kin[pc]; i++)
			m_pExtIdx[ibase + i] = pCpuIB[ki0[pc] + i] - kv0[pc] + vbase;
		vbase += kvn[pc];
		ibase += kin[pc];
	}
	m_ExtVertNum = totalV;
	m_ExtIdxNum = totalI;
	return 0;
}

//******************************************************************************
// Draw: each keyboard with its own base transform + key-press state
//   world = Scale . Trans(basePos[idx]) . RotX(-90) . RotZ(90) . RotX(roll) . Trans(playbackPos)
//******************************************************************************
int MTKeyboard11::DrawDX11(
		ID3D11DeviceContext* pContext,
		const XMMATRIX& viewProj,
		const XMFLOAT4& lightDir,
		float rollAngle,
		const XMFLOAT3& camPos
	)
{
	if (!m_Ready) return 0;

	// wall-clock elapsed since the previous frame, driving the key-press easing. Cap
	// it so a stall / pause (or the first frame) cannot snap every key in one step.
	unsigned long nowMs = timeGetTime();
	unsigned long elapsedMs = nowMs - m_LastAnimMs;
	if (elapsedMs > 100) elapsedMs = 100;
	m_LastAnimMs = nowMs;

	// normalize roll to [0,360) and pick the draw-face flip (matches MTPianoKeyboardMod)
	float roll = rollAngle;
	if (roll < 0.0f) roll += 360.0f;
	bool flipBack = (roll > 120.0f) && (roll < 300.0f);

	float scale = m_DesignMod.GetKeyboardResizeRatio();
	float playX = m_NoteDesign.GetPlayPosX(m_CurTickTime);

	XMMATRIX S  = XMMatrixScaling(scale, scale, scale);
	XMMATRIX R1 = XMMatrixRotationX(flipBack ? XM_PIDIV2 : -XM_PIDIV2);
	XMMATRIX R2 = XMMatrixRotationZ(XM_PIDIV2);
	XMMATRIX R3 = XMMatrixRotationX(XMConvertToRadians(rollAngle));
	XMMATRIX P  = XMMatrixTranslation(m_WorldMove.x + playX, m_WorldMove.y, m_WorldMove.z);

	for (unsigned long i = 0; i < m_NumKbd; i++) {
		SubKbd* pSub = &m_Subs[i];

		// push any pending key-press changes into this keyboard's vertex buffer
		_ApplyKeyStates(pContext, pSub, elapsedMs);

		D3DXVECTOR3 base = m_DesignMod.GetKeyboardBasePos(pSub->keyboardIndex, rollAngle);

		// pitch bend: shift in pitch (local X) by the strongest bend
		// (MTPianoKeyboardCtrlMod::GetMaxPitchBendShift). Single keyboard scans
		// every port; per-port keyboards scan just their own port's channels.
		if (m_pPitchBend != NULL) {
			float maxShift = 0.0f, cur = 0.0f;
			// NOTE: SM_MAX_PORT_NUM is 256, which overflows unsigned char to 0 -
			// a char loop bound made single-keyboard mode skip the scan entirely
			// (no bend). Use unsigned int for the port range.
			unsigned int pLo = m_SingleKbd ? 0u : (unsigned int)pSub->portNo;
			unsigned int pHi = m_SingleKbd ? (unsigned int)SM_MAX_PORT_NUM : (unsigned int)(pSub->portNo + 1);
			for (unsigned int p = pLo; p < pHi; p++) {
				for (unsigned char ch = 0; ch < SM_MAX_CH_NUM; ch++) {
					float s = m_DesignMod.GetPitchBendShift(
							m_pPitchBend->GetValue(p, ch),
							m_pPitchBend->GetSensitivity(p, ch));
					if (maxShift < (float)fabs(s)) { maxShift = (float)fabs(s); cur = s; }
				}
			}
			base.x += cur;
		}

		XMMATRIX B = XMMatrixTranslation(base.x, base.y, base.z);
		XMMATRIX world = S * B * R1 * R2 * R3 * P;
		pSub->prim.SetWorldMatrix(world);
		pSub->prim.SetTexture(m_pSRV);
		//ced 20260703: 無限鍵盤の拡張オクターブは同じ prim バッファの末尾に連結済みなので、この
		//1 回の Draw が通常鍵盤(0-127)と拡張部を全く同じ状態・同じ順序でまとめて描く。
		pSub->prim.Draw(pContext, viewProj, lightDir, -1, 0);
	}

	return 0;
}
