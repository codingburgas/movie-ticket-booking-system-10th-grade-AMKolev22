#pragma once
#include <future>
#include <random>
#include <database.hpp>
#include <../../vendor/cpr/include/cpr/cpr.h>
using NoReturn = std::expected<void, std::string>;



namespace SMTP {
	std::random_device dev;
	std::mt19937_64 codeGen(dev());
	std::uniform_int_distribution<std::mt19937_64::result_type> distCode(00000, 99999);
	NoReturn sendCode(std::string email) {
			auto dbHandle = Eccc::Core::Database::getInstance().get();
			if (!dbHandle.has_value())
				return std::unexpected(std::string(std::format("Error {}", dbHandle.error())));
			auto db = dbHandle.value();
			auto sql = db->getSession();
			int code = distCode(codeGen);
			cpr::Response r = cpr::Post(cpr::Url{ "https://mail.smtp2go.com:2525" },
				cpr::Authentication{ "admin_movie", "WacALj9l1CbKN0CH", cpr::AuthMode::BASIC },
				cpr::Multipart{
					{"from", "amkolev22@codingburgas.bg"},
					{"to", email},
					{"subject", "Verification code"},
					{"code", "123"}
				}
			);

			return {};
	};
}