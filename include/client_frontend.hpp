#ifndef CLIENT_FRONTEND_HPP_
#define CLIENT_FRONTEND_HPP_

#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QWidget>
#include <queue>
#include <string>
#include <tuple>
#include <vector>

namespace chat_app {

enum class UserConnectAction {
  kConnect = 0,
  kDisconnect,
};

// Class responsible purely for the user interaction.
// It forwards the messages the user wants to send to the Backend via event_queue_.
// How the class can be controlled:
// - A newly received message can be displayed by calling the DisplayNewMessage function.
// - The list of active users can be updated with UpdateActiveUsersList.
class ClientFrontend : public QWidget {
 public:
  ClientFrontend(std::queue<std::tuple<const std::string, const std::vector<std::string>>>* send_button_event_queue,
                 std::queue<UserConnectAction>* connect_actions_queue);
  void DisplayNewMessage(const std::string& author, const std::string& new_msg);
  void UpdateActiveUsersList(const std::shared_ptr<std::vector<std::string>> active_users);
  void SetConnectedStatus(const bool connected);

 private:
  void SetupUI();

  void SendButtonClicked();
  void ConnectButtonClicked();

  QListWidget* user_list_;
  QTextEdit* text_display_area_;
  QTextEdit* message_input_field_;
  QPushButton* send_button_;
  QPushButton* connect_button_;
  QLabel* connected_indicator_;

  std::queue<std::tuple<const std::string, const std::vector<std::string>>>* user_msg_queue_;
  std::queue<UserConnectAction>* connect_actions_queue_;
};
}  // namespace chat_app
#endif  // CLIENT_FRONTEND_HPP_