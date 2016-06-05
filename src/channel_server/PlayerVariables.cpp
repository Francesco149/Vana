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
#include "PlayerVariables.hpp"
#include "common/Database.hpp"
#include "channel_server/Player.hpp"

namespace vana {
namespace channel_server {

player_variables::player_variables(player *player) :
	m_player{player}
{
	load();
}

auto player_variables::save() -> void {
	auto &db = database::get_char_db();
	auto &sql = db.get_session();
	game_player_id char_id = m_player->get_id();

	sql.once << "DELETE FROM " << db.make_table("character_variables") << " WHERE character_id = :char",
		soci::use(char_id, "char");

	if (m_variables.size() > 0) {
		string key = "";
		string value = "";

		soci::statement st = (sql.prepare
			<< "INSERT INTO " << db.make_table("character_variables") << " "
			<< "VALUES (:char, :key, :value)",
			soci::use(char_id, "char"),
			soci::use(key, "key"),
			soci::use(value, "value"));

		for (const auto &kvp : m_variables) {
			key = kvp.first;
			value = kvp.second;
			st.execute(true);
		}
	}
}

auto player_variables::load() -> void {
	auto &db = database::get_char_db();
	auto &sql = db.get_session();
	soci::rowset<> rs = (sql.prepare << "SELECT * FROM " << db.make_table("character_variables") << " WHERE character_id = :char",
		soci::use(m_player->get_id(), "char"));

	for (const auto &row : rs) {
		m_variables[row.get<string>("key")] = row.get<string>("value");
	}
}

}
}