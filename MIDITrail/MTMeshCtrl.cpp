//******************************************************************************
//
// MIDITrail / MTMeshCtrl
//
// メッシュ制御クラス
//
// Copyright (C) 2012-2013 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTConfFile.h"
#include "MTMeshCtrl.h"

using namespace YNBaseLib;


//******************************************************************************
// コンストラクタ
//******************************************************************************
MTMeshCtrl::MTMeshCtrl(void)
{
	m_MeshFilePath[0] = _T('\0');
	m_PositionX = 0.0f;
	m_PositionY = 0.0f;
	m_PositionZ = 0.0f;
}

//******************************************************************************
// デストラクタ
//******************************************************************************
MTMeshCtrl::~MTMeshCtrl(void)
{
	Release();
}

//******************************************************************************
// 生成処理
//******************************************************************************
int MTMeshCtrl::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName
	)
{
	int result = 0;

	//パラメータ設定ファイル読み込み
	result = _LoadConfFile(pSceneName);
	if (result != 0) goto EXIT;

	result = m_Mesh.Initialize(pD3DDevice, m_MeshFilePath);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 移動
//******************************************************************************
int MTMeshCtrl::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice,
		D3DXVECTOR3 moveVector
	)
{
	int result = 0;
	D3DXMATRIX rotateMatrix;
	D3DXMATRIX moveMatrix;
	D3DXMATRIX worldMatrix;
	float rollAngle = 0.0f;

	//行列初期化
	D3DXMatrixIdentity(&rotateMatrix);
	D3DXMatrixIdentity(&moveMatrix);
	D3DXMatrixIdentity(&worldMatrix);

	//回転行列
	D3DXMatrixRotationY(&rotateMatrix, D3DXToRadian(rollAngle));

	//移動行列
	D3DXMatrixTranslation(
			&moveMatrix,
			m_PositionX + moveVector.x,
			m_PositionY + moveVector.y,
			m_PositionZ + moveVector.z
		);

	//行列の合成：移動→回転
	D3DXMatrixMultiply(&worldMatrix, &moveMatrix, &rotateMatrix);

	//変換行列設定
	m_Mesh.Transform(worldMatrix);

//EXIT:;
	return result;
}

//******************************************************************************
// 描画
//******************************************************************************
int MTMeshCtrl::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
   )
{
	int result = 0;

	//メッシュの描画
	result = m_Mesh.Draw(pD3DDevice);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// 解放
//******************************************************************************
void MTMeshCtrl::Release()
{
	m_Mesh.Release();
}

//******************************************************************************
// 設定ファイル読み込み
//******************************************************************************
int MTMeshCtrl::_LoadConfFile(
		const TCHAR* pSceneName
	)
{
	int result = 0;
	MTConfFile confFile;
	TCHAR xFileName[_MAX_PATH] = {_T('\0')};
	TCHAR imgFilePath[_MAX_PATH] = {_T('\0')};

	m_MeshFilePath[0] = _T('\0');

	result = confFile.Initialize(pSceneName);
	if (result != 0) goto EXIT;

	result = confFile.SetCurSection(_T("Mesh"));
	if (result != 0) goto EXIT;

	//Xファイル名
	result = confFile.GetStr(_T("XFile"), xFileName, _MAX_PATH, _T(""));
	if (result != 0) goto EXIT;

	//Xファイルパス作成
	if (_tcslen(xFileName) > 0) {
		//プロセス実行ファイルディレクトリパス取得
		result = YNPathUtil::GetModuleDirPath(m_MeshFilePath, _MAX_PATH);
		if (result != 0) goto EXIT;
		//メッシュファイルパス
		_tcscat_s(m_MeshFilePath, _MAX_PATH, xFileName);
	}

	//描画位置
	result = confFile.GetFloat(_T("PositionX"), &m_PositionX, 0.0f);
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("PositionY"), &m_PositionY, 0.0f);
	if (result != 0) goto EXIT;
	result = confFile.GetFloat(_T("PositionZ"), &m_PositionZ, 0.0f);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}


