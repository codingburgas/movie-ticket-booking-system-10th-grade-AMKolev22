#pragma once
#include <future>
#include <random>
#include <database.hpp>
#include <../../vendor/cpr/include/cpr/cpr.h>
#include <curl/curl.h>
using NoReturn = std::expected<void, std::string>;

struct upload_status {
	int lines_read = 0;
	std::string code;
	std::string recipient_email;
};

static size_t payload_source(void* ptr, size_t size, size_t nmemb, void* userp) {
	upload_status* upload_ctx = (upload_status*)userp;
	if ((size == 0) || (nmemb == 0) || ((size * nmemb) < 1))
		return 0;

	// Static array of headers and body lines, last element nullptr
	static std::vector<std::string> lines;
	if (upload_ctx->lines_read == 0) {
		// Compose the email text only once
		lines.clear();
		lines.push_back("Date: Mon, 29 Jun 2025 21:54:29 +0000\r\n");
		lines.push_back(std::format("To: <{}>\r\n", upload_ctx->recipient_email));
		lines.push_back("From: <amkolev22@codingburgas.bg>\r\n");
		lines.push_back("Subject: Verification code\r\n");
		lines.push_back("\r\n");  // blank line between headers and body
		lines.push_back(std::format("Your verification code is: {}\r\n", upload_ctx->code));
		
	}

	if (upload_ctx->lines_read >= (int)lines.size() || lines[upload_ctx->lines_read].empty())
		return 0;

	const std::string& data = lines[upload_ctx->lines_read];
	size_t len = data.size();
	memcpy(ptr, data.c_str(), len);
	upload_ctx->lines_read++;
	return len;
}

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

        // Generate 5-digit zero-padded code
        int codeInt = distCode(codeGen);
        std::string codeStr = std::format("{:05}", codeInt);

        CURL* curl = curl_easy_init();
        if (!curl) {
            return std::unexpected("Failed to init curl");
        }

        upload_status uploadCtx;
        uploadCtx.code = codeStr;
        uploadCtx.recipient_email = email;

        curl_easy_setopt(curl, CURLOPT_URL, "smtp://mail.smtp2go.com:2525");
        curl_easy_setopt(curl, CURLOPT_USERNAME, "admin_movie");
        curl_easy_setopt(curl, CURLOPT_PASSWORD, "WacALj9l1CbKN0CH");

        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);  // Enable STARTTLS

        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, "<amkolev22@codingburgas.bg>");

        struct curl_slist* recipients = nullptr;
        recipients = curl_slist_append(recipients, ("<" + email + ">").c_str());
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

        curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
        curl_easy_setopt(curl, CURLOPT_READDATA, &uploadCtx);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        CURLcode res = curl_easy_perform(curl);

        curl_slist_free_all(recipients);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            return std::unexpected(std::string("curl_easy_perform() failed: ") + curl_easy_strerror(res));
        }

        return {};
    }
}