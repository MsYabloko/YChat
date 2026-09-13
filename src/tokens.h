#ifndef YCHAT_TOKENS_H
#define YCHAT_TOKENS_H

namespace Tokens
{
    void init();
    std::string reg_new_refresh_token(const std::string& login);
    std::string refresh_token(const std::string& token);
    std::string get_login(const std::string& token);
}

#endif
