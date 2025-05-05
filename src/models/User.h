#pragma once
#include "../observer.hpp"
#include <optional>

class User : public Observer {
private:
    std::string name;

public:
    User(std::string name)
        : name(name) {
    }

    std::string getName() const { return name; }
    void setName(std::string val) { this->name = val; };

    // TEST UPDATE FUNCTION 
    void update(const Notification& notification) override {
        std::cout << "User " << name << " received notification: "
            << notification.getMessage() << " at " << notification.getTimestamp() << std::endl;
    }
};