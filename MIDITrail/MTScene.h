//******************************************************************************
//
// MIDITrail / MTScene
//
// MIDITrail �V�[�����N���X
//
// Copyright (C) 2010-2018 WADA Masashi. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "DXScene.h"
#include "SMIDILib.h"
#include <string>
#include <map>

using namespace SMIDILib;

#pragma warning(disable:4251)


//******************************************************************************
// MIDITrail �V�[�����N���X
//******************************************************************************
class MTScene : public DXScene
{
public:

	enum EffectType {
		EffectPianoKeyboard,
		EffectRipple,
		EffectPitchBend,
		EffectStars,
		EffectCounter,
		EffectBackgroundImage,
		EffectFileName
	};

	typedef std::map<std::string, float>  MTViewParamMap;
	typedef std::pair<std::string, float> MTViewParamMapPair;

public:

	//�R���X�g���N�^�^�f�X�g���N�^
	MTScene(void);
	virtual ~MTScene(void);

	//���̎擾
	virtual const TCHAR* GetName();

	//����
	virtual int Create(
					HWND hWnd,
					LPDIRECT3DDEVICE9 pD3DDevice,
					SMSeqData* pSeqData
				);

	//�ϊ�
	virtual int Transform(LPDIRECT3DDEVICE9 pD3DDevice);

	//�`��
	virtual int Draw(LPDIRECT3DDEVICE9 pD3DDevice);

	//�j��
	virtual void Release();

	//�E�B���h�E�N���b�N�C�x���g��M
	virtual int OnWindowClicked(
					UINT button,
					WPARAM wParam,
					LPARAM lParam
				);

	//���t�J�n�C�x���g��M
	virtual int OnPlayStart(LPDIRECT3DDEVICE9 pD3DDevice);

	//���t�I���C�x���g��M
	virtual int OnPlayEnd(LPDIRECT3DDEVICE9 pD3DDevice);

	//�V�[�P���T���b�Z�[�W��M
	virtual int OnRecvSequencerMsg(
					unsigned long param1,
					unsigned long param2
				);

	//�����߂�
	virtual int Rewind();

	//���_�擾�^�o�^
	virtual void GetDefaultViewParam(MTViewParamMap* pParamMap);
	virtual void GetViewParam(MTViewParamMap* pParamMap);
	virtual void SetViewParam(MTViewParamMap* pParamMap);
	virtual void MoveToStaticViewpoint(unsigned long viewpointNo);

	//���_���Z�b�g
	virtual void ResetViewpoint();

	//�\�����ʐݒ�
	virtual void SetEffect(EffectType type, bool isEnable);

	//���t���x�ݒ�
	virtual void SetPlaySpeedRatio(unsigned long ratio);

	//�p�����[�^�o�^�^�擾
	int SetParam(const char* pKey, const char* pValue);
	const char* GetParam(const char* pKey);

protected:

	typedef std::map<std::string, std::string> MTSceneParamDictionary;
	typedef std::pair<std::string, std::string> MTSceneParamDictionaryPair;

	//�V�[���p�����[�^
	MTSceneParamDictionary m_SceneParamDictionary;

};

#pragma warning(default:4251)


