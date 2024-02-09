#include "client_frontend.hpp"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>
#include <iostream>

namespace chat_app {
ClientFrontend::ClientFrontend(
    std::queue<std::tuple<const std::string, const std::vector<std::string>>> *send_button_event_queue)
    : QWidget(nullptr), event_queue_{send_button_event_queue} {
  setupUI();
}

void ClientFrontend::setupUI() {
  QHBoxLayout *mainLayout = new QHBoxLayout(this);

  // Chat Content (Messages) - Left Column
  QVBoxLayout *chatContentLayout = new QVBoxLayout;

  // Label for the text display area
  QLabel *textDisplayAreaLabel = new QLabel("Chat Messages:", this);
  chatContentLayout->addWidget(textDisplayAreaLabel);

  // Area to display sent and received messages
  textDisplayArea = new QTextEdit(this);
  textDisplayArea->setReadOnly(true);
  chatContentLayout->addWidget(textDisplayArea);

  mainLayout->addLayout(chatContentLayout);

  // User List, Message Input Field, and Send Button - Right Column
  QVBoxLayout *rightColumnLayout = new QVBoxLayout;

  // Label for the user list
  QLabel *userListLabel = new QLabel("Active Users:", this);
  rightColumnLayout->addWidget(userListLabel);

  // Panel to display other users
  userList = new QListWidget(this);
  userList->setSelectionMode(QAbstractItemView::MultiSelection);
  rightColumnLayout->addWidget(userList);

  // Label for the message input field
  QLabel *messageInputFieldLabel = new QLabel("Message:", this);
  rightColumnLayout->addWidget(messageInputFieldLabel);

  // Field to write and send a message
  messageInputField = new QTextEdit(this);
  rightColumnLayout->addWidget(messageInputField);

  // Button to send message
  sendButton = new QPushButton("Send", this);
  connect(sendButton, &QPushButton::clicked, this, &ClientFrontend::SendButtonClicked);
  rightColumnLayout->addWidget(sendButton);

  mainLayout->addLayout(rightColumnLayout);

  setLayout(mainLayout);
}

void ClientFrontend::SendButtonClicked() {
  QString message = messageInputField->toPlainText();
  // You can implement your message sending logic here
  // For now, just display the message in the chat window
  textDisplayArea->append("[You]: " + message);
  std::vector<std::string> selectedUsers;
  QList<QListWidgetItem *> selectedItems = userList->selectedItems();
  for (QListWidgetItem *item : selectedItems) {
    selectedUsers.push_back(item->text().toStdString());
  }

  // Print the selected users
  event_queue_->push(std::make_tuple(message.toStdString(), selectedUsers));
  // Clear the message input field after sending
  messageInputField->clear();
}

void ClientFrontend::DisplayNewMessage(const std::string &author, const std::string &new_msg) {
  textDisplayArea->append(QString::fromStdString("[" + author + "]: " + new_msg));
}

void ClientFrontend::UpdateActiveUsersList(const std::shared_ptr<std::vector<std::string>> active_users) {
  std::cout << "UpdateActiveUsersList\n";

  // Remove inactive users
  for (int i = 0; i < userList->count(); ++i) {
    QListWidgetItem *item = userList->item(i);
    if (std::find(active_users->begin(), active_users->end(), item->text().toStdString()) == active_users->end()) {
      delete userList->takeItem(i);
      --i;  // Adjust index after removing an item
    }
  }

  // Add new active users
  for (const auto &user : *active_users) {
    bool userExists = false;
    for (int i = 0; i < userList->count(); ++i) {
      if (userList->item(i)->text() == QString::fromStdString(user)) {
        userExists = true;
        break;
      }
    }
    if (!userExists) {
      userList->addItem(QString::fromStdString(user));
    }
  }
}

}  // namespace chat_app