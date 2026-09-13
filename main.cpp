#include <iostream>
#include <utility>
#include "libs/mongoose.h"
#include "libs/src/bcrypt.h"
#include <jsoncpp/json/json.h>

#include <SQLiteCpp/SQLiteCpp.h>
#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/VariadicBind.h>

#include <filesystem>
#include <algorithm>
#include <regex>

#include "src/tokens.h"
#include "src/messages.h"

static SQLite::Database users("users.db3", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

struct EmojiPack
{
    const std::string name;
    const std::string path;
    std::vector<std::string> pathes;
    EmojiPack(std::string _name, std::string _path)
    : name(std::move(_name)), path(std::move(_path))
    {
        for (const auto& dirEntry : std::filesystem::recursive_directory_iterator("data/" + path))
        {
            pathes.push_back(dirEntry.path().filename().string());
        }
        std::sort(std::begin(pathes), std::end(pathes));
    }
};

const EmojiPack packs[1] =
{
    EmojiPack("Колобки", "emoji/kolobki")
};

static std::string get_var(const mg_str& s, const char* name, int length)
{
    char var[length];
    mg_http_get_var(&s, name, var, sizeof(var));
    return var;
}

static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
    if(ev == MG_EV_HTTP_MSG)
    {
        auto *hm = (struct mg_http_message *) ev_data;

        if(mg_http_match_uri(hm, "/api/get_emojis"))
        {
            Json::Value root;
            root["array"] = Json::arrayValue;

            int emojipackcount = 0;
            for(const auto& emojipack : packs)
            {
                Json::Value pack;
                pack["name"] = emojipack.name;
                pack["array"] = Json::arrayValue;

                int emojipathcount = 0;
                for(const auto& emojipath : emojipack.pathes)
                {
                    pack["array"][emojipathcount] = emojipack.path + "/" + emojipath;
                    emojipathcount++;
                }

                root["array"][emojipackcount] = pack;
                emojipackcount++;
            }

            Json::FastWriter writer;
            std::string answer = writer.write(root);

            mg_http_reply(c, 200, "", answer.c_str());
        }
        else if(mg_http_match_uri(hm, "/api/register"))
        {
            std::string login = get_var(hm->body, "login", 255);
            std::string username = get_var(hm->body, "username", 255);
            std::string password = get_var(hm->body, "password", 255);

            std::regex login_regex("^[a-z0-9_-]+$");
            //std::regex username_regex(R"(^[\s\u0400-\u04FFa-zA-Z0-9!@#$%^&*()_+\-=\[\]{};':"\\|,.<>\/?]+$)");
            std::regex username_regex(R"([A-Za-z0-9_.-]+)");
            std::regex password_regex(R"(^[a-zA-Z0-9!@#$%^&*()_+\-=\[\]{};':"\\|,.<>\/?]+$)");

            bool everythingOK = std::regex_match(login, login_regex)
                    && std::regex_match(username, username_regex)
                    && std::regex_match(password, password_regex)
                    && !login.empty() && !username.empty() && !password.empty();

            if(!everythingOK)
            {
                mg_http_reply(c, 200, "", "{\"code\": 2}");
                return;
            }

            SQLite::Statement checkLogin(users, "SELECT 1 FROM users WHERE login=\"" + std::string(login) + "\"");
            if(!checkLogin.executeStep())
            {
                std::string hashedPassword = bcrypt::generateHash(password);
                //Success
                std::string sql = "INSERT INTO users VALUES (\"" + login + "\",\"" + username + "\",\"" + hashedPassword + "\")";
                users.exec(sql);
                mg_http_reply(c, 200, "", "{\"code\": 0}");
            }
            else
            {
                //User exists
                mg_http_reply(c, 200, "", "{\"code\": 1}");
            }
        }
        else if(mg_http_match_uri(hm, "/api/auth"))
        {
            std::string login = get_var(hm->query, "login", 255);
            std::string password = get_var(hm->query, "password", 255);

            SQLite::Statement checkLogin(users, "SELECT * FROM users WHERE login=\"" + std::string(login) + "\"");
            if(checkLogin.executeStep())
            {
                std::string hashed_password = checkLogin.getColumn("password_hash").getString();
                if(bcrypt::validatePassword(password, hashed_password))
                {
                    //Success
                    std::string reply = R"({"code": 0, "refresh_token": %m })";
                    mg_http_reply(c, 200, "", reply.c_str(), MG_ESC(Tokens::reg_new_refresh_token(login).c_str()));

                }
                else
                {
                    //Wrong password
                    mg_http_reply(c, 200, "", "{\"code\": 2}");
                }
            }
            else
            {
                //User does not exists
                mg_http_reply(c, 200, "", "{\"code\": 1}");
            }
        }
        else if(mg_http_match_uri(hm, "/api/refresh_token"))
        {
            std::string refresh_token = get_var(hm->query, "token", 1000);
            std::cout << "Refresh Token: " << refresh_token << std::endl;
            std::string newToken = Tokens::refresh_token(refresh_token);
            if(!newToken.empty())
            {
                //Success
                mg_http_reply(c, 200, "", R"({"code": 0, "token": %m })", MG_ESC(newToken.c_str()));
            }
            else
            {
                //Incorrect token
                mg_http_reply(c, 200, "", "{\"code\": 1}");
            }
        }
        else if(mg_http_match_uri(hm, "/api/check_auth"))
        {
            std::string token = get_var(hm->query, "token", 1000);
            std::string login = Tokens::get_login(token);
            std::string reply = "{\"code\": ";
            reply += login.empty() ? "1" : "0";
            reply += "}";
            mg_http_reply(c, 200, "", reply.c_str());
            // 1 - Failed
            // 0 - Success
        }
        else if(mg_http_match_uri(hm, "/api/send_message"))
        {
            std::string text = get_var(hm->body, "text", 2000);
            std::string token = get_var(hm->body, "token", 1000);
            std::string login = Tokens::get_login(token);
            if(!login.empty())
            {
                Messages::add_message(text, login);
                mg_http_reply(c, 200, "", "{\"code\": 0}");
            }
            else
            {
                //Incorrect token
                mg_http_reply(c, 200, "", "{\"code\": 1}");
            }
        }
        else if(mg_http_match_uri(hm, "/api/get_messages"))
        {
            std::string token = get_var(hm->query, "token", 1000);
            std::string login = Tokens::get_login(token);
            const int stack_count = 5;
            if(!login.empty())
            {
                Json::Value messages = Messages::get_messages(stack_count, login);
                messages["code"] = 0;
                Json::FastWriter writer;
                mg_http_reply(c, 200, "", writer.write(messages).c_str());
            }
            else
            {
                //Incorrect token
                mg_http_reply(c, 200, "", "{\"code\": 1}");
            }
        }
        else if(mg_http_match_uri(hm, "/api/get_old_messages"))
        {
            std::string token = get_var(hm->query, "token", 1000);
            std::string login = Tokens::get_login(token);
            int last_message = std::stoi(get_var(hm->query, "last_message", 1000));
            if(!login.empty())
            {
                Json::Value messages = Messages::get_old_messages(last_message, login);
                messages["code"] = 0;
                Json::FastWriter writer;
                mg_http_reply(c, 200, "", writer.write(messages).c_str());
            }
            else
            {
                //Incorrect token
                mg_http_reply(c, 200, "", "{\"code\": 1}");
            }
        }
        else if(mg_http_match_uri(hm, "/api/get_new_messages"))
        {
            std::string token = get_var(hm->query, "token", 1000);
            std::string login = Tokens::get_login(token);
            int last_message = std::stoi(get_var(hm->query, "last_message", 1000));
            if(!login.empty())
            {
                Json::Value messages = Messages::get_new_messages(last_message, login);
                messages["code"] = 0;
                Json::FastWriter writer;
                mg_http_reply(c, 200, "", writer.write(messages).c_str());
            }
            else
            {
                //Incorrect token
                mg_http_reply(c, 200, "", "{\"code\": 1}");
            }
        }
        else if(mg_http_match_uri(hm, "/api/get_info"))
        {
            std::string login = get_var(hm->query, "login", 255);
            SQLite::Statement getInfo(users, "SELECT * FROM users WHERE login=\"" + std::string(login) + "\"");
            if(getInfo.executeStep())
            {
                Json::Value account_info;
                account_info["username"] = getInfo.getColumn("username").getString();
                account_info["imgsrc"] = "avatars/mryabloko.jpg";
                account_info["code"] = 0;
                Json::FastWriter writer;
                mg_http_reply(c, 200, "", writer.write(account_info).c_str());
            }
            else
            {
                //User does not exists
                mg_http_reply(c, 200, "", "{\"code\": 1}");
            }
        }
        else
        {
            struct mg_http_serve_opts opts = {.root_dir = "./data"};
            mg_http_serve_dir(c, hm, &opts);
        }
    }
}

int main()
{
    users.exec("CREATE TABLE IF NOT EXISTS users (login TINYTEXT PRIMARY KEY, username TINYTEXT, password_hash TEXT)");

    Tokens::init();
    Messages::init();

    std::cout << "Success" << std::endl;

    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, "https://0.0.0.0:8000", fn, nullptr);
    for(;;)
    {
        mg_mgr_poll(&mgr, 1000);
    }

    return 0;
}
