#pragma once
#include <mongocxx/client.hpp>
#include <mongocxx/database.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/instance.hpp>
#include <bsoncxx/json.hpp>

namespace Database
{
	mongocxx::client Connect();
	mongocxx::database GetDatabase(const mongocxx::client& Connection, const std::string_view& DatabaseName);
	mongocxx::collection GetCollection(const mongocxx::database& Database, const std::string_view& CollectionName);
}