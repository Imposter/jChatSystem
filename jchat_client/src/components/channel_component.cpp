/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#include "components/channel_component.h"
#include "components/user_component.h"
#include "chat_client.h"
#include "protocol/version.h"
#include "protocol/components/channel_message_type.h"

namespace jchat {
ChannelComponent::ChannelComponent() {
}

ChannelComponent::~ChannelComponent() {
  // Remove channels
  if (!channels_.empty()) {
    channels_.clear();
  }
}

bool ChannelComponent::Initialize(ChatClient &client) {
  client_ = &client;
  return true;
}

bool ChannelComponent::Shutdown() {
  client_ = 0;

  // Remove channels
  if (!channels_.empty()) {
    channels_.clear();
  }

  return true;
}

void ChannelComponent::OnConnected() {

}

void ChannelComponent::OnDisconnected() {
  // Remove channels
  channels_mutex_.lock();
  if (!channels_.empty()) {
    channels_.clear();
  }
  channels_mutex_.unlock();
}

ComponentType ChannelComponent::GetType() {
  return kComponentType_Channel;
}

bool ChannelComponent::Handle(uint16_t message_type, TypedBuffer &buffer) {
  if (message_type == kChannelMessageType_JoinChannel_Complete) {
    uint16_t message_result = 0;
    if (!buffer.ReadUInt16(message_result)) {
      return false;
    }
    std::string channel_name;
    if (!buffer.ReadString(channel_name)) {
      return false;
    }
    OnJoinCompleted(static_cast<ChannelMessageResult>(message_result),
      channel_name);
    if (message_result != kChannelMessageResult_Ok
      && message_result != kChannelMessageResult_ChannelCreated) {
      return true;
    }

    // Create the ChatChannel and do necessary actions
    auto chat_channel = std::make_shared<ChatChannel>();
	chat_channel->Name = channel_name;
    chat_channel->Enabled = true;

    // Add the channel to the channel list
    channels_mutex_.lock();
    channels_.push_back(chat_channel);
    channels_mutex_.unlock();

    // Get user component
    std::shared_ptr<UserComponent> user_component;
    if (!client_->GetComponent(kComponentType_User, user_component)) {
      // Internal error, disconnect client
      return false;
    }

    // Get the chat client
    std::shared_ptr<ChatUser> chat_user;
    if (!user_component->GetChatUser(chat_user)) {
      // Internal error, disconnect client
      return false;
    }

    // Add the local user
    chat_channel->ClientsMutex.lock();
    chat_channel->Clients.push_back(chat_user);
    chat_channel->ClientsMutex.unlock();

    if (message_result == kChannelMessageResult_ChannelCreated) {
      chat_channel->OperatorsMutex.lock();
      chat_channel->Operators.push_back(chat_user);
      chat_channel->OperatorsMutex.unlock();

      OnChannelCreated(*chat_channel, *chat_user);
      OnChannelJoined(*chat_channel, *chat_user);

      return true;
    }

    uint64_t users_count = 0;
    if (!buffer.ReadUInt64(users_count)) {
      return false;
    }

    for (size_t i = 0; i < users_count; i++) {
      auto user = std::make_shared<ChatUser>();
      user->Enabled = true;
      user->Identified = true;

      if (!buffer.ReadString(user->Username)) {
        return false;
      }
      if (!buffer.ReadString(user->Hostname)) {
        return false;
      }
      bool is_operator = false;
      if (!buffer.ReadBoolean(is_operator)) {
        return false;
      }

      chat_channel->ClientsMutex.lock();
      chat_channel->Clients.push_back(user);
      chat_channel->ClientsMutex.unlock();
      if (is_operator) {
        chat_channel->OperatorsMutex.lock();
        chat_channel->Operators.push_back(user);
        chat_channel->OperatorsMutex.unlock();
      }
    }

    // Read bans
    uint64_t bans_count = 0;
    if (!buffer.ReadUInt64(bans_count)) {
      return false;
    }

    chat_channel->BannedUsersMutex.lock();
    for (size_t i = 0; i < bans_count; i++) {
      std::string banned_user;
      if (!buffer.ReadString(banned_user)) {
        chat_channel->BannedUsersMutex.unlock();
        return false;
      }
      chat_channel->BannedUsers.push_back(banned_user);
    }
    chat_channel->BannedUsersMutex.unlock();

    // TODO: Handle notifications that a user has joined/left a channel
    // TODO: Handle SendMessage
    // TODO: Handle KickUser
    // TODO: Handle BanUser
    // TODO: Add SendMessage to UserComponent

    // Trigger events
    OnChannelJoined(*chat_channel, *chat_user);

    return true;
  } else if (message_type == kChannelMessageType_LeaveChannel_Complete) {
    uint16_t message_result = 0;
    if (!buffer.ReadUInt16(message_result)) {
      return false;
    }
    std::string channel_name;
    if (!buffer.ReadString(channel_name)) {
      return false;
    }
    OnLeaveCompleted(static_cast<ChannelMessageResult>(message_result), channel_name);
    if (message_result != kChannelMessageResult_Ok
      && message_result != kChannelMessageResult_ChannelDestroyed) {
      return true;
    }

    // Get user component
    std::shared_ptr<UserComponent> user_component;
    if (!client_->GetComponent(kComponentType_User, user_component)) {
      // Internal error, disconnect client
      return false;
    }

    // Get the chat client
    std::shared_ptr<ChatUser> chat_user;
    if (!user_component->GetChatUser(chat_user)) {
      // Internal error, disconnect client
      return false;
    }

    // Remove the ChatChannel and do necessary actions
    channels_mutex_.lock();
    for (auto it = channels_.begin(); it != channels_.end(); ++it) {
      std::shared_ptr<ChatChannel> &chat_channel = *it;
      if (chat_channel->Enabled && chat_channel->Name == channel_name) {
        OnChannelLeft(*chat_channel, *chat_user);

        // Disable the channel
        chat_channel->Enabled = false;

        // Clear all information
        chat_channel->OperatorsMutex.lock();
        chat_channel->Operators.clear();
        chat_channel->OperatorsMutex.unlock();

        chat_channel->ClientsMutex.lock();
        chat_channel->Clients.clear();
        chat_channel->ClientsMutex.unlock();

        chat_channel->BannedUsersMutex.lock();
        chat_channel->BannedUsers.clear();
        chat_channel->BannedUsersMutex.unlock();

        // Remove the channel
        it = channels_.erase(it);
      }
    }
    channels_mutex_.unlock();

    return true;
  }

  return false;
}

bool ChannelComponent::JoinChannel(std::string channel_name) {
  TypedBuffer buffer = client_->CreateBuffer();
  buffer.WriteString(channel_name);
  return client_->Send(kComponentType_Channel, kChannelMessageType_JoinChannel,
    buffer);
}

bool ChannelComponent::LeaveChannel(std::string channel_name) {
  TypedBuffer buffer = client_->CreateBuffer();
  buffer.WriteString(channel_name);
  return client_->Send(kComponentType_Channel, kChannelMessageType_LeaveChannel,
    buffer);
}
}
