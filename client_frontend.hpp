#ifndef CLIENT_FRONTEND_HPP_
#define CLIENT_FRONTEND_HPP_

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QListWidget>

namespace chat_app {

    class ChatWindow : public QWidget {
    public:
        ChatWindow(QWidget *parent = nullptr);

    private:
        void setupUI();

        // Slot to handle sending message when send button is clicked
        void sendMessage();

        QListWidget *userList;
        QTextEdit *textDisplayArea;
        QTextEdit *messageInputField;
        QPushButton *sendButton;
    };
} // namespace chat_app
#endif  // CLIENT_FRONTEND_HPP_