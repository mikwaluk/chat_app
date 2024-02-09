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
  SetupUI();
}

// Creates the UI's layout and all elements.
void ClientFrontend::SetupUI() {
  QHBoxLayout *mainLayout = new QHBoxLayout(this);

  // Chat Content (Messages) - Left Column
  QVBoxLayout *chatContentLayout = new QVBoxLayout;

  // Label for the text display area
  QLabel *textDisplayAreaLabel = new QLabel("Chat Messages:", this);
  chatContentLayout->addWidget(textDisplayAreaLabel);

  // Area to display sent and received messages
  text_display_area_ = new QTextEdit(this);
  text_display_area_->setReadOnly(true);
  chatContentLayout->addWidget(text_display_area_);

  mainLayout->addLayout(chatContentLayout);

  // User List, Message Input Field, and Send Button - Right Column
  QVBoxLayout *rightColumnLayout = new QVBoxLayout;

  // Label for the user list
  QLabel *userListLabel = new QLabel("Active Users:", this);
  rightColumnLayout->addWidget(userListLabel);

  // Panel to display other users
  user_list_ = new QListWidget(this);
  user_list_->setSelectionMode(QAbstractItemView::MultiSelection);
  rightColumnLayout->addWidget(user_list_);

  // Label for the message input field
  QLabel *messageInputFieldLabel = new QLabel("Message:", this);
  rightColumnLayout->addWidget(messageInputFieldLabel);

  // Field to write and send a message
  message_input_field_ = new QTextEdit(this);
  rightColumnLayout->addWidget(message_input_field_);

  // Button to send message
  send_button_ = new QPushButton("Send", this);
  connect(send_button_, &QPushButton::clicked, this, &ClientFrontend::SendButtonClicked);
  rightColumnLayout->addWidget(send_button_);

  mainLayout->addLayout(rightColumnLayout);

  setLayout(mainLayout);
}

// When the user clicks on the Send button, this function:
// - displays the text, followed by "[You]: "
// - and also gathers information about the currently selected recipients (if there are any)
// Both the text message and the list of recipients are then pushed to the Frontend's queue.
void ClientFrontend::SendButtonClicked() {
  QString message = message_input_field_->toPlainText();
  text_display_area_->append("[You]: " + message);
  std::vector<std::string> selectedUsers;
  QList<QListWidgetItem *> selectedItems = user_list_->selectedItems();
  for (QListWidgetItem *item : selectedItems) {
    selectedUsers.push_back(item->text().toStdString());
  }

  event_queue_->push(std::make_tuple(message.toStdString(), selectedUsers));
  message_input_field_->clear();
}

// Appends a new message below the previous ones.
void ClientFrontend::DisplayNewMessage(const std::string &author, const std::string &new_msg) {
  text_display_area_->append(QString::fromStdString("[" + author + "]: " + new_msg));
}

// Updates the list of active users, while keeping the existing ones untouched.
// It's so complex because removing all of them and re-adding the current ones zeroed the selection,
// so it was impossible to send messages to specific users otherwise.
void ClientFrontend::UpdateActiveUsersList(const std::shared_ptr<std::vector<std::string>> active_users) {
  std::cout << "UpdateActiveUsersList\n";

  // Remove the inactive users
  for (int i = 0; i < user_list_->count(); ++i) {
    QListWidgetItem *item = user_list_->item(i);
    if (std::find(active_users->begin(), active_users->end(), item->text().toStdString()) == active_users->end()) {
      delete user_list_->takeItem(i);
      --i;
    }
  }

  // Add only the new active users
  for (const auto &user : *active_users) {
    bool userExists = false;
    for (int i = 0; i < user_list_->count(); ++i) {
      if (user_list_->item(i)->text() == QString::fromStdString(user)) {
        userExists = true;
        break;
      }
    }
    if (!userExists) {
      user_list_->addItem(QString::fromStdString(user));
    }
  }
}

}  // namespace chat_app