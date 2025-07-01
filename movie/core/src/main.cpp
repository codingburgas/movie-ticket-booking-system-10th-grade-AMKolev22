#include "database.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <future>
#include <cli.hpp>
#include <menus.hpp>
#include <fstream>
#include <filesystem>
#ifdef _WIN32
    #include <Windows.h>
#endif
#include <../../vendor/nlohmann/json.hpp>
#include <../../vendor/nlohmann/json_fwd.hpp>
#include <sendEmail.hpp>
using namespace Eccc::Core;
using namespace CliUtils;
namespace fs = std::filesystem;



#ifdef _WIN32
std::future<bool> enable() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
        return std::async(std::launch::async, []() -> bool {
        return false;
            });

    SetConsoleOutputCP(CP_UTF8);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);

    return std::async(std::launch::async, []() -> bool {
        return true;
        });
}
#endif


int main() {
    CliMenu* cli = new CliMenu();
    CliForm form;
    std::string email;
    if (!fs::exists("data/session.json")) {
        #ifdef _WIN32
                bool enabled = enable().get();
        #endif

            if (!enabled) {
                std::cerr << "Failed to enable terminal features.\n";
                return 1;
            }

            auto dbHandle = Database::getInstance().get();
            if (!dbHandle) {
                std::cerr << "Failed to get database instance: " << dbHandle.error() << "\n";
                return 1;
            }

            auto db = dbHandle.value();
            auto connectHandle = db->async_connectToDb();
            auto connectionResult = connectHandle.get();
            auto sql = db->getSession();

            if (!connectionResult) {
                std::cerr << "Failed to connect to database: " << connectionResult.error() << "\n";
                return 1;
            }

            std::cout << "Connected to the database\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            int choice = cli->display(menus["login"]);

            CliUtils::clearScreen();
            std::cout << "You selected: " << menus["login"][choice] << "\n";

            if (choice == 0) {
                fs::create_directory("data/");
                std::ofstream file("data/session.json");
                std::cout << "Login!";
            }
            else {
                std::cout << "Register";
                CliUtils::clearScreen();
                form.addField("Name");
                form.addField("Age");
                form.addField("Email");
                if (form.run()) {
                    nlohmann::json data;
                    data["name"] = form.getFieldValue("Name");
                    data["age"] = form.getFieldValue("Age");
                    data["email"] = form.getFieldValue("Email");
                    email = form.getFieldValue("Email");
                   fs::create_directory("data/");
                    std::ofstream file("data/session.json");
                    SMTP::sendCode(form.getFieldValue("Email"));
                    //SMTP::sendCode(form.getFieldValue("Email"));
                    file << data;
                }
                form.clearFields();
                form.addField("Code");
                if (form.run()) {
                    std::string enteredCodeStr = form.getFieldValue("Code");
                    int enteredCode = 0;
                    try {
                        enteredCode = std::stoi(enteredCodeStr);
                    }
                    catch (...) {
                        std::cout << "Invalid code format\n";
                        return 1; 
                    }

                    try {
                        int storedCode = 0;
                        *sql << "SELECT code FROM \"TwoFA\" WHERE \"userEmail\" = :email",
                            soci::into(storedCode),
                            soci::use(email); 

                        if (storedCode == enteredCode) {
                            std::cout << "Code verified successfully!\n";
                            *sql << "DELETE FROM \"TwoFA\" WHERE \"userEmail\" = :email",
                                soci::use(email);
                        }
                        else
                            std::cout << "Code does not match.\n";
                    }
                    catch (const std::exception& e) {
                        std::cerr << "DB error: " << e.what() << "\n";
                    }
                }

            }
    }
           else {

                curl_global_init(CURL_GLOBAL_DEFAULT);
                curl_version_info_data* info = curl_version_info(CURLVERSION_NOW);
                std::cout << "Supported protocols: " << info->protocols[0] << std::endl;
                for (int i = 0; info->protocols[i]; i++) {
                    std::cout << info->protocols[i] << std::endl;
                }
            }


    delete cli;
}
