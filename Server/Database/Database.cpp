#include "Database.hpp"
#include "Uri.hpp"

mongocxx::client Database::Connect()
{
	// Mongo instance
	const mongocxx::instance Instance;

	// Connect to the URI
	mongocxx::client Connection{ mongocxx::uri{ DATABASE_URI } };
	return Connection;
}

mongocxx::database Database::GetDatabase(const mongocxx::client& Connection, const std::string_view& DatabaseName)
{
	return Connection[DatabaseName];
}

mongocxx::collection Database::GetCollection(const mongocxx::database& Database, const std::string_view& CollectionName)
{
	return Database[CollectionName];
}