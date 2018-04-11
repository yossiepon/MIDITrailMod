//******************************************************************************
//
// Simple MIDI Library / SMInDevCtrl
//
// MIDI���̓f�o�C�X����N���X
//
// Copyright (C) 2012-2014 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMInDevCtrl.h"

using namespace YNBaseLib;

namespace SMIDILib {


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
SMInDevCtrl::SMInDevCtrl(void)
{
	//�|�[�g���
	m_PortInfo.isExist = false;
	m_PortInfo.devId = 0;
	m_PortInfo.hMidiIn = NULL;
	memset((void*)&(m_PortInfo.midiHdr), 0, sizeof(MIDIHDR));
	
	//�R�[���o�b�N�֐�
	m_pInReadCallBack = NULL;
	m_pCallBackUserParam = NULL;
	
	//�p�P�b�g��͌n
	m_isContinueSysEx = false;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
SMInDevCtrl::~SMInDevCtrl()
{
	m_InDevList.clear();
	ClosePortDev();
}

//******************************************************************************
// ������
//******************************************************************************
int SMInDevCtrl::Initialize()
{
	int result = 0;
	
	//�|�[�g���N���A
	result = ClearPortInfo();
	if (result != 0) goto EXIT;
	
	//MIDI���̓f�o�C�X�ꗗ���쐬
	result = _InitDevList();
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// �f�o�C�X���X�g������
//******************************************************************************
int SMInDevCtrl::_InitDevList()
{
	int result = 0;
	MMRESULT apiresult = 0;
	unsigned long devId = 0;
	unsigned long devNum = 0;
	MIDIINCAPS mic;
	SMInDevInfo devInfo;

	m_InDevList.clear();

	//MIDI�o�̓f�o�C�X�̐�
	devNum = midiInGetNumDevs();

	//MIDI�o�̓f�o�C�X�̏����擾����
	for (devId = 0; devId < devNum; devId++) {

		ZeroMemory(&mic, sizeof(MIDIINCAPS));
		ZeroMemory(&devInfo, sizeof(SMInDevInfo));

		apiresult= midiInGetDevCaps(devId, &mic, sizeof(MIDIINCAPS));
		if (apiresult != MMSYSERR_NOERROR) {
			result = YN_SET_ERR("MIDI In device access error.", apiresult, 0);
			goto EXIT;
		}
		devInfo.devId = devId;
		memcpy(devInfo.productName, mic.szPname, MAXPNAMELEN);

		//�擾�����������X�g�ɓo�^
		m_InDevList.push_back(devInfo);
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// �f�o�C�X���擾
//******************************************************************************
unsigned long SMInDevCtrl::GetDevNum()
{
	return (unsigned long)m_InDevList.size();
}

//******************************************************************************
// �f�o�C�X�v���_�N�g���̎擾
//******************************************************************************
int SMInDevCtrl::GetDevProductName(
		unsigned long index,
		std::string& name
	)
{
	int result = 0;
	SMInDevListItr itr;
	
	if (index >= m_InDevList.size()) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	
	itr = m_InDevList.begin();
	advance(itr, index);
	
	name = itr->productName;
	
EXIT:;
	return result;
}

//******************************************************************************
// �|�[�g�ɑΉ�����f�o�C�X��ݒ�
//******************************************************************************
int SMInDevCtrl::SetPortDev(
		const char* pProductName
	)
{
	int result = 0;
	bool isFound = false;
	SMInDevListItr itr;
	
	if (pProductName == NULL) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	
	for (itr = m_InDevList.begin(); itr != m_InDevList.end(); itr++) {
		if (strcmp(itr->productName, pProductName) == 0) {
			m_PortInfo.isExist = true;
			m_PortInfo.devId = itr->devId;
			//m_PortInfo.hMidiIn = NULL;
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
// MIDI�C�x���g�ǂݍ��݃R�[���o�b�N�֐��o�^
//******************************************************************************
void SMInDevCtrl::SetInReadCallBack(
		SMInReadCallBack pCallBack,
		void* pUserParam
	)
{
	m_pInReadCallBack = pCallBack;
	m_pCallBackUserParam = pUserParam;
}

//******************************************************************************
// �|�[�g�ɑΉ�����f�o�C�X���J��
//******************************************************************************
int SMInDevCtrl::OpenPortDev()
{
	int result = 0;
	MMRESULT apiresult = 0;
	HMIDIIN hMidiIn = NULL;
	unsigned char* pBuf = NULL;
	
	result = ClosePortDev();
	if (result != 0) goto EXIT;
	
	//�|�[�g�����݂��Ȃ���΃X�L�b�v
	if (!m_PortInfo.isExist) goto EXIT;;
	
	m_isContinueSysEx = false;
	
	//�f�o�C�X���J��
	apiresult = midiInOpen(
					&hMidiIn,			//�n���h���̃A�h���X
					m_PortInfo.devId,	//�f�o�C�X���ʎq
					(DWORD_PTR)_InReadCallBack,	//�R�[���o�b�N�֐�
					(DWORD_PTR)this,	//�R�[���o�b�N�֐��ɓn�����[�U�[�C���X�^���X�f�[�^
					CALLBACK_FUNCTION	//�R�[���o�b�N�t���O�F�R�[���o�b�N�֐�
				);
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI OUT device open error.", apiresult, 0);
		goto EXIT;
	}
	m_PortInfo.hMidiIn = hMidiIn;
	
	//MIDI���̓o�b�t�@�쐬
	try {
		pBuf = new unsigned char[SM_MIDIIN_BUF_SIZE];
	}
	catch (std::bad_alloc) {
		result = YN_SET_ERR("Could not allocate memory.", 0, 0);
		goto EXIT;
	}
	
	//�w�b�_�쐬
	memset((void*)&(m_PortInfo.midiHdr), 0, sizeof(MIDIHDR));
	m_PortInfo.midiHdr.lpData         = (LPSTR)pBuf;
	m_PortInfo.midiHdr.dwBufferLength = SM_MIDIIN_BUF_SIZE;
	m_PortInfo.midiHdr.dwFlags        = 0;
	pBuf = NULL;
	
	//MIDI���̓o�b�t�@����
	apiresult = midiInPrepareHeader(hMidiIn, &(m_PortInfo.midiHdr), sizeof(MIDIHDR));
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI API error.", apiresult, 0);
		goto EXIT;
	}
	
	//MIDI���̓o�b�t�@�o�^
	apiresult = midiInAddBuffer(hMidiIn, &(m_PortInfo.midiHdr), sizeof(MIDIHDR));
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI API error.", apiresult, 0);
		goto EXIT;
	}
	
	//MIDI���͊J�n
	apiresult = midiInStart(m_PortInfo.hMidiIn);
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI OUT device open error.", apiresult, 0);
		goto EXIT;
	}
	
EXIT:;
	delete [] pBuf;
	return result;
}

//******************************************************************************
// �|�[�g�ɑΉ�����f�o�C�X�����
//******************************************************************************
int SMInDevCtrl::ClosePortDev()
{
	int result = 0;
	UINT apiresult = 0;
	
	//�|�[�g�����݂��Ȃ���΃X�L�b�v
	if (!m_PortInfo.isExist) goto EXIT;
	
	//�|�[�g���J���ĂȂ���΃X�L�b�v
	if (m_PortInfo.hMidiIn == NULL) goto EXIT;
	
	//MIDI���͒�~
	//  �L���[�Ƀo�b�t�@�����݂���ꍇ�͌��݂̃o�b�t�@�͏����ς݂ɂ����
	//  MIDIHDR��dwBytesRecorded�����o�ɂ̓f�[�^�̎��ۂ̒���������
	//  �������L���[�ɂ����̃o�b�t�@�͎c���ꏈ���ς݂Ƃ͂���Ȃ�
	apiresult = midiInStop(m_PortInfo.hMidiIn);
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI OUT device close error.", apiresult, 0);
		goto EXIT;
	}
	
	//MIDI���͒�~
	//  �������̓��̓o�b�t�@���R�[���o�b�N�֐��ɕԂ�
	//  MIDIHDR��dwFlags�����o��MHDR_DONE�t���O���Z�b�g����
	apiresult = midiInReset(m_PortInfo.hMidiIn);
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI OUT device close error.", apiresult, 0);
		goto EXIT;
	}
	
	//MIDI���̓o�b�t�@����
	apiresult = midiInUnprepareHeader(m_PortInfo.hMidiIn, &(m_PortInfo.midiHdr), sizeof(MIDIHDR));
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI OUT device close error.", apiresult, 0);
		goto EXIT;
	}
	
	//�o�b�t�@�j��
	delete [] (unsigned char*)(m_PortInfo.midiHdr.lpData);
	m_PortInfo.midiHdr.lpData = NULL;
	
	//�f�o�C�X�����
	apiresult = midiInClose(m_PortInfo.hMidiIn);
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI OUT device close error.", 0, 0);
		goto EXIT;
	}
	m_PortInfo.hMidiIn = NULL;
	
EXIT:;
	return result;
}


//******************************************************************************
// �|�[�g���N���A
//******************************************************************************
int SMInDevCtrl::ClearPortInfo()
{
	int result = 0;
	
	result = ClosePortDev();
	if (result != 0) goto EXIT;
	
	m_PortInfo.isExist = false;
	m_PortInfo.devId = 0;
	m_PortInfo.hMidiIn = NULL;
	memset((void*)&(m_PortInfo.midiHdr), 0, sizeof(MIDIHDR));
	
EXIT:;
	return result;
}

//******************************************************************************
// MIDI IN �ǂݍ��݃R�[���o�b�N�֐�
//******************************************************************************
void SMInDevCtrl::_InReadCallBack(
		HMIDIIN hMidiIn,
		UINT wMsg,
		DWORD_PTR dwInstance,
		DWORD_PTR dwParam1,
		DWORD_PTR dwParam2
	)
{
	SMInDevCtrl* pInDevCtrl = NULL;
	
	pInDevCtrl = (SMInDevCtrl*)dwInstance;
	pInDevCtrl->_InReadProc(hMidiIn, wMsg, dwParam1, dwParam2);
	
	return;
}

//******************************************************************************
// MIDI IN �ǂݍ��ݏ���
//******************************************************************************
void SMInDevCtrl::_InReadProc(
		HMIDIIN hMidiIn,
		UINT wMsg,
		DWORD_PTR dwParam1,
		DWORD_PTR dwParam2
	)
{
	int result = 0;
	SMEvent event;
	
	switch (wMsg) {
		case MIM_OPEN:
			//MIDI���̓f�o�C�X�I�[�v��
			break;
		case MIM_CLOSE:
			//MIDI���̓f�o�C�X�N���[�Y
			break;
		case MIM_DATA:
			//MIDI���b�Z�[�W��M
			//  dwParam1 MIDI���b�Z�[�W
			//  dwParam2 �^�C���X�^���v
			m_isContinueSysEx = false;
			result = _InReadProcMIDI(dwParam1, dwParam2, &event);
			if (result != 0) goto EXIT;
			break;
		case MIM_LONGDATA:
			//�V�X�e���G�N�X�N���[�V�u��M
			//  dwParam1 MIDIHDR�\���̂ւ̃|�C���^
			//  dwParam2 �^�C���X�^���v
			result = _InReadProcSysEx((MIDIHDR*)dwParam1, dwParam2, &m_isContinueSysEx, &event);
			if (result != 0) goto EXIT;
			break;
		case MIM_ERROR:
			//������MIDI���b�Z�[�W��M
			break;
		case MIM_LONGERROR:
			//�����ȃG�N�X�N���[�V�u���b�Z�[�W��M
			break;
		case MIM_MOREDATA:
			//��������MIDI���b�Z�[�W
			//midiInOpen��MIDI_IO_STATUS���w�肵���ꍇ�̂ݔ�������
			break;
		default:
			break;
	}
	
	//�R�[���o�b�N�Ăяo��
	if ((m_pInReadCallBack != NULL) &&
		(event.GetType() != SMEvent::EventNone)) {
		result = m_pInReadCallBack(&event, m_pCallBackUserParam);
		if (result != 0) goto EXIT;
	}
	
EXIT:;
	if (result != 0) {
		YN_SHOW_ERR(NULL);
	}
	return;
}

//******************************************************************************
// MIDI���b�Z�[�W�ǂݍ��ݏ���
//******************************************************************************
int SMInDevCtrl::_InReadProcMIDI(
		DWORD_PTR midiMessage,
		DWORD_PTR timestamp,
		SMEvent* pEvent
	)
{
	int result = 0;
	unsigned char status = 0;
	unsigned char data[2] = { 0, 0 };
	unsigned long dataLength = 0;
	
	status  = (unsigned char)((midiMessage      ) & 0x000000FF);
	data[0] = (unsigned char)((midiMessage >>  8) & 0x000000FF);
	data[1] = (unsigned char)((midiMessage >> 16) & 0x000000FF);
	
	if ((status & 0xF0) != 0xF0) {
		//MIDI���b�Z�[�W
		dataLength = _GetMIDIMsgSize(status) - 1;
		result = pEvent->SetMIDIData(status, data, dataLength);
		if (result != 0) goto EXIT;
	}
	else if (status == 0xF0) {
		//�V�X�e���G�N�X�N���[�V�u���b�Z�[�W
		//���肦�Ȃ�API�̋���
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	else {
		//�V�X�e���R�������b�Z�[�W�܂��̓V�X�e�����A���^�C�����b�Z�[�W
		dataLength = _GetSysMsgSize(status) - 1;
		result = pEvent->SetSysMsgData(status, data, dataLength);
		if (result != 0) goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// �V�X�e���G�N�X�N���[�V�u�ǂݍ��ݏ���
//******************************************************************************
int SMInDevCtrl::_InReadProcSysEx(
		MIDIHDR* pMIDIHDR,
		DWORD_PTR timestamp,
		bool* pIsContinueSysEx,
		SMEvent* pEvent
	)
{
	int result = 0;
	unsigned char* pData = NULL;
	UINT apiresult = 0;
	
	//��M�f�[�^�T�C�Y���[���Ȃ牽�����Ȃ�
	if (pMIDIHDR->dwBytesRecorded == 0) goto EXIT;
	
	//�V�X�e���G�N�X�N���[�V�u����ǂݍ���
	if (!(*pIsContinueSysEx)) {
		pData = (unsigned char*)(pMIDIHDR->lpData);
		result = pEvent->SetSysExData(0xF0, pData + 1, pMIDIHDR->dwBytesRecorded - 1);
		if (result != 0) goto EXIT;
	}
	//2�Ԗڈȍ~�̃p�P�b�g
	else {
		pData = (unsigned char*)(pMIDIHDR->lpData);
		result = pEvent->SetSysExData(0xF7, pData, pMIDIHDR->dwBytesRecorded);
		if (result != 0) goto EXIT;
	}
	
	//�V�X�e���G�N�X�N���[�V�u�̏I�[���m�F
	if (pData[(pMIDIHDR->dwBytesRecorded)-1] == 0xF7) {
		//�V�X�e���G�N�X�N���[�V�u������
		*pIsContinueSysEx = false;
	}
	else {
		//������0xF7�łȂ���Ύ��Ƀf�[�^���܂�����
		*pIsContinueSysEx = true;
	}
	
	//MIDI���̓o�b�t�@����
	apiresult = midiInPrepareHeader(m_PortInfo.hMidiIn, &(m_PortInfo.midiHdr), sizeof(MIDIHDR));
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI API error.", apiresult, 0);
		goto EXIT;
	}
	
	//MIDI���̓o�b�t�@�o�^
	apiresult = midiInAddBuffer(m_PortInfo.hMidiIn, &(m_PortInfo.midiHdr), sizeof(MIDIHDR));
	if (apiresult != MMSYSERR_NOERROR) {
		result = YN_SET_ERR("MIDI API error.", apiresult, 0);
		goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// MIDI���b�Z�[�W�T�C�Y�擾
//******************************************************************************
unsigned long SMInDevCtrl::_GetMIDIMsgSize(unsigned char status)
{
	unsigned long size = 0;

	switch (status & 0xF0) {
		case 0x80: size = 3; break;  //�m�[�g�I�t
		case 0x90: size = 3; break;  //�m�[�g�I��
		case 0xA0: size = 3; break;  //�|���t�H�j�b�N�L�[�v���b�V���[
		case 0xB0: size = 3; break;  //�R���g���[���`�F���W
		case 0xC0: size = 2; break;  //�v���O�����`�F���W
		case 0xD0: size = 2; break;  //�`�����l���v���b�V���[
		case 0xE0: size = 3; break;  //�s�b�`�x���h
		case 0xF0:
			size = _GetSysMsgSize(status);
			break;
	}
	
	return size;
}

//******************************************************************************
// �V�X�e�����b�Z�[�W�T�C�Y�擾
//******************************************************************************
unsigned long SMInDevCtrl::_GetSysMsgSize(unsigned char status)
{
	unsigned long size = 0;
	
	switch (status) {
		case 0xF0: size = 0; break;  // F0 ... F7 �V�X�e���G�N�X�N���[�V�u
		case 0xF1: size = 2; break;  // F1 dd     �V�X�e���R�������b�Z�[�W�F�N�I�[�^�[�t���[��(MTC)
		case 0xF2: size = 3; break;  // F2 dl dm  �V�X�e���R�������b�Z�[�W�F�\���O�|�W�V�����|�C���^
		case 0xF3: size = 2; break;  // F3 dd     �V�X�e���R�������b�Z�[�W�F�\���O�Z���N�g
		case 0xF4: size = 1; break;  // F4 ����`
		case 0xF5: size = 1; break;  // F5 ����`
		case 0xF6: size = 1; break;  // F6 �V�X�e���R�������b�Z�[�W�F�`���[�����N�G�X�g
		case 0xF7: size = 1; break;  // F7 �G���h�I�u�V�X�e���G�N�X�N���[�V�u
		case 0xF8: size = 1; break;  // F8 �V�X�e�����A���^�C�����b�Z�[�W�F�^�C�~���O�N���b�N
		case 0xF9: size = 1; break;  // F9 ����`
		case 0xFA: size = 1; break;  // FA �V�X�e�����A���^�C�����b�Z�[�W�F�X�^�[�g
		case 0xFB: size = 1; break;  // FB �V�X�e�����A���^�C�����b�Z�[�W�F�R���e�B�j���[
		case 0xFC: size = 1; break;  // FC �V�X�e�����A���^�C�����b�Z�[�W�F�X�g�b�v
		case 0xFD: size = 1; break;  // FD ����`
		case 0xFE: size = 1; break;  // FE �V�X�e�����A���^�C�����b�Z�[�W�F�A�N�e�B�u�Z���V���O
		case 0xFF: size = 1; break;  // FF �V�X�e�����A���^�C�����b�Z�[�W�F�V�X�e�����Z�b�g
	}
	
	return size;
}

} // end of namespace


