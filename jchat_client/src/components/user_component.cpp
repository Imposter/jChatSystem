/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#include "components/user_component.h"
#include "chat_client.h"
#include "protocol/version.h"
#include "protocol/components/user_message_type.h"

namespace jchat {
UserComponent::UserComponent() {
}

UserComponent::~UserComponent() {
}

bool UserComponent::Initialize(ChatClient &client) {
  client_ = &client;
  user_ = std::make_shared<ChatUser>();

  return true;
}

bool UserComponent::Shutdown() {
  client_ = 0;
  user_.reset();

  return true;
}

void UserComponent::OnConnected() {

}

void UserComponent::OnDisconnected() {

}

ComponentType UserComponent::GetType() {
  return kComponentType_User;
}

bool UserComponent::Handle(uint16_t message_type, TypedBuffer &buffer) {
  if (message_type == kUserMessageType_Complete_Identify) {
    uint16_t message_result = 0;
    if (!buffer.ReadUInt16(message_result)) {
      return false;
    }
    OnIdentified(static_cast<UserMessageResult>(message_result));
    if (message_result == kUserMessageResult_Ok) {
      user_->Identified = true;
    }

    return true;
  }

  return false;
}

bool UserComponent::Identify(std::string username) {
  TypedBuffer buffer = client_->CreateBuffer();
  buffer.WriteString(username);
  return client_->Send(kComponentType_User, kUserMessageType_Identify,
    buffer);
}
}
