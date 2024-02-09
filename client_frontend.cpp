#include "client_frontend.hpp"

#include <QApplication>
#include <QWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QListWidget>

namespace chat_app {
    ChatWindow::ChatWindow(QWidget *parent) : QWidget(parent) {
        setupUI();
    }

    void ChatWindow::setupUI() {
        QHBoxLayout *mainLayout = new QHBoxLayout(this);

        // Container for the chat area
        QVBoxLayout *leftPanelLayout = new QVBoxLayout;

        // Area to display sent and received messages
        textDisplayArea = new QTextEdit(this);
        textDisplayArea->setReadOnly(true);
        leftPanelLayout->addWidget(textDisplayArea);

        mainLayout->addLayout(leftPanelLayout);

        // Container for the user list and message input area
        QVBoxLayout *rightPanelLayout = new QVBoxLayout;

        // Panel to display other users
        userList = new QListWidget(this);
        userList->addItem("User 1");
        userList->addItem("User 2");
        userList->addItem("User 3");
        userList->setSelectionMode(QAbstractItemView::MultiSelection);
        rightPanelLayout->addWidget(userList);

        // Field to write and send a message
        messageInputField = new QTextEdit(this);
        rightPanelLayout->addWidget(messageInputField);

        // Button to send message
        sendButton = new QPushButton("Send", this);
        connect(sendButton, &QPushButton::clicked, this, &ChatWindow::sendMessage);
        rightPanelLayout->addWidget(sendButton);

        mainLayout->addLayout(rightPanelLayout);

        setLayout(mainLayout);
    }

    void ChatWindow::sendMessage() {
        QString message = messageInputField->toPlainText();
        // You can implement your message sending logic here
        // For now, just display the message in the chat window
        textDisplayArea->append("You: " + message);
        // Clear the message input field after sending
        messageInputField->clear();
    }
} // namespace chat_app