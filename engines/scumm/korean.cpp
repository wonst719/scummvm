/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
#include "scumm/scumm.h"

#include "scumm/korean.h"

#include "common/file.h"

int strncasecmp2(const char *s1, const char *s2, int n) {
	char t1, t2;
	char u1, u2;

	while (n--) {
		if (!*s2)
			return (*s1 != 0);
		else if (!*s1)
			return -1;
		u1 = *s1++;
		u2 = *s2++;
		t1 = (('A' <= u1 && u1 <= 'Z') ? (u1 | 040) : u1);
		t2 = (('A' <= u2 && u2 <= 'Z') ? (u2 | 040) : u2);
		if (t1 != t2)
			return (t1 > t2) ? 1 : -1;
	}
	return 0;
}

namespace Scumm {

bool _koreanMode = 0;
bool _koreanOnly = 0;
bool _highRes = 0;

char _gameID[16];

char _parseBuffer[1024];

char **_KBuffer = 0;
int _numKLines = 0;

char *convertToKorean(const char *buf, bool descFlag) {
	static int currentLine;

	char *tbuf, *sbuf;
	bool found = 0;

	sbuf = tbuf = _parseBuffer;

	if (buf[0] == 0x07 && descFlag)
		buf++;
	int i;
	for (i = currentLine; i < _numKLines; i += 2) {
		if (!strncasecmp2(buf, _KBuffer[i], strlen(_KBuffer[i])) && strlen(_KBuffer[i]) == strlen(buf)) {
			strcpy(tbuf, _KBuffer[i + 1]);
			currentLine = i;
			found = 1;
			break;
		}
	}
	if (found == 0) {
		for (i = 0; i < currentLine; i += 2) {
			if (!strncasecmp2(buf, _KBuffer[i], strlen(_KBuffer[i])) && strlen(_KBuffer[i]) == strlen(buf)) {
				strcpy(tbuf, _KBuffer[i + 1]);
				currentLine = i;
				found = 1;
				break;
			}
		}
	}

	if (found == 0 && descFlag) {
		if (_highRes)
			strcpy(tbuf, buf);
		else
			*tbuf = 0;
	} else if (found == 0)
		if (_koreanOnly) {
			strcpy(tbuf, buf);
			while (*tbuf) {
				if (!strncasecmp2(tbuf, "<1>", strlen("<1>"))) {
					*tbuf = '\\';
					*(tbuf + 1) = '\\';
					*(tbuf + 2) = 'n';
				}
				if (!strncasecmp2(buf, "<3>", strlen("<3>"))) {
					*tbuf = '\\';
					*(tbuf + 1) = '\\';
					*(tbuf + 2) = 'n';
				}
				if (!strncasecmp2(tbuf, "<c>", strlen("<c>"))) {
					*tbuf = '\\';
					*(tbuf + 1) = '\\';
					*(tbuf + 2) = 'c';
				}
				tbuf++;
			}
			tbuf = sbuf;
		} else
			*tbuf = 0;

	return sbuf;
}

char *convertToKoreanValid(const char *buf, bool descFlag) {
	static int currentLine;

	char *tbuf, *sbuf;
	bool found = 0;

	sbuf = tbuf = _parseBuffer;

	if (buf[0] == 0x07 && descFlag)
		buf++;

	int i;
	for (i = currentLine; i < _numKLines; i += 2) {
		if (!strncasecmp2(buf, _KBuffer[i], strlen(_KBuffer[i])) && strlen(_KBuffer[i]) == strlen(buf)) {
			strcpy(tbuf, _KBuffer[i + 1]);
			currentLine = i;
			found = 1;
			break;
		}
	}
	if (found == 0) {
		for (i = 0; i < currentLine; i += 2) {
			if (!strncasecmp2(buf, _KBuffer[i], strlen(_KBuffer[i])) && strlen(_KBuffer[i]) == strlen(buf)) {
				strcpy(tbuf, _KBuffer[i + 1]);
				currentLine = i;
				found = 1;
				break;
			}
		}
	}

	if (found == 0)
		return 0;

	return sbuf;
}

static void FGETS(char *buf, int maxlen, Common::File &fp) {
	int i, n;
	char c;

	for (i = 0; i < maxlen; i++) {
		n = fp.read(&c, 1);
		if (n == 0) {
			buf[i] = 0;
			break;
		}
		buf[i] = c;
		if (c == '\n') {
			if (i < maxlen - 1)
				buf[i] = 0;
			break;
		}
	}
}
#ifndef fgets
#define fgets FGETS
#endif

void loadKoreanStrings() {
	Common::File fp;
	char fname[128];

	strcpy(fname, "sub/");
	strcat(fname, _gameID);
	strcat(fname, ".dat");

	if (!fp.open(fname)) {
		// warning("WARNING: Cannot load Korean V1 subtitle!, %s\n", fname);

		// retry without sub
		strcpy(fname, "");
		strcat(fname, _gameID);
		strcat(fname, ".dat");

		if (!fp.open(fname)) {
			warning("WARNING: Cannot load Korean V1 subtitle!, %s\n", fname);
			return;
		}
	}

	int k;
	char *buf = new char[1024];
	int len = 0;

	for (k = 0; !fp.eos(); k++) {
		FGETS(buf, 1023, fp);
		//printf("%d: <%s>\n", k+1, buf);
	}
	_numKLines = k;

	// memory size = file size
	k = fp.size();
	fp.seek(0, SEEK_SET);

	_KBuffer = new char *[_numKLines];
	_KBuffer[0] = new char[k];

	for (int i = 0; i < _numKLines; i++) {
		FGETS(buf, 1023, fp);
		buf[1023] = 0;
		len = strlen(buf);
		if (i > 0)
			_KBuffer[i] = _KBuffer[i - 1] + strlen(_KBuffer[i - 1]) + 1;
		if (len > 1000)
			warning("_KBuffer[%d]:%lx, len=%d\n", i, _KBuffer[i], len);
		if (strlen(buf)) {
			strcpy(_KBuffer[i], buf);
			char *b = _KBuffer[i];
			if (b[strlen(b) - 1] == '\n' || b[strlen(b) - 1] == 0x0a || b[strlen(b) - 1] == 0x0d)
				b[strlen(b) - 1] = 0;
			if (b[strlen(b) - 1] == '\n' || b[strlen(b) - 1] == 0x0a || b[strlen(b) - 1] == 0x0d)
				b[strlen(b) - 1] = 0;
		} else
			_KBuffer[i][0] = 0;
	}
	fp.close();

	warning("Korean subtitle file loaded -- total %d lines, %ld bytes\n", _numKLines, _KBuffer[_numKLines - 1] - _KBuffer[0] + len);

	delete[] buf;

	return;
}

bool unloadKorString() {
	if (_KBuffer && _KBuffer[0])
		delete[] _KBuffer[0];
	if (_KBuffer)
		delete[] _KBuffer;
	return true;
}

void unloadKoreanFiles() {
	unloadKorString();
}

void loadKoreanFiles(const char *id) {
	strcpy(_gameID, id);

	if (!strncasecmp2(_gameID, "monkeyega", 9))
		strcpy(_gameID, "monkey");
	if (!strncasecmp2(_gameID, "monkeyvga", 9))
		strcpy(_gameID, "monkey");
	if (!strncasecmp2(_gameID, "monkey1", 7))
		strcpy(_gameID, "monkey");
	if (!strncasecmp2(_gameID, "indy3ega", 8))
		strcpy(_gameID, "indy3");

	loadKoreanStrings();
}

} // End of namespace Scumm
