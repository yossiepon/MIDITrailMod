//******************************************************************************
//
// MIDITrail / MTPictBoardRing
//
// �s�N�`���{�[�h�����O�`��N���X
//
// Copyright (C) 2019 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTPictBoardRing.h"
#include "DXH.h"

using namespace YNBaseLib;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTPictBoardRing::MTPictBoardRing(void)
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
MTPictBoardRing::~MTPictBoardRing(void)
{
	Release();
}

//******************************************************************************
// �s�N�`���{�[�h����
//******************************************************************************
int MTPictBoardRing::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData,
		bool isReverseMode
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
	vertexNum = (SM_MAX_NOTE_NUM + 1) * 2;
	result = m_Primitive.CreateVertexBuffer(pD3DDevice, vertexNum);
	if (result != 0) goto EXIT;

	//�C���f�b�N�X�o�b�t�@����
	indexNum = SM_MAX_NOTE_NUM * 3 * 2;
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
					pIndex,			//�C���f�b�N�X�o�b�t�@�������݈ʒu
					isReverseMode
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
int MTPictBoardRing::Transform(
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
int MTPictBoardRing::Draw(
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
void MTPictBoardRing::Release()
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
int MTPictBoardRing::_CreateVertexOfBoard(
		MTPICTBOARD_VERTEX* pVertex,
		unsigned long* pIndex,
		bool isReverseMode
	)
{
	int result = 0;
	unsigned long i = 0;
	unsigned long virtexIndex = 0;
	unsigned long virtexIndexStart = 0;
	D3DXVECTOR3 basePos;
	D3DXVECTOR3 rotatedPos;
	float boardHeight = 0.0f;
	float boardWidth = 0.0f;
	float chStep = 0.0f;
	float angle = 0.0f;
	float direction = 0.0f;
	D3DXVECTOR2 clipAreaP1;
	D3DXVECTOR2 clipAreaP2;
	D3DXVECTOR2 textureP1;
	D3DXVECTOR2 textureP2;

	//�e�N�X�`���N���b�v�̈�̍��W
	clipAreaP1 = D3DXVECTOR2(0.0f, 0.0f);  //����
	clipAreaP2 = D3DXVECTOR2(1.0f, 1.0f);  //�E��

	//�e�X�N�`��X���W
	if (isReverseMode) {
		textureP1.x = clipAreaP1.x;
		textureP1.y = clipAreaP2.y;
		textureP2.x = clipAreaP2.x;
		textureP2.y = clipAreaP1.y;
		direction = -1.0f;
	}
	else {
		textureP1.x = clipAreaP2.x;
		textureP1.y = clipAreaP1.y;
		textureP2.x = clipAreaP1.x;
		textureP2.y = clipAreaP2.y;
		direction = 1.0f;
	}

	//����W
	chStep = m_NoteDesign.GetChStep();
	basePos = D3DXVECTOR3(
				m_NoteDesign.GetPlayPosX(0),
				m_NoteDesign.GetPortOriginY(0) + (chStep * (float)SM_MAX_CH_NUM) + chStep + 0.01f,
				m_NoteDesign.GetPortOriginZ(0));
	boardHeight = 2.0f * 3.1415926f * basePos.y;
	boardWidth = boardHeight * ((float)m_ImgInfo.Width / (float)m_ImgInfo.Height);
	basePos.x -= (boardWidth * m_NoteDesign.GetPictBoardRelativePos());

	//���_�쐬�FX�����̉~��
	virtexIndexStart = virtexIndex;
	pVertex[virtexIndex].p = basePos;
	pVertex[virtexIndex].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
	pVertex[virtexIndex].c = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	pVertex[virtexIndex].t = D3DXVECTOR2(textureP1.x, textureP1.y);
	virtexIndex++;
	pVertex[virtexIndex].p = basePos;
	pVertex[virtexIndex].p.x += boardWidth;
	pVertex[virtexIndex].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
	pVertex[virtexIndex].c = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	pVertex[virtexIndex].t = D3DXVECTOR2(textureP2.x, textureP1.y);
	for (i = 1; i < SM_MAX_NOTE_NUM; i++) {
		virtexIndex++;
		
		//��]��̒��_
		angle = (360.0f / (float)SM_MAX_NOTE_NUM) * (float)i;
		rotatedPos = DXH::RotateYZ(0.0f, 0.0f, basePos, angle);
		pVertex[virtexIndex].p = rotatedPos;
		pVertex[virtexIndex].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
		pVertex[virtexIndex].c = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
		pVertex[virtexIndex].t = D3DXVECTOR2(textureP1.x, textureP1.y + (direction * (float)i / (float)SM_MAX_NOTE_NUM));
		virtexIndex++;
		pVertex[virtexIndex].p = rotatedPos;
		pVertex[virtexIndex].p.x += boardWidth;
		pVertex[virtexIndex].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
		pVertex[virtexIndex].c = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
		pVertex[virtexIndex].t = D3DXVECTOR2(textureP2.x, textureP1.y + (direction * (float)i / (float)SM_MAX_NOTE_NUM));
		
		//�C���f�b�N�X�o�b�t�@
		//  ���O�̒��_0,1�ƒǉ��������_2,3�ŎO�p�`0-1-3��0-3-2��ǉ�
		//  1+--+3-+--+--+--
		//   | /| /| /| /|  ..
		//   |/ |/ |/ |/ |  ..
		//  0+--+2-+--+--+--
		pIndex[(i-1)*6 + 0] = (i-1)*2 + 0;  //0 1�ڂ̎O�p�`
		pIndex[(i-1)*6 + 1] = (i-1)*2 + 1;  //1 1�ڂ̎O�p�`
		pIndex[(i-1)*6 + 2] = (i-1)*2 + 3;  //3 1�ڂ̎O�p�`
		pIndex[(i-1)*6 + 3] = (i-1)*2 + 0;  //0 2�ڂ̎O�p�`
		pIndex[(i-1)*6 + 4] = (i-1)*2 + 3;  //3 2�ڂ̎O�p�`
		pIndex[(i-1)*6 + 5] = (i-1)*2 + 2;  //2 2�ڂ̎O�p�`
	}
	//�Ō�̒��_2,3�͍ŏ�0,1�̒��_�Ɠ����i�����O�����j
	virtexIndex++;
	pVertex[virtexIndex] =pVertex[0];
	pVertex[virtexIndex].t = D3DXVECTOR2(textureP1.x, textureP2.y);
	virtexIndex++;
	pVertex[virtexIndex] =pVertex[1];
	pVertex[virtexIndex].t = D3DXVECTOR2(textureP2.x, textureP2.y);

	//�C���f�b�N�X�o�b�t�@�i�����O�����j
	pIndex[(i-1)*6 + 0] = (i-1)*2 + 0;  //0 1�ڂ̎O�p�`
	pIndex[(i-1)*6 + 1] = (i-1)*2 + 1;  //1 1�ڂ̎O�p�`
	pIndex[(i-1)*6 + 2] = (i-1)*2 + 3;  //3 1�ڂ̎O�p�`
	pIndex[(i-1)*6 + 3] = (i-1)*2 + 0;  //0 2�ڂ̎O�p�`
	pIndex[(i-1)*6 + 4] = (i-1)*2 + 3;  //3 2�ڂ̎O�p�`
	pIndex[(i-1)*6 + 5] = (i-1)*2 + 2;  //2 2�ڂ̎O�p�`

	return result;
}

//******************************************************************************
// �e�N�X�`���摜�ǂݍ���
//******************************************************************************
int MTPictBoardRing::_LoadTexture(
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
void MTPictBoardRing::SetCurTickTime(
		unsigned long curTickTime
	)
{
	m_CurTickTime = curTickTime;
}

//******************************************************************************
// ���Z�b�g
//******************************************************************************
void MTPictBoardRing::Reset()
{
	m_CurTickTime = 0;
}

//******************************************************************************
// ���t�J�n
//******************************************************************************
void MTPictBoardRing::OnPlayStart()
{
	m_isPlay = true;
}

//******************************************************************************
// ���t�I��
//******************************************************************************
void MTPictBoardRing::OnPlayEnd()
{
	m_isPlay = false;
}

//******************************************************************************
// �\���ݒ�
//******************************************************************************
void MTPictBoardRing::SetEnable(
		bool isEnable
	)
{
	m_isEnable = isEnable;
}


