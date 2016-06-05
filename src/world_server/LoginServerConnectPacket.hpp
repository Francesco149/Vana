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

#include "common/external_ip.hpp"
#include "common/ip.hpp"
#include "common/packet_builder.hpp"
#include "common/types.hpp"
#include <vector>

namespace vana {
	namespace world_server {
		namespace packets {
			PACKET(register_channel, game_channel_id channel, const ip &channel_ip, const ip_matrix &ext_ip, connection_port port);
			PACKET(update_channel_pop, game_channel_id channel, int32_t population);
			PACKET(remove_channel, game_channel_id channel);
		}
	}
}