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

#ifndef ULTIMA4_CONTEXT_H
#define ULTIMA4_CONTEXT_H

#include "ultima/ultima4/location.h"
#include "ultima/ultima4/aura.h"
#include "ultima/ultima4/names.h"
#include "ultima/ultima4/person.h"
#include "ultima/ultima4/script.h"
#include "ultima/ultima4/types.h"
#include "ultima/ultima4/savegame.h"
#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Ultima4 {

class Object;
class Party;
class Person;
class Script;
class StatsArea;

enum TransportContext {
	TRANSPORT_FOOT      = 0x1,
	TRANSPORT_HORSE     = 0x2,
	TRANSPORT_SHIP      = 0x4,
	TRANSPORT_BALLOON       = 0x8,
	TRANSPORT_FOOT_OR_HORSE = TRANSPORT_FOOT | TRANSPORT_HORSE,
	TRANSPORT_ANY           = 0xffff
};

/**
 * Context class
 */
class Context : public Script::Provider {
public:
	Context();
	~Context();

	Party *_party;
	SaveGame *_saveGame;
	class Location *_location;
	int _line, col;
	StatsArea *_stats;
	int _moonPhase;
	int _windDirection;
	int _windCounter;
	bool _windLock;
	Aura *_aura;
	int _horseSpeed;
	int _opacity;
	TransportContext _transportContext;
	uint32 _lastCommandTime;
	class Object *_lastShip;

	/**
	 * Provides scripts with information
	 */
	virtual Common::String translate(Std::vector<Common::String> &parts) {
		if (parts.size() == 1) {
			if (parts[0] == "wind")
				return getDirectionName(static_cast<Direction>(_windDirection));
		}
		return "";
	}
};

extern Context *g_context;

} // End of namespace Ultima4
} // End of namespace Ultima

#endif