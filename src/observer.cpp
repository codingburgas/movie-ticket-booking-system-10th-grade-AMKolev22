#include "observer.hpp"

void NotificationSystem::addObserver(std::shared_ptr<Observer> observer) {
	this->observerSemaphore.acquire();
	this->observers.push_back(observer);
	this->observerSemaphore.release();
}

void NotificationSystem::removeObserver(std::shared_ptr<Observer> observer) {
	this->observerSemaphore.acquire();
	this->observers.erase(std::remove(this->observers.begin(), this->observers.end(), observer), this->observers.end());
	this->observerSemaphore.release();
}


void NotificationSystem::notify(const Notification& notification) {
    this->observerSemaphore.acquire();
    auto observersCopy = this->observers;
    this->observerSemaphore.release();

    for (const auto& observer : observersCopy) {
        observer->update(notification);
    }
}