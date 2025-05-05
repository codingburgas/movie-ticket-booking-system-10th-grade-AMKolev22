#define _CRT_SECURE_NO_WARNINGS
#pragma once

#include "pch.h"
#include "states.h"
#include "config.h"
/*
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
*/
// OBSERVER PATTERN

    class Observer {
    public:
        virtual void update(const Notification& notification) = 0;
        virtual ~Observer() = default;
    };


    // BASE CLASS USED FOR TYPE OF OBSERVING TYPE

        class Notification {
        private:
            NotificationType type;
            std::string message;
            std::string timestamp;

        public:
            Notification(NotificationType type, std::string message)
                : type(type), message(message) {
                auto now = std::chrono::system_clock::now();
                auto now_time = std::chrono::system_clock::to_time_t(now);
                timestamp = std::ctime(&now_time);
            }

            NotificationType getType() const { return type; }
            std::string getMessage() const { return message; }
            std::string getTimestamp() const { return timestamp; }
        };



        class NotificationSystem {
        private:
            std::vector<std::shared_ptr<Observer>> observers;
            std::binary_semaphore observerSemaphore{ 1 };

        public:
            void addObserver(std::shared_ptr<Observer> observer) {
                this->observerSemaphore.acquire();
                this->observers.push_back(observer);
                this->observerSemaphore.release();
            }

            void removeObserver(std::shared_ptr<Observer> observer) {
                this->observerSemaphore.acquire();
                this->observers.erase(std::remove(this->observers.begin(), this->observers.end(), observer), this->observers.end());
                this->observerSemaphore.release();
            }


            void notify(const Notification& notification) {
                observerSemaphore.acquire();
                auto observersCopy = observers;
                observerSemaphore.release();

                for (const auto& observer : observersCopy) {
                    observer->update(notification);
                }
            }
        };
