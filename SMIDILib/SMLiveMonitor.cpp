//******************************************************************************
//
// Simple MIDI Library / SMLiveMonitor
//
// ���C�u���j�^�N���X
//
// Copyright (C) 2012-2013 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "YNBaseLib.h"
#include "SMLiveMonitor.h"
#include "SMEventMIDI.h"
#include "SMEventSysEx.h"
#include "SMEventSysMsg.h"

using namespace YNBaseLib;

namespace SMIDILib {


//******************************************************************************
// �R���X�g���N�^
//******************************************************************************
SMLiveMonitor::SMLiveMonitor(void)
{	
	m_Status = StatusMonitorOFF;
	m_isMIDITHRU = true;
}

//******************************************************************************
// �f�X�g���N�^
//******************************************************************************
SMLiveMonitor::~SMLiveMonitor()
{
	//�|�[�g���N���A
	_ClearPortInfo();
	
	//MIDI�f�o�C�X�����
	_CloseMIDIDev();
}

//******************************************************************************
// ������
//******************************************************************************
int SMLiveMonitor::Initialize(
		SMMsgQueue* pMsgQueue
	)
{
	int result = 0;
	
	m_pMsgQue = pMsgQueue;	
	
	//MIDI�o�̓f�o�C�X������
	result = m_OutDevCtrl.Initialize();
	if (result != 0) goto EXIT;
	
	//MIDI���̓f�o�C�X������
	result = m_InDevCtrl.Initialize();
	if (result != 0) goto EXIT;
	
	//�|�[�g���N���A
	_ClearPortInfo();
	
	//�C�x���g�]���I�u�W�F�N�g������
	result = m_MsgTrans.Initialize(pMsgQueue);
	if (result != 0) goto EXIT;
	
	//�C�x���g�E�H�b�`���[������
	result = m_EventWatcher.Initialize(&m_MsgTrans);
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// ���̓|�[�g�Ή��f�o�C�X�o�^
//******************************************************************************
int SMLiveMonitor::SetInPortDev(
		const char* pProductName,
		bool isMIDITHRU
	)
{
	int result = 0;
	errno_t eresult = 0;
	
	eresult = strcpy_s(m_InPortDevName, MAXPNAMELEN, pProductName);
	if (eresult != 0) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	
	m_isMIDITHRU = isMIDITHRU;
	
EXIT:;
	return result;
}

//******************************************************************************
// �o�̓|�[�g�Ή��f�o�C�X�o�^
//******************************************************************************
int SMLiveMonitor::SetOutPortDev(
		const char* pProductName
	)
{
	int result = 0;
	errno_t eresult = 0;
	
	eresult = strcpy_s(m_OutPortDevName, MAXPNAMELEN, pProductName);
	if (eresult != 0) {
		result = YN_SET_ERR("Program error.", 0, 0);
		goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// ���̓|�[�g�f�o�C�X�\�����̎擾
//******************************************************************************
int SMLiveMonitor::GetInPortDevDisplayName(
		std::string& name
	)
{
	int result = 0;
	
	name = m_InPortDevName;
	
	return result;
}

//******************************************************************************
// ���j�^�J�n
//******************************************************************************
int SMLiveMonitor::Start()
{
	int result = 0;
	
	//���j�^���Ȃ牽�����Ȃ�
	if (m_Status == StatusMonitorON) goto EXIT;
		
	//MIDI�f�o�C�X���J��
	result = _OpenMIDIDev();
	if (result != 0) goto EXIT;
	
	m_Status = StatusMonitorON;
	
EXIT:;
	return result;
}

//******************************************************************************
// ���j�^��~
//******************************************************************************
int SMLiveMonitor::Stop()
{
	int result = 0;
	
	//�S�g���b�N�m�[�g�I�t
	result = m_OutDevCtrl.NoteOffAll();
	if (result != 0) goto EXIT;
	
	//MIDI�f�o�C�X�����
	result = _CloseMIDIDev();
	if (result != 0) goto EXIT;

	m_Status = StatusMonitorOFF;
	
EXIT:;
	return result;
}

//******************************************************************************
// �|�[�g���N���A
//******************************************************************************
void SMLiveMonitor::_ClearPortInfo()
{
	m_InPortDevName[0] = '\0';
	m_OutPortDevName[0] = '\0';
}

//******************************************************************************
// MIDI�f�o�C�X�I�[�v��
//******************************************************************************
int SMLiveMonitor::_OpenMIDIDev()
{
	int result = 0;
	
	//�o�̓|�[�g�̃f�o�C�X���J��
	if (strlen(m_OutPortDevName) > 0) {
		result = m_OutDevCtrl.SetPortDev(0, m_OutPortDevName);
		if (result != 0) goto EXIT;
	}
	result = m_OutDevCtrl.OpenPortDevAll();
	if (result != 0) goto EXIT;
	
	//���̓|�[�g�̃f�o�C�X���J��
	if (strlen(m_InPortDevName) > 0) {
		result = m_InDevCtrl.SetPortDev(m_InPortDevName);
		if (result != 0) goto EXIT;
		
		//�R�[���o�b�N�o�^
		m_InDevCtrl.SetInReadCallBack(_InReadCallBack, this);
	}
	result = m_InDevCtrl.OpenPortDev();
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// MIDI�f�o�C�X�N���[�Y
//******************************************************************************
int SMLiveMonitor::_CloseMIDIDev()
{
	int result = 0;
	
	//���̓|�[�g�̃f�o�C�X�����
	result = m_InDevCtrl.ClosePortDev();
	if (result != 0) goto EXIT;
	
	//�o�̓|�[�g�̃f�o�C�X�����
	result = m_OutDevCtrl.ClosePortDevAll();
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// MIDI IN �ǂݍ��݃R�[���o�b�N
//******************************************************************************
int SMLiveMonitor::_InReadCallBack(
		SMEvent* pEvent,
		void* pUserParam
	)
{
	int result = 0;
	SMLiveMonitor* pLiveMonitor = NULL;
	
	pLiveMonitor = (SMLiveMonitor*)pUserParam;
	
	if (pLiveMonitor != NULL) {
		result = pLiveMonitor->_InReadProc(pEvent);
		if (result != 0) goto EXIT;
	}
	
EXIT:;
	return result;
}

//******************************************************************************
// MIDI IN �ǂݍ��ݏ���
//******************************************************************************
int SMLiveMonitor::_InReadProc(SMEvent* pEvent)
{
	int result = 0;
	
	//MIDI�C�x���g��I�ʂ��ă��b�Z�[�W�L���[�ɓo�^
	//�R���g���[���`�F���W�̊Ď�����
	result = _InReadProcParseEvent(pEvent);
	if (result != 0) goto EXIT;
	
	//MIDI�o�̓f�o�C�X�ɏo��
	result = _InReadProcMIDITHRU(pEvent);
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// MIDI IN �ǂݍ��ݏ����F�C�x���g���
//******************************************************************************
int SMLiveMonitor::_InReadProcParseEvent(SMEvent* pEvent)
{
	int result = 0;
	unsigned char portNo = 0;
	
	//�C�x���g�E�H�b�`
	result = m_EventWatcher.WatchEvent(portNo, pEvent);
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// MIDI IN �ǂݍ��ݏ����FMIDITHRU����
//******************************************************************************
int SMLiveMonitor::_InReadProcMIDITHRU(SMEvent* pEvent)
{
	int result = 0;
	unsigned char portNo = 0;
	
	//MIDITHRU�I�t�Ȃ�Ȃɂ����Ȃ�
	if (!m_isMIDITHRU) goto EXIT;
	
	//MIDI�C�x���g���M
	if (pEvent->GetType() == SMEvent::EventMIDI) {
		result = _InReadProcSendMIDIEvent(portNo, pEvent);
		if (result != 0) goto EXIT;
	}
	//�V�X�e���G�N�X�N���[�V�u���M
	else if (pEvent->GetType() == SMEvent::EventSysEx) {
		result = _InReadProcSendSysExEvent(portNo, pEvent);
		if (result != 0) goto EXIT;
	}
	//�V�X�e�����b�Z�[�W���M
	else if (pEvent->GetType() == SMEvent::EventSysMsg) {
		result = _InReadProcSendSysMsgEvent(portNo, pEvent);
		if (result != 0) goto EXIT;
	}
	
EXIT:;	
	return result;
}

//******************************************************************************
// MIDI�C�x���g���M
//******************************************************************************
int SMLiveMonitor::_InReadProcSendMIDIEvent(
		unsigned char portNo,
		SMEvent* pEvent
	)
{
	int result = 0;
	unsigned long msg = 0;
	SMEventMIDI midiEvent;
	
	midiEvent.Attach(pEvent);
	
	//���b�Z�[�W�擾
	result = midiEvent.GetMIDIOutShortMsg(&msg);
	if (result != 0) goto EXIT;
	
	//���b�Z�[�W�o��
	result = m_OutDevCtrl.SendShortMsg(portNo, msg);
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// SysEx�C�x���g���M
//******************************************************************************
int SMLiveMonitor::_InReadProcSendSysExEvent(
		unsigned char portNo,
		SMEvent* pEvent
	)
{
	int result = 0;
	unsigned char* pVarMsg = NULL;
	unsigned long size = 0;
	SMEventSysEx sysExEvent;
	
	sysExEvent.Attach(pEvent);
	
	//���b�Z�[�W�擾
	sysExEvent.GetMIDIOutLongMsg(&pVarMsg, &size);
	
	//���b�Z�[�W�o�́F�o�͊����܂Ő��䂪�߂�Ȃ�
	result = m_OutDevCtrl.SendLongMsg(portNo, pVarMsg, size);
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

//******************************************************************************
// SysMsg�C�x���g���M
//******************************************************************************
int SMLiveMonitor::_InReadProcSendSysMsgEvent(
		unsigned char portNo,
		SMEvent* pEvent
	)
{
	int result = 0;
	unsigned long msg = 0;
	unsigned long size = 0;
	SMEventSysMsg sysMsgEvent;
	
	sysMsgEvent.Attach(pEvent);
	
	//���b�Z�[�W�擾
	result = sysMsgEvent.GetMIDIOutShortMsg(&msg, &size);
	if (result != 0) goto EXIT;
	
	//���b�Z�[�W�o��
	result = m_OutDevCtrl.SendShortMsg(portNo, msg);
	if (result != 0) goto EXIT;
	
EXIT:;
	return result;
}

} // end of namespace


