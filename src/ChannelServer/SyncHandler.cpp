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
#include "SyncHandler.h"
#include "Connectable.h"
#include "GameObjects.h"
#include "GuildPacket.h"
#include "InterHeader.h"
#include "InterHelper.h"
#include "PacketCreator.h"
#include "PacketReader.h"
#include "Party.h"
#include "Player.h"
#include "PlayerDataProvider.h"
#include "PlayerPacket.h"
#include "SendHeader.h"
#include "SyncPacket.h"
#include "WorldServerConnection.h"

void SyncHandler::playerChangeChannel(WorldServerConnection *player, PacketReader &packet) {
	int32_t playerid = packet.get<int32_t>();
	uint32_t ip = packet.get<uint32_t>();
	int16_t port = packet.get<int16_t>();

	Player *ccPlayer = PlayerDataProvider::Instance()->getPlayer(playerid);
	if (!ccPlayer) {
		return;
	}
	if (ip == 0) {
		PlayerPacket::sendBlockedMessage(ccPlayer, 0x01);
	}
	else {
		ccPlayer->setOnline(0); // Set online to 0 BEFORE CC packet is sent to player
		PlayerPacket::changeChannel(ccPlayer, ip, port);
		ccPlayer->saveAll(true);
		ccPlayer->setSaveOnDc(false);
	}
}

void SyncHandler::newConnectable(PacketReader &packet) {
	Connectable::Instance()->newPlayer(packet.get<int32_t>());
}


void SyncHandler::guildPacketHandlerWorld(PacketReader &packet) {
	switch (packet.get<int8_t>()) {
		case 0x01: PlayerDataProvider::Instance()->unloadGuild(packet.get<int32_t>()); break;
		case 0x02: GuildPacket::handleEmblemChange(packet); break;
		case 0x03: { // Remove player guild info
			Player *player = PlayerDataProvider::Instance()->getPlayer(packet.get<int32_t>());
			if (player == 0) 
				return;

			player->setGuildId(0);
			player->setGuildRank(5);
			break;
		}
		case 0x04: { // Add guild to player
			Player *player = PlayerDataProvider::Instance()->getPlayer(packet.get<int32_t>());
			if (player == 0) 
				return;

			player->setGuildId(packet.get<int32_t>());
			player->setGuildRank(packet.get<uint8_t>());
			player->setAllianceId(packet.get<int32_t>());
			player->setAllianceRank(packet.get<uint8_t>());
			break;
		}
		case 0x05: { // Update player guild and alliance rank
			Player *player = PlayerDataProvider::Instance()->getPlayer(packet.get<int32_t>());
			if (player == 0) 
				return;

			player->setGuildRank(packet.get<uint8_t>());
			player->setAllianceRank(packet.get<uint8_t>());
			break;
		}
		case 0x06: { // Remove/add money from/to the player
			Player *player = PlayerDataProvider::Instance()->getPlayer(packet.get<int32_t>());
			if (player == 0) 
				return;

			Quests::giveMesos(player, packet.get<int32_t>());
			break;
		}
		case 0x07: { // Change guild capacity
			Guild *gi = PlayerDataProvider::Instance()->getGuild(packet.get<int32_t>());
			if (gi == 0) 
				return;
			gi->capacity = packet.get<int32_t>();
			break;
		}
		case 0x08: GuildPacket::handleNameChange(packet); break;
		case 0x09: PlayerDataProvider::Instance()->loadGuild(packet.get<int32_t>()); break;
		case 0x0a: {
			int32_t guilds = packet.get<int32_t>();
			int32_t id, capacity, alliance;
			string name;
			GuildLogo logo;
			for (int32_t i = 0; i < guilds; i++) {
				id = packet.get<int32_t>();
				name = packet.getString();
				logo.logo = packet.get<int16_t>();
				logo.color = packet.get<uint8_t>();
				logo.background = packet.get<int16_t>();
				logo.backgroundColor = packet.get<uint8_t>();
				capacity = packet.get<int32_t>();
				alliance = packet.get<int32_t>();

				PlayerDataProvider::Instance()->addGuild(id, name, logo, capacity, alliance);
				break;
			}
		}
	}
}

void SyncHandler::alliancePacketHandlerWorld(PacketReader &packet) {
	switch (packet.get<int8_t>()) {
		case 0x01: { // (un)Load Alliance and set the allianceids and ranks
			uint8_t option = packet.get<uint8_t>();
			int32_t allianceid = packet.get<int32_t>();
			if (option == 0)
				PlayerDataProvider::Instance()->unloadAlliance(allianceid);
			else
				PlayerDataProvider::Instance()->loadAlliance(allianceid);
			uint8_t guilds = packet.get<uint8_t>();
			for (uint8_t i = 0; i < guilds; i++) {
				Guild *guild = PlayerDataProvider::Instance()->getGuild(packet.get<int32_t>());
				guild->allianceid = allianceid;

				int32_t players = packet.get<int32_t>();
				for (int32_t j = 0; j < players; j++) {
					int32_t playerid = packet.get<int32_t>();
					uint8_t rank = packet.get<uint8_t>();
					if (Player *player = PlayerDataProvider::Instance()->getPlayer(playerid)) {
						player->setAllianceId(allianceid);
						player->setAllianceRank(rank);
					}
				}
			}
			break;
		}
		case 0x02: { // Changing the Alliance Leader
			int32_t allianceid = packet.get<int32_t>();
			Player *to = PlayerDataProvider::Instance()->getPlayer(packet.get<int32_t>());
			Player *from = PlayerDataProvider::Instance()->getPlayer(packet.get<int32_t>());
			if ((to != 0 && to->getAllianceId() != allianceid) || (from != 0 && from->getAllianceId() != allianceid)) 
				return;
			if (to != 0) 
				to->setAllianceRank(2);
			if (from != 0) 
				from->setAllianceRank(1);
			break;
		}
		case 0x03: { // Changing the Alliance Capacity
			if (Alliance *alliance = PlayerDataProvider::Instance()->getAlliance(packet.get<int32_t>())) 
				alliance->capacity = packet.get<int32_t>();
			break;
		}
		case 0x04: { // Changing the alliance id and rank
			int32_t allianceid = packet.get<int32_t>();
			Guild *guild = PlayerDataProvider::Instance()->getGuild(packet.get<int32_t>());
			guild->allianceid = allianceid;
			int32_t players = packet.get<int32_t>();
			for (int32_t i_2 = 0; i_2 < players; i_2++) {
				if (Player *player = PlayerDataProvider::Instance()->getPlayer(packet.get<int32_t>())) {
					player->setAllianceId(allianceid);
					player->setAllianceRank(packet.get<uint8_t>());
				}
			}
			break;
		}
		case 0x05: { // Changing the rank of someone
			int32_t allianceid = packet.get<int32_t>();
			Player *victim = PlayerDataProvider::Instance()->getPlayer(packet.get<int32_t>());
			if (victim != 0 || victim->getAllianceId() != allianceid || victim->getGuildId() == 0) 
				return;
			victim->setAllianceRank(packet.get<uint8_t>());
			break;
		}
		case 0x06: { // Channel Server Alliance Data Packet
			int32_t alliances = packet.get<int32_t>();
			int32_t id, capacity;
			string name;
			for (int32_t i = 0; i < alliances; i++) {
				id = packet.get<int32_t>();
				name = packet.getString();
				capacity = packet.get<int32_t>();
				PlayerDataProvider::Instance()->addAlliance(id, name, capacity);
			}
			break;
		}
	}
}

void SyncHandler::handleDataSync(PacketReader &packet) {
	int8_t type = packet.get<int8_t>();
	switch (type) {
		case Sync::ChannelStart:
			handleChannelStart(packet);
			break;
		case Sync::Party::Disband:
			disbandParty(packet);
			break;
		case Sync::Party::Create: {
			int32_t partyid = packet.get<int32_t>();
			int32_t leaderid = packet.get<int32_t>();
			Party *party = new Party(partyid);
			Player *leader = PlayerDataProvider::Instance()->getPlayer(leaderid);
			if (leader == 0) {
				party->addMember(leaderid);
			}
			else {
				party->addMember(leader);
			}
			party->setLeader(leaderid, true);
			PlayerDataProvider::Instance()->addParty(party);
			break;
		}
		case Sync::Party::SwitchLeader: {
			int32_t partyid = packet.get<int32_t>();
			if (Party *party = PlayerDataProvider::Instance()->getParty(partyid)) {
				int32_t newleader = packet.get<int32_t>();
				party->setLeader(newleader);
			}
			break;
		}
		case Sync::Party::RemoveMember: {
			int32_t partyid = packet.get<int32_t>();
			if (Party *party = PlayerDataProvider::Instance()->getParty(partyid)) {
				int32_t playerid = packet.get<int32_t>();
				Player *member = PlayerDataProvider::Instance()->getPlayer(playerid);
				if (member == 0) {
					party->deleteMember(playerid);
				}
				else {
					party->deleteMember(member);
				}
			}
			break;
		}
		case Sync::Party::AddMember: {
			int32_t partyid = packet.get<int32_t>();
			if (Party *party = PlayerDataProvider::Instance()->getParty(partyid)) {
				int32_t playerid = packet.get<int32_t>();
				Player *member = PlayerDataProvider::Instance()->getPlayer(playerid);
				if (member == 0) {
					party->addMember(playerid);
				}
				else {
					party->addMember(member);
				}
			}
			break;
		}
	}
}

void SyncHandler::handleChannelStart(PacketReader &packet) {
	int32_t numberparties = packet.get<int32_t>();
	for (int32_t i = 0; i < numberparties; i++) {
		int32_t partyid = packet.get<int32_t>();
		int8_t membersnum = packet.get<int8_t>();
		Party *party = new Party(partyid);
		for (int8_t j = 0; j < membersnum; j++) {
			int32_t memberid = packet.get<int32_t>();
			party->addMember(memberid);
		}
		int32_t leaderid = packet.get<int32_t>();
		party->setLeader(leaderid, true);
		PlayerDataProvider::Instance()->addParty(party);
	}
}

void SyncHandler::disbandParty(PacketReader &packet) {
	int32_t partyid = packet.get<int32_t>();
	if (Party *party = PlayerDataProvider::Instance()->getParty(partyid)) {
		party->disband();
		PlayerDataProvider::Instance()->removeParty(party->getId());
	}
}