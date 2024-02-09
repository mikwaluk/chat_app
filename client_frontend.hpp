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

    class ChatWindow : public QWidget {
    public:
        ChatWindow(std::queue<std::tuple<const std::string, const std::vector<std::string>>>* send_button_event_queue);

    private:
        void setupUI();

        // Slot to handle sending message when send button is clicked
        void sendButtonPressed();

        QListWidget *userList;
        QTextEdit *textDisplayArea;
        QTextEdit *messageInputField;
        QPushButton *sendButton;
        std::queue<std::tuple<const std::string, const std::vector<std::string>>>* event_queue_;
    };
} // namespace chat_app
#endif  // CLIENT_FRONTEND_HPP_