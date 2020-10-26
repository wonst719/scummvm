/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2004 The ScummVM Kor. Project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "common/stdafx.h"
#include "common/scummsys.h"
#ifndef __GP32__
#include <SDL.h>
#else
#include "backends/gp32/gp32std_grap.h"
#endif
#include <string.h>

#include "scumm/ks_check.h"
#include "scumm/korean.h"

#include "common/file.h"

#ifdef _MSC_VER
// Visual C++�� strncasecmp�� �������� �ʴ´�
#define strncasecmp strnicmp
#endif

#ifndef STRNCASECMP
// eMbedded Visual C++�� strnicmp�� �������� �ʴ´� (Woolforce)
int strncasecmp(const char *s1, const char *s2, int n)
{
	char t1, t2;
	char u1, u2;

    while(n--) {
		if (!*s2) return (*s1 != 0);
		else if (!*s1) return -1;
		u1= *s1++;
		u2= *s2++;
		t1 = (('A' <= u1 && u1 <= 'Z') ? (u1 | 040) : u1);
		t2 = (('A' <= u2 && u2 <= 'Z') ? (u2 | 040) : u2);
		if (t1 != t2) return (t1 > t2) ? 1 : -1;
    }
    return 0;
}
#endif

namespace Scumm {

bool _koreanMode = 0;
bool _koreanOnly = 0;
bool _highRes = 0;

char _gameID[16];

char _parseBuffer[1024]; // �޸𸮸� ��������...

StringKor _strKSet1[MAX_KOR];
StringKor _strKDesc[MAX_KOR];
StringKorSmush _strKSmush[MAX_KOR];

char **_KBuffer = 0;	// �ѱ� ��� ������ ����� ��
int _numKLines = 0;		// ����� �� ���� ��

int desc_num;			//description�� ���� ��� ��ȣ.

int _korFontWidth = 0;	// �ѱ��� ��Ʈ �ʺ�
int _korFontHeight = 0;	// �ѱ��� ��Ʈ ����
int _engFontWidth = 0;	// ������ ��Ʈ �ʺ�
int _engFontHeight = 0;	// ������ ��Ʈ ����

K_Color *_kPalette;

byte *_korFontPtr = 0;
byte *_engFontPtr = 0;

char *convertToKorean(char *buf, bool descFlag)
{
	static int currentLine;

	char *tbuf, *sbuf;
	bool found = 0;

	sbuf = tbuf = _parseBuffer;
/*
		bool mFlg;
		mFlg = 0;
		for(int mi = 0; mi < xxi; mi++) {
			if(!strncasecmp(buf, strBuf[mi], strlen(buf))){
				mFlg = 1;
				break;
			}
		}
		if(!mFlg) {
			strcpy(strBuf[xxi], buf);
			xxi++;
			fprintf(korOutput, "%s\n", buf);
		} //�ӽ� ���Ͽ� ����
*/
	if(buf[0] == 0x07 && descFlag)
		*buf++;

	int i;
	for(i = currentLine; i < _numKLines; i += 2) {
		if(!strncasecmp(buf, _KBuffer[i], strlen(_KBuffer[i])) && strlen(_KBuffer[i]) == strlen(buf)) {
			strcpy(tbuf, _KBuffer[i + 1]);
			currentLine = i;
			found = 1;
			break;
		}
	}
	if(found == 0) {
		for(i = 0; i < currentLine/*_numKLines*/; i += 2) { //�ѹ� �� ã�� ��
			if(!strncasecmp(buf, _KBuffer[i], strlen(_KBuffer[i])) && strlen(_KBuffer[i]) == strlen(buf)) {
				strcpy(tbuf, _KBuffer[i + 1]);
				currentLine = i;
				found = 1;
				break;
			}
		}
	}

	if(found == 0 && descFlag) {
		if(_highRes)
			strcpy(tbuf, buf);
		else
			*tbuf = 0;
	} else if(found == 0)
		if(_koreanOnly) {
			strcpy(tbuf, buf); //�� ã����, ���� �������� ��ü...
			while(*tbuf){
				if(!strncasecmp(tbuf, "<1>", strlen("<1>"))) {
					*tbuf = '\\';
					*(tbuf + 1) = '\\';
					*(tbuf + 2) = 'n';
				}
				if(!strncasecmp(buf, "<3>", strlen("<3>"))) {
					*tbuf = '\\';
					*(tbuf + 1) = '\\';
					*(tbuf + 2) = 'n';
				}
				if(!strncasecmp(tbuf, "<c>", strlen("<c>"))) {
					*tbuf = '\\';
					*(tbuf +1) = '\\';
					*(tbuf +2) = 'c';
					// *(buf+3)=' ';
				}
				tbuf++;
			}
			tbuf = sbuf;
		} else
			*tbuf = 0; //����� �ȳ��ö��� ��ü��.

	return sbuf;
}

void addKString(char *buf, uint16 xpos, uint16 ypos, short delay, uint8 col)
{
	int num;
	for(num = 0; num < MAX_KOR - 1; num++)
		if(!(_strKSet1[num].buffer[0] && 0))
			break;
//	if(num == MAX_KOR - 1)
//		break;

	StringKor *s = &_strKSet1[num];
	s->xpos = xpos;
	s->ypos = ypos;
	s->delay = delay;
	s->color = col;
	strcpy(s->buffer, buf);
	s->remainflag = 1;
	s->remainstart = 1;
	s->remainend = 0;
}

void addKDesc(char *buf, uint16 xpos, uint16 ypos, short delay, uint8 col)
{
	desc_num = 0;
	StringKor *s = &_strKDesc[desc_num];
	s->xpos = xpos;
	s->ypos = ypos;
	s->delay = delay;
	s->color = col;
	strcpy(s->buffer, buf);
	s->remainflag = 1;
	s->remainstart = 1;
	s->remainend = 0;
}

void addKSmush(char *buf, long xpos, long ypos, uint8 col)
{
	int num;
	for(num = 0; num < MAX_KOR - 1; num++)
		if(!(_strKSmush[num].buffer[0] && _strKSmush[num].remain))
			break;
//	if(num == MAX_KOR - 1)
//		break;

	StringKorSmush *s = &_strKSmush[num];
	s->xpos = MAX(xpos, 2);
	s->ypos = MAX(ypos, 2);
	//s->xright = 320 - xpos;
	s->color = col;
	s->remain = 1;
	strcpy(s->buffer, buf);
}

void loadKoreanStrings()
{
	FILE *fp;
	char fname[128];
	const char *path;

#if defined(YOPY)
	path = "/usr/share/scummvmk/sub/";
#elif defined(__GP32__)
	path = "gp:\\gpmm\\scummvm\\sub\\";
#else
	path = "sub/";
#endif

	strcpy(fname, path);
	strcat(fname, _gameID);
	strcat(fname, ".dat");

	if(!(fp = fopen(fname, "rt"))) {
		printf("WARNING: �ѱ� �ڸ� ������ �ε��� �� �����ϴ�!\n");
		return;
	}

	int k;
	char buf[1024];

	for(k = 0; !feof(fp); k++) {
		fgets(buf, 1024, fp);
	} //������ ���� ���� ���

	fseek(fp, 0, SEEK_SET);

	_KBuffer = new char *[k];
	_numKLines = k;

	for(int i = 0; i < _numKLines; i++) {
		fgets(buf, 1024, fp);
		_KBuffer[i] = new char[strlen(buf) + 1]; //\0
		if(strlen(buf)) {
			strcpy(_KBuffer[i], buf);
			char *b = _KBuffer[i];
			if(b[strlen(b) - 1] == '\n' || b[strlen(b) - 1] == 0x0a || b[strlen(b) - 1] == 0x0d)
				b[strlen(b) - 1] = 0;
			if(b[strlen(b) - 1] == '\n' || b[strlen(b) - 1] == 0x0a || b[strlen(b) - 1] == 0x0d)
				b[strlen(b) - 1] = 0;
		}
	}
	fclose(fp);

	printf("�ѱ� �ڸ� ������ �ε�Ǿ����ϴ� -- �� %d ����\n", _numKLines);

	return;
}

bool unloadKorString()
{
	for(int i = 0; i < _numKLines; i++)
		delete []_KBuffer[i];
	delete []_KBuffer;
	return true;
}

void unloadKoreanFiles()
{
	unloadKorString();
}

void unloadEmergencyFont()
{
	delete []_korFontPtr;
	delete []_engFontPtr;
}

//FIXME/TODO: �Ʒ��� �ڵ� �ߺ� ����.
void loadEmergencyFont()
{
	Common::File fp;
	int numChar = 0;
	char fontFile[128];

	strcpy(fontFile, "korean.fnt");
	numChar = 2350;
#ifndef __GP32__
	if (fp.open(fontFile, Common::File::kFileReadMode, "./font/")) {
#else
	if (fp.open(fontFile, Common::File::kFileReadMode, "gp:\\gpmm\\scummvm\\font\\")) {
#endif
		fp.seek(2, SEEK_CUR);
		_korFontWidth = fp.readByte();
		_korFontHeight = fp.readByte();
		_korFontPtr = new byte[((_korFontWidth + 7) / 8) * _korFontHeight * numChar];

		fp.read(_korFontPtr, ((_korFontWidth + 7) / 8) * _korFontHeight * numChar);
		fp.close();
		printf("V1 �ѱ� ��Ʈ�� �ε�Ǿ����ϴ�.\n");
	} else {
		printf("V1 �ѱ� ��Ʈ�� �ε��� �� �����ϴ�!\n");
	}

	strcpy(fontFile, "english.fnt");
	numChar = 256;
#ifndef __GP32__
	if (fp.open(fontFile, Common::File::kFileReadMode, "./font/")) {
#else
	if (fp.open(fontFile, Common::File::kFileReadMode, "gp:\\gpmm\\scummvm\\font\\")) {
#endif
		fp.seek(2, SEEK_CUR);
		_engFontWidth = fp.readByte();
		_engFontHeight = fp.readByte();
		_engFontPtr = new byte[((_engFontWidth + 7) / 8) * _engFontHeight * numChar];

		fp.read(_engFontPtr, ((_engFontWidth + 7) / 8) * _engFontHeight * numChar);
		fp.close();
		printf("V1 ���� ��Ʈ�� �ε�Ǿ����ϴ�.\n");
	} else {
		printf("V1 ���� ��Ʈ�� �ε��� �� �����ϴ�!\n");
	}
}

byte *getFontPtr(int idx)
{
	if(idx & 0x7f00) {
		idx = ((idx % 256) - 0xb0) * 94 + (idx / 256) - 0xa1;
		return 	_korFontPtr + ((_korFontWidth + 7) / 8) * _korFontHeight * idx;
	} else {
		return 	_engFontPtr + ((_engFontWidth + 7) / 8) * _engFontHeight * idx;
	}
}

int getTopMargin(uint8 eb)
{
	switch(eb) {
	case 'g':
	case 'p':
	case 'q':
	case 'y':
	case ',':
	case '!':
		return 1;
	default:
		return 0;
		break;
	}
}

int getEngWidth(uint8 eb)
{
	switch(eb) {
	case '_':
		return 9;
	case '5':
	case 'J':
	case 'M':
	case 'W':
	case 'm':
	case 'w':
	case '*':
		return 8;
	case 'e':
		return 7;
	case 'f':
	case 'I':
	case 'i':
	case 'l':
	case 'c':
		return 6;
	case '.':
	case '!':
		return 5;
	case ' ':
		return 4;
	default:
		return 7;
		break;
	}
}

#ifndef __GP32__

void putEmergencyChar(uint16 *dst, int _screenWidth, uint16 _color, int chr)
{
	const byte revBitMask[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
	int y, x;
	byte bits = 0;
	const byte *src = getFontPtr(chr);

	int height, width;

	height = (chr & 0x7f00) ? _korFontHeight : _engFontHeight;
	width = (chr & 0x7f00) ? _korFontWidth : _engFontWidth;

	for (y = 0; y < height /*&& y + drawTop < vs->height*/; y++) {
		for (x = 0; x < width; x++) {
			if ((x % 8) == 0)
				bits = *src++;
			if ((bits & revBitMask[x % 8])/* && y + drawTop >= 0*/) {
				*dst = _color;
			}
			dst++;
		}
		dst += _screenWidth - width;
	}
}

uint16 getRGB(K_Surface *screen, uint8 R, uint8 G, uint8 B)
{
	return SDL_MapRGB(screen->format, R, G, B);
}

void putEmergencyFont(K_Surface *screen, int xpos, int ypos, int scrw, int scrh, uint16 color, const char *buf)
{
	const byte *buffer = (const byte *)buf;

	int clipw = 0;

	int px = xpos;
	int py = ypos;
	if ( SDL_MUSTLOCK(screen) ) {
		if ( SDL_LockSurface(screen) < 0 ) {
			printf("putEmergencyFont() : Lock ����\n");
			return;
		}
	}
	uint16 *dst;

	int c;

	int w = 8;
	int h = _engFontHeight;

	int engMargin = 0;
	
	do {
		c = *buffer++;
		if (c == 0) {
			break;
		}
		if (c == 0x0B)
			continue;
		if (c == '\\') {
			if ( *buffer == 'n' || *buffer == 'N' ) {
				buffer++;
				px = xpos;
				py += h + 2;
				dst = (uint16 *)screen->pixels + py * screen->pitch / 2 + px;
			} else if (*buffer == 'c' || *buffer == 'C' )	{
				uint8 d = *buffer++;
				if (color) // ���� ��(0,0,0)�� �ƴ϶��
					color = getRGB(screen, _kPalette[d].r, _kPalette[d].g, _kPalette[d].b);
			}
			continue;
		} else if ( c == 13 ) {
			if ( *buffer == 10 ) {
				px = xpos;
				py += h + 2;
			}
			continue;
		}
		if (c & 0x80) {
			c += (*buffer++) * 256; //LE
			w = _korFontWidth;
			h = _korFontHeight;
			engMargin = 0;
		} else {
			w = getEngWidth(c);
			h = _engFontHeight;
			engMargin = getTopMargin(c);
		}

		if(px < 0 || px + w > (scrw + (1 - clipw))) { // ���η� ȭ�� ��
				px = xpos;
				py += h + 2;
		}
		dst = (uint16 *)screen->pixels + (py + engMargin) * screen->pitch / 2 + px;

		int offsetX[9] = { -1,  0, 1, 1, 1, 0, -1, -1, 0 };
		int offsetY[9] = {  -1, -1, -1, 0, 1, 1, 1, 0, 0 };
		int cTable[9] =  {  0,  0, 0, 0, 0, 0, 0, 0, color };
		int i = 0;

		int showShadow = 1;

		if (!showShadow)
			i = 8;
	
		for (; i < 9; i++) {
			dst = (uint16 *)screen->pixels + (py + engMargin + offsetY[i]) * screen->pitch / 2 + (px + offsetX[i]);
			putEmergencyChar(dst, (screen->pitch / 2), cTable[i], c);
		}
		dst += w;
		px += w;
	} while (1);

	if(SDL_MUSTLOCK(screen))
		SDL_UnlockSurface(screen);
}


#else

void putEmergencyChar(uint16 *buffer, int origx, int origy, uint16 _color, int chr)
{
	const byte revBitMask[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
	int y, x;
	byte bits = 0;
	const byte *src = getFontPtr(chr);

	int height, width;

	height = (chr & 0x7f00) ? _korFontHeight : _engFontHeight;
	width = (chr & 0x7f00) ? _korFontWidth : _engFontWidth;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			if ((x % 8) == 0)
				bits = *src++;
			if ((bits & revBitMask[x % 8])) {
				if(origx + x >= 0 && origy + y >= 0 && origx + x < 320 && origy + y < 240)
					gpd_drawPixel16(buffer, origx + x, origy + y, _color);
			}
		}
	}
}

void putEmergencyFont(K_Surface *screen, int xpos, int ypos, int scrw, int scrh, uint16 color, const char *buf)
{
	const byte *buffer = (const byte *)buf;

	int clipw = 0;

	int px = xpos;
	int py = ypos;

	int c;

	int w = 8;
	int h = _engFontHeight;

	int engMargin = 0;
	
	do {
		c = *buffer++;
		if (c == 0) {
			break;
		}
		if (c == 0x0B)
			continue;
		if (c == '\\') {
			if ( *buffer == 'n' || *buffer == 'N' ) {
				buffer++;
				px = xpos;
				py += h + 2;
			} else if (*buffer == 'c' || *buffer == 'C' )	{
				uint8 d = *buffer++;
				if (color) // ���� ��(0,0,0)�� �ƴ϶��
					color = _kPalette[d];
			}
			continue;
		} else if ( c == 13 ) {
			if ( *buffer == 10 ) {
				px = xpos;
				py += h + 2;
			}
			continue;
		}
		if (c & 0x80) {
			c += (*buffer++) * 256; //LE
			w = _korFontWidth;
			h = _korFontHeight;
			engMargin = 0;
		} else {
			w = getEngWidth(c);
			h = _engFontHeight;
			engMargin = getTopMargin(c);
		}

		if(px < 0 || px + w > (scrw + (1 - clipw))) { // ���η� ȭ�� ��
				px = xpos;
				py += h + 2;
		}

		int offsetX[9] = { -1,  0, 1, 1, 1, 0, -1, -1, 0 };
		int offsetY[9] = {  -1, -1, -1, 0, 1, 1, 1, 0, 0 };
		int cTable[9] =  {  0,  0, 0, 0, 0, 0, 0, 0, color };
		int i = 0;

		int showShadow = 1;

		if (!showShadow)
			i = 8;

		for (; i < 9; i++) {
			putEmergencyChar(screen, (px + offsetX[i]), (py + engMargin + offsetY[i]), cTable[i], c);
		}
		px += w;
	} while (1);
}

#endif

////////////////////////////////////

void loadKoreanFiles(const char *id)
{
	strcpy(_gameID, id);

	if(!strncasecmp(_gameID, "monkeyega", 9))
		strcpy(_gameID, "monkey");
	if(!strncasecmp(_gameID, "monkeyvga", 9))
		strcpy(_gameID, "monkey");
	if(!strncasecmp(_gameID, "monkey1", 7))
		strcpy(_gameID, "monkey");
	if(!strncasecmp(_gameID, "indy3ega", 8))
		strcpy(_gameID, "indy3");

	// �ε��ϴ� ��ġ��?
	loadEmergencyFont();
	loadKoreanStrings();
}

} // End of namespace Scumm
