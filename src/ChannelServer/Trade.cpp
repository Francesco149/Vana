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
#include "Trade.h"
#include "GameLogicUtilities.h"
#include "Inventory.h"
#include "InventoryPacket.h"
#include "ItemDataProvider.h"
#include "Player.h"
#include "Players.h"
#include "Trades.h"
#include "VersionConstants.h"

ActiveTrade::ActiveTrade(Player *sender, Player *receiver, int32_t id) :
sender(new TradeInfo()),
receiver(new TradeInfo()),
id(id)
{
	sender->setTrading(true);
	receiver->setTrading(false);
	sender->setTradeId(id);
	receiver->setTradeId(id);
	senderid = sender->getId();
	receiverid = receiver->getId();
}

bool ActiveTrade::bothCanTrade() {
	TradeInfo *send = getSenderTrade();
	TradeInfo *recv = getReceiverTrade();
	Player *one = getSender();
	Player *two = getReceiver();

	int32_t comparison = recv->mesos + one->getInventory()->getMesos();
	if (comparison < 0) {// Determine if sender can receive all the mesos
		return false;
	}
	if (recv->count > 0 && !canTrade(one, recv)) { // And all the items
		return false;
	}

	comparison = send->mesos + two->getInventory()->getMesos();
	if (comparison < 0) { // Determine if receiver can receive all the mesos
		return false;
	}
	if (send->count > 0 && !canTrade(two, send)) { // And all the items
		return false;
	}

	return true;
}

bool ActiveTrade::canTrade(Player *target, TradeInfo *unit) {
	bool yes = true;
	boost::array<int8_t, Inventories::InventoryCount> totals = {0};
	unordered_map<int32_t, int16_t> added;
	for (int8_t i = 0; i < TradeInfo::TradeSize; i++) {
		// Create item structure to determine needed slots among stackable items
		// Also, determine needed slots for nonstackables
		if (unit->slot[i]) {
			Item *check = unit->items[i];
			int32_t itemid = check->id;
			int8_t inv = GameLogicUtilities::getInventory(itemid);
			if (inv == Inventories::EquipInventory || GameLogicUtilities::isRechargeable(itemid)) // Equips and rechargeables always take 1 slot, no need to clutter unordered map
				totals[inv - 1]++;
			else {
				if (added.find(itemid) != added.end()) // Already initialized this item
					added[itemid] += check->amount;
				else
					added[itemid] = check->amount;
			}
		}
	}
	for (int8_t i = 0; i < TradeInfo::TradeSize; i++) { // Determine precisely how many slots are needed for stackables
		if (unit->slot[i]) {
			Item *check = unit->items[i];
			int32_t itemid = check->id;
			int8_t inv = GameLogicUtilities::getInventory(itemid);
			if (inv != Inventories::EquipInventory && !GameLogicUtilities::isRechargeable(itemid)) { // Already did these
				if (added.find(itemid) == added.end()) // Already did this item
					continue;
				int16_t maxslot = ItemDataProvider::Instance()->getMaxSlot(itemid);
				int32_t current_amount = target->getInventory()->getItemAmount(itemid);
				int32_t last_slot = (current_amount % maxslot); // Get the number of items in the last slot
				int32_t item_sum = last_slot + added[itemid];
				bool needslots = false;
				if (last_slot > 0) { // Items in the last slot, potential for needing slots
					if (item_sum > maxslot)
						needslots = true;
				}
				else // Full in the last slot, for sure need all slots
					needslots = true;
				if (needslots) {
					uint8_t numslots = (uint8_t)(item_sum / maxslot);
					uint8_t remainder = (uint8_t)(item_sum % maxslot);
					if (remainder > 0)
						totals[inv - 1]++;
					totals[inv - 1] += numslots;
				}
				added.erase(itemid);
			}
		}
	}
	for (int8_t i = 0; i < Inventories::InventoryCount; i++) { // Determine if needed slots are available
		if (totals[i] > 0) {
			int8_t incrementor = 0;
			for (int8_t g = 1; g <= target->getInventory()->getMaxSlots(i + 1); g++) {
				if (target->getInventory()->getItem(i + 1, g) == 0)
					incrementor++;
				if (incrementor >= totals[i])
					break;
			}
			if (incrementor < totals[i]) {
				yes = false;
				break;
			}
		}
	}
	return yes;
}


void ActiveTrade::giveItems(Player *player, TradeInfo *info) {
	if (info->count > 0) {
		for (int8_t i = 0; i < TradeInfo::TradeSize; i++) {
			if (info->slot[i]) {
				Item *item = info->items[i];
				Inventory::addItem(player, new Item(item));
				delete item;
			}
		}
	}
}

void ActiveTrade::giveMesos(Player *player, TradeInfo *info, bool traded) {
	if (info->mesos > 0) {
		int32_t taxlevel = TradeHandler::getTaxLevel(info->mesos);
		if (traded && taxlevel != 0) {
			int64_t mesos = info->mesos * taxlevel / 10000;
			info->mesos -= (int32_t)(mesos);
		}
		player->getInventory()->modifyMesos(info->mesos);
	}
}

void ActiveTrade::returnTrade() {
	TradeInfo *send = getSenderTrade();
	TradeInfo *recv = getReceiverTrade();
	Player *one = getSender();
	Player *two = getReceiver();
	if (one != 0) {
		giveItems(one, send);
		giveMesos(one, send);
	}
	if (two != 0) {
		giveItems(two, recv);
		giveMesos(two, recv);
	}
}

void ActiveTrade::swapTrade() {
	TradeInfo *send = getSenderTrade();
	TradeInfo *recv = getReceiverTrade();
	Player *one = getSender();
	Player *two = getReceiver();
	giveItems(one, recv);
	giveItems(two, send);
	giveMesos(one, recv, true);
	giveMesos(two, send, true);
}

bool ActiveTrade::bothAccepted() {
	return (getSenderTrade()->accepted && getReceiverTrade()->accepted);
}

void ActiveTrade::accept(TradeInfo *unit) {
	unit->accepted = true;
}

int32_t ActiveTrade::addMesos(Player *holder, TradeInfo *unit, int32_t amount) {
	unit->mesos += amount;
	holder->getInventory()->modifyMesos(-amount, true);
	return unit->mesos;
}

Item * ActiveTrade::addItem(Player *holder, TradeInfo *unit, Item *item, int8_t tradeslot, int16_t inventoryslot, int8_t inventory, int16_t amount) {
	Item *use = new Item(item);
	if (amount == item->amount || inventory == Inventories::EquipInventory) {
		holder->getInventory()->setItem(inventory, inventoryslot, 0);
		InventoryPacket::moveItem(holder, inventory, inventoryslot, 0);
		holder->getInventory()->deleteItem(inventory, inventoryslot);
	}
	else {
		item->amount -= amount;
		holder->getInventory()->changeItemAmount(item->id, item->amount);
		InventoryPacket::updateItemAmounts(holder, inventory, inventoryslot, item->amount, 0, 0);
		use->amount = amount;
	}
	InventoryPacket::blankUpdate(holder); // Should prevent locking up in .70, don't know why it locks
	unit->count++;
	int8_t index = tradeslot - 1;
	unit->items[index] = use;
	unit->slot[index] = true;
	return use;
}

Player * ActiveTrade::getSender() {
	return Players::Instance()->getPlayer(senderid);
}

Player * ActiveTrade::getReceiver() {
	return Players::Instance()->getPlayer(receiverid);
}