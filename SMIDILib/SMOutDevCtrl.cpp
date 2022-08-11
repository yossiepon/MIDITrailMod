//******************************************************************************
//
// Simple MIDI Library / SMOutDevCtrl
//
// MIDI�o�̓f�o�C�X����N���X
//
// Copyright (C) 2010-2021 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMOutDevCtrl.h"

using namespace YNBaseLib;

namespace SMIDILib {


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
SMOutDevCtrl::SMOutDevCtrl(void)
{
	unsigned char portNo = 0;

	for (portNo = 0; portNo < SM_MIDIOUT_PORT_NUM_MAX; portNo++) {
		m_PortInfo[portNo].isExist = false;
		m_PortInfo[portNo].devId = 0xFFFFFFFF;
		m_PortInfo[portNo].hMIDIOut = NULL;
	}
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
SMOutDevCtrl::~SMOutDevCtrl(void)
{
}

//******************************************************************************
// ������
//******************************************************************************
int SMOutDevCtrl::Initialize()
{
	int result = 0;

	//�|�[�g���N���A
	result = ClearPortInfo();
	if (result != 0) goto EXIT;

	//MIDI�o�̓f�o�C�X�ꗗ���쐬
	result = _InitDevList();
	if (result != 0) goto EXIT;

EXIT:;
	return result;
}

//******************************************************************************
// �f�o�C�X���X�g������
//******************************************************************************
int SMOutDevCtrl::_InitDevList()
{
	int result = 0;
	MMRESULT apiresult = 0;
	unsigned long devId = 0;
	unsigned long devNum = 0;
	MIDIOUTCAPS moc;
	SMOutDevInfo devInfo;

	m_OutDevList.clear();

	//MIDI�o�̓f�o�C�X�̐�
	devNum = midiOutGetNumDevs();

	//MIDI�o�̓f�o�C�X�̏����擾����
	for (devId = 0; devId < devNum; devId++) {

		ZeroMemory(&moc, sizeof(MIDIOUTCAPS));
		ZeroMemory(&devInfo, sizeof(SMOutDevInfo));

		apiresult= midiOutGetDevCaps(devId, &moc, sizeof(MIDIOUTCAPS));
		if (apiresult != MMSYSERR_NOERROR) {
			result = YN_SET_ERR("MIDI OUT device access error.", apiresult, 0);
			goto EXIT;
		}
		devInfo.devId = devId;
		memcpy(devInfo.productName, moc.szPname, MAXPNAMELEN);

		//�擾�����������X�g�ɓo�^
		m_OutDevList.push_back(devInfo);
	}

EXIT:;
	return result;
}

//******************************************************************************
// �f�o�C�X���擾
//******************************************************************************
unsigned long SMOutDevCtrl::GetDevNum()
{
	return (unsigned long)m_OutDevList.size();
}

//******************************************************************************
// �f�o�C�X�v���_�N�g���̎擾
//******************************************************************************
int SMOutDevCtrl::GetDevProductName(
		unsigned long index,
		std::string& name
	)
{
	int result = 0;
	SMOutDevListItr itr;

	if (index >= m_OutDevList.size()) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	itr = m_OutDevList.begin();
	advance(itr, index);

	name = itr->productName;

EXIT:;
	return result;
}

//******************************************************************************
// �|�[�g�ɑΉ�����f�o�C�X��ݒ�
//******************************************************************************
int SMOutDevCtrl::SetPortDev(
		unsigned char portNo,
		const char* pProductName
	)
{
	int result = 0;
	bool isFound = false;
	SMOutDevListItr itr;

	if (portNo >= SM_MIDIOUT_PORT_NUM_MAX) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	if (pProductName == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	for (itr = m_OutDevList.begin(); itr != m_OutDevList.end(); itr++) {
		if (strcmp(itr->productName, pProductName) == 0) {
			m_PortInfo[portNo].isExist = true;
			m_PortInfo[portNo].devId = itr->devId;
			//m_PortInfo[portNo].hMIDIOut = NULL;
			isFound = true;
			break;
		}
	}
	if (!isFound) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �|�[�g�ɑΉ�����f�o�C�XID���擾
//******************************************************************************
int SMOutDevCtrl::GetPortDevId(
		unsigned char portNo,
		unsigned long* pDevId
	)
{
	int result = 0;

	if (portNo >= SM_MIDIOUT_PORT_NUM_MAX) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	if (pDevId == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	if (!m_PortInfo[portNo].isExist) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	*pDevId = m_PortInfo[portNo].devId;

EXIT:;
	return result;
}

//******************************************************************************
// �S�|�[�g�ɑΉ�����f�o�C�X���J��
//******************************************************************************
int SMOutDevCtrl::OpenPortDevAll()
{
	int result = 0;
	UINT apiresult = 0;
	unsigned char portNo = 0;
	unsigned char prevPortNo = 0;
	unsigned long devId;
	HMIDIOUT hMIDIOut = NULL;
	bool isOpen = false;

	result = ClosePortDevAll();
	if (result != 0) goto EXIT;

	for (portNo = 0; portNo < SM_MIDIOUT_PORT_NUM_MAX; portNo++) {

		//�|�[�g�����݂��Ȃ���΃X�L�b�v
		if (!m_PortInfo[portNo].isExist) continue;

		//�|�[�g�ɑΉ�����f�o�C�XID���擾
		devId = m_PortInfo[portNo].devId;

		//�ʂ̃|�[�g�œ����f�o�C�X�����łɊJ���Ă���ꍇ�̑Ώ�
		isOpen = false;
		for (prevPortNo = 0; prevPortNo < portNo; prevPortNo++) {
			if (devId == m_PortInfo[prevPortNo].devId) {
				m_PortInfo[portNo].hMIDIOut = m_PortInfo[prevPortNo].hMIDIOut;
				isOpen = true;
				break;
			}
		}

		//�V�K�Ƀf�o�C�X���J��
		if (!isOpen) {
			apiresult = midiOutOpen(
							&hMIDIOut,      //�n���h��
							devId,          //MIDI�o�̓f�o�C�X���ʎq
							NULL,           //�Đ��i���󋵃R�[���o�b�N�֐�
							NULL,           //�R�[���o�b�N�֐��ɓn�����[�U�[�C���X�^���X�f�[�^
							CALLBACK_NULL   //�R�[���o�b�N�t���O�F�R�[���o�b�N�Ȃ�
						);
			if (apiresult != MMSYSERR_NOERROR) {
				result = YN_SET_ERR("MIDI OUT device open error.", apiresult, 0);
				goto EXIT;
			}
			m_PortInfo[portNo].hMIDIOut = hMIDIOut;
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// �S�|�[�g�ɑΉ�����f�o�C�X�����
//******************************************************************************
int SMOutDevCtrl::ClosePortDevAll()
{
	int result = 0;
	UINT apiresult = 0;
	unsigned char portNo = 0;
	unsigned char nextPortNo = 0;

	for (portNo = 0; portNo < SM_MIDIOUT_PORT_NUM_MAX; portNo++) {

		//�|�[�g�����݂��Ȃ���΃X�L�b�v
		if (!m_PortInfo[portNo].isExist) continue;

		//�f�o�C�X���J���ĂȂ���΃X�L�b�v
		if (m_PortInfo[portNo].hMIDIOut == NULL) continue;

		//�f�o�C�X�����
		apiresult = midiOutClose(m_PortInfo[portNo].hMIDIOut);
		if (apiresult != MMSYSERR_NOERROR) {
			result = YN_SET_ERR("MIDI OUT device close error.", 0, 0);
			goto EXIT;
		}
		m_PortInfo[portNo].hMIDIOut = NULL;

		//�ʂ̃|�[�g�œ����f�o�C�X���J���Ă���ꍇ�̑Ώ�
		for (nextPortNo = portNo+1; nextPortNo < SM_MIDIOUT_PORT_NUM_MAX; nextPortNo++) {
			if (m_PortInfo[portNo].devId == m_PortInfo[nextPortNo].devId) {
				m_PortInfo[nextPortNo].hMIDIOut = NULL;
			}
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// �|�[�g���N���A
//******************************************************************************
int SMOutDevCtrl::ClearPortInfo()
{
	int result = 0;
	unsigned char portNo = 0;

	result = ClosePortDevAll();
	if (result != 0) goto EXIT;

	for (portNo = 0; portNo < SM_MIDIOUT_PORT_NUM_MAX; portNo++) {
		m_PortInfo[portNo].isExist = false;
		m_PortInfo[portNo].devId = 0xFFFFFFFF;
		m_PortInfo[portNo].hMIDIOut = NULL;
	}

EXIT:;
	return result;
}

//******************************************************************************
// MIDI�f�[�^���M�i�V���[�g���b�Z�[�W�j
//******************************************************************************
int SMOutDevCtrl::SendShortMsg(
		unsigned char portNo,
		unsigned long msg
	)
{
	int result = 0;
	UINT apiresult = 0;
	HMIDIOUT hMIDIOut = NULL;

	//�T�|�[�g�͈͊O�̃|�[�g���w�肳�ꂽ�ꍇ�͉������Ȃ�
	if (portNo >= SM_MIDIOUT_PORT_NUM_MAX) goto EXIT;

	//�|�[�g�����݂��Ȃ���Ή������Ȃ�
	if (!m_PortInfo[portNo].isExist) goto EXIT;

	//�f�o�C�X���J����Ă��Ȃ���΃G���[
	if (m_PortInfo[portNo].hMIDIOut == NULL) {
		result = YN_SET_ERR("Program error.", portNo, 0);
		goto EXIT;
	}
	hMIDIOut = m_PortInfo[portNo].hMIDIOut;

	//���b�Z�[�W�o�́FMIDI�d�l�ɂ���0.3msec�|����
	apiresult = midiOutShortMsg(hMIDIOut, msg);
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI OUT device output error.", apiresult, msg);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// MIDI�f�[�^���M�i�����O���b�Z�[�W�j
//******************************************************************************
int SMOutDevCtrl::SendLongMsg(
		unsigned char portNo,
		unsigned char* pMsg,
		unsigned long size
	)
{
	int result = 0;
	UINT apiresult = 0;
	HMIDIOUT hMIDIOut = NULL;
	MIDIHDR mh;

	//�p�����[�^�`�F�b�N
	if (pMsg == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}

	//�T�|�[�g�͈͊O�̃|�[�g���w�肳�ꂽ�ꍇ�͉������Ȃ�
	if (portNo >= SM_MIDIOUT_PORT_NUM_MAX) goto EXIT;

	//�|�[�g�����݂��Ȃ���Ή������Ȃ�
	if (!m_PortInfo[portNo].isExist) goto EXIT;

	//�f�o�C�X���J����Ă��Ȃ���΃G���[
	if (m_PortInfo[portNo].hMIDIOut == NULL) {
		result = YN_SET_ERR("Program error.", portNo, 0);
		goto EXIT;
	}
	hMIDIOut = m_PortInfo[portNo].hMIDIOut;

	//�w�b�_�쐬
	memset((void*)&mh, 0, sizeof(MIDIHDR));
	mh.lpData         = (LPSTR)pMsg;
	mh.dwBufferLength = size;
	mh.dwFlags        = 0;

	//�o�̓o�b�t�@����
	apiresult = midiOutPrepareHeader(hMIDIOut, &mh, sizeof(MIDIHDR));
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI OUT device output error.", apiresult, size);
		goto EXIT;
	}
	//���b�Z�[�W�o�́FMIDI�d�l�ɂ���0.3msec�ȏ�|����
	apiresult = midiOutLongMsg(hMIDIOut, &mh, sizeof(MIDIHDR));
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI OUT device output error.", apiresult, size);
		goto EXIT;
	}

	//�o�͊����܂ő҂����킹
	while ((mh.dwFlags & MHDR_DONE) == 0) {
		//�R�[���o�b�NI/F���Ȃ��̂ł������邵���E�E�E
	}

	//�o�̓o�b�t�@���
	apiresult = midiOutUnprepareHeader(hMIDIOut, &mh, sizeof(MIDIHDR));
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI OUT device output error.", apiresult, size);
		goto EXIT;
	}

EXIT:;
	return result;
}

//******************************************************************************
// �S�|�[�g�m�[�g�I�t
//******************************************************************************
int SMOutDevCtrl::NoteOffAll()
{
	int result = 0;
	int i = 0;
	UINT apiresult = 0;
	unsigned long msg = 0;
	unsigned char portNo = 0;

	for (portNo = 0; portNo < SM_MIDIOUT_PORT_NUM_MAX; portNo++) {

		//�|�[�g�ƃf�o�C�X�����݂��Ȃ���΃X�L�b�v
		if (!m_PortInfo[portNo].isExist) continue;
		if (m_PortInfo[portNo].hMIDIOut == NULL) continue;

		//�S�g���b�N�m�[�g�I�t
		for (i = 0; i < 16; i++) {
			msg = (0x7B << 8) | (0xB0 | i);
			apiresult = midiOutShortMsg(m_PortInfo[portNo].hMIDIOut, msg);
			if (apiresult != MMSYSERR_NOERROR) {
				result = YN_SET_ERR("MIDI OUT device output error.", apiresult, portNo);
				goto EXIT;
			}
		}
	}

EXIT:;
	return result;
}

//******************************************************************************
// �S�|�[�g�T�E���h�I�t
//******************************************************************************
int SMOutDevCtrl::SoundOffAll()
{
	int result = 0;
	int i = 0;
	UINT apiresult = 0;
	unsigned long msg = 0;
	unsigned char portNo = 0;

	for (portNo = 0; portNo < SM_MIDIOUT_PORT_NUM_MAX; portNo++) {

		//�|�[�g�ƃf�o�C�X�����݂��Ȃ���΃X�L�b�v
		if (!m_PortInfo[portNo].isExist) continue;
		if (m_PortInfo[portNo].hMIDIOut == NULL) continue;

		//�S�g���b�N�T�E���h�I�t
		for (i = 0; i < 16; i++) {
			msg = (0x78 << 8) | (0xB0 | i);
			apiresult = midiOutShortMsg(m_PortInfo[portNo].hMIDIOut, msg);
			if (apiresult != MMSYSERR_NOERROR) {
				result = YN_SET_ERR("MIDI OUT device output error.", apiresult, portNo);
				goto EXIT;
			}
		}
	}

EXIT:;
	return result;
}

} // end of namespace

