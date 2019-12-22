//******************************************************************************
//
// MIDITrail / MTPictBoard
//
// �s�N�`���{�[�h�`��N���X
//
// Copyright (C) 2010-2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTPictBoard.h"

using namespace YNBaseLib;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTPictBoard::MTPictBoard(void)
{
	m_pTexture = NULL;
	ZeroMemory(&m_ImgInfo, sizeof(D3DXIMAGE_INFO));
	m_CurTickTime = 0;
	m_isPlay = false;
	m_isEnable = true;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTPictBoard::~MTPictBoard(void)
{
	Release();
}

//******************************************************************************
// �s�N�`���{�[�h����
//******************************************************************************
int MTPictBoard::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData
	)
{
	int result = 0;
	SMBarList barList;
	unsigned long vertexNum = 0;
	unsigned long indexNum = 0;
	MTPICTBOARD_VERTEX* pVertex = NULL;
	unsigned long* pIndex = NULL;

	Release();

	//�m�[�g�f�U�C���I�u�W�F�N�g������
	result = m_NoteDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	//�e�N�X�`���ǂݍ���
	result = _LoadTexture(pD3DDevice, pSceneName);
	if (result != 0) goto EXIT;

	//�v���~�e�B�u������
	result = m_Primitive.Initialize(
					sizeof(MTPICTBOARD_VERTEX),	//���_�T�C�Y
					_GetFVFFormat(),			//���_FVF�t�H�[�}�b�g
					D3DPT_TRIANGLESTRIP			//�v���~�e�B�u���
				);
	if (result != 0) goto EXIT;

	//���_�o�b�t�@����
	vertexNum = 4;
	result = m_Primitive.CreateVertexBuffer(pD3DDevice, vertexNum);
	if (result != 0) goto EXIT;

	//�C���f�b�N�X�o�b�t�@����
	indexNum = 4;
	result = m_Primitive.CreateIndexBuffer(pD3DDevice, indexNum);
	if (result != 0) goto EXIT;

	//�o�b�t�@�̃��b�N
	result = m_Primitive.LockVertex((void**)&pVertex);
	if (result != 0) goto EXIT;
	result = m_Primitive.LockIndex(&pIndex);
	if (result != 0) goto EXIT;

	//�o�b�t�@�ɒ��_�ƃC���f�b�N�X����������
	result = _CreateVertexOfBoard(
					pVertex,		//���_�o�b�t�@�������݈ʒu
					pIndex			//�C���f�b�N�X�o�b�t�@�������݈ʒu
				);
	if (result != 0) goto EXIT;

	//�o�b�t�@�̃��b�N����
	result = m_Primitive.UnlockVertex();
	if (result != 0) goto EXIT;
	result = m_Primitive.UnlockIndex();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �ړ�
//******************************************************************************
int MTPictBoard::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice,
		D3DXVECTOR3 camVector,
		float rollAngle
	)
{
	int result = 0;
	float curPos = 0.0f;
	D3DXVECTOR3 moveVector;
	D3DXMATRIX rotateMatrix;
	D3DXMATRIX moveMatrix;
	D3DXMATRIX worldMatrix;

	//�s�񏉊���
	D3DXMatrixIdentity(&rotateMatrix);
	D3DXMatrixIdentity(&moveMatrix);
	D3DXMatrixIdentity(&worldMatrix);

	//��]�s��
	D3DXMatrixRotationX(&rotateMatrix, D3DXToRadian(rollAngle));

	//���t�ʒu
	curPos = m_NoteDesign.GetPlayPosX(m_CurTickTime);

	//�ړ��s��
	moveVector = m_NoteDesign.GetWorldMoveVector();
	D3DXMatrixTranslation(&moveMatrix, moveVector.x + curPos, moveVector.y, moveVector.z);

	//�s��̍���
	D3DXMatrixMultiply(&worldMatrix, &rotateMatrix, &moveMatrix);

	//�ϊ��s��ݒ�
	m_Primitive.Transform(worldMatrix);

	return result;
}

//******************************************************************************
// �`��
//******************************************************************************
int MTPictBoard::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;

	if (!m_isEnable) goto EXIT;

	//�e�N�X�`���X�e�[�W�ݒ�
	//  �J���[���Z�F����1���g�p  ����1�F�e�N�X�`��
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	// �A���t�@���Z�F����1���g�p  ����1�F�e�N�X�`��
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

	//�e�N�X�`���t�B���^
	pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	//�`��
	result = m_Primitive.Draw(pD3DDevice, m_pTexture);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ���
//******************************************************************************
void MTPictBoard::Release()
{
	m_Primitive.Release();
	
	if (m_pTexture != NULL) {
		m_pTexture->Release();
		m_pTexture = NULL;
	}
}

//******************************************************************************
// �s�N�`���{�[�h���_����
//******************************************************************************
int MTPictBoard::_CreateVertexOfBoard(
		MTPICTBOARD_VERTEX* pVertex,
		unsigned long* pIndex
	)
{
	int result = 0;
	unsigned long i = 0;
	D3DXVECTOR3 vectorLU;
	D3DXVECTOR3 vectorRU;
	D3DXVECTOR3 vectorLD;
	D3DXVECTOR3 vectorRD;
	float boardHeight = 0.0f;
	float boardWidth = 0.0f;
	float chStep = 0.0f;

	//     +   1+----+3   +
	//    /|   / �� /    /|gridH    y x
	//   + | 0+----+2   + |�E       |/
	// ��| +   7+----+5 | +      z--+0
	//   |/    / �� /   |/
	//   +   6+----+4   + �� 4 �����_(0,0,0)
	//        gridW

	//�Đ��ʒ��_���W�擾
	m_NoteDesign.GetPlaybackSectionVirtexPos(
			0,
			&vectorLU,
			&vectorRU,
			&vectorLD,
			&vectorRD
		);

	boardHeight = vectorLU.y - vectorLD.y;
	boardWidth = boardHeight * ((float)m_ImgInfo.Width / (float)m_ImgInfo.Height);
	chStep = m_NoteDesign.GetChStep();

	//���_���W�F���̖�
	pVertex[0].p = D3DXVECTOR3(vectorLU.x,            vectorLU.y, vectorLU.z+chStep+0.01f); //0
	pVertex[1].p = D3DXVECTOR3(vectorLU.x+boardWidth, vectorLU.y, vectorLU.z+chStep+0.01f); //1
	pVertex[2].p = D3DXVECTOR3(vectorLD.x,            vectorLD.y, vectorLD.z+chStep+0.01f); //6
	pVertex[3].p = D3DXVECTOR3(vectorLD.x+boardWidth, vectorLD.y, vectorLD.z+chStep+0.01f); //7

	//�Đ��ʂƂ̑��Έʒu�ɂ��炷
	//  ���Έʒu 0.0f �� �摜�̍��[���Đ��ʂƒ�������
	//  ���Έʒu 0.5f �� �摜�̒������Đ��ʂƒ�������
	//  ���Έʒu 1.0f �� �摜�̉E�[���Đ��ʂƒ�������
	for (i = 0; i < 4; i++) {
		pVertex[i].p.x += -(boardWidth * m_NoteDesign.GetPictBoardRelativePos());
	}

	//�@��
	pVertex[0].n = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVertex[1].n = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVertex[2].n = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVertex[3].n = D3DXVECTOR3(0.0f, 0.0f, -1.0f);

	//�e���_�̃f�B�t���[�Y�F
	for (i = 0; i < 4; i++) {
		pVertex[i].c = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	}

	//�e���_�̃e�N�X�`�����W
	pVertex[0].t = D3DXVECTOR2(0.0f, 0.0f);
	pVertex[1].t = D3DXVECTOR2(1.0f, 0.0f);
	pVertex[2].t = D3DXVECTOR2(0.0f, 1.0f);
	pVertex[3].t = D3DXVECTOR2(1.0f, 1.0f);

	//�C���f�b�N�X�FTRIANGLESTRIP
	pIndex[0] = 0;
	pIndex[1] = 1;
	pIndex[2] = 2;
	pIndex[3] = 3;

	return result;
}

//******************************************************************************
// �e�N�X�`���摜�ǂݍ���
//******************************************************************************
int MTPictBoard::_LoadTexture(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName
	)
{
	int result = 0;
	HRESULT hresult = D3D_OK;
	TCHAR imgFilePath[_MAX_PATH] = {_T('\0')};
	TCHAR bmpFileName[_MAX_PATH] = {_T('\0')};
	MTConfFile confFile;

	result = confFile.Initialize(pSceneName);
	if (result != 0) goto EXIT;

	//�r�b�g�}�b�v�t�@�C����
	result = confFile.SetCurSection(_T("Bitmap"));
	if (result != 0) goto EXIT;
	result = confFile.GetStr(_T("Board"), bmpFileName, _MAX_PATH, MT_IMGFILE_BOARD);
	if (result != 0) goto EXIT;

	//�v���Z�X���s�t�@�C���f�B���N�g���p�X�擾
	result = YNPathUtil::GetModuleDirPath(imgFilePath, _MAX_PATH);
	if (result != 0) goto EXIT;

	//�摜�t�@�C���p�X�쐬
	_tcscat_s(imgFilePath, _MAX_PATH, bmpFileName);

	//�ǂݍ��މ摜�̏c���T�C�Y���擾���Ă���
	hresult = D3DXGetImageInfoFromFile(imgFilePath, &m_ImgInfo);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

	//�e�N�X�`���摜�Ƃ��ēǂݍ���
	hresult = D3DXCreateTextureFromFile(
					pD3DDevice,		//�e�N�X�`���Ɋ֘A�t����f�o�C�X
					imgFilePath,	//�t�@�C����
					&m_pTexture		//�쐬���ꂽ�e�N�X�`���I�u�W�F�N�g
				);
	if (FAILED(hresult)) {
		result = YN_SET_ERR("DirectX API error.", hresult, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �`�b�N�^�C���ݒ�
//******************************************************************************
void MTPictBoard::SetCurTickTime(
		unsigned long curTickTime
	)
{
	m_CurTickTime = curTickTime;
}

//******************************************************************************
// ���Z�b�g
//******************************************************************************
void MTPictBoard::Reset()
{
	m_CurTickTime = 0;
}

//******************************************************************************
// ���t�J�n
//******************************************************************************
void MTPictBoard::OnPlayStart()
{
	m_isPlay = true;
}

//******************************************************************************
// ���t�I��
//******************************************************************************
void MTPictBoard::OnPlayEnd()
{
	m_isPlay = false;
}

//******************************************************************************
// �\���ݒ�
//******************************************************************************
void MTPictBoard::SetEnable(
		bool isEnable
	)
{
	m_isEnable = isEnable;
}


