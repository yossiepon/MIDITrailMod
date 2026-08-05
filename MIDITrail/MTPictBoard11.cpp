//******************************************************************************
//
// MIDITrail / MTPictBoard11
//
// DX11 picture board renderer.
//
// Copyright (C) 2010-2025 WADA Masashi. All Rights Reserved.
// Copyright (C) 2025 yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "DXTexture11.h"
#include "MTPictBoard11.h"

using namespace YNBaseLib;
using namespace DirectX;
using namespace DirectX::SimpleMath;


//******************************************************************************
// コンストラクタ / デストラクタ
//******************************************************************************
MTPictBoard11::MTPictBoard11()
{
	m_pSRV = NULL;
	m_ImgWidth = 0;
	m_ImgHeight = 0;
	m_CurTickTime = 0;
	m_isPlay = false;
	m_isReady = false;
}

MTPictBoard11::~MTPictBoard11()
{
	Release();
}

//******************************************************************************
// 生成
//******************************************************************************
int MTPictBoard11::Create(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;

	Release();

	if (pSeqData == NULL) goto EXIT;

	result = m_NoteDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	result = _LoadTexture(pDevice, pSceneName);
	if (result != 0) goto EXIT;

	result = _CreateVertices(pDevice, pContext);
	if (result != 0) goto EXIT;

	m_Primitive.SetLightEnable(false);
	m_isReady = true;

EXIT:;
	return result;
}

//******************************************************************************
// 解放
//******************************************************************************
void MTPictBoard11::Release()
{
	m_Primitive.Release();
	if (m_pSRV != NULL) {
		m_pSRV->Release();
		m_pSRV = NULL;
	}
	m_isReady = false;
}

//******************************************************************************
// 頂点生成
//******************************************************************************
int MTPictBoard11::_CreateVertices(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext
	)
{
	int result = 0;
	DXPRIMITIVE11_VERTEX* pVertex = NULL;
	unsigned long* pIndex = NULL;

	// 4 頂点 6 インデックス（TRIANGLELIST で 2 三角形）
	result = m_Primitive.CreateVertexBuffer(pDevice, 4);
	if (result != 0) goto EXIT;
	result = m_Primitive.CreateIndexBuffer(pDevice, 6);
	if (result != 0) goto EXIT;

	result = m_Primitive.LockVertex(pContext, &pVertex);
	if (result != 0) goto EXIT;
	result = m_Primitive.LockIndex(pContext, &pIndex);
	if (result != 0) goto EXIT;

	{
		// 再生面の頂点座標を取得してボードの位置を決定
		Vector3 vectorLU, vectorRU, vectorLD, vectorRD;
		m_NoteDesign.GetPlaybackSectionVirtexPos(
				0, &vectorLU, &vectorRU, &vectorLD, &vectorRD);

		float boardHeight = vectorLU.y - vectorLD.y;
		float boardWidth = 0.0f;
		if (m_ImgHeight > 0) {
			boardWidth = boardHeight * ((float)m_ImgWidth / (float)m_ImgHeight);
		}
		float chStep = m_NoteDesign.GetChStep();

		// 頂点座標（左面、chStep 分ずらす）
		//  0+----+1
		//   |    |
		//  2+----+3

		auto setVtx = [&](unsigned long i, float px, float py, float pz, float u, float v) {
			pVertex[i].pos[0] = px;
			pVertex[i].pos[1] = py;
			pVertex[i].pos[2] = pz;
			pVertex[i].normal[0] = 0.0f;
			pVertex[i].normal[1] = 0.0f;
			pVertex[i].normal[2] = -1.0f;
			pVertex[i].color = 0xFFFFFFFF;
			pVertex[i].uv[0] = u;
			pVertex[i].uv[1] = v;
		};

		setVtx(0, vectorLU.x,             vectorLU.y, vectorLU.z + chStep + 0.01f, 0.0f, 0.0f);
		setVtx(1, vectorLU.x + boardWidth, vectorLU.y, vectorLU.z + chStep + 0.01f, 1.0f, 0.0f);
		setVtx(2, vectorLD.x,             vectorLD.y, vectorLD.z + chStep + 0.01f, 0.0f, 1.0f);
		setVtx(3, vectorLD.x + boardWidth, vectorLD.y, vectorLD.z + chStep + 0.01f, 1.0f, 1.0f);

		// 再生面との相対位置にずらす
		float offset = -(boardWidth * m_NoteDesign.GetPictBoardRelativePos());
		for (int i = 0; i < 4; i++) {
			pVertex[i].pos[0] += offset;
		}

		// TRIANGLELIST
		pIndex[0] = 0; pIndex[1] = 1; pIndex[2] = 2;
		pIndex[3] = 2; pIndex[4] = 1; pIndex[5] = 3;
	}

	m_Primitive.UnlockVertex(pContext);
	m_Primitive.UnlockIndex(pContext);

EXIT:;
	return result;
}

//******************************************************************************
// テクスチャ読み込み
//******************************************************************************
int MTPictBoard11::_LoadTexture(
		ID3D11Device* pDevice,
		const TCHAR* pSceneName
	)
{
	int result = 0;
	TCHAR imgFilePath[_MAX_PATH] = {_T('\0')};
	TCHAR bmpFileName[_MAX_PATH] = {_T('\0')};
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

	result = DXTexture11::LoadFromFile(pDevice, imgFilePath, &m_pSRV, &m_ImgWidth, &m_ImgHeight);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 更新
//******************************************************************************
void MTPictBoard11::Transform(
		const Vector3& camVector,
		float rollAngle
	)
{
	float curPos = m_NoteDesign.GetPlayPosX(m_CurTickTime);
	Vector3 moveVec = m_NoteDesign.GetWorldMoveVector();
	Matrix world = Matrix::CreateRotationX(XMConvertToRadians(rollAngle))
	             * Matrix::CreateTranslation(moveVec.x + curPos, moveVec.y, moveVec.z);

	m_Primitive.SetWorldMatrix(world);
	m_Primitive.SetTexture(m_pSRV);
}

//******************************************************************************
// 描画
//******************************************************************************
int MTPictBoard11::DrawDX11(
		ID3D11DeviceContext* pContext,
		const Matrix& viewProj,
		const Vector4& lightDir,
		float rollAngle
	)
{
	if (!m_isEnable || !m_isReady) return 0;
	return m_Primitive.Draw(pContext, viewProj, lightDir);
}

//******************************************************************************
// チックタイム / リセット / 再生制御
//******************************************************************************
void MTPictBoard11::Update(unsigned long curTickTime, unsigned long playTimeMSec)
{
	m_CurTickTime = curTickTime;
}

void MTPictBoard11::Reset()
{
	m_CurTickTime = 0;
}

void MTPictBoard11::OnPlayStart()
{
	m_isPlay = true;
}

void MTPictBoard11::OnPlayEnd()
{
	m_isPlay = false;
}
