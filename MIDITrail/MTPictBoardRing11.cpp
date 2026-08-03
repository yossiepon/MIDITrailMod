//******************************************************************************
//
// MIDITrail / MTPictBoardRing11
//
// DX11 Ring-scene picture board (M4.13) - port of MTPictBoardRing.
//
//******************************************************************************

#include "stdafx.h"
#include "YNBaseLib.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTPictBoardRing11.h"
#include "DXTexture11.h"
#include "DXH.h"

using namespace YNBaseLib;
using namespace DirectX;

#define MTPBRING11_SEG_NUM   (SM_MAX_NOTE_NUM)   // 128 segments around the ring
#define MTPBRING11_VTX_NUM   ((MTPBRING11_SEG_NUM + 1) * 2)
#define MTPBRING11_IDX_NUM   (MTPBRING11_SEG_NUM * 6)


MTPictBoardRing11::MTPictBoardRing11()
{
	m_pSRV = NULL;
	m_WorldMove = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_CurTickTime = 0;
	m_Ready = false;
}

MTPictBoardRing11::~MTPictBoardRing11()
{
	Release();
}

void MTPictBoardRing11::Release()
{
	if (m_pSRV != NULL) { m_pSRV->Release(); m_pSRV = NULL; }
	m_Prim.Release();
	m_Ready = false;
}

//******************************************************************************
// Create: build the cylindrical board band + load the texture
//******************************************************************************
int MTPictBoardRing11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;
	unsigned int imgW = 0, imgH = 0;
	D3DXVECTOR3 basePos, mv;
	float chStep = 0.0f, boardHeight = 0.0f, boardWidth = 0.0f;
	DXP11_VERTEX* pv = NULL;
	unsigned long* pi = NULL;
	unsigned long i = 0;

	Release();

	result = DXPrimitive11::InitPipeline(pDevice);
	if (result != 0) goto EXIT;

	result = m_NoteDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	result = _LoadTexture(pDevice, pSceneName, &imgW, &imgH);
	if ((result != 0) || (m_pSRV == NULL) || (imgW == 0) || (imgH == 0)) { result = 0; goto EXIT; }

	mv = m_NoteDesign.GetWorldMoveVector();
	m_WorldMove = XMFLOAT3(mv.x, mv.y, mv.z);

	// base point (outside the channel ring); board wrapped around the circumference
	chStep = m_NoteDesign.GetChStep();
	basePos = D3DXVECTOR3(
			m_NoteDesign.GetPlayPosX(0),
			m_NoteDesign.GetPortOriginY(0) + (chStep * (float)SM_MAX_CH_NUM) + chStep + 0.01f,
			m_NoteDesign.GetPortOriginZ(0));
	boardHeight = 2.0f * 3.1415926f * basePos.y;
	boardWidth = boardHeight * ((float)imgW / (float)imgH);
	basePos.x -= (boardWidth * m_NoteDesign.GetPictBoardRelativePos());

	result = m_Prim.CreateVertexBuffer(pDevice, MTPBRING11_VTX_NUM);
	if (result != 0) goto EXIT;
	result = m_Prim.CreateIndexBuffer(pDevice, MTPBRING11_IDX_NUM);
	if (result != 0) goto EXIT;

	result = m_Prim.LockVertex(pContext, &pv);
	if (result != 0) goto EXIT;
	// non-reverse UV: U = 1 at x=base, 0 at x=base+boardWidth; V = i/128 around the ring
	for (i = 0; i < MTPBRING11_SEG_NUM; i++) {
		D3DXVECTOR3 p = (i == 0) ? basePos
				: DXH::RotateYZ(0.0f, 0.0f, basePos, (360.0f / (float)MTPBRING11_SEG_NUM) * (float)i);
		float v = (float)i / (float)MTPBRING11_SEG_NUM;
		DXP11_VERTEX* a = &pv[i * 2];
		DXP11_VERTEX* b = &pv[i * 2 + 1];
		a->pos[0]=p.x;             a->pos[1]=p.y; a->pos[2]=p.z; a->uv[0]=1.0f; a->uv[1]=v;
		b->pos[0]=p.x+boardWidth;  b->pos[1]=p.y; b->pos[2]=p.z; b->uv[0]=0.0f; b->uv[1]=v;
		a->normal[0]=-1.0f; a->normal[1]=0.0f; a->normal[2]=0.0f; a->color=0xFFFFFFFF;
		b->normal[0]=-1.0f; b->normal[1]=0.0f; b->normal[2]=0.0f; b->color=0xFFFFFFFF;
	}
	// closing pair (= verts 0,1 at V=1) so the band wraps seamlessly
	pv[MTPBRING11_SEG_NUM * 2]     = pv[0]; pv[MTPBRING11_SEG_NUM * 2].uv[1]     = 1.0f;
	pv[MTPBRING11_SEG_NUM * 2 + 1] = pv[1]; pv[MTPBRING11_SEG_NUM * 2 + 1].uv[1] = 1.0f;
	m_Prim.UnlockVertex(pContext);

	result = m_Prim.LockIndex(pContext, &pi);
	if (result != 0) goto EXIT;
	// one quad per segment between pair s and pair s+1 (verts contiguous; the last
	// segment's "next pair" is the closing pair at 2*SEG_NUM)
	for (i = 0; i < MTPBRING11_SEG_NUM; i++) {
		unsigned long o = i * 2;
		pi[i * 6 + 0] = o + 0; pi[i * 6 + 1] = o + 1; pi[i * 6 + 2] = o + 3;
		pi[i * 6 + 3] = o + 0; pi[i * 6 + 4] = o + 3; pi[i * 6 + 5] = o + 2;
	}
	m_Prim.UnlockIndex(pContext);

	m_Prim.SetMaterialAmbient(1.0f, 1.0f, 1.0f);
	m_Prim.SetTexture(m_pSRV);
	m_Ready = true;

EXIT:;
	return result;
}

//******************************************************************************
// Load the board texture (scene conf [Bitmap] Board, default data\Board.png)
//******************************************************************************
int MTPictBoardRing11::_LoadTexture(
		ID3D11Device* pDevice,
		const TCHAR* pSceneName,
		unsigned int* pW,
		unsigned int* pH
	)
{
	int result = 0;
	TCHAR imgFilePath[_MAX_PATH] = { _T('\0') };
	TCHAR bmpFileName[_MAX_PATH] = { _T('\0') };
	MTConfFile confFile;

	result = confFile.Initialize(pSceneName);
	if (result != 0) goto EXIT;
	result = confFile.SetCurSection(_T("Bitmap"));
	if (result != 0) goto EXIT;
	result = confFile.GetStr(_T("Board"), bmpFileName, _MAX_PATH, MT_IMGFILE_BOARD);
	if (result != 0) goto EXIT;

	result = YNPathUtil::GetModuleDirPath(imgFilePath, _MAX_PATH);
	if (result != 0) goto EXIT;
	_tcscat_s(imgFilePath, _MAX_PATH, bmpFileName);

	result = DXTexture11::LoadFromFile(pDevice, imgFilePath, &m_pSRV, pW, pH);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// Draw: world = RotX(roll) * Trans(worldMove + (playPosX,0,0))
//******************************************************************************
int MTPictBoardRing11::DrawDX11(
		ID3D11DeviceContext* pContext,
		const XMMATRIX& viewProj,
		const XMFLOAT4& lightDir,
		float rollAngle
	)
{
	if (!m_Ready) return 0;

	float curPos = m_NoteDesign.GetPlayPosX(m_CurTickTime);
	XMMATRIX world = XMMatrixRotationX(XMConvertToRadians(rollAngle))
	               * XMMatrixTranslation(m_WorldMove.x + curPos, m_WorldMove.y, m_WorldMove.z);
	m_Prim.SetWorldMatrix(world);
	m_Prim.SetTexture(m_pSRV);
	return m_Prim.Draw(pContext, viewProj, lightDir, -1, 0);
}
