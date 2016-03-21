/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#include "components/channel_component.h"
#include "components/user_component.h"
#include "chat_server.h"
#include "protocol/version.h"
#include "protocol/components/channel_message_type.h"
#include "string.hpp"

namespace jchat {
ChannelComponent::ChannelComponent() {
}

ChannelComponent::~ChannelComponent() {
  if (!channels_.empty()) {
    for (auto channel : channels_) {
      delete channel;
    }
    channels_.clear();
  }
}

bool ChannelComponent::Initialize(ChatServer &server) {
  server_ = &server;
  return true;
}

bool ChannelComponent::Shutdown() {
  server_ = 0;

  // Remove channels
  channels_mutex_.lock();
  if (!channels_.empty()) {
    for (auto channel : channels_) {
      delete channel;
    }
    channels_.clear();
  }
  channels_mutex_.unlock();

  return true;
}

bool ChannelComponent::OnStart() {
  return true;
}

bool ChannelComponent::OnStop() {
  // Remove channels
  channels_mutex_.lock();
  if (!channels_.empty()) {
    for (auto channel : channels_) {
      delete channel;
    }
    channels_.clear();
  }
  channels_mutex_.unlock();

  return true;
}

void ChannelComponent::OnClientConnected(RemoteChatClient &client) {

}

void ChannelComponent::OnClientDisconnected(RemoteChatClient &client) {
  // TODO: Notify all clients in participating channels that the client has
  // disconnected
  // NOTE: See note in UserComponent.cpp
}

ComponentType ChannelComponent::GetType() {
  return kComponentType_Channel;
}

bool ChannelComponent::Handle(RemoteChatClient &client, uint16_t message_type,
  TypedBuffer &buffer) {
  if (message_type == kChannelMessageType_JoinChannel) {
    std::string channel_name;
    if (!buffer.ReadString(channel_name)) {
      return false;
    }

    // Get user component
    UserComponent *user_component = 0;
    if (!server_->GetComponent(kComponentType_User, user_component)) {
      // Internal error, disconnect client
      return false;
    }

    // Get the chat client
    ChatUser *chat_user = 0;
    if (!user_component->GetChatUser(client, chat_user)) {
      // Internal error, disconnect client
      return false;
    }

    // Check if the user is logged in
    if (!chat_user->Identified) {
      TypedBuffer send_buffer = server_->CreateBuffer();
      send_buffer.WriteUInt16(kChannelMessageResult_NotIdentified);
      server_->SendUnicast(client, kComponentType_Channel,
        kChannelMessageType_JoinChannel_Complete, send_buffer);

      return true;
    }

    // Check if the channel name is valid
    if (!String::Contains(channel_name, "#")) {
      TypedBuffer send_buffer = server_->CreateBuffer();
      send_buffer.WriteUInt16(kChannelMessageResult_InvalidChannelName);
      server_->SendUnicast(client, kComponentType_Channel,
        kChannelMessageType_JoinChannel_Complete, send_buffer);

      return true;
    }

    // Check if the channel exists
    Channel *chat_channel = 0;
    channels_mutex_.lock();
    for (auto channel : channels_) {
      if (channel->Name == channel_name) {
        chat_channel = channel;
        break;
      }
    }
    channels_mutex_.unlock();

    if (chat_channel == 0) {
      // Create the channel and add the user to it
      ChatChannel *chat_channel = new ChatChannel();
      chat_channel->Name = channel_name;
      chat_channel->Operators[&client] = chat_user;
      chat_channel->Clients[&client] = chat_user;

      // Add the channel to the component
      channels_mutex_.lock();
      channels_.push_back(chat_channel):
      channels_mutex_.unlock();

      // Notify the client that the channel was created and that they are
      // the operator operator and member of it
      TypedBuffer send_buffer = server_->CreateBuffer();
      send_buffer.WriteUInt16(kChannelMessageResult_ChannelCreated);
      server_->SendUnicast(client, kComponentType_Channel,
        kChannelMessageType_JoinChannel_Complete, send_buffer);

      // Trigger the events
      OnChannelCreated(channel_name);
      OnChannelJoined(*chat_user);
    } else {
      // Add the user to the channel
      chat_channel->ClientsMutex.lock();
      chat_channel->ClientsMutex[&client] = chat_user;
      chat_channel->ClientsMutex.unlock();

      // Notify the client that it joined the channel and give it a list of
      // current clients
      TypedBuffer client_buffer = server_->CreateBuffer();
      client_buffer.WriteUInt16(kChannelMessageResult_Ok); // Channel joined

      chat_channel->OperatorsMutex.lock();
      client_buffer.WriteUInt32(chat_channel->Operators.size());
      for (auto &pair : chat_channel->Operators) {
        client_buffer.WriteString(pair.second->Username);
        client_buffer.WriteString(pair.second->Hostname);
      }
      chat_channel->OperatorsMutex.unlock();

      chat_channel->ClientsMutex.lock();
      client_buffer.WriteUInt32(chat_channel->Clients.size());
      for (auto &pair : chat_channel->Clients) {
        client_buffer.WriteString(pair.second->Username);
        client_buffer.WriteString(pair.second->Hostname);
      }
      chat_channel->ClientsMutex.unlock();
      server_->SendUnicast(client, kComponentType_Channel,
        kChannelMessageType_JoinChannel_Complete, client_buffer);

      // Notify all clients in the channel that the user has joined
      TypedBuffer clients_buffer = server_->CreateBuffer();
      clients_buffer.WriteUInt16(kChannelMessageResult_UserJoined);
      clients_buffer.WriteString(chat_user->Username);
      clients_buffer.WriteString(chat_user->Hostname);

      chat_channel->ClientsMutex.lock();
      for (auto &pair : chat_channel->Clients) {
        server_->SendUnicast(pair.first, kComponentType_Channel,
          kChannelMessageType_JoinChannel, clients_buffer);
      }
      chat_channel->ClientsMutex.unlock();

      OnChannelJoined(*chat_user);

      return true;
    }

    return true;
  }

  return false;
}
}