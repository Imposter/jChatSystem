/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#include "components/channel_component.h"
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
    OnJoinCompleted(static_cast<ChannelMessageResult>(message_result), channel_name);
    if (message_result != kChannelMessageResult_Ok
      && message_result != kChannelMessageResult_ChannelCreated) {
      return true;
    }

    // TODO: Create the ChatChannel and do necessary actions
    auto chat_channel = std::make_shared<ChatChannel>();
	if (!buffer.ReadString(chat_channel->Name)) {
		return false;
	}

	if (message_result == kChannelMessageResult_ChannelCreated) {

	}

	// TODO: If they created the channel, add them as the operator and only client until info changes
	
	// TODO: Parse channel information
	uint64_t operators_count = 0;
	if (!buffer.ReadUInt64(operators_count)) {
		return false;
	}

	for (size_t i = 0; i < operators_count; i++) {
		auto chat_user = std::make_shared<ChatUser>();
	}

	// TODO: Handle notifications that a user has joined/left a channel
	// TODO: Handle SendMessage
	// TODO: Handle KickUser
	// TODO: Handle BanUser

	// TODO: Add SendMessage to UserComponent
    

    // Trigger events


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

    // TODO: Remove the ChatChannel and do necessary actions

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
