#include <future>
#include <random>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <database.hpp>
#include <../../vendor/cpr/include/cpr/cpr.h>
#include <curl/curl.h>
#include <expected>
#include <string>
#include <vector>
#include <format>

using NoReturn = std::expected<void, std::string>;



struct upload_status {
    int lines_read = 0;
    std::string code;
    std::string recipient_email;
    std::vector<std::string> lines;  // Store email content here
};

// Helper to get current date in RFC 2822 format, e.g. "Mon, 29 Jun 2025 21:54:29 +0000"
std::string getCurrentDate() {
    using namespace std::chrono;
    auto now = system_clock::now();
    auto t_c = system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t_c);
#else
    localtime_r(&t_c, &tm);
#endif
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S %z", &tm);
    return std::string(buffer);
}

static size_t payload_source(void* ptr, size_t size, size_t nmemb, void* userp) {
    upload_status* upload_ctx = (upload_status*)userp;
    if ((size == 0) || (nmemb == 0) || ((size * nmemb) < 1))
        return 0;

    if (upload_ctx->lines_read >= (int)upload_ctx->lines.size())
        return 0;

    const std::string& data = upload_ctx->lines[upload_ctx->lines_read];
    size_t len = data.size();
    memcpy(ptr, data.c_str(), len);
    upload_ctx->lines_read++;
    return len;
}

namespace SMTP {
    std::random_device dev;
    std::mt19937_64 codeGen(dev());
    std::uniform_int_distribution<std::mt19937_64::result_type> distCode(0, 99999);

    void sendCode(std::string email) {
        std::cout << email << std::endl;
        // Get DB handle (optional)
        auto dbHandle = Eccc::Core::Database::getInstance().get();
        auto db = dbHandle.value();
        auto sql = db->getSession();

        int codeInt = distCode(codeGen);
        std::string codeStr = std::format("{:05}", codeInt);

        CURL* curl = curl_easy_init();

        upload_status uploadCtx;
        uploadCtx.code = codeStr;
        uploadCtx.recipient_email = email;
        uploadCtx.lines.push_back("MIME-Version: 1.0\r\n");
        uploadCtx.lines.push_back("Content-Type: text/plain; charset=utf-8\r\n");
        uploadCtx.lines.push_back(std::format("Date: {}\r\n", getCurrentDate()));
        uploadCtx.lines.push_back(std::format("To: <{}>\r\n", uploadCtx.recipient_email));
        uploadCtx.lines.push_back("From: <amkolev22@codingburgas.bg>\r\n");
        uploadCtx.lines.push_back("Subject: Verification code\r\n");
        uploadCtx.lines.push_back("\r\n");  
        uploadCtx.lines.push_back(std::format("Your verification code is: {}\r\n", uploadCtx.code));
        uploadCtx.lines.push_back(".\r\n");
        #ifdef SKIP_PEER_VERIFICATION
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        #endif
                curl_easy_setopt(curl, CURLOPT_URL, "smtp://mail.smtp2go.com:587");
                curl_easy_setopt(curl, CURLOPT_MAIL_AUTH, "amkolev22@codingburgas.bg");
                curl_easy_setopt(curl, CURLOPT_USERNAME, "admin_movie");
                curl_easy_setopt(curl, CURLOPT_PASSWORD, "WacALj9l1CbKN0CH");
                curl_easy_setopt(curl, CURLOPT_MAIL_FROM, "<amkolev22@codingburgas.bg>");
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

                struct curl_slist* recipients = nullptr;
                std::string recipient = "<" + email + ">";
                recipients = curl_slist_append(recipients, recipient.c_str());
                curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

                curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
                curl_easy_setopt(curl, CURLOPT_READDATA, &uploadCtx);
                curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

                curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);

                CURLcode res = curl_easy_perform(curl);
                //std::cout << res;
            curl_slist_free_all(recipients);
            curl_easy_cleanup(curl);
            
            try {
                *sql << "INSERT INTO \"TwoFA\" (\"userEmail\", code) VALUES (:email, :code)", soci::use(email), soci::use(std::stoi(uploadCtx.code));
            }
            catch (soci::soci_error& e) {
                std::cout << e.what();
            }


        // TODO save code in DB, send back, etc.

        //return std::expected<void, std::string>{};
    }
}