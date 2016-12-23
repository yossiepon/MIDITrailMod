//******************************************************************************
//
// MIDITrail / MTNoteLyrics
//
// �m�[�g�̎��`��N���X
//
// Copyright (C) 2010-2012 WADA Masashi. All Rights Reserved.
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTParam.h"
#include "MTConfFile.h"
#include "MTNoteLyrics.h"
#include <new>

using namespace YNBaseLib;


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTNoteLyrics::MTNoteLyrics(void)
{
	m_pNoteStatus = NULL;
	m_PlayTimeMSec = 0;
	m_CurTickTime = 0;
	m_CurNoteIndex = 0;
	m_ActiveNoteNum = 0;
	m_CamVector = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	ZeroMemory(&m_Material, sizeof(D3DMATERIAL9));
	m_isEnable = true;
	m_isSkipping = false;
	ZeroMemory(m_KeyDownRate, sizeof(float) * MTNOTELYRICS_MAX_PORT_NUM * SM_MAX_CH_NUM * SM_MAX_NOTE_NUM);
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTNoteLyrics::~MTNoteLyrics(void)
{
	Release();
}

//******************************************************************************
// �m�[�g�̎�����
//******************************************************************************
int MTNoteLyrics::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData,
		MTNotePitchBend* pNotePitchBend
   )
{
	int result = 0;
	SMTrack track;

	Release();

	//�m�[�g�f�U�C���I�u�W�F�N�g������
	result = m_NoteDesign.Initialize(pSceneName, pSeqData);
	if (result != 0) goto EXIT;

	//�g���b�N�擾
	result = pSeqData->GetMergedTrack(&track);
	if (result != 0) goto EXIT;

	//�m�[�g���X�g�擾�FstartTime, endTime �̓��A���^�C��(msec)
	result = track.GetNoteListWithRealTime(&m_NoteListRT, pSeqData->GetTimeDivision());
	if (result != 0) goto EXIT;

	//�m�[�g���z�񐶐�
	result = _CreateNoteStatus();
	if (result != 0) goto EXIT;

	//���_����
	result = _CreateVertex(pD3DDevice);
	if (result != 0) goto EXIT;

	//�}�e���A���쐬
	_MakeMaterial(&m_Material);

	//�s�b�`�x���h���
	m_pNotePitchBend = pNotePitchBend;

EXIT:;
	return result;
}

//******************************************************************************
// �ړ�
//******************************************************************************
int MTNoteLyrics::Transform(
		LPDIRECT3DDEVICE9 pD3DDevice,
		D3DXVECTOR3 camVector,
		float rollAngle
	)
{
	int result = 0;
	D3DXVECTOR3 moveVector;
	D3DXMATRIX rotateMatrix;
	D3DXMATRIX moveMatrix;
	D3DXMATRIX worldMatrix;

	m_CamVector = camVector;

	//�̎��̒��_�X�V
	result = _TransformLyrics(pD3DDevice);
	if (result != 0) goto EXIT;

	//�s�񏉊���
	D3DXMatrixIdentity(&rotateMatrix);
	D3DXMatrixIdentity(&moveMatrix);
	D3DXMatrixIdentity(&worldMatrix);

	//��]�s��
	//D3DXMatrixRotationX(&rotateMatrix, D3DXToRadian(rollAngle + 180.0f));
	D3DXMatrixRotationX(&rotateMatrix, D3DXToRadian(rollAngle));

	//�ړ��s��
	moveVector = m_NoteDesign.GetWorldMoveVector();
	D3DXMatrixTranslation(&moveMatrix, moveVector.x, moveVector.y, moveVector.z);

	//�s��̍���
	D3DXMatrixMultiply(&worldMatrix, &rotateMatrix, &moveMatrix);

	//�ϊ��s��ݒ�
	m_Primitive.Transform(worldMatrix);

EXIT:;
	return result;
}

//******************************************************************************
// �̎��̒��_�X�V
//******************************************************************************
int MTNoteLyrics::_TransformLyrics(
		LPDIRECT3DDEVICE9 pD3DDevice
   )
{
	int result = 0;

	//�X�L�b�v���Ȃ牽�����Ȃ�
	if (m_isSkipping) goto EXIT;

	//�̎��̏�ԍX�V
	result = _UpdateStatusOfLyrics(pD3DDevice);
	if (result != 0) goto EXIT;

	//�̎��̒��_�X�V
	result = _UpdateVertexOfLyrics(pD3DDevice);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �̎��̏�ԍX�V
//******************************************************************************
int MTNoteLyrics::_UpdateStatusOfLyrics(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	unsigned long i = 0;
	bool isFound = false;
	bool isRegist = false;
	SMNote note;

	//�̎��f�B�P�C�E�����[�X����(msec)
	unsigned long decayDuration = m_NoteDesign.GetRippleDecayDuration();
	unsigned long releaseDuration   = m_NoteDesign.GetRippleReleaseDuration();

	//�m�[�g�����X�V����
	for (i = 0; i < MTNOTELYRICS_MAX_LYRICS_NUM; i++) {
		if (m_pNoteStatus[i].isActive) {
			//�m�[�g���擾
			result = m_NoteListRT.GetNote(m_pNoteStatus[i].index, &note);
			if (result != 0) goto EXIT;

			//�������m�[�g��ԍX�V
			result = _UpdateNoteStatus(
							m_PlayTimeMSec,
							decayDuration,
							releaseDuration,
							note,
							&(m_pNoteStatus[i])
						);
			if (result != 0) goto EXIT;
		}
	}

	//�O�񌟍��I���ʒu���甭���J�n�m�[�g������
	while (m_CurNoteIndex < m_NoteListRT.GetSize()) {
		//�m�[�g���擾
		result = m_NoteListRT.GetNote(m_CurNoteIndex, &note);
		if (result != 0) goto EXIT;

		//���t���Ԃ��L�[�����J�n���ԁi�����J�n���O�j�ɂ��ǂ���Ă��Ȃ���Ό����I��
		if (m_PlayTimeMSec < note.startTime) {
			break;
		}

		//�m�[�g���o�^����
		isRegist = false;
		if ((note.startTime <= m_PlayTimeMSec) && (m_PlayTimeMSec <= note.endTime) && (note.lyric[0] != '\0')) {
			isRegist = true;
		}

		//�m�[�g���o�^
		//  �L�[���~���^�㏸���̏����o�^�ΏۂƂ��Ă��邽��
		//  ����m�[�g�ŕ����G���g�������ꍇ�����邱�Ƃɒ��ӂ���
		if (isRegist) {
			//���łɓ���C���f�b�N�X�œo�^�ς݂̏ꍇ�͉������Ȃ�
			isFound = false;
			for (i = 0; i < MTNOTELYRICS_MAX_LYRICS_NUM; i++) {
				if ((m_pNoteStatus[i].isActive)
				 && (m_pNoteStatus[i].index == m_CurNoteIndex)) {
					isFound = true;
					break;
				}
			}
			//�󂢂Ă���Ƃ���ɒǉ�����
			if (!isFound) {
				for (i = 0; i < MTNOTELYRICS_MAX_LYRICS_NUM; i++) {
					if (!(m_pNoteStatus[i].isActive)) {
						m_pNoteStatus[i].isActive = true;
						m_pNoteStatus[i].index = m_CurNoteIndex;
						m_pNoteStatus[i].keyStatus = BeforeNoteON;
						m_pNoteStatus[i].keyDownRate = 0.0f;

						m_pNoteStatus[i].fontTexture.SetFont(_T("HGSSoeiKakugothicUB"), 64, 0x00FFFFFF, false);
						m_pNoteStatus[i].fontTexture.CreateTexture(pD3DDevice, note.lyric);
						break;
					}
				}
			}
			//�������m�[�g��ԍX�V
			result = _UpdateNoteStatus(
							m_PlayTimeMSec,
							decayDuration,
							releaseDuration,
							note,
							&(m_pNoteStatus[i])
						);
			if (result != 0) goto EXIT;
		}
		m_CurNoteIndex++;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �������m�[�g��ԍX�V
//******************************************************************************
int MTNoteLyrics::_UpdateNoteStatus(
		unsigned long playTimeMSec,
		unsigned long decayDuration,
		unsigned long releaseDuration,
		SMNote note,
		NoteStatus* pNoteStatus
	)
{
	int result= 0;

	//�����I���m�[�g
	if(playTimeMSec > note.endTime) {
		//�m�[�g����j��
		pNoteStatus->isActive = false;
		pNoteStatus->index = 0;
		pNoteStatus->keyStatus = BeforeNoteON;
		pNoteStatus->keyDownRate = 0.0f;
		pNoteStatus->fontTexture.Clear();

		goto EXIT;
	}

	unsigned long noteLen = note.endTime - note.startTime;

	float decayRatio = 0.3f;
	float sustainRatio = 0.4f;
	float releaseRatio = 0.3f;

	//�̎��f�B�P�C���Ԃ���������蒷���ꍇ�A�f�B�P�C���������Ԃ܂łɏC������
	if(noteLen < decayDuration) {

		//decayDuration = noteLen;
		//releaseDuration = 0;

		//decayRatio = 0.3f;
		//sustainRatio = 0.0f;
		//releaseRatio = 0.0f;
	}
	//�̎��f�B�P�C�{�����[�X���Ԃ���������蒷���ꍇ�A�����[�X�J�n���Ԃ��f�B�P�C���Ԍo�ߒ���ɏC������
	else if(noteLen < (decayDuration + releaseDuration)) {

		releaseDuration = noteLen - decayDuration;
		decayRatio = 0.5f;
		sustainRatio = 0.0f;
		releaseRatio = 0.5f;
	}
	//���������i�̎��f�B�P�C�{�����[�X���ԁj�~�Q�ȓ��̏ꍇ�A�؂�ւ��_���f�B�P�C�I�����Ԃƃ����[�X�J�n���Ԃ̒��Ԃɂ���
	else if(noteLen < (decayDuration + releaseDuration) * 2) {

		unsigned long midTime = (note.startTime + decayDuration) / 2 + (note.endTime - releaseDuration) / 2;

		decayDuration = midTime - note.startTime;
		releaseDuration = note.endTime - midTime;

		decayRatio = 0.5f;
		sustainRatio = 0.0f;
		releaseRatio = 0.5f;
	}

	//�m�[�gON��i�������j
	if (playTimeMSec < (note.startTime + decayDuration)) {
		pNoteStatus->keyStatus = BeforeNoteON;
		if (decayDuration == 0) {
			pNoteStatus->keyDownRate = 0.0f;
		}
		else {
			pNoteStatus->keyDownRate = decayRatio * (float)(playTimeMSec - note.startTime) / (float)decayDuration;
		}
	}
	//�m�[�gON�����ォ�烊���[�X�O�܂�
	else if (((note.startTime + decayDuration) <= playTimeMSec)
			&& (playTimeMSec <= (note.endTime - releaseDuration))) {
		pNoteStatus->keyStatus = NoteON;

		unsigned long denominator = noteLen - (decayDuration + releaseDuration);
		if(denominator > 0) {
			pNoteStatus->keyDownRate = decayRatio + sustainRatio
					* (float)(playTimeMSec - (note.startTime + decayDuration)) / (float)denominator;
		} else {
			pNoteStatus->keyDownRate = decayRatio + sustainRatio;
		}
	}
	//�m�[�gOFF�O�i�����[�X���j
	else if (((note.endTime - releaseDuration) < playTimeMSec) && (playTimeMSec <= note.endTime)) {
		pNoteStatus->keyStatus = AfterNoteOFF;
		if (releaseDuration == 0) {
			pNoteStatus->keyDownRate = 1.0f;
		}
		else {
			pNoteStatus->keyDownRate = decayRatio + sustainRatio + releaseRatio
					* (float)(playTimeMSec - (note.endTime - releaseDuration)) / (float)releaseDuration;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// �̎��̒��_�X�V
//******************************************************************************
int MTNoteLyrics::_UpdateVertexOfLyrics(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	MTNOTELYRICS_VERTEX* pVertex = NULL;
	D3DXMATRIX mtxWorld;
	unsigned long i = 0;
	unsigned long activeNoteNum = 0;
	bool isTimeout = false;

	if (pD3DDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//�o�b�t�@�̃��b�N
	result = m_Primitive.LockVertex((void**)&pVertex);
	if (result != 0) goto EXIT;

	ZeroMemory(m_KeyDownRate, sizeof(float) * MTNOTELYRICS_MAX_PORT_NUM * SM_MAX_CH_NUM * SM_MAX_NOTE_NUM);

	//�������m�[�g�̉̎��ɂ��Ē��_���X�V
	for (i = 0; i < MTNOTELYRICS_MAX_LYRICS_NUM; i++) {
		if (m_pNoteStatus[i].isActive) {
			//�m�[�g���擾
			SMNote note;
			result = m_NoteListRT.GetNote(m_pNoteStatus[i].index, &note);
			if (result != 0) goto EXIT;

			//�����ΏۃL�[����]
			//  ���łɓ���m�[�g�ɑ΂��Ē��_���X�V���Ă���ꍇ
			//  ���������O���������ꍇ�Ɍ��蒸�_���X�V����
			if ((note.portNo < MTNOTELYRICS_MAX_PORT_NUM)
			 && (m_KeyDownRate[note.portNo][note.chNo][note.noteNo] < m_pNoteStatus[i].keyDownRate)) {
				//���_�X�V�F�̎��̕`��ʒu�ƃT�C�Y��ς���
				_SetVertexPosition(
						&(pVertex[activeNoteNum*6]),	//���_�o�b�t�@�������݈ʒu
						note,							//�m�[�g���
						&(m_pNoteStatus[i]),			//�m�[�g���
						i								//�m�[�g��ԓo�^�C���f�b�N�X�ʒu
					);
				m_pTextures[activeNoteNum] = m_pNoteStatus[i].fontTexture.GetTexture();
		 		activeNoteNum++;

				m_KeyDownRate[note.portNo][note.chNo][note.noteNo] = m_pNoteStatus[i].keyDownRate;
			}

		}
	}
	m_ActiveNoteNum = activeNoteNum;

	//�o�b�t�@�̃��b�N����
	result = m_Primitive.UnlockVertex();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �`��
//******************************************************************************
int MTNoteLyrics::Draw(
		LPDIRECT3DDEVICE9 pD3DDevice
   )
{
	int result = 0;

	if (pD3DDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	if (!m_isEnable) goto EXIT;

	//�e�N�X�`���X�e�[�W�ݒ�
	//  �J���[���Z�F��Z  ����1�F�e�N�X�`��  ����2�F�|���S��
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	// �A���t�@���Z�F����1���g�p  ����1�F�|���S��
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	//�e�N�X�`���t�B���^
	pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	//�����_�����O�X�e�[�g�ݒ�F���Z����
	//pD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	//�v���~�e�B�u�`��
	if (m_ActiveNoteNum > 0) {
		//�o�b�t�@�S�̂łȂ��̎��̐��ɍ��킹�ĕ`�悷��v���~�e�B�u�����炷
		result = m_Primitive.DrawLyrics(pD3DDevice, m_pTextures, 2 * m_ActiveNoteNum);
		if (result != 0) goto EXIT;
	}

	//�����_�����O�X�e�[�g�ݒ�F�ʏ�̃A���t�@����
	//pD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

EXIT:;
	return result;
}

//******************************************************************************
// ���
//******************************************************************************
void MTNoteLyrics::Release()
{
	m_Primitive.Release();

	delete [] m_pNoteStatus;
	m_pNoteStatus = NULL;
}

//******************************************************************************
// �m�[�g���z�񐶐�
//******************************************************************************
int MTNoteLyrics::_CreateNoteStatus()
{
	int result = 0;
	unsigned long i = 0;

	//�m�[�g���z�񐶐�
	try {
		m_pNoteStatus = new NoteStatus[MTNOTELYRICS_MAX_LYRICS_NUM];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

	//ZeroMemory(m_pNoteStatus, sizeof(NoteStatus) * MTNOTELYRICS_MAX_LYRICS_NUM);

	for (i = 0; i < MTNOTELYRICS_MAX_LYRICS_NUM; i++) {
		m_pNoteStatus[i].isActive = false;
		m_pNoteStatus[i].keyStatus = BeforeNoteON;
		m_pNoteStatus[i].index = 0;
		m_pNoteStatus[i].keyDownRate = 0.0f;
	}

EXIT:;
	return result;
}

//******************************************************************************
// ���_����
//******************************************************************************
int MTNoteLyrics::_CreateVertex(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	unsigned long vertexNum = 0;
	MTNOTELYRICS_VERTEX* pVertex = NULL;

	//�v���~�e�B�u������
	result = m_Primitive.Initialize(
					sizeof(MTNOTELYRICS_VERTEX),//���_�T�C�Y
					_GetFVFFormat(),			//���_FVF�t�H�[�}�b�g
					D3DPT_TRIANGLELIST			//�v���~�e�B�u���
				);
	if (result != 0) goto EXIT;

	//���_�o�b�t�@����
	vertexNum = 6 * MTNOTELYRICS_MAX_LYRICS_NUM;
	result = m_Primitive.CreateVertexBuffer(pD3DDevice, vertexNum);
	if (result != 0) goto EXIT;

	//�o�b�t�@�̃��b�N
	result = m_Primitive.LockVertex((void**)&pVertex);
	if (result != 0) goto EXIT;

	ZeroMemory(pVertex, sizeof(MTNOTELYRICS_VERTEX) * 6 * MTNOTELYRICS_MAX_LYRICS_NUM);

	//�o�b�t�@�̃��b�N����
	result = m_Primitive.UnlockVertex();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ���_�̍��W�ݒ�
//******************************************************************************
int MTNoteLyrics::_SetVertexPosition(
		MTNOTELYRICS_VERTEX* pVertex,
		SMNote note,
		NoteStatus* pNoteStatus,
		unsigned long rippleNo
	)
{
	int result = 0;
	unsigned long i = 0;
	float rh, rw = 0.0f;
	float alpha = 0.0f;
	D3DXVECTOR3 center;
	D3DXCOLOR color;
	short pbValue = 0;
	unsigned char pbSensitivity = SM_DEFAULT_PITCHBEND_SENSITIVITY;

	pbValue =       m_pNotePitchBend->GetValue(note.portNo, note.chNo);
	pbSensitivity = m_pNotePitchBend->GetSensitivity(note.portNo, note.chNo);

	//�m�[�g�{�b�N�X���S���W�擾
	center = m_NoteDesign.GetNoteBoxCenterPosX(
					m_CurTickTime,
					note.portNo,
					note.chNo,
					note.noteNo,
					pbValue,
					pbSensitivity
				);

	//�̎��T�C�Y
	unsigned long tx, ty;
	pNoteStatus->fontTexture.GetTextureSize(&tx, &ty);
	rh = tx * m_NoteDesign.GetDecayCoefficient(pNoteStatus->keyDownRate) / 64.0f;
	rw = ty * m_NoteDesign.GetDecayCoefficient(pNoteStatus->keyDownRate) / 64.0f;

	//�`��I���m�F
	if ((rh <= 0.0f) || (rw <= 0.0f)) {
		goto EXIT;
	}

	//�̎����Đ����ʏォ��J�������ɏ��������������ĕ`�悷��
	//�܂��̎����m�����ꕽ�ʏ�ŏd�Ȃ�Ȃ��悤�ɕ`�悷��
	//  Z�t�@�C�e�B���O�ɂ���Ĕ������邿����₩������������
	//  �O���t�B�b�N�J�[�h�ɂ���Č��ۂ��قȂ�
	if (center.x < m_CamVector.x) {
		center.x -= 0.002f * MTNOTELYRICS_MAX_LYRICS_NUM - (rippleNo + 1) * 0.002f;
	}
	else {
		center.x -= (rippleNo + 1) * 0.002f;
	}


	//���_���W
	//pVertex[0].p = D3DXVECTOR3(center.x, center.y+(rh/2.0f), center.z+(rw/2.0f));
	//pVertex[1].p = D3DXVECTOR3(center.x, center.y+(rh/2.0f), center.z-(rw/2.0f));
	//pVertex[2].p = D3DXVECTOR3(center.x, center.y-(rh/2.0f), center.z+(rw/2.0f));
	//pVertex[3].p = pVertex[2].p;
	//pVertex[4].p = pVertex[1].p;
	//pVertex[5].p = D3DXVECTOR3(center.x, center.y-(rh/2.0f), center.z-(rw/2.0f));
	pVertex[0].p = D3DXVECTOR3(center.x, center.y+(rh/2.0f), center.z-(rw/2.0f));
	pVertex[1].p = D3DXVECTOR3(center.x, center.y+(rh/2.0f), center.z+(rw/2.0f));
	pVertex[2].p = D3DXVECTOR3(center.x, center.y-(rh/2.0f), center.z+(rw/2.0f));
	pVertex[3].p = pVertex[0].p;
	pVertex[4].p = pVertex[2].p;
	pVertex[5].p = D3DXVECTOR3(center.x, center.y-(rh/2.0f), center.z-(rw/2.0f));
	//�@��
	for (i = 0; i < 6; i++) {
		pVertex[i].n = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
	}

	//�����x�����X�ɗ��Ƃ�
	alpha = m_NoteDesign.GetRippleAlpha(pNoteStatus->keyDownRate);

	//�e���_�̃f�B�t���[�Y�F
	for (i = 0; i < 6; i++) {
		color = m_NoteDesign.GetNoteBoxColor(
								note.portNo,
								note.chNo,
								note.noteNo
							);
		pVertex[i].c = D3DXCOLOR(color.r, color.g, color.b, alpha);
	}

	//�e�N�X�`�����W
	//pVertex[0].t = D3DXVECTOR2(0.0f, 0.0f);
	//pVertex[1].t = D3DXVECTOR2(1.0f, 0.0f);
	//pVertex[2].t = D3DXVECTOR2(0.0f, 1.0f);
	//pVertex[3].t = pVertex[2].t;
	//pVertex[4].t = pVertex[1].t;
	//pVertex[5].t = D3DXVECTOR2(1.0f, 1.0f);
	pVertex[0].t = D3DXVECTOR2(1.0f, 0.0f);
	pVertex[1].t = D3DXVECTOR2(0.0f, 0.0f);
	pVertex[2].t = D3DXVECTOR2(0.0f, 1.0f);
	pVertex[3].t = pVertex[0].t;
	pVertex[4].t = pVertex[2].t;
	pVertex[5].t = D3DXVECTOR2(1.0f, 1.0f);

EXIT:
	return result;
}

//******************************************************************************
// �}�e���A���쐬
//******************************************************************************
void MTNoteLyrics::_MakeMaterial(
		D3DMATERIAL9* pMaterial
	)
{
	ZeroMemory(pMaterial, sizeof(D3DMATERIAL9));
	
	//�g�U��
	pMaterial->Diffuse.r = 1.0f;
	pMaterial->Diffuse.g = 1.0f;
	pMaterial->Diffuse.b = 1.0f;
	pMaterial->Diffuse.a = 1.0f;
	//�����F�e�̐F
	pMaterial->Ambient.r = 0.5f;
	pMaterial->Ambient.g = 0.5f;
	pMaterial->Ambient.b = 0.5f;
	pMaterial->Ambient.a = 1.0f;
	//���ʔ��ˌ�
	pMaterial->Specular.r = 0.2f;
	pMaterial->Specular.g = 0.2f;
	pMaterial->Specular.b = 0.2f;
	pMaterial->Specular.a = 1.0f;
	//���ʔ��ˌ��̑N���x
	pMaterial->Power = 10.0f;
	//�����F
	pMaterial->Emissive.r = 0.5f;
	pMaterial->Emissive.g = 0.5f;
	pMaterial->Emissive.b = 0.5f;
	pMaterial->Emissive.a = 1.0f;
}

//******************************************************************************
// �J�����g�`�b�N�^�C���ݒ�
//******************************************************************************
void MTNoteLyrics::SetCurTickTime(
		unsigned long curTickTime
	)
{
	m_CurTickTime = curTickTime;
}

//******************************************************************************
// ���t���Ԑݒ�
//******************************************************************************
void MTNoteLyrics::SetPlayTimeMSec(
		unsigned long playTimeMsec
	)
{
	m_PlayTimeMSec = playTimeMsec;
}

//******************************************************************************
// ���Z�b�g
//******************************************************************************
void MTNoteLyrics::Reset()
{
	unsigned long i = 0;

	m_PlayTimeMSec = 0;
	m_CurTickTime = 0;
	m_CurNoteIndex = 0;
	m_ActiveNoteNum = 0;

	for (i = 0; i < MTNOTELYRICS_MAX_LYRICS_NUM; i++) {
		m_pNoteStatus[i].isActive = false;
		m_pNoteStatus[i].index = 0;
		m_pNoteStatus[i].keyStatus = BeforeNoteON;
		m_pNoteStatus[i].keyDownRate = 0.0f;
		m_pNoteStatus[i].fontTexture.Clear();
	}

	return;
}

//******************************************************************************
// �\���ݒ�
//******************************************************************************
void MTNoteLyrics::SetEnable(
		bool isEnable
	)
{
	m_isEnable = isEnable;
}

//******************************************************************************
// �X�L�b�v��Ԑݒ�
//******************************************************************************
void MTNoteLyrics::SetSkipStatus(
		bool isSkipping
	)
{
	m_isSkipping = isSkipping;
}


