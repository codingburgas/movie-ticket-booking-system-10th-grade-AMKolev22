#pragma once
#include <limits>
#include <string>
#include <vector>
#include <iostream>
#include <ranges>
#include <movies.h>
#include "shows.h"
#include <database.hpp>
#include "booking.h"
#include "admin.h"

#include "browse.h"
#ifdef _WIN32
	#include <conio.h>
    #include <windows.h>
#endif
namespace CliUtils {
	void clearScreen() {
		std::cout << "\x1b[2J\x1b[H";
	}

	void hideCursor() {
		std::cout << "\x1b[?25l";
	}

	void showCursor() {
		std::cout << "\x1b[?25h";
	}

    char getInput() {
#ifdef _WIN32
        return _getch();
#endif
    }

    void displayMenu(bool isAdmin, soci::session& sql, std::string name = "") {
        std::vector<std::string> userCommands = {
        "Search Movies", "Search Shows", "Book a Ticket", "View Bookings", "Cancel Booking", "Help"
        };
        std::vector<std::string> adminCommands = {
            "Add Movie", "Add Show", " Statistics", "Add Cinema", "Add Halls", "Add Seats", "Add City", "Browse Cinemas", "Cinema Details"
        };

        std::vector<std::string> allCommands = userCommands;
        if (isAdmin) {
            allCommands.insert(allCommands.end(), adminCommands.begin(), adminCommands.end());
        }

        const int columns = 3;
        int selected = 0;
        int total = allCommands.size();

        while (true) {
            clearScreen();
            std::cout << "=== Movie Ticket Booking System ===\n";
            std::cout << "Welcome back, " << name << "\n";
            std::cout << "Use arrow keys to navigate. Press Enter to select. Q to quit.\n\n";

            for (int i = 0; i < total; ++i) {
                if (i % columns == 0) std::cout << "\n";
                if (i == selected)
                    std::cout << " > [" << allCommands[i] << "] ";
                else
                    std::cout << "   " << allCommands[i] << "   ";
            }

            int key = _getch();
            if (key == 224) {
                key = _getch();
                if (key == 72 && selected - columns >= 0) selected -= columns;       
                else if (key == 80 && selected + columns < total) selected += columns; 
                else if (key == 75 && selected > 0) selected--;                        
                else if (key == 77 && selected < total - 1) selected++;                
            }
            else if (key == '\r') {
                system("cls");
                std::cout << "You selected: " << allCommands[selected] << "\n";
                
                if (selected == 0 ) {
                    searchMovies(sql);
                }
                else if (selected == 1) {
                    viewShows(sql);
                }
                else if (selected == 2){
                    bookTickets(sql);
                }
                else if (selected == 3) {
                    viewBookings(sql);
                }
                else if (selected == 6) {
                    addMovie(sql, isAdmin = true);
                }
                else if (selected == 7) {
                    addShow(sql, isAdmin = true);
                }
                else if (selected == 8) {
                    viewStatistics(sql, isAdmin = true);
                }
                else if (selected == 9){
                    addCinema(sql, isAdmin = true);
                }
                else if (selected == 10) {
                    int cinemaId;
                    std::cout << "Please enter cinema id: ";
                    std::cin >> cinemaId;
                    addHallsToCinema(sql, cinemaId, isAdmin = true);

                }
                else if (selected == 11) {
                    std::cout << "Please enter hall id, rows and seats per row!";
                    int hallId, rows, perRow;
                    std::cin >> hallId >> rows >> perRow;
                    addSeatsToHall(sql,hallId, rows, perRow);
                }
                else if (selected == 12) {
                    addCity(sql, isAdmin = true);
                }
                else if (selected == 13) {
                    browseCinemas(sql);
                }
                else if (selected == 14) {
                    std::cout << "Enter cinema id";
                    int id;
                    std::cin >> id;
                    viewCinemaDetails(sql, id);
                }

                
            }
            else if (key == 'q' || key == 'Q') {
                std::cout << "\n Exiting menu...\n";
                break;
            }
        }
    }

}
class CliGeneral {
	public:
		virtual int display(std::vector<std::string>& options) = 0;
};

class CliMenu : public CliGeneral {
public:
	int display(std::vector<std::string>& options) override {
            int selected = 0;
            int input;

            CliUtils::hideCursor();
            while (true) {
                CliUtils::clearScreen();
                std::cout << "\x1b[1;36m=== Login Menu ===\x1b[0m\n\n";
                for (size_t i = 0; i < options.size(); ++i) {
                    if ((int)i == selected) {
                        std::cout << "\x1b[1;42m> " << options[i] << " \x1b[0m\n";
                    }
                    else {
                        std::cout << "  " << options[i] << "\n";
                    }
                }

                input = CliUtils::getInput();

#ifdef _WIN32
                if (input == -32) {
                    input = CliUtils::getInput();
                    if (input == 72 && selected > 0) selected--;
                    else if (input == 80 && selected < options.size() - 1) selected++;
                }
                else if (input == '\r') {
                    break;
                }
#endif
            }
            CliUtils::showCursor();
            return selected;
        }
};



class CliForm {
private:
    struct Field {
        std::string label;
        std::string value;
    };

    std::vector<Field> fields;
    int selected = 0;
    HANDLE hInput;
    DWORD prevMode;

public:
    CliForm() {
        hInput = GetStdHandle(STD_INPUT_HANDLE);
        GetConsoleMode(hInput, &prevMode);
        SetConsoleMode(hInput, ENABLE_WINDOW_INPUT);
    }

    ~CliForm() {
        SetConsoleMode(hInput, prevMode); // restore console mode
    }

    void addField(const std::string& label) {
        fields.push_back({ label, "" });
    }

    void clearFields() {
        fields.clear();
    }

    bool run() {
        fields.push_back({ "[CONTINUE]", "" });

        INPUT_RECORD inputRecord;
        DWORD events;

        while (true) {
            draw();
            ReadConsoleInput(hInput, &inputRecord, 1, &events);

            if (inputRecord.EventType == KEY_EVENT && inputRecord.Event.KeyEvent.bKeyDown) {
                int key = inputRecord.Event.KeyEvent.wVirtualKeyCode;

                if (key == VK_UP) {
                    selected = (selected - 1 + fields.size()) % fields.size();
                }
                else if (key == VK_DOWN || key == VK_TAB) {
                    selected = (selected + 1) % fields.size();
                }
                else if (key == VK_RETURN) {
                    if (selected == fields.size() - 1) {
                        this->submitted = true;
                        return true;
                    }
                    else {
                        editField(selected);
                    }
                }
                else if (key == VK_ESCAPE) {
                    this->submitted = false;
                    return false;
                }
            }
        }
    }

    std::string getFieldValue(const std::string& label) const {
        for (const auto& field : fields) {
            if (field.label == label)
                return field.value;
        }
        return "";
    }

private:
    bool submitted = false;
    void draw() {
        system("cls");
        std::cout << "===== AUTH FORM =====\n\n";

        for (int i = 0; i < fields.size(); ++i) {
            if (i == selected) SetColor(240);
            std::cout << fields[i].label << ": " << fields[i].value << "\n";
            if (i == selected) SetColor(7);
        }

    }

    void editField(int index) {
        int y = 2 + index; 

        SetCursorPosition(0, y);

        std::cout << std::string(80, ' ');

        SetCursorPosition(0, y);
        std::cout << fields[index].label << ": ";
        SetColor(7);

        std::string input;
        std::getline(std::cin, input);
        fields[index].value = input;

        SetCursorPosition(0, y);
        std::cout << fields[index].label << ": " << fields[index].value;
        SetColor(7);
    }


    void submit() {
        system("cls");
        std::cout << "===== SUBMITTED DATA =====\n\n";
        for (int i = 0; i < fields.size() - 1; ++i) {
            std::cout << fields[i].label << ": " << fields[i].value << "\n";
        }
        std::cout << "\nPress any key to exit...";
        system("pause >nul");
    }

    void SetCursorPosition(int x, int y) {
        COORD coord = { (SHORT)x, (SHORT)y };
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    }

    void SetColor(WORD color) {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
    }
};