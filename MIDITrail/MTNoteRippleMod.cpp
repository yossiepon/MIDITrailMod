//******************************************************************************
//
// MIDITrail / MTNoteRippleMod
//
// �m�[�g�g��`��Mod�N���X
//
// Copyright (C) 2012 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "MTNoteRippleMod.h"
#include "MTNoteLyrics.h"
#include <new>

using namespace YNBaseLib;

//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
MTNoteRippleMod::MTNoteRippleMod(void) : MTNoteRipple()
{
	m_pNoteDesignMod = NULL;
	m_PlayTimeMSec = 0;
	m_CurNoteIndex = 0;
	m_pNoteStatusMod = NULL;
	ZeroMemory(m_KeyDownRate, sizeof(float) * MTNOTERIPPLE_MAX_PORT_NUM * SM_MAX_CH_NUM * SM_MAX_NOTE_NUM);
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
MTNoteRippleMod::~MTNoteRippleMod(void)
{
	Release();
}

//******************************************************************************
// �m�[�g�g�䐶��
//******************************************************************************
int MTNoteRippleMod::Create(
		LPDIRECT3DDEVICE9 pD3DDevice,
		const TCHAR* pSceneName,
		SMSeqData* pSeqData,
		MTNotePitchBend* pNotePitchBend
   )
{
	int result = 0;
	SMTrack track;

	//�������уm�[�g�f�U�C��Mod�I�u�W�F�N�g�����͊��N���X����I�[�o���C�h�o�R�ōs��

	// ���N���X�̐����������Ăяo��
	result = MTNoteRipple::Create(pD3DDevice, pSceneName, pSeqData, pNotePitchBend);
	if (result != 0) goto EXIT;

	//�g���b�N�擾
	result = pSeqData->GetMergedTrack(&track);
	if (result != 0) goto EXIT;

	//�m�[�g���X�g�擾�FstartTime, endTime �̓��A���^�C��(msec)
	result = track.GetNoteListWithRealTime(&m_NoteListRT, pSeqData->GetTimeDivision());
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �m�[�g�f�U�C������
//******************************************************************************
int MTNoteRippleMod::_CreateNoteDesign()
{
	int result = 0;

	try {
		//�m�[�g�f�U�C��Mod�I�u�W�F�N�g����
		m_pNoteDesignMod = new MTNoteDesignMod();
		m_pNoteDesign = m_pNoteDesignMod;
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �g��̒��_�X�V
//******************************************************************************
int MTNoteRippleMod::_TransformRipple(
		LPDIRECT3DDEVICE9 pD3DDevice
   )
{
	int result = 0;

	//�X�L�b�v���Ȃ牽�����Ȃ�
	if (m_isSkipping) goto EXIT;

	//�g��̏�ԍX�V
	result = _UpdateStatusOfRipple(pD3DDevice);
	if (result != 0) goto EXIT;

	//�g��̒��_�X�V
	result = _UpdateVertexOfRipple(pD3DDevice);
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �g��̏�ԍX�V
//******************************************************************************
int MTNoteRippleMod::_UpdateStatusOfRipple(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	unsigned long i = 0;
	bool isFound = false;
	bool isRegist = false;
	SMNote note;

	//�g��f�B�P�C�E�����[�X����(msec)
	unsigned long decayDuration = m_pNoteDesignMod->GetRippleDecayDuration();
	unsigned long releaseDuration   = m_pNoteDesignMod->GetRippleReleaseDuration();

	//�m�[�g�����X�V����
	for (i = 0; i < MTNOTERIPPLE_MAX_RIPPLE_NUM; i++) {
		if (m_pNoteStatusMod[i].isActive) {
			//�m�[�g���擾
			result = m_NoteListRT.GetNote(m_pNoteStatusMod[i].index, &note);
			if (result != 0) goto EXIT;

			//�������m�[�g��ԍX�V
			result = _UpdateNoteStatus(
							m_PlayTimeMSec,
							decayDuration,
							releaseDuration,
							note,
							&(m_pNoteStatusMod[i])
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
		if ((note.startTime <= m_PlayTimeMSec) && (m_PlayTimeMSec <= note.endTime) && (note.lyric[0] == '\0')) {
			isRegist = true;
		}

		//�m�[�g���o�^
		//  �L�[���~���^�㏸���̏����o�^�ΏۂƂ��Ă��邽��
		//  ����m�[�g�ŕ����G���g�������ꍇ�����邱�Ƃɒ��ӂ���
		if (isRegist) {
			//���łɓ���C���f�b�N�X�œo�^�ς݂̏ꍇ�͉������Ȃ�
			isFound = false;
			for (i = 0; i < MTNOTERIPPLE_MAX_RIPPLE_NUM; i++) {
				if ((m_pNoteStatusMod[i].isActive)
				 && (m_pNoteStatusMod[i].index == m_CurNoteIndex)) {
					isFound = true;
					break;
				}
			}
			//�󂢂Ă���Ƃ���ɒǉ�����
			if (!isFound) {
				for (i = 0; i < MTNOTERIPPLE_MAX_RIPPLE_NUM; i++) {
					if (!(m_pNoteStatusMod[i].isActive)) {
						m_pNoteStatusMod[i].isActive = true;
						m_pNoteStatusMod[i].keyStatus = BeforeNoteON;
						m_pNoteStatusMod[i].index = m_CurNoteIndex;
						m_pNoteStatusMod[i].keyDownRate = 0.0f;
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
							&(m_pNoteStatusMod[i])
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
int MTNoteRippleMod::_UpdateNoteStatus(
		unsigned long playTimeMSec,
		unsigned long decayDuration,
		unsigned long releaseDuration,
		SMNote note,
		NoteStatusMod* pNoteStatus
	)
{
	int result= 0;

	//�����I���m�[�g
	if(playTimeMSec > note.endTime) {
		//�m�[�g����j��
		pNoteStatus->isActive = false;
		pNoteStatus->keyStatus = BeforeNoteON;
		pNoteStatus->index = 0;
		pNoteStatus->keyDownRate = 0.0f;

		goto EXIT;
	}

	unsigned long noteLen = note.endTime - note.startTime;

	float decayRatio = 0.3f;
	float sustainRatio = 0.4f;
	float releaseRatio = 0.3f;

	//�g��f�B�P�C���Ԃ���������蒷���ꍇ�A�f�B�P�C���������Ԃ܂łɏC������
	if(noteLen < decayDuration) {

		//decayDuration = noteLen;
		//releaseDuration = 0;

		//decayRatio = 0.3f;
		//sustainRatio = 0.0f;
		//releaseRatio = 0.0f;
	}
	//�g��f�B�P�C�{�����[�X���Ԃ���������蒷���ꍇ�A�����[�X�J�n���Ԃ��f�B�P�C���Ԍo�ߒ���ɏC������
	else if(noteLen < (decayDuration + releaseDuration)) {

		releaseDuration = noteLen - decayDuration;
		decayRatio = 0.5f;
		sustainRatio = 0.0f;
		releaseRatio = 0.5f;
	}
	//���������i�g��f�B�P�C�{�����[�X���ԁj�~�Q�ȓ��̏ꍇ�A�؂�ւ��_���f�B�P�C�I�����Ԃƃ����[�X�J�n���Ԃ̒��Ԃɂ���
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
// �g��̒��_�X�V
//******************************************************************************
int MTNoteRippleMod::_UpdateVertexOfRipple(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	MTNOTERIPPLE_VERTEX* pVertex = NULL;
	D3DXMATRIX mtxWorld;
	unsigned long i = 0;
	unsigned long j = 0;
	unsigned long activeNoteNum = 0;
	bool isTimeout = false;

	if (pD3DDevice == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//�o�b�t�@�̃��b�N
	result = m_Primitive.LockVertex((void**)&pVertex);
	if (result != 0) goto EXIT;

	ZeroMemory(m_KeyDownRate, sizeof(float) * MTNOTERIPPLE_MAX_PORT_NUM * SM_MAX_CH_NUM * SM_MAX_NOTE_NUM);

	// �g��㏑����
	unsigned long overwriteTimes = m_pNoteDesignMod->GetRippleOverwriteTimes();

	//�������m�[�g�̔g��ɂ��Ē��_���X�V
	for (i = 0; i < MTNOTERIPPLE_MAX_RIPPLE_NUM; i++) {
		if (m_pNoteStatusMod[i].isActive) {
			//�m�[�g���擾
			SMNote note;
			result = m_NoteListRT.GetNote(m_pNoteStatusMod[i].index, &note);
			if (result != 0) goto EXIT;

			//�����ΏۃL�[����]
			//  ���łɓ���m�[�g�ɑ΂��Ē��_���X�V���Ă���ꍇ
			//  ���������O���������ꍇ�Ɍ��蒸�_���X�V����
			if ((note.portNo < MTNOTERIPPLE_MAX_PORT_NUM)
			 && (m_KeyDownRate[note.portNo][note.chNo][note.noteNo] < m_pNoteStatusMod[i].keyDownRate)) {
				//���_�X�V�F�g��̕`��ʒu�ƃT�C�Y��ς���
				for(j = 0; j < overwriteTimes; j++) {
					_SetVertexPosition(
							&(pVertex[activeNoteNum*6]),	//���_�o�b�t�@�������݈ʒu
							note,							//�m�[�g���
							&(m_pNoteStatusMod[i]),			//�m�[�g���
							i								//�m�[�g��ԓo�^�C���f�b�N�X�ʒu
						);
			 		activeNoteNum++;
			 	}

				m_KeyDownRate[note.portNo][note.chNo][note.noteNo] = m_pNoteStatusMod[i].keyDownRate;
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
int MTNoteRippleMod::Draw(
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

	//�����_�����O�X�e�[�g�ݒ�F�u�����h�w��l
	pD3DDevice->SetRenderState(D3DRS_SRCBLEND, m_pNoteDesignMod->GetRippleSrcBlend());
	pD3DDevice->SetRenderState(D3DRS_DESTBLEND, m_pNoteDesignMod->GetRippleDestBlend());

	//�v���~�e�B�u�`��
	if (m_ActiveNoteNum > 0) {
		//�o�b�t�@�S�̂łȂ��g��̐��ɍ��킹�ĕ`�悷��v���~�e�B�u�����炷
		result = m_Primitive.Draw(pD3DDevice, m_pTexture, 2 * m_ActiveNoteNum);
		if (result != 0) goto EXIT;
	}

	//�����_�����O�X�e�[�g�ݒ�F�ʏ�̃A���t�@����
	pD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

EXIT:;
	return result;
}

//******************************************************************************
// ���
//******************************************************************************
void MTNoteRippleMod::Release()
{
	delete m_pNoteDesignMod;
	m_pNoteDesignMod = NULL;
	m_pNoteDesign = NULL;

	delete [] m_pNoteStatusMod;
	m_pNoteStatusMod = NULL;

	MTNoteRipple::Release();
}

//******************************************************************************
// �m�[�g���z�񐶐�
//******************************************************************************
int MTNoteRippleMod::_CreateNoteStatus()
{
	int result = 0;
	unsigned long i = 0;

	//�m�[�g���z�񐶐�
	try {
		m_pNoteStatusMod = new NoteStatusMod[MTNOTERIPPLE_MAX_RIPPLE_NUM];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}

	for (i = 0; i < MTNOTERIPPLE_MAX_RIPPLE_NUM; i++) {
		m_pNoteStatusMod[i].isActive = false;
		m_pNoteStatusMod[i].keyStatus = BeforeNoteON;
		m_pNoteStatusMod[i].index = 0;
		m_pNoteStatusMod[i].keyDownRate = 0.0f;
	}

EXIT:;
	return result;
}

//******************************************************************************
// ���_����
//******************************************************************************
int MTNoteRippleMod::_CreateVertex(
		LPDIRECT3DDEVICE9 pD3DDevice
	)
{
	int result = 0;
	unsigned long vertexNum = 0;
	MTNOTERIPPLE_VERTEX* pVertex = NULL;

	// �g��㏑����
	unsigned long overwriteTimes = m_pNoteDesignMod->GetRippleOverwriteTimes();

	//�v���~�e�B�u������
	result = m_Primitive.Initialize(
					sizeof(MTNOTERIPPLE_VERTEX),//���_�T�C�Y
					_GetFVFFormat(),			//���_FVF�t�H�[�}�b�g
					D3DPT_TRIANGLELIST			//�v���~�e�B�u���
				);
	if (result != 0) goto EXIT;

	//���_�o�b�t�@����
	vertexNum = 6 * MTNOTERIPPLE_MAX_RIPPLE_NUM * overwriteTimes;
	result = m_Primitive.CreateVertexBuffer(pD3DDevice, vertexNum);
	if (result != 0) goto EXIT;

	//�o�b�t�@�̃��b�N
	result = m_Primitive.LockVertex((void**)&pVertex);
	if (result != 0) goto EXIT;

	ZeroMemory(pVertex, sizeof(MTNOTERIPPLE_VERTEX) * 6 * MTNOTERIPPLE_MAX_RIPPLE_NUM * overwriteTimes);

	//�o�b�t�@�̃��b�N����
	result = m_Primitive.UnlockVertex();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// ���_�̍��W�ݒ�
//******************************************************************************
int MTNoteRippleMod::_SetVertexPosition(
		MTNOTERIPPLE_VERTEX* pVertex,
		SMNote note,
		NoteStatusMod* pNoteStatus,
		unsigned long rippleNo
	)
{
	int result = 0;
	unsigned long i = 0;
	float rh, rw = 0.0f;
	float spacing = 0.0f;
	float alpha = 0.0f;
	D3DXVECTOR3 center;
	D3DXCOLOR color;
	short pbValue = 0;
	unsigned char pbSensitivity = SM_DEFAULT_PITCHBEND_SENSITIVITY;

	pbValue =       m_pNotePitchBend->GetValue(note.portNo, note.chNo);
	pbSensitivity = m_pNotePitchBend->GetSensitivity(note.portNo, note.chNo);

	//�m�[�g�{�b�N�X���S���W�擾
	center = m_pNoteDesign->GetNoteBoxCenterPosX(
					m_CurTickTime,
					note.portNo,
					note.chNo,
					note.noteNo,
					pbValue,
					pbSensitivity
				);

	//�g��T�C�Y
	rh = m_pNoteDesignMod->GetRippleHeight(pNoteStatus->keyDownRate);
	rw = m_pNoteDesignMod->GetRippleWidth(pNoteStatus->keyDownRate);

	//�g��`��Ԋu
	spacing = m_pNoteDesignMod->GetRippleSpacing();

	//�`��I���m�F
	if ((rh <= 0.0f) || (rw <= 0.0f)) {
		goto EXIT;
	}

	//�g����Đ����ʏォ��J�������ɏ��������������ĕ`�悷��
	//�܂��g�䓯�m�����ꕽ�ʏ�ŏd�Ȃ�Ȃ��悤�ɕ`�悷��
	//  Z�t�@�C�e�B���O�ɂ���Ĕ������邿����₩������������
	//  �O���t�B�b�N�J�[�h�ɂ���Č��ۂ��قȂ�
	if (center.x < m_CamVector.x) {
		center.x -= spacing * (MTNOTELYRICS_MAX_LYRICS_NUM + MTNOTERIPPLE_MAX_RIPPLE_NUM - (rippleNo + 1));
	}
	else {
		center.x -= spacing * (MTNOTELYRICS_MAX_LYRICS_NUM + rippleNo + 1);
	}

	//���_���W
	pVertex[0].p = D3DXVECTOR3(center.x, center.y+(rh/2.0f), center.z+(rw/2.0f));
	pVertex[1].p = D3DXVECTOR3(center.x, center.y+(rh/2.0f), center.z-(rw/2.0f));
	pVertex[2].p = D3DXVECTOR3(center.x, center.y-(rh/2.0f), center.z+(rw/2.0f));
	pVertex[3].p = pVertex[2].p;
	pVertex[4].p = pVertex[1].p;
	pVertex[5].p = D3DXVECTOR3(center.x, center.y-(rh/2.0f), center.z-(rw/2.0f));

	//�@��
	for (i = 0; i < 6; i++) {
		pVertex[i].n = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	}

	//�����x�����X�ɗ��Ƃ�
	alpha = m_pNoteDesignMod->GetRippleAlpha(pNoteStatus->keyDownRate);

	//�e���_�̃f�B�t���[�Y�F
	for (i = 0; i < 6; i++) {
		color = m_pNoteDesign->GetNoteBoxColor(
			note.portNo,
			note.chNo,
			note.noteNo
		);
		pVertex[i].c = D3DXCOLOR(color.r, color.g, color.b, alpha);
	}

	//�e�N�X�`�����W
	pVertex[0].t = D3DXVECTOR2(0.0f, 0.0f);
	pVertex[1].t = D3DXVECTOR2(1.0f, 0.0f);
	pVertex[2].t = D3DXVECTOR2(0.0f, 1.0f);
	pVertex[3].t = pVertex[2].t;
	pVertex[4].t = pVertex[1].t;
	pVertex[5].t = D3DXVECTOR2(1.0f, 1.0f);

EXIT:
	return result;
}

//******************************************************************************
// �}�e���A���쐬
//******************************************************************************
void MTNoteRippleMod::_MakeMaterial(
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
// ���t���Ԑݒ�
//******************************************************************************
void MTNoteRippleMod::SetPlayTimeMSec(
		unsigned long playTimeMsec
	)
{
	m_PlayTimeMSec = playTimeMsec;
}

//******************************************************************************
// ���Z�b�g
//******************************************************************************
void MTNoteRippleMod::Reset()
{
	unsigned long i = 0;

	m_PlayTimeMSec = 0;
	m_CurTickTime = 0;
	m_CurNoteIndex = 0;
	m_ActiveNoteNum = 0;

	for (i = 0; i < MTNOTERIPPLE_MAX_RIPPLE_NUM; i++) {
		m_pNoteStatusMod[i].isActive = false;
		m_pNoteStatusMod[i].keyStatus = BeforeNoteON;
		m_pNoteStatusMod[i].index = 0;
		m_pNoteStatusMod[i].keyDownRate = 0.0f;
	}

	return;
}
