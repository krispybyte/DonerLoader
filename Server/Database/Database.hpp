#pragma once
#include <mongocxx/client.hpp>
#include <mongocxx/database.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/instance.hpp>
#include <bsoncxx/json.hpp>

namespace Database
{
	inline mongocxx::client Connection;
	inline mongocxx::database Db;
	inline mongocxx::collection Users;

	mongocxx::client Connect();
	mongocxx::database GetDatabase(const std::string_view& DatabaseName);
	mongocxx::collection GetCollection(const mongocxx::database& Database, const std::string_view& CollectionName);
	bool VerifyLogin(const std::string& Username, const std::string& Password);
}