//******************************************************************************
//
// MIDITrail / MTCaptionBase11
//
// Common base class for DX11 caption renderers.
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "StdAfx.h"
#include "MTCaptionBase11.h"

using namespace DirectX::SimpleMath;


//******************************************************************************
// Constructor / Destructor
//******************************************************************************
MTCaptionBase11::MTCaptionBase11()
{
	m_Color = Color(1.0f, 1.0f, 1.0f, 1.0f);
	m_isReady = false;
}

MTCaptionBase11::~MTCaptionBase11()
{
	Release();
}

//******************************************************************************
// Release
//******************************************************************************
void MTCaptionBase11::Release()
{
	m_FontTexture.Clear();
	m_Primitive.Release();
	m_isReady = false;
}

//******************************************************************************
// Set color
//******************************************************************************
void MTCaptionBase11::SetColor(Color color)
{
	m_Color = color;
}

//******************************************************************************
// Get texture size
//******************************************************************************
void MTCaptionBase11::GetTextureSize(unsigned long* pHeight, unsigned long* pWidth)
{
	m_FontTexture.GetTextureSize(pHeight, pWidth);
}
