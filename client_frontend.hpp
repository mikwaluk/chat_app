#ifndef CLIENT_FRONTEND_HPP_
#define CLIENT_FRONTEND_HPP_

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QListWidget>
#include <string>
#include <vector>
#include <queue>
#include <tuple>

namespace chat_app {

    class ClientFrontend : public QWidget {
    public:
        ClientFrontend(std::queue<std::tuple<const std::string, const std::vector<std::string>>>* send_button_event_queue);
        void DisplayNewMessage(const std::string& author, const std::string& new_msg);
        void UpdateActiveUsersList(const std::shared_ptr<std::vector<std::string>> active_users);
    private:
        void setupUI();

        // Slot to handle sending message when send button is clicked
        void SendButtonClicked();
        QListWidget *userList;
        QTextEdit *textDisplayArea;
        QTextEdit *messageInputField;
        QPushButton *sendButton;
        std::queue<std::tuple<const std::string, const std::vector<std::string>>>* event_queue_;
    };
} // namespace chat_app
#endif  // CLIENT_FRONTEND_HPP_