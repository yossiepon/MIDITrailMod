//******************************************************************************
//
// MIDITrail / DXMath9
//
// M5: self-contained replacement for the small d3dx9 math / color subset that
// MIDITrail uses, so the build no longer depends on the DirectX SDK (June 2010).
//
// Backed by <d3d9.h> (Windows SDK) base types + standard math only. Layouts are
// byte-identical to the real D3DX types (D3DXMATRIX derives from D3DMATRIX,
// D3DXVECTOR3 from D3DVECTOR, D3DXCOLOR matches D3DCOLORVALUE), and the matrix
// builders reproduce the exact D3DX left-handed formulas, so existing code -
// including the DX11 path's XMLoadFloat4x4((const XMFLOAT4X4*)&mat) reinterpret -
// keeps behaving identically.
//
//******************************************************************************

#pragma once

#include <d3d9.h>   // D3DMATRIX, D3DVECTOR, D3DCOLOR, D3DCOLORVALUE (Windows SDK)
#include <math.h>

#ifndef D3DX_PI
#define D3DX_PI 3.14159265358979323846
#endif

#define D3DXToRadian(degree) ((float)((degree) * (D3DX_PI / 180.0)))
#define D3DXToDegree(radian) ((float)((radian) * (180.0 / D3DX_PI)))


//==============================================================================
// D3DXVECTOR2
//==============================================================================
struct D3DXVECTOR2
{
	float x, y;

	D3DXVECTOR2() {}
	D3DXVECTOR2(float fx, float fy) : x(fx), y(fy) {}

	operator float* ()             { return &x; }
	operator const float* () const { return &x; }

	D3DXVECTOR2 operator + (const D3DXVECTOR2& v) const { return D3DXVECTOR2(x + v.x, y + v.y); }
	D3DXVECTOR2 operator - (const D3DXVECTOR2& v) const { return D3DXVECTOR2(x - v.x, y - v.y); }
	D3DXVECTOR2 operator * (float s) const { return D3DXVECTOR2(x * s, y * s); }
};


//==============================================================================
// D3DXVECTOR3 (layout matches D3DVECTOR: float x, y, z)
//==============================================================================
struct D3DXVECTOR3 : public D3DVECTOR
{
	D3DXVECTOR3() {}
	D3DXVECTOR3(float fx, float fy, float fz) { x = fx; y = fy; z = fz; }
	D3DXVECTOR3(const D3DVECTOR& v) { x = v.x; y = v.y; z = v.z; }

	operator float* ()             { return &x; }
	operator const float* () const { return &x; }

	D3DXVECTOR3& operator += (const D3DXVECTOR3& v) { x += v.x; y += v.y; z += v.z; return *this; }
	D3DXVECTOR3& operator -= (const D3DXVECTOR3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
	D3DXVECTOR3& operator *= (float s) { x *= s; y *= s; z *= s; return *this; }
	D3DXVECTOR3& operator /= (float s) { x /= s; y /= s; z /= s; return *this; }

	D3DXVECTOR3 operator + () const { return *this; }
	D3DXVECTOR3 operator - () const { return D3DXVECTOR3(-x, -y, -z); }

	D3DXVECTOR3 operator + (const D3DXVECTOR3& v) const { return D3DXVECTOR3(x + v.x, y + v.y, z + v.z); }
	D3DXVECTOR3 operator - (const D3DXVECTOR3& v) const { return D3DXVECTOR3(x - v.x, y - v.y, z - v.z); }
	D3DXVECTOR3 operator * (float s) const { return D3DXVECTOR3(x * s, y * s, z * s); }
	D3DXVECTOR3 operator / (float s) const { return D3DXVECTOR3(x / s, y / s, z / s); }

	bool operator == (const D3DXVECTOR3& v) const { return x == v.x && y == v.y && z == v.z; }
	bool operator != (const D3DXVECTOR3& v) const { return x != v.x || y != v.y || z != v.z; }
};

inline D3DXVECTOR3 operator * (float s, const D3DXVECTOR3& v) { return D3DXVECTOR3(v.x * s, v.y * s, v.z * s); }


//==============================================================================
// D3DXVECTOR4
//==============================================================================
struct D3DXVECTOR4
{
	float x, y, z, w;

	D3DXVECTOR4() {}
	D3DXVECTOR4(float fx, float fy, float fz, float fw) : x(fx), y(fy), z(fz), w(fw) {}

	operator float* ()             { return &x; }
	operator const float* () const { return &x; }

	D3DXVECTOR4 operator + (const D3DXVECTOR4& v) const { return D3DXVECTOR4(x + v.x, y + v.y, z + v.z, w + v.w); }
	D3DXVECTOR4 operator - (const D3DXVECTOR4& v) const { return D3DXVECTOR4(x - v.x, y - v.y, z - v.z, w - v.w); }
	D3DXVECTOR4 operator * (float s) const { return D3DXVECTOR4(x * s, y * s, z * s, w * s); }
};


//==============================================================================
// D3DXMATRIX (layout matches D3DMATRIX: row-major _11.._44 / m[4][4])
//==============================================================================
struct D3DXMATRIX : public D3DMATRIX
{
	D3DXMATRIX() {}
	D3DXMATRIX(const D3DMATRIX& mat) { *((D3DMATRIX*)this) = mat; }
	D3DXMATRIX(float f11, float f12, float f13, float f14,
	           float f21, float f22, float f23, float f24,
	           float f31, float f32, float f33, float f34,
	           float f41, float f42, float f43, float f44)
	{
		_11 = f11; _12 = f12; _13 = f13; _14 = f14;
		_21 = f21; _22 = f22; _23 = f23; _24 = f24;
		_31 = f31; _32 = f32; _33 = f33; _34 = f34;
		_41 = f41; _42 = f42; _43 = f43; _44 = f44;
	}

	float& operator () (unsigned int row, unsigned int col)       { return m[row][col]; }
	float  operator () (unsigned int row, unsigned int col) const { return m[row][col]; }

	operator float* ()             { return &_11; }
	operator const float* () const { return &_11; }

	D3DXMATRIX operator * (const D3DXMATRIX& mat) const
	{
		D3DXMATRIX out;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				out.m[i][j] = m[i][0] * mat.m[0][j]
				            + m[i][1] * mat.m[1][j]
				            + m[i][2] * mat.m[2][j]
				            + m[i][3] * mat.m[3][j];
			}
		}
		return out;
	}
	D3DXMATRIX& operator *= (const D3DXMATRIX& mat) { *this = *this * mat; return *this; }
};


//==============================================================================
// D3DXCOLOR (layout matches D3DCOLORVALUE: float r, g, b, a)
//==============================================================================
struct D3DXCOLOR
{
	float r, g, b, a;

	D3DXCOLOR() {}
	D3DXCOLOR(float fr, float fg, float fb, float fa) : r(fr), g(fg), b(fb), a(fa) {}
	D3DXCOLOR(const D3DCOLORVALUE& c) : r(c.r), g(c.g), b(c.b), a(c.a) {}
	D3DXCOLOR(DWORD argb)
	{
		const float f = 1.0f / 255.0f;
		a = f * (float)(unsigned char)(argb >> 24);
		r = f * (float)(unsigned char)(argb >> 16);
		g = f * (float)(unsigned char)(argb >>  8);
		b = f * (float)(unsigned char)(argb >>  0);
	}

	operator DWORD () const
	{
		DWORD dr = (DWORD)(r <= 0.0f ? 0 : (r >= 1.0f ? 255 : (r * 255.0f + 0.5f)));
		DWORD dg = (DWORD)(g <= 0.0f ? 0 : (g >= 1.0f ? 255 : (g * 255.0f + 0.5f)));
		DWORD db = (DWORD)(b <= 0.0f ? 0 : (b >= 1.0f ? 255 : (b * 255.0f + 0.5f)));
		DWORD da = (DWORD)(a <= 0.0f ? 0 : (a >= 1.0f ? 255 : (a * 255.0f + 0.5f)));
		return (da << 24) | (dr << 16) | (dg << 8) | db;
	}

	operator D3DCOLORVALUE& ()             { return *((D3DCOLORVALUE*)this); }
	operator const D3DCOLORVALUE& () const { return *((const D3DCOLORVALUE*)this); }
	operator float* ()             { return &r; }
	operator const float* () const { return &r; }

	D3DXCOLOR operator * (float s) const { return D3DXCOLOR(r * s, g * s, b * s, a * s); }
	D3DXCOLOR operator + (const D3DXCOLOR& c) const { return D3DXCOLOR(r + c.r, g + c.g, b + c.b, a + c.a); }
};


//==============================================================================
// Matrix builders - exact D3DX left-handed formulas
//==============================================================================
inline D3DXMATRIX* D3DXMatrixIdentity(D3DXMATRIX* p)
{
	p->_11 = 1; p->_12 = 0; p->_13 = 0; p->_14 = 0;
	p->_21 = 0; p->_22 = 1; p->_23 = 0; p->_24 = 0;
	p->_31 = 0; p->_32 = 0; p->_33 = 1; p->_34 = 0;
	p->_41 = 0; p->_42 = 0; p->_43 = 0; p->_44 = 1;
	return p;
}

inline D3DXMATRIX* D3DXMatrixMultiply(D3DXMATRIX* out, const D3DXMATRIX* a, const D3DXMATRIX* b)
{
	D3DXMATRIX r;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			r.m[i][j] = a->m[i][0] * b->m[0][j]
			          + a->m[i][1] * b->m[1][j]
			          + a->m[i][2] * b->m[2][j]
			          + a->m[i][3] * b->m[3][j];
		}
	}
	*out = r;
	return out;
}

inline D3DXMATRIX* D3DXMatrixTranslation(D3DXMATRIX* out, float x, float y, float z)
{
	D3DXMatrixIdentity(out);
	out->_41 = x; out->_42 = y; out->_43 = z;
	return out;
}

inline D3DXMATRIX* D3DXMatrixScaling(D3DXMATRIX* out, float sx, float sy, float sz)
{
	D3DXMatrixIdentity(out);
	out->_11 = sx; out->_22 = sy; out->_33 = sz;
	return out;
}

inline D3DXMATRIX* D3DXMatrixRotationX(D3DXMATRIX* out, float angle)
{
	float c = cosf(angle), s = sinf(angle);
	D3DXMatrixIdentity(out);
	out->_22 = c; out->_23 = s;
	out->_32 = -s; out->_33 = c;
	return out;
}

inline D3DXMATRIX* D3DXMatrixRotationY(D3DXMATRIX* out, float angle)
{
	float c = cosf(angle), s = sinf(angle);
	D3DXMatrixIdentity(out);
	out->_11 = c; out->_13 = -s;
	out->_31 = s; out->_33 = c;
	return out;
}

inline D3DXMATRIX* D3DXMatrixRotationZ(D3DXMATRIX* out, float angle)
{
	float c = cosf(angle), s = sinf(angle);
	D3DXMatrixIdentity(out);
	out->_11 = c; out->_12 = s;
	out->_21 = -s; out->_22 = c;
	return out;
}

inline D3DXMATRIX* D3DXMatrixPerspectiveFovLH(D3DXMATRIX* out, float fovy, float aspect, float zn, float zf)
{
	float yScale = 1.0f / tanf(fovy * 0.5f);
	float xScale = yScale / aspect;
	out->_11 = xScale; out->_12 = 0;      out->_13 = 0;                  out->_14 = 0;
	out->_21 = 0;      out->_22 = yScale; out->_23 = 0;                  out->_24 = 0;
	out->_31 = 0;      out->_32 = 0;      out->_33 = zf / (zf - zn);     out->_34 = 1;
	out->_41 = 0;      out->_42 = 0;      out->_43 = -zn * zf / (zf - zn); out->_44 = 0;
	return out;
}

inline D3DXMATRIX* D3DXMatrixOrthoOffCenterLH(D3DXMATRIX* out, float l, float r, float b, float t, float zn, float zf)
{
	out->_11 = 2.0f / (r - l); out->_12 = 0;             out->_13 = 0;                 out->_14 = 0;
	out->_21 = 0;             out->_22 = 2.0f / (t - b); out->_23 = 0;                 out->_24 = 0;
	out->_31 = 0;             out->_32 = 0;             out->_33 = 1.0f / (zf - zn);   out->_34 = 0;
	out->_41 = (l + r) / (l - r); out->_42 = (t + b) / (b - t); out->_43 = zn / (zn - zf); out->_44 = 1;
	return out;
}

inline D3DXMATRIX* D3DXMatrixLookAtLH(D3DXMATRIX* out, const D3DXVECTOR3* eye, const D3DXVECTOR3* at, const D3DXVECTOR3* up)
{
	// zaxis = normal(at - eye)
	float zx = at->x - eye->x, zy = at->y - eye->y, zz = at->z - eye->z;
	float zl = sqrtf(zx * zx + zy * zy + zz * zz);
	zx /= zl; zy /= zl; zz /= zl;
	// xaxis = normal(cross(up, zaxis))
	float xx = up->y * zz - up->z * zy;
	float xy = up->z * zx - up->x * zz;
	float xz = up->x * zy - up->y * zx;
	float xl = sqrtf(xx * xx + xy * xy + xz * xz);
	xx /= xl; xy /= xl; xz /= xl;
	// yaxis = cross(zaxis, xaxis)
	float yx = zy * xz - zz * xy;
	float yy = zz * xx - zx * xz;
	float yz = zx * xy - zy * xx;

	out->_11 = xx; out->_12 = yx; out->_13 = zx; out->_14 = 0;
	out->_21 = xy; out->_22 = yy; out->_23 = zy; out->_24 = 0;
	out->_31 = xz; out->_32 = yz; out->_33 = zz; out->_34 = 0;
	out->_41 = -(xx * eye->x + xy * eye->y + xz * eye->z);
	out->_42 = -(yx * eye->x + yy * eye->y + yz * eye->z);
	out->_43 = -(zx * eye->x + zy * eye->y + zz * eye->z);
	out->_44 = 1;
	return out;
}


//==============================================================================
// Vector helpers
//==============================================================================
inline D3DXVECTOR3* D3DXVec3Normalize(D3DXVECTOR3* out, const D3DXVECTOR3* v)
{
	float len = sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);
	if (len > 0.0f) {
		float inv = 1.0f / len;
		out->x = v->x * inv; out->y = v->y * inv; out->z = v->z * inv;
	} else {
		out->x = 0.0f; out->y = 0.0f; out->z = 0.0f;
	}
	return out;
}


//==============================================================================
// Dead DX9 texture-loading compatibility.
//
// A few surviving classes (MTPianoKeyboard, MTNoteRipple) still carry their old
// DX9 device-bound texture/draw methods. These are NEVER called in the DX11 path
// (it loads textures via DXTexture11 / WIC and renders with the *11 classes), but
// they must still compile. These minimal stubs let them build without the SDK;
// the real (live) code paths never reach them.
//==============================================================================
#ifndef D3DX_FILTER_LINEAR
#define D3DX_DEFAULT       ((UINT)-1)
#define D3DX_FILTER_NONE   (1 << 0)
#define D3DX_FILTER_LINEAR (3 << 0)
#endif

typedef struct _D3DXIMAGE_INFO {
	UINT            Width;
	UINT            Height;
	UINT            Depth;
	UINT            MipLevels;
	D3DFORMAT       Format;
	D3DRESOURCETYPE ResourceType;
	DWORD           ImageFileFormat;
} D3DXIMAGE_INFO;

inline HRESULT D3DXGetImageInfoFromFile(LPCTSTR, D3DXIMAGE_INFO*)
{
	return E_NOTIMPL;
}

inline HRESULT D3DXCreateTextureFromFile(LPDIRECT3DDEVICE9, LPCTSTR, LPDIRECT3DTEXTURE9* ppTexture)
{
	if (ppTexture != NULL) *ppTexture = NULL;
	return E_NOTIMPL;
}

inline HRESULT D3DXCreateTextureFromFileEx(LPDIRECT3DDEVICE9, LPCTSTR, UINT, UINT, UINT, DWORD,
		D3DFORMAT, D3DPOOL, DWORD, DWORD, D3DCOLOR, D3DXIMAGE_INFO*, PALETTEENTRY*,
		LPDIRECT3DTEXTURE9* ppTexture)
{
	if (ppTexture != NULL) *ppTexture = NULL;
	return E_NOTIMPL;
}
