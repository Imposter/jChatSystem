/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#include "components/user_component.h"
#include "chat_server.h"
#include "protocol/version.h"
#include "protocol/components/user_message_type.h"
#include "utility.hpp"
#include "string.hpp"

namespace jchat {
UserComponent::UserComponent() {
}

UserComponent::~UserComponent() {
  if (!users_.empty()) {
    users_.clear();
  }
}

bool UserComponent::Initialize(ChatServer &server) {
  server_ = &server;
  return true;
}

bool UserComponent::Shutdown() {
  server_ = 0;

  // Remove users
  users_mutex_.lock();
  if (!users_.empty()) {
    users_.clear();
  }
  users_mutex_.unlock();

  return true;
}

bool UserComponent::OnStart() {
  return true;
}

bool UserComponent::OnStop() {
  // Remove users
  users_mutex_.lock();
  if (!users_.empty()) {
    users_.clear();
  }
  users_mutex_.unlock();

  return true;
}

void UserComponent::OnClientConnected(RemoteChatClient &client) {
  // Create a chat user class instance that we can use to store information
  // about the client
  auto chat_user = std::make_shared<ChatUser>();

  // Store the user
  users_mutex_.lock();
  users_[&client] = chat_user;
  users_mutex_.unlock();

  // Set as unidentified
  chat_user->Identified = false;

  // Give the client a guest username (which will prevent it from accessing
  // anything until it has identified)
  chat_user->Username = "guest-" + std::to_string(Utility::Random(100000,
    999999));

  // Set the IP address as the endpoint until the client identifies
  chat_user->Hostname = client.Endpoint.GetAddressString();
}

void UserComponent::OnClientDisconnected(RemoteChatClient &client) {
  users_mutex_.lock();

  std::shared_ptr<ChatUser> &user = users_[&client];

  // TODO: Do anything with the user that we need to

  // Delete user
  users_.erase(&client);
  users_mutex_.unlock();
}

ComponentType UserComponent::GetType() {
  return kComponentType_User;
}

bool UserComponent::Handle(RemoteChatClient &client, uint16_t message_type,
  TypedBuffer &buffer) {
  if (message_type == kUserMessageType_Identify) {
    std::string username;
    if (!buffer.ReadString(username)) {
      return false;
    }

    // Check if the username is valid
    if (username.size() > JCHAT_COMMON_CHAT_USER_USERNAME_MAX_LENGTH) {
      TypedBuffer send_buffer = server_->CreateBuffer();
      send_buffer.WriteUInt16(kUserMessageResult_UsernameTooLong);
      server_->SendUnicast(client, kComponentType_User,
        kUserMessageType_Complete_Identify, send_buffer);
      return true;
    }

    if (String::Contains(username, "#")) {
      TypedBuffer send_buffer = server_->CreateBuffer();
      send_buffer.WriteUInt16(kUserMessageResult_InvalidUsername);
      server_->SendUnicast(client, kComponentType_User,
        kUserMessageType_Complete_Identify, send_buffer);
      return true;
    }

    // Check if the client is already identified
    users_mutex_.lock();
    std::shared_ptr<ChatUser> chat_user = users_[&client];
    users_mutex_.unlock();

    if (chat_user && chat_user->Identified) {
      TypedBuffer send_buffer = server_->CreateBuffer();
      send_buffer.WriteUInt16(kUserMessageResult_AlreadyIdentified);
      server_->SendUnicast(client, kComponentType_User,
        kUserMessageType_Complete_Identify, send_buffer);
      return true;
    }

    // Check if the username is in use
    users_mutex_.lock();
    for (auto &pair : users_) {
      if (pair.second->Identified && pair.second->Username == username) {
        TypedBuffer send_buffer = server_->CreateBuffer();
        send_buffer.WriteUInt16(kUserMessageResult_UsernameInUse);
        server_->SendUnicast(client, kComponentType_User,
          kUserMessageType_Complete_Identify, send_buffer);
        users_mutex_.unlock();
        return true;
      }
    }
    users_mutex_.unlock();

    // Set as identified and hash the hostname
    chat_user->Identified = true;
    chat_user->Hostname = Utility::HashString(chat_user->Hostname.c_str(),
      chat_user->Hostname.size());

    TypedBuffer send_buffer = server_->CreateBuffer();
    send_buffer.WriteUInt16(kUserMessageResult_Ok);
    server_->SendUnicast(client, kComponentType_User,
      kUserMessageType_Complete_Identify, send_buffer);

    return true;
  }

  return false;
}

bool UserComponent::GetChatUser(RemoteChatClient &client,
  std::shared_ptr<ChatUser> &out_user) {
  users_mutex_.lock();
  if (users_.find(&client) == users_.end()) {
    users_mutex_.unlock();
    return false;
  } else {
    out_user = users_[&client];
    users_mutex_.unlock();
    return true;
  }
}
}
