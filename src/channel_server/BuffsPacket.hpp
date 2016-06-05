/*
Copyright (C) 2008-2016 Vana Development Team

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
#pragma once

#include "common/PacketBuilder.hpp"
#include "common/SkillConstants.hpp"
#include "common/SplitPacketBuilder.hpp"
#include "common/Types.hpp"

namespace vana {
	namespace channel_server {
		class player;
		struct buff_packet_values;

		namespace packets {
			SPLIT_PACKET(add_buff, game_player_id player_id, int32_t buff_id, const seconds &time, const buff_packet_values &buff, int16_t added_info);
			SPLIT_PACKET(end_buff, game_player_id player_id, const buff_packet_values &buff);
		}
	}
}