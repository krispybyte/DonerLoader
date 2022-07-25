#include "Database.hpp"
#include "Uri.hpp"
#include "../Cryptography/AES256.hpp"
#include "../Cryptography/Hex.hpp"

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

Network::LoginStatusIds Database::Login(const std::string& Username, const std::string& Password, const json& Hwid)
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

std::string Database::FindRememberMe(const std::string& Username)
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

			const std::string DbRememberMe = static_cast<std::string>(Json["remember_me"]);

			// If remember me isn't set, we'll
			// a new one and return it
			if (DbRememberMe.empty())
			{
				return SetNewRememberMe(Document);
			}

			// If remember me is set, well
			// return the existing value
			return DbRememberMe;
		}
		catch (std::exception& Ex)
		{
			std::cerr << "[" << Username << "] Exception:" << Ex.what() << '\n';
		}
	}
}

std::string Database::RememberMe(const bsoncxx::v_noabi::document::view& Document)
{
	// Unfinished
	return {};
}

std::string Database::SetNewRememberMe(const bsoncxx::v_noabi::document::view& Document)
{
	// Unfinished
	return {};
}