#include "Database.hpp"
#include "Uri.hpp"

namespace Database
{
	mongocxx::client Connection;
	mongocxx::database Db;
	mongocxx::collection Users;
}

mongocxx::client Database::Connect()
{
	// Mongo instance
	const mongocxx::instance Instance;

	// Connect to the URI
	mongocxx::client Connection{ mongocxx::uri{ DATABASE_URI } };
	return Connection;
}

mongocxx::database Database::GetDatabase(const std::string_view& DatabaseName)
{
	return Connection[DatabaseName];
}

mongocxx::collection Database::GetCollection(const mongocxx::database& Database, const std::string_view& CollectionName)
{
	return Database[CollectionName];
}

Network::LoginStatusIds Database::VerifyLogin(const std::string& Username, const std::string& Password, const json& Hwid)
{
	mongocxx::cursor UsersCursor = Users.find({});

	// Iterate the users collection
	for (const auto& Document : UsersCursor)
	{
		try
		{
			// Parse the document into a json object
			const json Json = json::parse(bsoncxx::to_json(Document));

			// Get the username
			const std::string DbUsername = static_cast<std::string>(Json["username"]);

			if (DbUsername != Username)
			{
				continue;
			}

			// Get the password
			const std::string DbPassword = static_cast<std::string>(Json["password"]);

			if (DbPassword != Password)
			{
				return Network::LoginStatusIds::WrongCredentials;
			}

			// Get the hwid
			const std::string DbHwid = static_cast<std::string>(Json["hwid"]);

			// Set the hwid if it's not set in the db
			if (DbHwid.empty())
			{
				SetFieldValue<std::string>(Users, Document, "hwid", Hwid.dump());
				return Network::LoginStatusIds::Success;
			}

			// Get the hwid as a json
			const json DbHwidJson = json::parse(DbHwid);

			// If the hwid is set in the db we will check for mismatches
			if (DbHwidJson != Hwid)
			{
				return Network::LoginStatusIds::WrongHwid;
			}

			return Network::LoginStatusIds::Success;
		}
		catch (std::exception& Ex)
		{
			std::cerr << "[" << Username << "] Exception:" << Ex.what() << '\n';
		}
	}

	return Network::LoginStatusIds::WrongCredentials;
}