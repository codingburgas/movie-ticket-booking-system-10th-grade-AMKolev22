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
                   fs::create_directory("data/");
                    std::ofstream file("data/session.json");
                    SMTP::sendCode(form.getFieldValue("Email"));
                    file << data;

                }
            }

    }

    delete cli;
}
