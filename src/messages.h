#ifndef YCHAT_MESSAGES_H
#define YCHAT_MESSAGES_H

#include <jsoncpp/json/json.h>

namespace Messages
{
    void init();
    void add_message(const std::string& text, const std::string& login);
    Json::Value get_messages(int stack_count, const std::string& login);
    Json::Value get_new_messages(int last_message, const std::string& login);
    Json::Value get_old_messages(int last_message, const std::string& login);
}

#endif
