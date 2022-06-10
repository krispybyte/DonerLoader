#pragma once
#include <Shared.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/database.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/instance.hpp>
#include <bsoncxx/json.hpp>
using namespace bsoncxx::builder::basic;
#include <json/single_include/nlohmann/json.hpp>
using namespace nlohmann;

namespace Database
{
	extern mongocxx::client Connection;
	extern mongocxx::database Db;
	extern mongocxx::collection Users;

	mongocxx::client Connect();
	mongocxx::database GetDatabase(const std::string_view& DatabaseName);
	mongocxx::collection GetCollection(const mongocxx::database& Database, const std::string_view& CollectionName);
	template<typename T>
	void SetFieldValue(mongocxx::collection& Collection, const bsoncxx::v_noabi::document::view& Document, const std::string_view& FieldName, T NewFieldValue)
	{
		Collection.update_one(Document, make_document(kvp("$set", make_document(kvp(FieldName, NewFieldValue)))));
	}
	Network::LoginStatusIds Login(const std::string& Username, const std::string& Password, const json& Hwid);
	std::string FindRememberMe(const std::string& Username);
	std::string RememberMe(const bsoncxx::v_noabi::document::view& Document);
	extern inline std::string SetNewRememberMe(const bsoncxx::v_noabi::document::view& Document);
}