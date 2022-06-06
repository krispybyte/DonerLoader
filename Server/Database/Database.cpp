#include "Database.hpp"
#include "Uri.hpp"
#include <json/single_include/nlohmann/json.hpp>
using namespace nlohmann;

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

bool Database::VerifyLogin(const std::string& Username, const std::string& Password)
{
	mongocxx::cursor UsersCursor = Users.find({});

	// Iterate the users collection
	for (const auto& Document : UsersCursor)
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
			continue;
		}

		return true;
	}

	return false;
}