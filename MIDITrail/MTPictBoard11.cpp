//******************************************************************************
//
// MIDITrail / MTPictBoard11
//
// DX11 picture board (M4.5) - port of MTPictBoard.
//
//******************************************************************************

#include "stdafx.h"
#include "YNBaseLib.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTPictBoard11.h"
#include "DXTexture11.h"

using namespace YNBaseLib;
using namespace DirectX;


MTPictBoard11::MTPictBoard11()
{
	m_pSRV = NULL;
	m_WorldMove = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_CurTickTime = 0;
	m_Ready = false;
}

MTPictBoard11::~MTPictBoard11()
{
	Release();
}

void MTPictBoard11::Release()
{
	if (m_pSRV != NULL) { m_pSRV->Release(); m_pSRV = NULL; }
	m_Prim.Release();
	m_Ready = false;
}

//******************************************************************************
// Create: load the board texture and build the quad in the playback section
//******************************************************************************
int MTPictBoard11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;
	unsigned int imgW = 0, imgH = 0;
	D3DXVECTOR3 vLU, vRU, vLD, vRD, mv;
	float boardHeight = 0.0f, boardWidth = 0.0f, chStep = 0.0f, relX = 0.0f;
	DXP11_VERTEX* pv = NULL;
	unsigned long* pi = NULL;

	Release();

	result = DXPrimitive11::InitPipeline(pDevice);
	if (result != 0) goto EXIT;

	result = m_NoteDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	// texture (scene conf [Bitmap] Board, default data\Board.png) - silently
	// disabled if it can't be loaded, matching the DX9 behavior.
	result = _LoadTexture(pDevice, pSceneName, &imgW, &imgH);
	if ((result != 0) || (m_pSRV == NULL) || (imgW == 0) || (imgH == 0)) { result = 0; goto EXIT; }

	mv = m_NoteDesign.GetWorldMoveVector();
	m_WorldMove = XMFLOAT3(mv.x, mv.y, mv.z);

	m_NoteDesign.GetPlaybackSectionVirtexPos(0, &vLU, &vRU, &vLD, &vRD);
	boardHeight = vLU.y - vLD.y;
	boardWidth = boardHeight * ((float)imgW / (float)imgH);
	chStep = m_NoteDesign.GetChStep();
	relX = -(boardWidth * m_NoteDesign.GetPictBoardRelativePos());

	result = m_Prim.CreateVertexBuffer(pDevice, 4);
	if (result != 0) goto EXIT;
	result = m_Prim.CreateIndexBuffer(pDevice, 6);
	if (result != 0) goto EXIT;

	result = m_Prim.LockVertex(pContext, &pv);
	if (result != 0) goto EXIT;
	// 0 = LU, 1 = RU(+w), 2 = LD, 3 = RD(+w); z pushed slightly behind the notes
	pv[0].pos[0]=vLU.x+relX;            pv[0].pos[1]=vLU.y; pv[0].pos[2]=vLU.z+chStep+0.01f; pv[0].uv[0]=0.0f; pv[0].uv[1]=0.0f;
	pv[1].pos[0]=vLU.x+boardWidth+relX; pv[1].pos[1]=vLU.y; pv[1].pos[2]=vLU.z+chStep+0.01f; pv[1].uv[0]=1.0f; pv[1].uv[1]=0.0f;
	pv[2].pos[0]=vLD.x+relX;            pv[2].pos[1]=vLD.y; pv[2].pos[2]=vLD.z+chStep+0.01f; pv[2].uv[0]=0.0f; pv[2].uv[1]=1.0f;
	pv[3].pos[0]=vLD.x+boardWidth+relX; pv[3].pos[1]=vLD.y; pv[3].pos[2]=vLD.z+chStep+0.01f; pv[3].uv[0]=1.0f; pv[3].uv[1]=1.0f;
	for (int k = 0; k < 4; k++) {
		pv[k].normal[0]=0.0f; pv[k].normal[1]=0.0f; pv[k].normal[2]=-1.0f;
		pv[k].color = 0xFFFFFFFF;
	}
	m_Prim.UnlockVertex(pContext);

	result = m_Prim.LockIndex(pContext, &pi);
	if (result != 0) goto EXIT;
	pi[0]=0; pi[1]=1; pi[2]=2; pi[3]=2; pi[4]=1; pi[5]=3;
	m_Prim.UnlockIndex(pContext);

	m_Prim.SetMaterialAmbient(1.0f, 1.0f, 1.0f);   // unlit; texture * white
	m_Prim.SetTexture(m_pSRV);

	m_Ready = true;

EXIT:;
	return result;
}

//******************************************************************************
// Load the board texture from the scene config ([Bitmap] Board)
//******************************************************************************
int MTPictBoard11::_LoadTexture(
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
// Draw: follows the now-line on X like the notes
//******************************************************************************
int MTPictBoard11::DrawDX11(
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
