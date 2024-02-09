#include "user_manager.hpp"

#include <iostream>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

namespace chat_app {

UserManager::UserManager(std::chrono::milliseconds user_timeout, std::chrono::milliseconds check_period)
    : user_timeout_{user_timeout},
      check_period_{check_period},
      user_activity_map_{},
      std::thread(&UserManager::RemoveInactiveUsers, this) {}

// This function runs indefinitely in a separate thread.
// It checks for each user stored in user_activity_map_, it it has reached its activity timeout,
// and removes that user from the list if so.
void UserManager::RemoveInactiveUsers() {
  while (true) {
    std::unique_lock<std::mutex> lk(mut);
    auto const now = std::chrono::system_clock::now();
    auto const time_threshold = now - user_timeout_;
    for (auto it = user_activity_map_.begin(), end = user_activity_map_.end(); it != end;) {
      if ((*it).second < time_threshold) {
        // Remove the user's entry if their last activity time is older than the current time minus timeout
        it = user_activity_map_.erase(it);
      } else {
        ++it;
      }
    }
    lk.unlock();
    std::this_thread::sleep_for(check_period_);
  }
}

// This function resets the activity timeout for a specific user
// simply by adding (or overwriting) its entry in the user_activity_map_ with the current time.
void UserManager::UpdateActiveUser(const std::string& user_name) {
  std::cout << "UserManager: update active user " << user_name << "\n";
  std::unique_lock<std::mutex> lk(mut);
  user_activity_map_[user_name] = std::chrono::system_clock::now();
  lk.unlock();
}
std::vector<std::string> UserManager::GetActiveUsers() const {
  auto active_users = std::vector<std::string>{};
  for (const auto& [key, _] : user_activity_map_) {
    active_users.push_back(key);
  }
  return active_users;
}

}  // namespace chat_app