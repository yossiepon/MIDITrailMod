int MTPianoKeyboard11::_CreateVertexOfKeyWhite1(
		unsigned char noteNo,
		KbdVertex* pVertex,
		unsigned long* pIndex,
		Color* pColor
	)
{
	int result = 0;
	unsigned long i = 0;
	float centerX       = m_KeyboardDesign.GetKeyCenterPosX(noteNo);
	float heightY       = m_KeyboardDesign.GetWhiteKeyHeight();
	float whiteKeyWidth = m_KeyboardDesign.GetWhiteKeyWidth();
	float whiteKeyLen   = m_KeyboardDesign.GetWhiteKeyLen();
	float blackKeyWidth = m_KeyboardDesign.GetBlackKeyWidth();
	float blackKeyLen   = m_KeyboardDesign.GetBlackKeyLen();
	float deltaKeyLen   = whiteKeyLen - blackKeyLen;
	float spc           = m_KeyboardDesign.GetKeySpaceSize();
	float nextCenterX   = m_KeyboardDesign.GetKeyCenterPosX(noteNo+1);
	Color keyColor;
	Vector2 t0, t1, t2, t3, t4, t5, t6, t7, tsc;

	//白鍵カラー
	if (pColor == NULL) {
		keyColor = m_KeyboardDesign.GetWhiteKeyColor();
	}
	else {
		keyColor = *pColor;
	}

	//----------------------------------------------------------------
	//上の面
	//----------------------------------------------------------------
	// 6+--+5
	//  |  |
	//  |  |
	//  |  |4
	// 3+--+--+2
	//  |     |   +z
	//  |     |    |
	//  |     |    |
	// 0+-----+1   +---> +x
	//     |
	//    posX

	//頂点
	pVertex[0].p = Vector3(centerX - (whiteKeyWidth/2.0f),           heightY, 0.0f);
	pVertex[1].p = Vector3(centerX + (whiteKeyWidth/2.0f),           heightY, 0.0f);
	pVertex[2].p = Vector3(centerX + (whiteKeyWidth/2.0f),           heightY, deltaKeyLen - spc);
	pVertex[3].p = Vector3(centerX - (whiteKeyWidth/2.0f),           heightY, deltaKeyLen - spc);
	pVertex[4].p = Vector3(nextCenterX - (blackKeyWidth/2.0f) - spc, heightY, deltaKeyLen - spc);
	pVertex[5].p = Vector3(nextCenterX - (blackKeyWidth/2.0f) - spc, heightY, whiteKeyLen);
	pVertex[6].p = Vector3(centerX - (whiteKeyWidth/2.0f),           heightY, whiteKeyLen);

	if (m_KeyboardDesign.GetKeyDispRangeEnd() == noteNo) {
		pVertex[4].p = pVertex[2].p;
		pVertex[5].p = Vector3(centerX + (whiteKeyWidth/2.0f), heightY, whiteKeyLen);
	}

	//法線／色
	for (i = 0; i < 7; i++) {
		pVertex[i].n = Vector3(0.0f, 1.0f, 0.0f);
		pVertex[i].c = keyColor.BGRA();
	}

	//インデックス
	unsigned long indexUP[] = { 0, 2, 1, 0, 3, 2, 3, 5, 4, 3, 6, 5 };
	for (i = 0; i < 12; i++) {
		pIndex[i] = m_BufInfo[noteNo].vertexPos + indexUP[i];
	}

	//各頂点のテクスチャ座標
	m_KeyboardDesign.GetWhiteKeyTexturePosTop(noteNo, &t0, &t1, &t2, &t3, &t4, &t5, &t6, &t7);
	pVertex[0].t = t0;
	pVertex[1].t = t1;
	pVertex[2].t = t2;
	pVertex[3].t = t3;
	pVertex[4].t = t4;
	pVertex[5].t = t5;
	pVertex[6].t = t6;

	//----------------------------------------------------------------
	//側面 0-1
	//----------------------------------------------------------------
	// 0      1
	// 7+----+8
	//  |    |
	// 9+----+10

	//頂点
	pVertex[7].p  = pVertex[0].p;
	pVertex[8].p  = pVertex[1].p;
	pVertex[9].p  = Vector3(pVertex[0].p.x, 0.0f, pVertex[0].p.z); // 0'
	pVertex[10].p = Vector3(pVertex[1].p.x, 0.0f, pVertex[1].p.z); // 1'

	//法線／色
	for (i = 7; i < 11; i++) {
		pVertex[i].n = Vector3(0.0f, 0.0f, -1.0f);
		pVertex[i].c = keyColor.BGRA();
	}

	//インデックス
	unsigned long index01[] = { 7, 8, 9, 8, 10, 9 };
	for (i = 0; i < 6; i++) {
		pIndex[12 + i] = m_BufInfo[noteNo].vertexPos + index01[i];
	}

	//各頂点のテクスチャ座標
	m_KeyboardDesign.GetWhiteKeyTexturePosFront(noteNo, &t0, &t1, &t2, &t3);
	pVertex[7].t  = t0;
	pVertex[8].t  = t1;
	pVertex[9].t  = t2;
	pVertex[10].t = t3;

	//----------------------------------------------------------------
	//側面 1-2
	//----------------------------------------------------------------
	// 2 12+--+14
	//     |  |
	//     |  |
	// 1 11+--+13

	//頂点
	pVertex[11].p = pVertex[1].p;
	pVertex[12].p = pVertex[2].p;
	pVertex[13].p = Vector3(pVertex[1].p.x, 0.0f, pVertex[1].p.z);  // 1'
	pVertex[14].p = Vector3(pVertex[2].p.x, 0.0f, pVertex[2].p.z);  // 2'

	//法線／色
	for (i = 11; i < 15; i++) {
		pVertex[i].n = Vector3(1.0f, 0.0f, 0.0f);
		pVertex[i].c = keyColor.BGRA();
	}

	//インデックス
	unsigned long index12[] = { 11, 12, 13, 12, 14, 13 };
	for (i = 0; i < 6; i++) {
		pIndex[18 + i] = m_BufInfo[noteNo].vertexPos + index12[i];
	}

	//----------------------------------------------------------------
	//側面 2-4
	//----------------------------------------------------------------
	//   18+--+17
	//     |  |
	// 4 16+--+15 2

	//頂点
	pVertex[15].p = pVertex[2].p;
	pVertex[16].p = pVertex[4].p;
	pVertex[17].p = Vector3(pVertex[2].p.x, 0.0f, pVertex[2].p.z);  // 2'
	pVertex[18].p = Vector3(pVertex[4].p.x, 0.0f, pVertex[4].p.z);  // 4'

	//法線／色
	for (i = 15; i < 19; i++) {
		pVertex[i].n = Vector3(0.0f, 0.0f, 1.0f);
		pVertex[i].c = keyColor.BGRA();
	}

	//インデックス
	unsigned long index24[] = { 15, 16, 17, 16, 18, 17 };
	for (i = 0; i < 6; i++) {
		pIndex[24 + i] = m_BufInfo[noteNo].vertexPos + index24[i];
	}

	//----------------------------------------------------------------
	//側面 4-5
	//----------------------------------------------------------------
	// 5 20+--+22
	//     |  |
	//     |  |
	// 4 19+--+21

	//頂点
	pVertex[19].p = pVertex[4].p;
	pVertex[20].p = pVertex[5].p;
	pVertex[21].p = Vector3(pVertex[4].p.x, 0.0f, pVertex[4].p.z);  // 4'
	pVertex[22].p = Vector3(pVertex[5].p.x, 0.0f, pVertex[5].p.z);  // 5'

	//法線／色
	for (i = 19; i < 23; i++) {
		pVertex[i].n = Vector3(1.0f, 0.0f, 0.0f);
		pVertex[i].c = keyColor.BGRA();
	}

	//インデックス
	unsigned long index45[] = { 19, 20, 21, 20, 22, 21 };
	for (i = 0; i < 6; i++) {
		pIndex[30 + i] = m_BufInfo[noteNo].vertexPos + index45[i];
	}

	//----------------------------------------------------------------
	//側面 5-6
	//----------------------------------------------------------------
	//   26+--+25
	//     |  |
	// 6 24+--+23 5

	//頂点
	pVertex[23].p = pVertex[5].p;
	pVertex[24].p = pVertex[6].p;
	pVertex[25].p = Vector3(pVertex[5].p.x, 0.0f, pVertex[5].p.z);  // 5'
	pVertex[26].p = Vector3(pVertex[6].p.x, 0.0f, pVertex[6].p.z);  // 6'

	//法線／色
	for (i = 23; i < 27; i++) {
		pVertex[i].n = Vector3(0.0f, 0.0f, 1.0f);
		pVertex[i].c = keyColor.BGRA();
	}

	//インデックス
	unsigned long index56[] = { 23, 24, 25, 24, 26, 25 };
	for (i = 0; i < 6; i++) {
		pIndex[36 + i] = m_BufInfo[noteNo].vertexPos + index56[i];
	}

	//----------------------------------------------------------------
	//側面 6-0
	//----------------------------------------------------------------
	// 29+--+27 6
	//   |  |
	//   |  |
	// 30+--+28 0

	//頂点
	pVertex[27].p = pVertex[6].p;
	pVertex[28].p = pVertex[0].p;
	pVertex[29].p = Vector3(pVertex[6].p.x, 0.0f, pVertex[6].p.z);  // 6'
	pVertex[30].p = Vector3(pVertex[0].p.x, 0.0f, pVertex[0].p.z);  // 0'

	//法線／色
	for (i = 27; i < 31; i++) {
		pVertex[i].n = Vector3(-1.0f, 0.0f, 0.0f);
		pVertex[i].c = keyColor.BGRA();
	}

	//インデックス
	unsigned long index60[] = { 27, 28, 29, 28, 30, 29 };
	for (i = 0; i < 6; i++) {
		pIndex[42 + i] = m_BufInfo[noteNo].vertexPos + index60[i];
	}

	//----------------------------------------------------------------
	//下の面
	//----------------------------------------------------------------
	//  37 6+--+5 36
	//      |  |
	//      |  |
	//      |  |4 35
	//  34 3+--+--+2 33
	//      |     |     +z
	//      |     |      |
	//      |     |      |
	//  31 0+-----+1 32  +---> +x
	//         |
	//        posX

	//頂点
	pVertex[31].p = Vector3(pVertex[0].p.x, 0.0f, pVertex[0].p.z);  // 0'
	pVertex[32].p = Vector3(pVertex[1].p.x, 0.0f, pVertex[1].p.z);  // 1'
	pVertex[33].p = Vector3(pVertex[2].p.x, 0.0f, pVertex[2].p.z);  // 2'
	pVertex[34].p = Vector3(pVertex[3].p.x, 0.0f, pVertex[3].p.z);  // 3'
	pVertex[35].p = Vector3(pVertex[4].p.x, 0.0f, pVertex[4].p.z);  // 4'
	pVertex[36].p = Vector3(pVertex[5].p.x, 0.0f, pVertex[5].p.z);  // 5'
	pVertex[37].p = Vector3(pVertex[6].p.x, 0.0f, pVertex[6].p.z);  // 6'

	//法線／色
	for (i = 31; i < 38; i++) {
		pVertex[i].n = Vector3(0.0f, -1.0f, 0.0f);
		pVertex[i].c = keyColor.BGRA();
	}

	//インデックス
	unsigned long indexDW[] = { 31, 32, 33, 31, 33, 34, 34, 35, 36, 34, 36, 37 };
	for (i = 0; i < 12; i++) {
		pIndex[48 + i] = m_BufInfo[noteNo].vertexPos + indexDW[i];
	}

	//----------------------------------------------------------------
	//単一色のテクスチャ座標
	//----------------------------------------------------------------
	m_KeyboardDesign.GetWhiteKeyTexturePosSingleColor(noteNo, &tsc);
	for (i = 11; i < 38; i++) {
		pVertex[i].t = tsc;
	}

	return result;
}

//******************************************************************************
// キーボード頂点生成：白鍵B
//******************************************************************************


int MTPianoKeyboard11::_CreateVertexOfKeyWhite2(
		unsigned char noteNo,
		KbdVertex* pVertex,
		unsigned long* pIndex,
		Color* pColor
	)
{
	int result = 0;
	unsigned long i = 0;
	float centerX       = m_KeyboardDesign.GetKeyCenterPosX(noteNo);
	float heightY       = m_KeyboardDesign.GetWhiteKeyHeight();
	float whiteKeyWidth = m_KeyboardDesign.GetWhiteKeyWidth();
	float whiteKeyLen   = m_KeyboardDesign.GetWhiteKeyLen();
	float blackKeyWidth = m_KeyboardDesign.GetBlackKeyWidth();
	float blackKeyLen   = m_KeyboardDesign.GetBlackKeyLen();
	float deltaKeyLen   = whiteKeyLen - blackKeyLen;
	float spc           = m_KeyboardDesign.GetKeySpaceSize();
	float prevCenterX   = m_KeyboardDesign.GetKeyCenterPosX(noteNo-1);
	float nextCenterX   = m_KeyboardDesign.GetKeyCenterPosX(noteNo+1);
	Color keyColor;
	Vector2 t0, t1, t2, t3, t4, t5, t6, t7, tsc;

	//白鍵カラー
	if (pColor == NULL) {
		keyColor = m_KeyboardDesign.GetWhiteKeyColor();
	}
	else {
		keyColor = *pColor;
	}

	//----------------------------------------------------------------
	//上の面
	//----------------------------------------------------------------
	//   6+-+5
	//    | |
	//    | |
	//   7| |4
	// 3+-+-+-+2
	//  |     |   +z
	//  |     |    |
	//  |     |    |
	// 0+-----+1   +---> +x
	//     |
	//    posX

	//頂点
	pVertex[0].p = Vector3(centerX - (whiteKeyWidth/2.0f),           heightY, 0.0f);
	pVertex[1].p = Vector3(centerX + (whiteKeyWidth/2.0f),           heightY, 0.0f);
	pVertex[2].p = Vector3(centerX + (whiteKeyWidth/2.0f),           heightY, deltaKeyLen - spc);
	pVertex[3].p = Vector3(centerX - (whiteKeyWidth/2.0f),           heightY, deltaKeyLen - spc);
	pVertex[4].p = Vector3(nextCenterX - (blackKeyWidth/2.0f) - spc, heightY, deltaKeyLen - spc);
	pVertex[5].p = Vector3(nextCenterX - (blackKeyWidth/2.0f) - spc, heightY, whiteKeyLen);
	pVertex[6].p = Vector3(prevCenterX + (blackKeyWidth/2.0f) + spc, heightY, whiteKeyLen);
	pVertex[7].p = Vector3(prevCenterX + (blackKeyWidth/2.0f) + spc, heightY, deltaKeyLen - spc);

	if (m_KeyboardDesign.GetKeyDispRangeStart() == noteNo) {
		pVertex[7].p = pVertex[3].p;
		pVertex[6].p = Vector3(centerX - (whiteKeyWidth/2.0f), heightY, whiteKeyLen);
	}
	if (m_KeyboardDesign.GetKeyDispRangeEnd() == noteNo) {
		pVertex[4].p = pVertex[2].p;
		pVertex[5].p = Vector3(centerX + (whiteKeyWidth/2.0f), heightY, whiteKeyLen);
	}

	//法線／色
	for (i = 0; i < 8; i++) {
		pVertex[i].n = Vector3(0.0f, 1.0f, 0.0f);
		pVertex[i].c = keyColor.BGRA();
	}

	//インデックス
	unsigned long indexUP[] = { 0, 2, 1, 0, 3, 2, 7, 5, 4, 7, 6, 5 };
	for (i = 0; i < 12; i++) {
		pIndex[i] = m_BufInfo[noteNo].vertexPos + indexUP[i];
	}

	//各頂点のテクスチャ座標
	m_KeyboardDesign.GetWhiteKeyTexturePosTop(noteNo, &t0, &t1, &t2, &t3, &t4, &t5, &t6, &t7);
	pVertex[0].t = t0;
	pVertex[1].t = t1;
	pVertex[2].t = t2;
	pVertex[3].t = t3;
	pVertex[4].t = t4;
	pVertex[5].t = t5;
	pVertex[6].t = t6;
	pVertex[7].t = t7;

	//----------------------------------------------------------------
	//側面 0-1
	//----------------------------------------------------------------
	//  0      1
	//  8+----+9
	//   |    |
	// 10+----+11

	//頂点
	pVertex[8].p  = pVertex[0].p;
	pVertex[9].p  = pVertex[1].p;
	pVertex[10].p = Vector3(pVertex[0].p.x, 0.0f, pVertex[0].p.z); // 0'
	pVertex[11].p = Vector3(pVertex[1].p.x, 0.0f, pVertex[1].p.z); // 1'

	//法線／色
	for (i = 8; i < 12; i++) {
		pVertex[i].n = Vector3(0.0f, 0.0f, -1.0f);
		pVertex[i].c = keyColor.BGRA();
	}

	//インデックス
	unsigned long index01[] = { 8, 9, 10, 9, 11, 10 };
	for (i = 0; i < 6; i++) {
		pIndex[12 + i] = m_BufInfo[noteNo].vertexPos + index01[i];
	}

	//各頂点のテクスチャ座標
	m_KeyboardDesign.GetWhiteKeyTexturePosFront(noteNo, &t0, &t1, &t2, &t3);
	pVertex[8].t  = t0;
	pVertex[9].t  = t1;
	pVertex[10].t = t2;
	pVertex[11].t = t3;

	//----------------------------------------------------------------
	//側面 1-2
	//----------------------------------------------------------------
	// 2 13+--+15
	//     |  |
	//     |  |
	// 1 12+--+14

	//頂点
	pVertex[12].p = pVertex[1].p;
	pVertex[13].p = pVertex[2].p;
	pVertex[14].p = Vector3(pVertex[1].p.x, 0.0f, pVertex[1].p.z);  // 1'
	pVertex[15].p = Vector3(pVertex[2].p.x, 0.0f, pVertex[2].p.z);  // 2'

	//法線／色
	for (i = 12; i < 16; i++) {
		pVertex[i].n = Vector3(1.0f, 0.0f, 0.0f);
		pVertex[i].c = keyColor.BGRA();
	}

	//インデックス
	unsigned long index12[] = { 12, 13, 14, 13, 15, 14 };
	for (i = 0; i < 6; i++) {
		pIndex[18 + i] = m_BufInfo[noteNo].vertexPos + index12[i];
	}

	//----------------------------------------------------------------
	//側面 2-3
	//----------------------------------------------------------------
	//   19+--+18
	//     |  |
	// 3 17+--+16 2

	//頂点
	pVertex[16].p = pVertex[2].p;
	pVertex[17].p = pVertex[3].p;
	pVertex[18].p = Vector3(pVertex[2].p.x, 0.0f, pVertex[2].p.z);  // 2'
	pVertex[19].p = Vector3(pVertex[3].p.x, 0.0f, pVertex[3].p.z);  // 3'

	//法線／色
	for (i = 16; i < 20; i++) {
		pVertex[i].n = Vector3(0.0f, 0.0f, 1.0f);
		pVertex[i].c = keyColor.BGRA();
	}

	//インデックス
	unsigned long index24[] = { 16, 17, 18, 17, 19, 18 };
	for (i = 0; i < 6; i++) {
		pIndex[24 + i] = m_BufInfo[noteNo].vertexPos + index24[i];
	}

	//----------------------------------------------------------------
	//側面 4-5
	//----------------------------------------------------------------
	// 5 21+--+23
	//     |  |
	//     |  |
	// 4 20+--+22

	//頂点
	pVertex[20].p = pVertex[4].p;
	pVertex[21].p = pVertex[5].p;
	pVertex[22].p = Vector3(pVertex[4].p.x, 0.0f, pVertex[4].p.z);  // 4'
	pVertex[23].p = Vector3(pVertex[5].p.x, 0.0f, pVertex[5].p.z);  // 5'

	//法線／色
	for (i = 20; i < 24; i++) {
		pVertex[i].n = Vector3(1.0f, 0.0f, 0.0f);
		pVertex[i].c = keyColor.BGRA();
	}

	//インデックス
	unsigned long index45[] = { 20, 21, 22, 21, 23, 22 };
	for (i = 0; i < 6; i++) {
		pIndex[30 + i] = m_BufInfo[noteNo].vertexPos + index45[i];
	}

	//----------------------------------------------------------------
	//側面 5-6
	//----------------------------------------------------------------
	//   27+--+26
	//     |  |
	// 6 25+--+24 5

	//頂点
	pVertex[24].p = pVertex[5].p;
	pVertex[25].p = pVertex[6].p;
	pVertex[26].p = Vector3(pVertex[5].p.x, 0.0f, pVertex[5].p.z);  // 5'
	pVertex[27].p = Vector3(pVertex[6].p.x, 0.0f, pVertex[6].p.z);  // 6'

	//法線／色
	for (i = 24; i < 28; i++) {
		pVertex[i].n = Vector3(0.0f, 0.0f, 1.0f);
		pVertex[i].c = keyColor.BGRA();
	}

	//インデックス
	unsigned long index56[] = { 24, 25, 26, 25, 27, 26 };
	for (i = 0; i < 6; i++) {
		pIndex[36 + i] = m_BufInfo[noteNo].vertexPos + index56[i];
	}

	//----------------------------------------------------------------
	//側面 6-7
	//----------------------------------------------------------------
	// 30+--+28 6
	//   |  |
	//   |  |
	// 31+--+29 7

	//頂点
	pVertex[28].p = pVertex[6].p;
	pVertex[29].p = pVertex[7].p;
	pVertex[30].p = Vector3(pVertex[6].p.x, 0.0f, pVertex[6].p.z);  // 6'
	pVertex[31].p = Vector3(pVertex[7].p.x, 0.0f, pVertex[7].p.z);  // 7'

	//法線／色
	for (i = 28; i < 32; i++) {
		pVertex[i].n = Vector3(-1.0f, 0.0f, 0.0f);
		pVertex[i].c = keyColor.BGRA();
	}

	//インデックス
	unsigned long index67[] = { 28, 29, 30, 29, 31, 30 };
	for (i = 0; i < 6; i++) {
		pIndex[42 + i] = m_BufInfo[noteNo].vertexPos + index67[i];
	}

	//----------------------------------------------------------------
	//側面 3-0
	//----------------------------------------------------------------
	// 34+--+32 3
	//   |  |
	//   |  |
	// 35+--+33 0

	//頂点
	pVertex[32].p = pVertex[3].p;
	pVertex[33].p = pVertex[0].p;
	pVertex[34].p = Vector3(pVertex[3].p.x, 0.0f, pVertex[3].p.z);  // 3'
	pVertex[35].p = Vector3(pVertex[0].p.x, 0.0f, pVertex[0].p.z);  // 0'

	//法線／色
	for (i = 32; i < 36; i++) {
		pVertex[i].n = Vector3(-1.0f, 0.0f, 0.0f);
		pVertex[i].c = keyColor.BGRA();
	}

	//インデックス
	unsigned long index30[] = { 32, 33, 34, 33, 35, 34 };
	for (i = 0; i < 6; i++) {
		pIndex[48 + i] = m_BufInfo[noteNo].vertexPos + index30[i];
	}

	//----------------------------------------------------------------
	//下の面
	//----------------------------------------------------------------
	//   42 6+-+5 41
	//       | |
	//       | |
	//   43 7| |4 40
	// 39 3+-+-+-+2 38
	//     |     |     +z
	//     |     |      |
	//     |     |      |
	// 36 0+-----+1 37  +---> +x
	//        |
	//       posX

	//頂点
	pVertex[36].p = Vector3(pVertex[0].p.x, 0.0f, pVertex[0].p.z);  // 0'
	pVertex[37].p = Vector3(pVertex[1].p.x, 0.0f, pVertex[1].p.z);  // 1'
	pVertex[38].p = Vector3(pVertex[2].p.x, 0.0f, pVertex[2].p.z);  // 2'
	pVertex[39].p = Vector3(pVertex[3].p.x, 0.0f, pVertex[3].p.z);  // 3'
	pVertex[40].p = Vector3(pVertex[4].p.x, 0.0f, pVertex[4].p.z);  // 4'
	pVertex[41].p = Vector3(pVertex[5].p.x, 0.0f, pVertex[5].p.z);  // 5'
	pVertex[42].p = Vector3(pVertex[6].p.x, 0.0f, pVertex[6].p.z);  // 6'
	pVertex[43].p = Vector3(pVertex[7].p.x, 0.0f, pVertex[7].p.z);  // 7'

	//法線／色
	for (i = 36; i < 44; i++) {
		pVertex[i].n = Vector3(0.0f, -1.0f, 0.0f);
		pVertex[i].c = keyColor.BGRA();
	}

	//インデックス
	unsigned long indexDW[] = { 36, 37, 38, 36, 38, 39, 43, 40, 41, 43, 41, 42 };
	for (i = 0; i < 12; i++) {
		pIndex[54 + i] = m_BufInfo[noteNo].vertexPos + indexDW[i];
	}

	//----------------------------------------------------------------
	//単一色のテクスチャ座標
	//----------------------------------------------------------------
	m_KeyboardDesign.GetWhiteKeyTexturePosSingleColor(noteNo, &tsc);
	for (i = 12; i < 44; i++) {
		pVertex[i].t = tsc;
	}

	return result;
}

//******************************************************************************
// キーボード頂点生成：白鍵C
//******************************************************************************


int MTPianoKeyboard11::_CreateVertexOfKeyWhite3(
		unsigned char noteNo,
		KbdVertex* pVertex,
		unsigned long* pIndex,
		Color* pColor
	)
{
	int result = 0;
	unsigned long i = 0;
	float centerX       = m_KeyboardDesign.GetKeyCenterPosX(noteNo);
	float heightY       = m_KeyboardDesign.GetWhiteKeyHeight();
	float whiteKeyWidth = m_KeyboardDesign.GetWhiteKeyWidth();
	float whiteKeyLen   = m_KeyboardDesign.GetWhiteKeyLen();
	float blackKeyWidth = m_KeyboardDesign.GetBlackKeyWidth();
	float blackKeyLen   = m_KeyboardDesign.GetBlackKeyLen();
	float deltaKeyLen   = whiteKeyLen - blackKeyLen;
	float spc           = m_KeyboardDesign.GetKeySpaceSize();
	float prevCenterX   = m_KeyboardDesign.GetKeyCenterPosX(noteNo-1);
	Color keyColor;
	Vector2 t0, t1, t2, t3, t4, t5, t6, t7, tsc;

	//白鍵カラー
	if (pColor == NULL) {
		keyColor = m_KeyboardDesign.GetWhiteKeyColor();
	}
	else {
		keyColor = *pColor;
	}

	//----------------------------------------------------------------
	//上の面
	//----------------------------------------------------------------
	//    5+--+4
	//     |  |
	//     |  |
	//    6|  |
	// 3+--+--+2
	//  |     |   +z
	//  |     |    |
	//  |     |    |
	// 0+-----+1   +---> +x
	//     |
	//    posX

	//頂点
	pVertex[0].p = Vector3(centerX - (whiteKeyWidth/2.0f),           heightY, 0.0f);
	pVertex[1].p = Vector3(centerX + (whiteKeyWidth/2.0f),           heightY, 0.0f);
	pVertex[2].p = Vector3(centerX + (whiteKeyWidth/2.0f),           heightY, deltaKeyLen - spc);
	pVertex[3].p = Vector3(centerX - (whiteKeyWidth/2.0f),           heightY, deltaKeyLen - spc);
	pVertex[4].p = Vector3(centerX + (whiteKeyWidth/2.0f),           heightY, whiteKeyLen);
	pVertex[5].p = Vector3(prevCenterX + (blackKeyWidth/2.0f) + spc, heightY, whiteKeyLen);
	pVertex[6].p = Vector3(prevCenterX + (blackKeyWidth/2.0f) + spc, heightY, deltaKeyLen - spc);

	if (m_KeyboardDesign.GetKeyDispRangeStart() == noteNo) {
		pVertex[5].p = Vector3(centerX - (whiteKeyWidth/2.0f), heightY, whiteKeyLen);
		pVertex[6].p = pVertex[3].p;
	}

	//法線／色
	for (i = 0; i < 7; i++) {
		pVertex[i].n = Vector3(0.0f, 1.0f, 0.0f);
		pVertex[i].c = keyColor.BGRA();
	}

	//インデックス
	unsigned long indexUP[] = { 0, 2, 1, 0, 3, 2, 2, 6, 4, 6, 5, 4 };
	for (i = 0; i < 12; i++) {
		pIndex[i] = m_BufInfo[noteNo].vertexPos + indexUP[i];
	}

	//各頂点のテクスチャ座標
	m_KeyboardDesign.GetWhiteKeyTexturePosTop(noteNo, &t0, &t1, &t2, &t3, &t4, &t5, &t6, &t7);
	pVertex[0].t = t0;
	pVertex[1].t = t1;
	pVertex[2].t = t2;
	pVertex[3].t = t3;
	pVertex[4].t = t5;
	pVertex[5].t = t6;
	pVertex[6].t = t7;

	//----------------------------------------------------------------
	//側面 0-1
	//----------------------------------------------------------------
	// 0      1
	// 7+----+8
	//  |    |
	// 9+----+10

	//頂点
	pVertex[7].p  = pVertex[0].p;
	pVertex[8].p  = pVertex[1].p;
	pVertex[9].p  = Vector3(pVertex[0].p.x, 0.0f, pVertex[0].p.z); // 0'
	pVertex[10].p = Vector3(pVertex[1].p.x, 0.0f, pVertex[1].p.z); // 1'

	//法線／色
	for (i = 7; i < 11; i++) {
		pVertex[i].n = Vector3(0.0f, 0.0f, -1.0f);
		pVertex[i].c = keyColor.BGRA();
	}

	//インデックス
	unsigned long index01[] = { 7, 8, 9, 8, 10, 9 };
	for (i = 0; i < 6; i++) {
		pIndex[12 + i] = m_BufInfo[noteNo].vertexPos + index01[i];
	}

	//各頂点のテクスチャ座標
	m_KeyboardDesign.GetWhiteKeyTexturePosFront(noteNo, &t0, &t1, &t2, &t3);
	pVertex[7].t  = t0;
	pVertex[8].t  = t1;
	pVertex[9].t  = t2;
	pVertex[10].t = t3;

	//----------------------------------------------------------------
	//側面 1-4
	//----------------------------------------------------------------
	// 4 12+--+14
	//     |  |
	//     |  |
	// 1 11+--+13

	//頂点
	pVertex[11].p = pVertex[1].p;
	pVertex[12].p = pVertex[4].p;
	pVertex[13].p = Vector3(pVertex[1].p.x, 0.0f, pVertex[1].p.z);  // 1'
	pVertex[14].p = Vector3(pVertex[4].p.x, 0.0f, pVertex[4].p.z);  // 4'

	//法線／色
	for (i = 11; i < 15; i++) {
		pVertex[i].n = Vector3(1.0f, 0.0f, 0.0f);
		pVertex[i].c = keyColor.BGRA();
	}

	//インデックス
	unsigned long index14[] = { 11, 12, 13, 12, 14, 13 };
	for (i = 0; i < 6; i++) {
		pIndex[18 + i] = m_BufInfo[noteNo].vertexPos + index14[i];
	}

	//----------------------------------------------------------------
	//側面 4-5
	//----------------------------------------------------------------
	//   18+--+17
	//     |  |
	// 5 16+--+15 4

	//頂点
	pVertex[15].p = pVertex[4].p;
	pVertex[16].p = pVertex[5].p;
	pVertex[17].p = Vector3(pVertex[4].p.x, 0.0f, pVertex[4].p.z);  // 4'
	pVertex[18].p = Vector3(pVertex[5].p.x, 0.0f, pVertex[5].p.z);  // 5'

	//法線／色
	for (i = 15; i < 19; i++) {
		pVertex[i].n = Vector3(0.0f, 0.0f, 1.0f);
		pVertex[i].c = keyColor.BGRA();
	}

	//インデックス
	unsigned long index45[] = { 15, 16, 17, 16, 18, 17 };
	for (i = 0; i < 6; i++) {
		pIndex[24 + i] = m_BufInfo[noteNo].vertexPos + index45[i];
	}

	//----------------------------------------------------------------
	//側面 5-6
	//----------------------------------------------------------------
	// 21+--+19 5
	//   |  |
	//   |  |
	// 22+--+20 6

	//頂点
	pVertex[19].p = pVertex[5].p;
	pVertex[20].p = pVertex[6].p;
	pVertex[21].p = Vector3(pVertex[5].p.x, 0.0f, pVertex[5].p.z);  // 5'
	pVertex[22].p = Vector3(pVertex[6].p.x, 0.0f, pVertex[6].p.z);  // 6'

	//法線／色
	for (i = 19; i < 23; i++) {
		pVertex[i].n = Vector3(-1.0f, 0.0f, 0.0f);
		pVertex[i].c = keyColor.BGRA();
	}

	//インデックス
	unsigned long index56[] = { 19, 20, 21, 20, 22, 21 };
	for (i = 0; i < 6; i++) {
		pIndex[30 + i] = m_BufInfo[noteNo].vertexPos + index56[i];
	}

	//----------------------------------------------------------------
	//側面 6-3
	//----------------------------------------------------------------
	//   26+--+25
	//     |  |
	// 3 24+--+23 6

	//頂点
	pVertex[23].p = pVertex[6].p;
	pVertex[24].p = pVertex[3].p;
	pVertex[25].p = Vector3(pVertex[6].p.x, 0.0f, pVertex[6].p.z);  // 6'
	pVertex[26].p = Vector3(pVertex[3].p.x, 0.0f, pVertex[3].p.z);  // 3'

	//法線／色
	for (i = 23; i < 27; i++) {
		pVertex[i].n = Vector3(0.0f, 0.0f, 1.0f);
		pVertex[i].c = keyColor.BGRA();
	}

	//インデックス
	unsigned long index63[] = { 23, 24, 25, 24, 26, 25 };
	for (i = 0; i < 6; i++) {
		pIndex[36 + i] = m_BufInfo[noteNo].vertexPos + index63[i];
	}

	//----------------------------------------------------------------
	//側面 3-0
	//----------------------------------------------------------------
	// 29+--+27 3
	//   |  |
	//   |  |
	// 30+--+28 0

	//頂点
	pVertex[27].p = pVertex[3].p;
	pVertex[28].p = pVertex[0].p;
	pVertex[29].p = Vector3(pVertex[3].p.x, 0.0f, pVertex[3].p.z);  // 3'
	pVertex[30].p = Vector3(pVertex[0].p.x, 0.0f, pVertex[0].p.z);  // 0'

	//法線／色
	for (i = 27; i < 31; i++) {
		pVertex[i].n = Vector3(-1.0f, 0.0f, 0.0f);
		pVertex[i].c = keyColor.BGRA();
	}

	//インデックス
	unsigned long index30[] = { 27, 28, 29, 28, 30, 29 };
	for (i = 0; i < 6; i++) {
		pIndex[42 + i] = m_BufInfo[noteNo].vertexPos + index30[i];
	}

	//----------------------------------------------------------------
	//下の面
	//----------------------------------------------------------------
	//    36 5+--+4 35
	//        |  |
	//        |  |
	//    37 6|  |
	// 34 3+--+--+2 33
	//     |     |     +z
	//     |     |      |
	//     |     |      |
	// 31 0+-----+1 32  +---> +x
	//        |
	//       posX

	//頂点
	pVertex[31].p = Vector3(pVertex[0].p.x, 0.0f, pVertex[0].p.z);  // 0'
	pVertex[32].p = Vector3(pVertex[1].p.x, 0.0f, pVertex[1].p.z);  // 1'
	pVertex[33].p = Vector3(pVertex[2].p.x, 0.0f, pVertex[2].p.z);  // 2'
	pVertex[34].p = Vector3(pVertex[3].p.x, 0.0f, pVertex[3].p.z);  // 3'
	pVertex[35].p = Vector3(pVertex[4].p.x, 0.0f, pVertex[4].p.z);  // 4'
	pVertex[36].p = Vector3(pVertex[5].p.x, 0.0f, pVertex[5].p.z);  // 5'
	pVertex[37].p = Vector3(pVertex[6].p.x, 0.0f, pVertex[6].p.z);  // 6'

	//法線／色
	for (i = 31; i < 38; i++) {
		pVertex[i].n = Vector3(0.0f, -1.0f, 0.0f);
		pVertex[i].c = keyColor.BGRA();
	}

	//インデックス
	unsigned long indexDW[] = { 31, 32, 33, 31, 33, 34, 33, 35, 37, 37, 35, 36 };
	for (i = 0; i < 12; i++) {
		pIndex[48 + i] = m_BufInfo[noteNo].vertexPos + indexDW[i];
	}

	//----------------------------------------------------------------
	//単一色のテクスチャ座標
	//----------------------------------------------------------------
	m_KeyboardDesign.GetWhiteKeyTexturePosSingleColor(noteNo, &tsc);
	for (i = 11; i < 38; i++) {
		pVertex[i].t = tsc;
	}

	return result;
}

//******************************************************************************
// キーボード頂点生成：黒鍵
//******************************************************************************


int MTPianoKeyboard11::_CreateVertexOfKeyBlack(
		unsigned char noteNo,
		KbdVertex* pVertex,
		unsigned long* pIndex,
		Color* pColor
	)
{
	int result = 0;
	unsigned long i = 0;
	float centerX        = m_KeyboardDesign.GetKeyCenterPosX(noteNo);
	float heightY        = m_KeyboardDesign.GetWhiteKeyHeight();
	float whiteKeyLen    = m_KeyboardDesign.GetWhiteKeyLen();
	float blackKeyWidth  = m_KeyboardDesign.GetBlackKeyWidth();
	float blackKeyHeight = m_KeyboardDesign.GetBlackKeyHeight();
	float blackKeyLen    = m_KeyboardDesign.GetBlackKeyLen();
	float deltaKeyLen    = whiteKeyLen - blackKeyLen;
	float blackKeySlope  = m_KeyboardDesign.GetBlackKeySlopeLen();
	Vector3 nVector;
	Vector3 normalizedVector;
	Color keyColor;
	Vector2 t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, tsc;
	bool isColored = false;

	//黒鍵カラー取得
	if (pColor == NULL) {
		keyColor = m_KeyboardDesign.GetBlackKeyColor();
	}
	else {
		keyColor = *pColor;
		isColored = true;
	}

	//----------------------------------------------------------------
	//上の面
	//----------------------------------------------------------------
	//   6+-+5
	//    | |
	//    | |
	// 7 3+-+2 4
	//   0+-+1
	//     |   +z
	//     |    |
	//     |    |
	//   --+--  +---> +x
	//     |
	//    posX

	//頂点
	pVertex[0].p = Vector3(centerX - (blackKeyWidth/2.0f), heightY,        deltaKeyLen);
	pVertex[1].p = Vector3(centerX + (blackKeyWidth/2.0f), heightY,        deltaKeyLen);
	pVertex[2].p = Vector3(centerX + (blackKeyWidth/2.0f), blackKeyHeight, deltaKeyLen + blackKeySlope);
	pVertex[3].p = Vector3(centerX - (blackKeyWidth/2.0f), blackKeyHeight, deltaKeyLen + blackKeySlope);
	pVertex[4].p = pVertex[2].p;
	pVertex[5].p = Vector3(centerX + (blackKeyWidth/2.0f), blackKeyHeight, whiteKeyLen);
	pVertex[6].p = Vector3(centerX - (blackKeyWidth/2.0f), blackKeyHeight, whiteKeyLen);
	pVertex[7].p = pVertex[3].p;

	//法線／色：0-1-2-3面
	nVector = Vector3(0.0f, 0.12f, -0.08f);
	normalizedVector = nVector; normalizedVector.Normalize();
	for (i = 0; i < 4; i++) {
		pVertex[i].n = normalizedVector;
		pVertex[i].c = keyColor.BGRA();
	}
	//法線／色：4-5-6-7面
	for (i = 4; i < 8; i++) {
		pVertex[i].n = Vector3(0.0f, 1.0f, 0.0f);
		pVertex[i].c = keyColor.BGRA();
	}

	//インデックス
	unsigned long indexUP[] = { 0, 2, 1, 0, 3, 2, 4, 7, 5, 7, 6, 5 };
	for (i = 0; i < 12; i++) {
		pIndex[i] = m_BufInfo[noteNo].vertexPos + indexUP[i];
	}

	//各頂点のテクスチャ座標
	m_KeyboardDesign.GetBlackKeyTexturePos(
			noteNo, &t0, &t1, &t2, &t3, &t4, &t5, &t6, &t7, &t8, &t9, isColored
		);
	pVertex[0].t = t0;
	pVertex[1].t = t1;
	pVertex[2].t = t2;
	pVertex[3].t = t3;
	pVertex[4].t = t2;
	pVertex[5].t = t4;
	pVertex[6].t = t5;
	pVertex[7].t = t3;

	//----------------------------------------------------------------
	//側面 0-1
	//----------------------------------------------------------------
	//  0      1
	//  8+----+9
	//   |    |
	// 10+----+11

	//頂点
	pVertex[8].p  = pVertex[0].p;
	pVertex[9].p  = pVertex[1].p;
	pVertex[10].p = Vector3(pVertex[0].p.x, 0.0f, pVertex[0].p.z); // 0'
	pVertex[11].p = Vector3(pVertex[1].p.x, 0.0f, pVertex[1].p.z); // 1'

	//法線／色
	for (i = 8; i < 12; i++) {
		pVertex[i].n = Vector3(0.0f, 0.0f, -1.0f);
		pVertex[i].c = keyColor.BGRA();
	}

	//インデックス
	unsigned long index01[] = { 8, 9, 10, 9, 11, 10 };
	for (i = 0; i < 6; i++) {
		pIndex[12 + i] = m_BufInfo[noteNo].vertexPos + index01[i];
	}

	//各頂点のテクスチャ座標
	m_KeyboardDesign.GetBlackKeyTexturePosSingleColor(noteNo, &tsc, isColored);
	for (i = 8; i < 12; i++) {
		pVertex[i].t = tsc;
	}

	//----------------------------------------------------------------
	//側面 1-2-5
	//----------------------------------------------------------------
	// 5 14+--+16
	//     |  |
	//     |  |
	// 2 13+  |
	//      \ |
	// 1 12 +-+15

	//頂点
	pVertex[12].p  = pVertex[1].p;
	pVertex[13].p  = pVertex[2].p;
	pVertex[14].p  = pVertex[5].p;
	pVertex[15].p = Vector3(pVertex[1].p.x, 0.0f, pVertex[1].p.z); // 1'
	pVertex[16].p = Vector3(pVertex[5].p.x, 0.0f, pVertex[5].p.z); // 5'

	//法線／色
	for (i = 12; i < 17; i++) {
		pVertex[i].n = Vector3(1.0f, 0.0f, 0.0f);
		pVertex[i].c = keyColor.BGRA();
	}

	//インデックス
	unsigned long index125[] = { 12, 13, 15, 13, 16, 15, 13, 14, 16 };
	for (i = 0; i < 9; i++) {
		pIndex[18 + i] = m_BufInfo[noteNo].vertexPos + index125[i];
	}

	//各頂点のテクスチャ座標
	pVertex[12].t = t1;
	pVertex[13].t = t2;
	pVertex[14].t = t4;
	pVertex[15].t = t6;
	pVertex[16].t = t7;

	//----------------------------------------------------------------
	//側面 5-6
	//----------------------------------------------------------------
	//   20+--+19
	//     |  |
	// 6 18+--+17 5

	//頂点
	pVertex[17].p = pVertex[5].p;
	pVertex[18].p = pVertex[6].p;
	pVertex[19].p = Vector3(pVertex[5].p.x, 0.0f, pVertex[5].p.z);  // 5'
	pVertex[20].p = Vector3(pVertex[6].p.x, 0.0f, pVertex[6].p.z);  // 6'

	//法線／色
	for (i = 17; i < 21; i++) {
		pVertex[i].n = Vector3(0.0f, 0.0f, 1.0f);
		pVertex[i].c = keyColor.BGRA();
	}

	//インデックス
	unsigned long index56[] = { 17, 18, 19, 18, 20, 19 };
	for (i = 0; i < 6; i++) {
		pIndex[27 + i] = m_BufInfo[noteNo].vertexPos + index56[i];
	}

	//各頂点のテクスチャ座標
	for (i = 17; i < 21; i++) {
		pVertex[i].t = tsc;
	}

	//----------------------------------------------------------------
	//側面 6-3-0
	//----------------------------------------------------------------
	// 24+--+21 6
	//   |  |
	//   |  |
	//   |  +22 3
	//   | /
	// 25+-+23  0

	//頂点
	pVertex[21].p  = pVertex[6].p;
	pVertex[22].p  = pVertex[3].p;
	pVertex[23].p  = pVertex[0].p;
	pVertex[24].p = Vector3(pVertex[6].p.x, 0.0f, pVertex[6].p.z); // 6'
	pVertex[25].p = Vector3(pVertex[0].p.x, 0.0f, pVertex[0].p.z); // 0'

	//法線／色
	for (i = 21; i < 26; i++) {
		pVertex[i].n = Vector3(-1.0f, 0.0f, 0.0f);
		pVertex[i].c = keyColor.BGRA();
	}

	//インデックス
	unsigned long index630[] = { 21, 22, 24, 22, 25, 24, 22, 23, 25 };
	for (i = 0; i < 9; i++) {
		pIndex[33 + i] = m_BufInfo[noteNo].vertexPos + index630[i];
	}

	//各頂点のテクスチャ座標
	pVertex[21].t = t5;
	pVertex[22].t = t3;
	pVertex[23].t = t0;
	pVertex[24].t = t9;
	pVertex[25].t = t8;

	//----------------------------------------------------------------
	//下の面
	//----------------------------------------------------------------
	//   29 6+-+5 28
	//       | |
	//       | |
	//       | |
	//   26 0+-+1 27
	//        |      +z
	//        |       |
	//        |       |
	//      --+--     +---> +x
	//        |
	//       posX

	//頂点
	pVertex[26].p = Vector3(pVertex[0].p.x, 0.0f, pVertex[0].p.z); // 0'
	pVertex[27].p = Vector3(pVertex[1].p.x, 0.0f, pVertex[1].p.z); // 1'
	pVertex[28].p = Vector3(pVertex[5].p.x, 0.0f, pVertex[5].p.z); // 5'
	pVertex[29].p = Vector3(pVertex[6].p.x, 0.0f, pVertex[6].p.z); // 6'

	//法線／色
	for (i = 26; i < 30; i++) {
		pVertex[i].n = Vector3(0.0f, -1.0f, 0.0f);
		pVertex[i].c = keyColor.BGRA();
	}

	//インデックス
	unsigned long indexDW[] = { 26, 27, 28, 26, 28, 29 };
	for (i = 0; i < 6; i++) {
		pIndex[42 + i] = m_BufInfo[noteNo].vertexPos + indexDW[i];
	}

	//各頂点のテクスチャ座標
	for (i = 26; i < 30; i++) {
		pVertex[i].t = tsc;
	}

	return result;
}
