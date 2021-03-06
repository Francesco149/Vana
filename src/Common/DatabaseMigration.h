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
#ifndef DATABASEMIGRATION_H
#define DATABASEMIGRATION_H

#include "Types.h"
#include <map>
#include <string>

using std::map;
using std::string;

class DatabaseMigration {
public:
	DatabaseMigration(bool update);
	bool checkVersion();
	void update();
	void update(size_t version);

	class Runner;
private:
	void loadDatabaseInfo();
	void loadSQLFiles();
	static void createInfoTable();
	static void updateInfoTable(size_t version);

	typedef map<int32_t, string> SQLFiles;

	size_t m_version;
	size_t m_sql_version; // Version of the .sql files
	bool m_update;
	SQLFiles m_sql_files;
};

#endif