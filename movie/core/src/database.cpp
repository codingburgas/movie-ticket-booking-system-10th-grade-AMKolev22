#include "database.hpp"
#include <iostream>

Eccc::Core::Database* Eccc::Core::Database::databaseInstance  = nullptr;
std::once_flag Eccc::Core::Database::database_flag;

std::future<NoReturn> Eccc::Core::Database::connectToDb() {
	try {
		sql.open(soci::postgresql, "dbname=daddy-db user=postgres password=daddy host=localhost port=5432");
		return {};
	}
	catch (const soci::soci_error& database_error) {
		return std::async(std::launch::async, [error = database_error]() -> NoReturn {
			return std::unexpected(std::string(std::format("An error connecting to the database happened: {}", error.what())));
		});
	}
	catch (const std::runtime_error& standard_excpetion) {
		return std::async(std::launch::async, [error = standard_excpetion]() -> NoReturn {
			return std::unexpected(std::string(std::format("An exception happened: {}", error.what())));
			});
	}
	catch (...) {
		return std::async(std::launch::async, []() -> NoReturn {
			return std::unexpected("An error connecting to the database happened");
		});
	}
}


Eccc::Core::Database::ASYNC_NoReturn Eccc::Core::Database::async_connectToDb() {
	try {

		sql.open(soci::postgresql, "dbname=daddy-db user=postgres password=daddy host=localhost port=5432");
		return std::async(std::launch::async, []() -> NoReturn {
			return {};
		});
	}

	catch (const soci::soci_error& database_error) {
		return std::async(std::launch::async, [error = database_error]() -> NoReturn {
			return std::unexpected(std::format("An error connecting to the database happened: {}", error.what()));
		});
	}

	catch (const std::runtime_error& standard_excpetion) {
		return std::async(std::launch::async, [error = standard_excpetion]() -> NoReturn {
			return std::unexpected(std::format("Some other expection: {}", error.what()));
		});
	}

	catch (...) {
		return std::async(std::launch::async, []() -> NoReturn {
			return std::unexpected(std::format("General error."));
		});
	}
}




