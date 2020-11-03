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
 * $Header: /cvsroot/scummkor/scummvm-kor/scumm/korean.h,v 1.4 2004/08/08 04:49:53 wonst719 Exp $
 */
#ifndef SCUMM_KOREAN_H
#define SCUMM_KOREAN_H

namespace Scumm {

extern bool _koreanMode;
extern bool _koreanOnly;
extern bool _highRes;

extern void loadKoreanFiles(const char *id);
extern void unloadKoreanFiles();

extern char *convertToKorean(const char *buf, bool descFlag);
extern char *convertToKoreanValid(const char *buf, bool descFlag);

} // End of namespace Scumm

#endif
