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

#include "ultima8/misc/pent_include.h"

#include "ultima8/world/monster_egg.h"
#include "ultima8/usecode/uc_machine.h"
#include "ultima8/world/actors/actor.h"
#include "ultima8/world/item_factory.h"
#include "ultima8/world/current_map.h"
#include "ultima8/graphics/shape_info.h"
#include "ultima8/world/actors/monster_info.h"
#include "ultima8/world/world.h"
#include "ultima8/world/get_object.h"

#include "ultima8/filesys/idata_source.h"
#include "ultima8/filesys/odata_source.h"
#include "ultima8/world/get_object.h"

namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(MonsterEgg, Item);

MonsterEgg::MonsterEgg() {

}


MonsterEgg::~MonsterEgg() {

}

uint16 MonsterEgg::hatch() {
	//!! do we need to check probability here?
	//!! monster activity? combat? anything?

	int shapeNum = getMonsterShape();

	// CHECKME: why does this happen? (in the plane of Earth near the end)
	if (shapeNum == 0)
		return 0;

	Actor *newactor = ItemFactory::createActor(shapeNum, 0, 0,
	                  FLG_FAST_ONLY | FLG_DISPOSABLE | FLG_IN_NPC_LIST,
	                  0, 0, 0, true);
	if (!newactor) {
		perr << "MonsterEgg::hatch failed to create actor (" << shapeNum
		     << ")." << std::endl;
		return 0;
	}
	uint16 objID = newactor->getObjId();

	// set stats
	if (!newactor->loadMonsterStats()) {
		perr << "MonsterEgg::hatch failed to set stats for actor (" << shapeNum
		     << ")." << std::endl;
	}

	if (!newactor->canExistAt(x, y, z)) {
		newactor->destroy();
		return 0;
	}

	// mapnum has to be set to the current map. Reason: Beren teleports to
	// newactor->getMapNum() when newactor is assaulted.
	newactor->setMapNum(World::get_instance()->getCurrentMap()->getNum());
	newactor->setNpcNum(objID);
	newactor->move(x, y, z);

	newactor->cSetActivity(getActivity());

	return objID;
}

void MonsterEgg::saveData(ODataSource *ods) {
	Item::saveData(ods);
}

bool MonsterEgg::loadData(IDataSource *ids, uint32 version) {
	if (!Item::loadData(ids, version)) return false;

	return true;
}

uint32 MonsterEgg::I_monsterEggHatch(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	MonsterEgg *megg = p_dynamic_cast<MonsterEgg *>(item);
	if (!megg) return 0;

	return megg->hatch();
}

uint32 MonsterEgg::I_getMonId(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	MonsterEgg *megg = p_dynamic_cast<MonsterEgg *>(item);
	if (!megg) return 0;

	return megg->getMapNum() >> 3;
}

} // End of namespace Ultima8