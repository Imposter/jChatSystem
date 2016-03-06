/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef jchat_common_message_type_h_
#define jchat_common_message_type_h_

// Required libraries
#include <stdint.h>

namespace jchat {
enum MessageType : uint8_t {
  kMessageType_Hello = 1
};
}

#endif // jchat_common_message_type_h_
