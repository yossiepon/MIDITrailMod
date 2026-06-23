//******************************************************************************
//
// MIDITrail / MTNoteLyrics11
//
// DX11 note-lyrics renderer - port of MTNoteLyrics.
//
//******************************************************************************

#include "stdafx.h"
#include "YNBaseLib.h"
#include "MTNoteLyrics11.h"
#include "DXTexture11.h"
#include "MTFont2Bmp.h"
#include <stdlib.h>
#include <new>

using namespace YNBaseLib;
using namespace DirectX;


MTNoteLyrics11::MTNoteLyrics11()
{
	m_pDevice = NULL;
	m_RingMode = false;
	m_pExtPitchBend = NULL;
	m_pCpuBuf = NULL;
	m_VertCapacity = 0;
	m_pStatus = NULL;
	m_PlayTimeMSec = 0;
	m_CurTickTime = 0;
	m_CurNoteIndex = 0;
	m_LastMSec = 0;
	m_WorldMove = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_isEnable = true;
	m_Ready = false;
	ZeroMemory(m_pDrawSRV, sizeof(m_pDrawSRV));
	ZeroMemory(m_KeyDownRate, sizeof(m_KeyDownRate));
}

MTNoteLyrics11::~MTNoteLyrics11()
{
	Release();
}

void MTNoteLyrics11::_ClearStatus(LyricStatus* p)
{
	if (p->pSRV != NULL) { p->pSRV->Release(); p->pSRV = NULL; }
	p->isActive = false;
	p->keyStatus = BeforeNoteON;
	p->index = 0;
	p->keyDownRate = 0.0f;
	p->texW = 0;
	p->texH = 0;
}

void MTNoteLyrics11::Release()
{
	m_Prim.Release();
	if (m_pStatus != NULL) {
		for (unsigned long i = 0; i < MTNOTELYRICS11_MAX_LYRICS; i++) _ClearStatus(&m_pStatus[i]);
		delete [] m_pStatus;
		m_pStatus = NULL;
	}
	if (m_pCpuBuf != NULL) { free(m_pCpuBuf); m_pCpuBuf = NULL; }
	m_VertCapacity = 0;
	m_NoteListRT.Clear();
	m_Ready = false;
}

//******************************************************************************
// Create
//******************************************************************************
int MTNoteLyrics11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;
	D3DXVECTOR3 mv;
	SMNoteList* pRT = NULL;
	SMNote note;
	unsigned long i = 0;

	Release();
	m_pDevice = pDevice;

	result = DXPrimitive11::InitPipeline(pDevice);
	if (result != 0) goto EXIT;

	result = m_NoteDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	//ring モードではリング配置用の設計も初期化（位置/worldMove に使用）
	if (m_RingMode) {
		result = m_NoteDesignRing.Initialize(pSceneName, pSeqData);
		if (result != 0) goto EXIT;
	}

	m_PitchBend.Initialize();

	// realtime (ms) merged note list - carries each note's lyric. The cached list
	// is owned by SMSeqData (freed after the scene build), so keep our own copy.
	result = pSeqData->GetMergedNoteListRealTime(&pRT);
	if (result != 0) goto EXIT;
	if (pRT != NULL) {
		unsigned long cnt = pRT->GetSize();
		for (i = 0; i < cnt; i++) {
			if (pRT->GetNote(i, &note) != 0) continue;
			m_NoteListRT.AddNote(note);
		}
	}

	mv = m_RingMode ? m_NoteDesignRing.GetWorldMoveVector() : m_NoteDesign.GetWorldMoveVector();
	m_WorldMove = XMFLOAT3(mv.x, mv.y, mv.z);

	try {
		m_pStatus = new LyricStatus[MTNOTELYRICS11_MAX_LYRICS];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}
	for (i = 0; i < MTNOTELYRICS11_MAX_LYRICS; i++) {
		m_pStatus[i].pSRV = NULL;
		_ClearStatus(&m_pStatus[i]);
	}

	// 6 verts (2 triangles) per lyric quad
	m_VertCapacity = 6 * MTNOTELYRICS11_MAX_LYRICS;
	result = m_Prim.CreateVertexBuffer(pDevice, m_VertCapacity);
	if (result != 0) goto EXIT;

	// identity index buffer (quads are an explicit triangle list)
	{
		unsigned long* pi = NULL;
		result = m_Prim.CreateIndexBuffer(pDevice, m_VertCapacity);
		if (result != 0) goto EXIT;
		result = m_Prim.LockIndex(pContext, &pi);
		if (result != 0) goto EXIT;
		for (i = 0; i < m_VertCapacity; i++) pi[i] = i;
		m_Prim.UnlockIndex(pContext);
	}

	m_pCpuBuf = malloc((size_t)m_VertCapacity * sizeof(DXP11_VERTEX));
	if (m_pCpuBuf == NULL) { result = YN_SET_ERR("Could not allocate memory.", 0, 0); goto EXIT; }

	m_Prim.SetMaterialAmbient(1.0f, 1.0f, 1.0f);   // unlit; texture * vertex color
	m_Prim.SetAdditiveBlend(false);                // normal alpha blend (DX9 INVSRCALPHA)

	m_PlayTimeMSec = 0;
	m_CurTickTime = 0;
	m_CurNoteIndex = 0;
	m_LastMSec = 0;
	m_Ready = true;

EXIT:;
	return result;
}

//******************************************************************************
// backward seek -> rewind the forward note scan + drop all active lyrics
//******************************************************************************
void MTNoteLyrics11::SetPlayTimeMSec(unsigned long ms)
{
	if (ms + 200 < m_LastMSec) {
		Reset();
	}
	if (ms > m_LastMSec) m_LastMSec = ms;
	m_PlayTimeMSec = ms;
}

void MTNoteLyrics11::Reset()
{
	m_PlayTimeMSec = 0;
	m_CurTickTime = 0;
	m_CurNoteIndex = 0;
	m_LastMSec = 0;
	if (m_pStatus != NULL) {
		for (unsigned long i = 0; i < MTNOTELYRICS11_MAX_LYRICS; i++) _ClearStatus(&m_pStatus[i]);
	}
}

//******************************************************************************
// activate newly started lyric notes / age active ones (device-free logic
// ported from MTNoteLyrics::_UpdateStatusOfLyrics)
//******************************************************************************
void MTNoteLyrics11::_UpdateStatus()
{
	unsigned long i = 0;
	SMNote note;

	unsigned long decayDuration   = m_NoteDesign.GetRippleDecayDuration();
	unsigned long releaseDuration = m_NoteDesign.GetRippleReleaseDuration();

	// age the active lyrics
	for (i = 0; i < MTNOTELYRICS11_MAX_LYRICS; i++) {
		if (!m_pStatus[i].isActive) continue;
		if (m_NoteListRT.GetNote(m_pStatus[i].index, &note) != 0) continue;
		_UpdateNoteStatus(m_PlayTimeMSec, decayDuration, releaseDuration, note, &m_pStatus[i]);
	}

	// scan forward from the last position for newly started lyric notes
	while (m_CurNoteIndex < m_NoteListRT.GetSize()) {
		if (m_NoteListRT.GetNote(m_CurNoteIndex, &note) != 0) break;

		// notes are start-sorted: once a note starts in the future we can stop
		if (m_PlayTimeMSec < note.startTime) break;

		bool isRegist = ((note.startTime <= m_PlayTimeMSec) && (m_PlayTimeMSec <= note.endTime)
				&& (note.lyric[0] != '\0'));
		if (isRegist) {
			// already registered with this index? skip
			bool isFound = false;
			for (i = 0; i < MTNOTELYRICS11_MAX_LYRICS; i++) {
				if (m_pStatus[i].isActive && (m_pStatus[i].index == m_CurNoteIndex)) { isFound = true; break; }
			}
			if (!isFound) {
				for (i = 0; i < MTNOTELYRICS11_MAX_LYRICS; i++) {
					if (!m_pStatus[i].isActive) {
						_ClearStatus(&m_pStatus[i]);
						m_pStatus[i].isActive = true;
						m_pStatus[i].index = m_CurNoteIndex;
						m_pStatus[i].keyStatus = BeforeNoteON;
						m_pStatus[i].keyDownRate = 0.0f;
						_CreateLyricTexture(note.lyric, &m_pStatus[i].pSRV,
								&m_pStatus[i].texW, &m_pStatus[i].texH);
						break;
					}
				}
			}
			if (i < MTNOTELYRICS11_MAX_LYRICS) {
				_UpdateNoteStatus(m_PlayTimeMSec, decayDuration, releaseDuration, note, &m_pStatus[i]);
			}
		}
		m_CurNoteIndex++;
	}
}

//******************************************************************************
// per-active-note envelope (ported verbatim from MTNoteLyrics::_UpdateNoteStatus)
//******************************************************************************
void MTNoteLyrics11::_UpdateNoteStatus(
		unsigned long playTimeMSec,
		unsigned long decayDuration,
		unsigned long releaseDuration,
		const SMNote& note,
		LyricStatus* pStatus
	)
{
	// finished note -> drop it (releases its texture)
	if (playTimeMSec > note.endTime) {
		_ClearStatus(pStatus);
		return;
	}

	unsigned long noteLen = note.endTime - note.startTime;

	float decayRatio = 0.3f;
	float sustainRatio = 0.4f;
	float releaseRatio = 0.3f;

	if (noteLen < decayDuration) {
		// (DX9 leaves the default split here)
	}
	else if (noteLen < (decayDuration + releaseDuration)) {
		releaseDuration = noteLen - decayDuration;
		decayRatio = 0.5f; sustainRatio = 0.0f; releaseRatio = 0.5f;
	}
	else if (noteLen < (decayDuration + releaseDuration) * 2) {
		unsigned long midTime = (note.startTime + decayDuration) / 2 + (note.endTime - releaseDuration) / 2;
		decayDuration = midTime - note.startTime;
		releaseDuration = note.endTime - midTime;
		decayRatio = 0.5f; sustainRatio = 0.0f; releaseRatio = 0.5f;
	}

	if (playTimeMSec < (note.startTime + decayDuration)) {
		pStatus->keyStatus = BeforeNoteON;
		pStatus->keyDownRate = (decayDuration == 0) ? 0.0f
				: decayRatio * (float)(playTimeMSec - note.startTime) / (float)decayDuration;
	}
	else if (((note.startTime + decayDuration) <= playTimeMSec)
			&& (playTimeMSec <= (note.endTime - releaseDuration))) {
		pStatus->keyStatus = NoteON;
		unsigned long denominator = noteLen - (decayDuration + releaseDuration);
		pStatus->keyDownRate = (denominator > 0)
				? decayRatio + sustainRatio * (float)(playTimeMSec - (note.startTime + decayDuration)) / (float)denominator
				: decayRatio + sustainRatio;
	}
	else if (((note.endTime - releaseDuration) < playTimeMSec) && (playTimeMSec <= note.endTime)) {
		pStatus->keyStatus = AfterNoteOFF;
		pStatus->keyDownRate = (releaseDuration == 0) ? 1.0f
				: decayRatio + sustainRatio + releaseRatio * (float)(playTimeMSec - (note.endTime - releaseDuration)) / (float)releaseDuration;
	}
}

//******************************************************************************
// build the quad vertices for the active lyrics (ported from
// _UpdateVertexOfLyrics + _SetVertexPosition). Returns the quad count, and
// fills m_pDrawSRV[] with the matching texture per quad.
//******************************************************************************
unsigned long MTNoteLyrics11::_BuildVertices(
		const XMFLOAT3& camPos,
		DXP11_VERTEX* pBuf
	)
{
	unsigned long i = 0;
	unsigned long activeNum = 0;
	SMNote note;

	ZeroMemory(m_KeyDownRate, sizeof(m_KeyDownRate));

	for (i = 0; i < MTNOTELYRICS11_MAX_LYRICS; i++) {
		if (!m_pStatus[i].isActive) continue;
		if (m_pStatus[i].pSRV == NULL) continue;
		if (m_NoteListRT.GetNote(m_pStatus[i].index, &note) != 0) continue;

		// one quad per (port,ch,note), keeping the strongest envelope
		if (note.portNo >= MTNOTELYRICS11_MAX_PORT) continue;
		if (m_KeyDownRate[note.portNo][note.chNo][note.noteNo] >= m_pStatus[i].keyDownRate) continue;

		short pbValue = _Bend()->GetValue(note.portNo, note.chNo);
		unsigned char pbSens = _Bend()->GetSensitivity(note.portNo, note.chNo);

		D3DXVECTOR3 center = m_RingMode
				? m_NoteDesignRing.GetNoteBoxCenterPosX(
					m_CurTickTime, note.portNo, note.chNo, note.noteNo, pbValue, pbSens)
				: m_NoteDesign.GetNoteBoxCenterPosX(
					m_CurTickTime, note.portNo, note.chNo, note.noteNo, pbValue, pbSens);

		float coef = m_NoteDesign.GetDecayCoefficient(m_pStatus[i].keyDownRate);
		float rh = (float)m_pStatus[i].texH * coef / 64.0f;   // Y extent (text height)
		float rw = (float)m_pStatus[i].texW * coef / 64.0f;   // Z extent (text width)
		if ((rh <= 0.0f) || (rw <= 0.0f)) continue;

		// nudge in X to keep overlapping lyrics from z-fighting on the flat plane
		// (planar layout only; the ring lays notes around the YZ circle so skip it)
		if (!m_RingMode) {
			if (center.x < camPos.x) {
				center.x -= 0.002f * MTNOTELYRICS11_MAX_LYRICS - (i + 1) * 0.002f;
			} else {
				center.x -= (i + 1) * 0.002f;
			}
		}

		float alpha = m_NoteDesign.GetRippleAlpha(m_pStatus[i].keyDownRate);
		D3DXCOLOR col = m_NoteDesign.GetNoteBoxColor(note.portNo, note.chNo, note.noteNo);
		unsigned long R = (unsigned long)(col.r * 255.0f + 0.5f); if (R > 255) R = 255;
		unsigned long G = (unsigned long)(col.g * 255.0f + 0.5f); if (G > 255) G = 255;
		unsigned long B = (unsigned long)(col.b * 255.0f + 0.5f); if (B > 255) B = 255;
		unsigned long A = (unsigned long)(alpha * 255.0f + 0.5f); if (A > 255) A = 255;
		unsigned long vcol = (A << 24) | (R << 16) | (G << 8) | B;

		DXP11_VERTEX* pv = &pBuf[activeNum * 6];
		// YZ quad facing -X (camera looks +X). uv mirrors the DX9 winding.
		float yT = center.y + rh / 2.0f, yB = center.y - rh / 2.0f;
		float zL = center.z - rw / 2.0f, zR = center.z + rw / 2.0f;
		float x  = center.x;
		// 0
		pv[0].pos[0]=x; pv[0].pos[1]=yT; pv[0].pos[2]=zL; pv[0].uv[0]=1.0f; pv[0].uv[1]=0.0f;
		pv[1].pos[0]=x; pv[1].pos[1]=yT; pv[1].pos[2]=zR; pv[1].uv[0]=0.0f; pv[1].uv[1]=0.0f;
		pv[2].pos[0]=x; pv[2].pos[1]=yB; pv[2].pos[2]=zR; pv[2].uv[0]=0.0f; pv[2].uv[1]=1.0f;
		pv[3].pos[0]=x; pv[3].pos[1]=yT; pv[3].pos[2]=zL; pv[3].uv[0]=1.0f; pv[3].uv[1]=0.0f;
		pv[4].pos[0]=x; pv[4].pos[1]=yB; pv[4].pos[2]=zR; pv[4].uv[0]=0.0f; pv[4].uv[1]=1.0f;
		pv[5].pos[0]=x; pv[5].pos[1]=yB; pv[5].pos[2]=zL; pv[5].uv[0]=1.0f; pv[5].uv[1]=1.0f;
		for (int k = 0; k < 6; k++) {
			pv[k].normal[0] = -1.0f; pv[k].normal[1] = 0.0f; pv[k].normal[2] = 0.0f;
			pv[k].color = vcol;
		}

		m_pDrawSRV[activeNum] = m_pStatus[i].pSRV;
		activeNum++;

		m_KeyDownRate[note.portNo][note.chNo][note.noteNo] = m_pStatus[i].keyDownRate;
	}

	return activeNum;
}

//******************************************************************************
// rasterize one lyric string to an RGBA texture (white text, alpha = coverage)
//******************************************************************************
int MTNoteLyrics11::_CreateLyricTexture(
		const TCHAR* pStr,
		ID3D11ShaderResourceView** ppSRV,
		unsigned long* pW,
		unsigned long* pH
	)
{
	int result = 0;
	MTFont2Bmp font;
	unsigned long w = 0, h = 0, x = 0, y = 0;
	unsigned char* pPixels = NULL;

	*ppSRV = NULL;
	*pW = 0;
	*pH = 0;
	if (m_pDevice == NULL) return 0;

	if (font.SetFont(_T("HGSSoeiKakugothicUB"), 64, false) != 0) return 0;
	if (font.CreateBmp(pStr) != 0) return 0;
	font.GetBmpSize(&h, &w);
	if ((w == 0) || (h == 0)) return 0;

	try {
		pPixels = new unsigned char[(size_t)w * h * 4];
	}
	catch (std::bad_alloc) {
		return 0;
	}

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			BYTE v = font.GetBmpPixcel(x, y);   // 0x00..0x10 coverage, 0xFF = out of range
			unsigned long a = (v == 0xFF) ? 0 : ((unsigned long)v * 16);
			if (a > 255) a = 255;
			size_t idx = ((size_t)y * w + x) * 4;
			pPixels[idx + 0] = 255;
			pPixels[idx + 1] = 255;
			pPixels[idx + 2] = 255;
			pPixels[idx + 3] = (unsigned char)a;
		}
	}

	result = DXTexture11::CreateFromRGBA(m_pDevice, pPixels, w, h, ppSRV);
	delete [] pPixels;
	if (result != 0) { *ppSRV = NULL; return 0; }

	*pW = w;
	*pH = h;
	return 0;
}

//******************************************************************************
// Draw
//******************************************************************************
int MTNoteLyrics11::DrawDX11(
		ID3D11DeviceContext* pContext,
		const XMMATRIX& viewProj,
		const XMFLOAT4& lightDir,
		float rollAngle,
		const XMFLOAT3& camPos
	)
{
	unsigned long activeNum = 0;
	unsigned long i = 0;
	DXP11_VERTEX* pv = NULL;

	if (!m_Ready || !m_isEnable) return 0;

	_UpdateStatus();

	activeNum = _BuildVertices(camPos, (DXP11_VERTEX*)m_pCpuBuf);
	if (activeNum == 0) return 0;
	if (activeNum * 6 > m_VertCapacity) activeNum = m_VertCapacity / 6;

	if (m_Prim.LockVertex(pContext, &pv) != 0) return 0;
	memcpy(pv, m_pCpuBuf, (size_t)activeNum * 6 * sizeof(DXP11_VERTEX));
	m_Prim.UnlockVertex(pContext);

	// world = RotX(roll) * Trans(worldMove)  (same frame as the note field)
	XMMATRIX world = XMMatrixRotationX(XMConvertToRadians(rollAngle))
	               * XMMatrixTranslation(m_WorldMove.x, m_WorldMove.y, m_WorldMove.z);
	m_Prim.SetWorldMatrix(world);

	// each lyric has its own text texture, so draw its 2 triangles individually
	for (i = 0; i < activeNum; i++) {
		m_Prim.SetTexture(m_pDrawSRV[i]);
		m_Prim.Draw(pContext, viewProj, lightDir, 2, (int)(2 * i));
	}
	return 0;
}
