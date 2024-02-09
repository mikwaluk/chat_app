#ifndef USER_THREAD_HPP_
#define USER_THREAD_HPP_

#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace chat_app {

class UserManager : public std::thread {
 public:
  UserManager(std::chrono::milliseconds user_timeout = std::chrono::milliseconds{1000},
              std::chrono::milliseconds check_period = std::chrono::milliseconds{100});
  void UpdateActiveUser(const std::string& user_name);
  std::vector<std::string> GetActiveUsers() const;

 private:
  const std::chrono::milliseconds user_timeout_;
  std::chrono::milliseconds check_period_;
  std::map<std::string, std::chrono::time_point<std::chrono::system_clock>> user_activity_map_;
  bool stop_;
  std::mutex mut;
  void RemoveInactiveUsers();

  void Stop();
};
}  // namespace chat_app
#endif  // USER_MANAGER_HPP_