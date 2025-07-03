#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <future>
#include <cli.hpp>
#include <menus.hpp>
#include <fstream>
#include <typeinfo>
#include <filesystem>

#include <models/Booking.h>
#include <models/Cinema.h>
#include <models/City.h>
#include <models/Hall.h>
#include <models/Show.hpp>

#ifdef _WIN32
    #include <Windows.h>
#endif
#include <../../vendor/nlohmann/json.hpp>
#include <../../vendor/nlohmann/json_fwd.hpp>
using namespace Eccc::Core;
using Eccc::Core::Database;
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
    std::string age;
    std::string name;
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

            if (!fs::exists("data/session.json")) {
                int choice = cli->display(menus["login"]);

                CliUtils::clearScreen();
                std::cout << "You selected: " << menus["login"][choice] << "\n";

                if (choice == 0) {
                    std::cout << "Login";
                    CliUtils::clearScreen();
                    form.addField("Email");
                    nlohmann::json data;

                    if (!form.run()) {
                        std::cout << "Email input cancelled.\n";
                        return 1;
                    }

                    email = form.getFieldValue("Email");
                    data["email"] = email;

                    try {
                        soci::session* sql = db->getSession();
                        std::string name;
                        *sql << "SELECT name FROM public.\"User\" WHERE email = :email",
                            soci::into(name),
                            soci::use(email);

                        data["name"] = name;
                    }
                    catch (const std::exception& e) {
                        std::cerr << "Error retrieving user name: " << e.what() << "\n";
                        return 1;
                    }

                    SMTP::sendCode(email);
                    form.clearFields();

                    std::cout << std::format("A code was sent to {}\n", email);
                    form.addField("Code");

                    if (!form.run()) {
                        std::cout << "Code input cancelled.\n";
                        return 1;
                    }

                    std::string enteredCodeStr = form.getFieldValue("Code");
                    int enteredCode = 0;
                    try {
                        enteredCode = std::stoi(enteredCodeStr);
                    }
                    catch (...) {
                        std::cout << "Invalid code format or code\n";
                        return 1;
                    }

                    try {
                        int storedCode = 0;
                        soci::session* sql = db->getSession();
                        *sql << "SELECT code FROM \"TwoFA\" WHERE \"userEmail\" = :email",
                            soci::into(storedCode),
                            soci::use(email);

                        if (storedCode == enteredCode) {
                            int isAdmin = 1;
                            std::cout << std::format("Login successful! Welcome,  {}\n", email);
                            fs::create_directory("data/");

                            std::cout << "Writing session data: " << data.dump(4) << "\n";

                            std::ofstream file("data/session.json");
                            if (!file) {
                                std::cerr << "Failed to open session.json for writing.\n";
                                return 1;
                            }
                            file << data;

                            *sql << "DELETE FROM \"TwoFA\" WHERE \"userEmail\" = :email",
                                soci::use(email);

                            CliUtils::clearScreen();
                            CliUtils::displayMenu(bool(isAdmin), *sql);
                        }
                        else {
                            std::cout << "Code does not match.\n";
                        }
                    }
                    catch (const std::exception& e) {
                        std::cerr << "DB error: " << e.what() << "\n";
                    }

                    CliUtils::clearScreen();
                }


                else {

                    std::cout << "Register";
                    CliUtils::clearScreen();
                    form.addField("Name");
                    form.addField("Age");
                    form.addField("Email");
                    nlohmann::json data;
                    if (form.run()) {
                        data["name"] = form.getFieldValue("Name");
                        data["age"] = form.getFieldValue("Age");
                        data["email"] = form.getFieldValue("Email");
                        email = form.getFieldValue("Email");
                        name = form.getFieldValue("Name");
                        age = std::stoi(form.getFieldValue("Age"));
                        SMTP::sendCode(form.getFieldValue("Email"));
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
                            soci::session* sql = db->getSession();
                           *sql << "SELECT code FROM \"TwoFA\" WHERE \"userEmail\" = :email",
                                soci::into(storedCode),
                                soci::use(email);

                            if (storedCode == enteredCode) {
                                int isAdmin = 1;
                                std::cout << "Code verified successfully!\n";
                                fs::create_directory("data/");
                                std::ofstream file("data/session.json");
                                file << data;
                                *sql << "DELETE FROM \"TwoFA\" WHERE \"userEmail\" = :email",
                                    soci::use(email);

                                *sql << "INSERT INTO public.\"User\" (\"name\", \"isAdmin\", \"email\") VALUES (:name, :isAdmin, :email)",
                                    soci::use(name),
                                    soci::use(1),
                                    soci::use(email);

                                CliUtils::clearScreen();

                                CliUtils::displayMenu(bool(isAdmin), *sql);
        
                            }
                            else
                                std::cout << "Code does not match.\n";
                        }
                        catch (const std::exception& e) {
                            std::cerr << "DB error: " << e.what() << "\n";
                        }
                    }
                    CliUtils::clearScreen();

                }
            }

       else {

        try {
            auto sql = dbHandle.value()->getSession();
            int isAdmin = 1;
            //std::cout << typeid(sql).name();
            std::cout << email;
            std::ifstream file("data/session.json");
            nlohmann::json data;
            if (file.is_open()) {
                file >> data;
                
            }
            std::cout << data["email"];
            std::string name = data["name"];

            std::cout << "NAME: " << name;

            (*sql) << "SELECT \"isAdmin\" FROM public.\"User\" WHERE \"email\" = :email",
            soci::into(isAdmin),
            soci::use<std::string>(email);

            CliUtils::clearScreen();
            CliUtils::displayMenu(bool(isAdmin), *sql, name);
            std::string command;
            while (true) {
              std::getline(std::cin, command);
              std::transform(command.begin(), command.end(), command.begin(), ::tolower);
              if (command == "help")
                  CliUtils::displayMenu(bool(isAdmin), *sql, name);
            }


            }

            catch (const std::exception& e) {
                std::cerr << "Error checking admin status: " << e.what() << "\n";
            }
        }

    delete cli;
}
