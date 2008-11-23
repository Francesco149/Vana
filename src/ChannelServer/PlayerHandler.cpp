/*
Copyright (C) 2008 Vana Development Team

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
#include "PlayerHandler.h"
#include "Maps.h"
#include "Mobs.h"
#include "Movement.h"
#include "Player.h"
#include "Players.h"
#include "PlayerPacket.h"
#include "PlayersPacket.h"
#include "Randomizer.h"
#include "PacketReader.h"
#include "SkillsPacket.h"

void PlayerHandler::handleDamage(Player *player, PacketReader &packet) {
	packet.skipBytes(4); // Ticks
	uint8_t type = packet.getByte();
	uint8_t hit = 0;
	uint8_t stance = 0;
	packet.skipBytes(1); // Element - 0x00 = elementless, 0x01 = ice, 0x02 = fire, 0x03 = lightning
	int16_t job = player->getJob();
	int16_t disease = 0;
	int32_t mapmobid = 0; // Map Mob ID
	Mob *mob = 0;
	int32_t mobid = 0; // Actual Mob ID - i.e. 8800000 for Zak
	int32_t nodamageid = 0;
	int32_t damage = packet.getInt();
	bool applieddamage = false;
	PGMRInfo pgmr;
	MobAttackInfo attack;
	switch (type) {
		case 0xFE: // Map/fall damage is an oddball packet
			break;
		default: // Code in common, minimizes repeated code
			packet.skipBytes(4); // Mob ID
			mapmobid = packet.getInt();
			mob = Maps::getMap(player->getMap())->getMob(mapmobid);
			if (mob == 0) {
				// Hacking
				return;
			}

			mobid = mob->getMobID();
			if (type != 0xFF) {
				try {
					attack = mob->getAttackInfo(type);
					disease = attack.disease;
				}
				catch (std::out_of_range) {
					// Not having data about linked monsters causes crashes with linked monsters
					// For example - those quest Dark Lords or the El Nath PQ Lycanthropes
					// We have no way of transferring the data at the moment

					// Now we leave attack and disease at their default values
				}
			}
			hit = packet.getByte(); // Knock direction
			break;
	}
	switch (type) { // Account for special sections of the damage packet
		case 0xFE:
			break;		
		default: // Else: Power Guard/Mana Reflection
			pgmr.reduction = packet.getByte();
			packet.skipBytes(1); // I think reduction is a short, but it's a byte in the S -> C packet, so..
			if (pgmr.reduction != 0) {
				if (packet.getByte() == 0)
					pgmr.isphysical = false;
				pgmr.mapmobid = packet.getInt();
				packet.skipBytes(1); // 0x06 for Power Guard, 0x00 for Mana Reflection?
				packet.skipBytes(4); // Mob position garbage
				pgmr.pos.x = packet.getShort();
				pgmr.pos.y = packet.getShort();
				pgmr.damage = damage;
				if (pgmr.isphysical) // Only Power Guard decreases damage
					damage = (damage - (damage * pgmr.reduction / 100)); 

				mob->applyDamage(player->getId(), (pgmr.damage * pgmr.reduction / 100));
			}
			break;
	}
	switch (type) { // Packet endings
		case 0xFE:
			disease = packet.getShort(); // Disease
			break;
		default:  {
			stance = packet.getByte(); // Power Stance
			if (stance > 0) {
				int32_t skillid = 0;
				if (player->getActiveBuffs()->getActiveSkillLevel(1121002) > 0)
					skillid = 1121002;
				else if (player->getActiveBuffs()->getActiveSkillLevel(1221002) > 0)
					skillid = 1221002;
				else if (player->getActiveBuffs()->getActiveSkillLevel(1321002) > 0)
					skillid = 1321002;
				if (skillid == 0 || player->getSkills()->getSkillLevel(skillid) == 0) {
					// Hacking
					return;
				}
			}
			break;
		}
	}
	if (damage == -1) {
		switch (job) {
			case 412: nodamageid = 4120002; break; // Fake
			case 422: nodamageid = 4220002; break; // Fake
			case 112: nodamageid = 1120005; break; // Guardian
			case 122: nodamageid = 1220006; break; // Guardian
		}
		if (nodamageid == 0 || player->getSkills()->getSkillLevel(nodamageid) == 0) {
			// Hacking
			return;
		}
	}
	if (disease > 0 && damage != 0) { // Fake/Guardian don't prevent disease
		// Status ailment processing here
	}
	if (damage > 0 && !player->hasGMEquip()) {
		if (player->getActiveBuffs()->getActiveSkillLevel(4211005) > 0 && player->getInventory()->getMesos() > 0) { // Meso Guard 
			int16_t mesorate = Skills::skills[4211005][player->getActiveBuffs()->getActiveSkillLevel(4211005)].x; // Meso Guard meso %
			int16_t hp = player->getHP();
			int16_t mesoloss = (int16_t)(mesorate * damage / 2 / 100);
			int32_t mesos = player->getInventory()->getMesos();
			int32_t newmesos = mesos - mesoloss;
			if (newmesos < 0) { // Special damage calculation for not having enough mesos
				double mesos2 = mesos + 0.0; // You can't get a double from math involving 2 ints, even if a decimal results
				double reduction = 2.0 - ((mesos2 / mesoloss) / 2);
				damage = (uint16_t)(damage / reduction); // This puts us pretty close to the damage observed clientside, needs improvement
			}
			else
				damage /= 2; // Usually displays 1 below the actual damage but is sometimes accurate - no clue why
			player->getInventory()->setMesos(newmesos);
			SkillsPacket::showSkillEffect(player, 4211005);
			player->damageHP((uint16_t) damage);
			if (attack.deadlyattack && player->getMP() > 0)
				player->setMP(1);
			if (attack.mpburn > 0)
				player->damageMP(attack.mpburn);
			applieddamage = true;
		}
		if (player->getActiveBuffs()->getActiveSkillLevel(2001002) > 0) { // Magic Guard
			int16_t mp = player->getMP();
			int16_t hp = player->getHP();
			if (attack.deadlyattack) {
				if (mp > 0)
					player->setMP(1);
				player->setHP(1);
			}
			else if (attack.mpburn > 0) {
				player->damageMP(attack.mpburn);
				player->damageHP((uint16_t) damage);
			}
			else {
				int16_t reduc = Skills::skills[2001002][player->getActiveBuffs()->getActiveSkillLevel(2001002)].x;
				uint16_t mpdamage = (uint16_t)((damage * reduc) / 100);
				uint16_t hpdamage = (uint16_t)(damage - mpdamage);
				bool ison = false;
				if (job % 10 == 2) {
					int32_t infinity = player->getJob() * 10000 + 1004;
					if (player->getActiveBuffs()->getActiveSkillLevel(infinity) > 0)
						ison = true;
				}
				if (mpdamage < mp || ison) {
					player->damageMP(mpdamage);
					player->damageHP(hpdamage);
				}
				else if (mpdamage >= mp) {
					player->setMP(0);
					player->damageHP(hpdamage + (mpdamage - mp));
				}
			}
			applieddamage = true;
		}
		if (((job / 100) == 1) && ((job % 10) == 2)) { // Achilles for 4th job warriors
			float achx = 1000.0;
			int32_t sid = 1120004;
			switch (job) {
				case 112: sid = 1120004; break;
				case 122: sid = 1220005; break;
				case 132: sid = 1230005; break;
			}
			uint8_t slv = player->getSkills()->getSkillLevel(sid);
			if (slv > 0)
				achx = Skills::skills[sid][slv].x;
			double red = (2.0 - achx / 1000.0);
			player->damageHP((uint16_t) (damage / red));
			if (attack.deadlyattack && player->getMP() > 0)
				player->setMP(1);
			if (attack.mpburn > 0)
				player->damageMP(attack.mpburn);
			applieddamage = true;
		}
		if (applieddamage == false) {
			if (attack.deadlyattack) {
				if (player->getMP() > 0)
					player->setMP(1);
				player->setHP(1);
			}
			else
				player->damageHP((uint16_t) damage);
			if (attack.mpburn > 0)
				player->damageMP(attack.mpburn);
		}
	}
	PlayersPacket::damagePlayer(player, damage, mobid, hit, type, stance, nodamageid, pgmr);
}

void PlayerHandler::handleFacialExpression(Player *player, PacketReader &packet) {
	int32_t face = packet.getInt();
	PlayersPacket::faceExpression(player, face);
}

void PlayerHandler::handleGetInfo(Player *player, PacketReader &packet) {
	packet.skipBytes(4);
	int32_t playerid = packet.getInt();
	PlayersPacket::showInfo(player, Players::Instance()->getPlayer(playerid), packet.getByte());
}

void PlayerHandler::handleHeal(Player *player, PacketReader &packet) {
	packet.skipBytes(4);
	int16_t hp = packet.getShort();
	int16_t mp = packet.getShort();
	player->modifyHP(hp);
	player->modifyMP(mp);
}

void PlayerHandler::handleMoving(Player *player, PacketReader &packet) {
	packet.reset(7);
	Movement::parseMovement(player, packet);
	packet.reset(7);
	PlayersPacket::showMoving(player, packet.getBuffer(), packet.getBufferLength());
}

void PlayerHandler::handleSpecialSkills(Player *player, PacketReader &packet) {
	int32_t skillid = packet.getInt();
	switch (skillid) {
		case 1121001: // Monster Magnet x3
		case 1221001:
		case 1321001:
		case 3221001: // Pierce
		case 2121001: // Big Bang x3
		case 2221001:
		case 2321001: {
			SpecialSkillInfo info;
			info.skillid = skillid;
			info.level = packet.getByte();
			info.direction = packet.getByte();
			info.w_speed = packet.getByte();
			player->setSpecialSkill(info);
			SkillsPacket::showSpecialSkill(player, info);
			break;
		}
		case 4211001: { // Chakra
			int16_t dex = player->getDex();
			int16_t luk = player->getLuk();
			int16_t recovery = Skills::skills[4211001][player->getSkills()->getSkillLevel(4211001)].y;
			int16_t maximum = (luk * 66 / 10 + dex) * 2 / 10 * (recovery / 100 + 1);
			int16_t minimum = (luk * 33 / 10 + dex) * 2 / 10 * (recovery / 100 + 1);
			// Maximum = (luk * 6.6 + dex) * 0.2 * (recovery% / 100 + 1)
			// Minimum = (luk * 3.3 + dex) * 0.2 * (recovery% / 100 + 1)
			// I used 66 / 10 and 2 / 10 respectively to get 6.6 and 0.2 without using floating points
			int16_t range = maximum - minimum;
			player->modifyHP(Randomizer::Instance()->randShort(range) + minimum);
			break;
		}
	}
}