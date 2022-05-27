#include "Common.hpp"
#include "Utilities/Utilities.hpp"
#include "Database/Database.hpp"

int main()
{
	SetConsoleTitleA("Server");

	// Establish a connection to the database
	const mongocxx::client Connection = Database::Connect();

	// Select a database
	const mongocxx::database Database = Database::GetDatabase(Connection, "test");

	// Get a collection from the atabase
	mongocxx::collection Users = Database::GetCollection(Database, "users");

	// Iterate the collection
	mongocxx::cursor UsersCursor = Users.find({});
	for (const auto& Document : UsersCursor)
	{
		// Parse the document into a json object
		const json JsonDocument = json::parse(bsoncxx::to_json(Document));

		// Get the username
		const std::string Username = static_cast<std::string>(JsonDocument["username"]);
		std::cout << "[+] Username (test): " << Username.c_str() << std::endl;
	}

	// Load all modules
	if (!Modules::Load(Network::ModuleIds::Test8MB, "Modules/Test8MB.dll") ||
		!Modules::Load(Network::ModuleIds::Test1KB, "Modules/Test1KB.dll"))
	{
		std::cout << "[!] Failed loading a module." << '\n';
		return std::getchar();
	}

	// Create server
	Network::Create(NETWORK_PORT_INT);

	return std::getchar();
}