/*
Copyright (C) 2008-2009 Vana Development Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#include "Drop.h"
#include "DropsPacket.h"
#include "Maps.h"
#include "Pets.h"
#include "Player.h"
#include "Players.h"
#include "TimeUtilities.h"
#include <limits>

Drop::Drop(int32_t mapid, int32_t mesos, const Pos &pos, int32_t owner, bool playerdrop) :
questid(0),
owner(owner),
mapid(mapid),
mesos(mesos),
// Initializing dropped time to max-value to prevent timers from
// deleting the drop in case doDrop did not get called right away
dropped(std::numeric_limits<int32_t>::max()),
playerid(0),
playerdrop(playerdrop),
type(0),
tradeable(true),
pos(pos)
{
	Maps::getMap(mapid)->addDrop(this);
}

Drop::Drop(int32_t mapid, const Item &item, const Pos &pos, int32_t owner, bool playerdrop) :
questid(0),
owner(owner),
mapid(mapid),
mesos(0),
dropped(std::numeric_limits<int32_t>::max()),
playerid(0),
playerdrop(playerdrop),
type(0),
tradeable(true),
pos(pos),
item(item)
{
	Maps::getMap(mapid)->addDrop(this);
}

int32_t Drop::getObjectId() {
	return (mesos > 0 ? mesos : item.id);
}

int16_t Drop::getAmount() {
	return item.amount;
}

void Drop::doDrop(const Pos &origin) {
	setDropped(TimeUtilities::getTickCount());
	if (!isQuest()) {
		if (!isTradeable()) {
			DropsPacket::showDrop(0, this, 3, false, origin);
			this->removeDrop(false);
		}
		else
			DropsPacket::showDrop(0, this, 1, true, origin);
	}
	else if (Player *player = Players::Instance()->getPlayer(playerid)) {
		if (player->getMap() == this->mapid) {
			DropsPacket::showDrop(player, this, 1, true, origin);
		}
	}
}

void Drop::showDrop(Player *player) {
	if (isQuest() && player->getId() != playerid)
		return;
	DropsPacket::showDrop(player, this, 2, false, Pos());
}

void Drop::takeDrop(Player *player, int32_t petid) {
	Maps::getMap(mapid)->removeDrop(this->id);
	if (petid == 0)
		DropsPacket::takeDrop(player, this);
	else
		DropsPacket::takeDrop(player, this, player->getPets()->getPet(petid)->getIndex());
	delete this;
}

void Drop::removeDrop(bool showPacket) {
	Maps::getMap(this->mapid)->removeDrop(this->id);
	if (showPacket)
		DropsPacket::removeDrop(this);
	delete this;
}