#include <random>
#include <iostream>
#include "tokens.h"

#include <SQLiteCpp/SQLiteCpp.h>
#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Statement.h>
#include <SQLiteCpp/VariadicBind.h>

static SQLite::Database tokens("tokens.db3", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

static void remove_expired()
{
    tokens.exec("DELETE FROM tokens WHERE DATETIME('now') >= expires");
    tokens.exec("DELETE FROM refresh_tokens WHERE DATETIME('now') >= expires");
}

static std::string gen_rand_str(int length)
{
    static const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "!@$:%^&*()_-+=|";

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, (sizeof(alphanum) / sizeof(char)) - 2);

    std::string token;
    for (int i = 0; i < length; i++)
    {
        token += alphanum[dis(gen)];
    }

    return token;
}

static std::string generate_new_token()
{
    while (true)
    {
        std::string str = gen_rand_str(255);
        SQLite::Statement checkToken(tokens, "SELECT 1 FROM tokens WHERE token= ? ");
        checkToken.bind(1, str);
        SQLite::Statement checkRefToken(tokens, "SELECT 1 FROM refresh_tokens WHERE token= ?");
        checkRefToken.bind(1, str);
        if(!checkToken.executeStep() && !checkRefToken.executeStep())
        {
            return str;
        }
    }
}

void Tokens::init()
{
    tokens.exec("CREATE TABLE IF NOT EXISTS tokens (login TINYTEXT, token TINYTEXT PRIMARY KEY, expires TIMESTAMP)");
    tokens.exec("CREATE TABLE IF NOT EXISTS refresh_tokens (login TINYTEXT, token TINYTEXT PRIMARY KEY, expires TIMESTAMP)");
}

std::string Tokens::reg_new_refresh_token(const std::string& login)
{
    remove_expired();
    std::string refreshtoken = generate_new_token();
    SQLite::Statement insert(tokens, R"(INSERT INTO refresh_tokens VALUES (?, ?,  DATETIME(DATETIME('now'), '+14 days') ))");
    insert.bind(1, login);
    insert.bind(2, refreshtoken);
    insert.tryExecuteStep();
    return refreshtoken;
}

std::string Tokens::refresh_token(const std::string& refr_token)
{
    remove_expired();
    std::string gen_token = generate_new_token();
    SQLite::Statement getRefreshTokenInfo(tokens, "SELECT * FROM refresh_tokens WHERE token=\"" + refr_token + "\"");
    if(getRefreshTokenInfo.executeStep())
    {
        std::string login = getRefreshTokenInfo.getColumn("login").getString();

        SQLite::Statement updateExpiresTime(tokens, "UPDATE refresh_tokens SET expires=DATETIME(DATETIME('now'), '+1 month') WHERE token= ? ");
        updateExpiresTime.bind(1, refr_token);
        updateExpiresTime.tryExecuteStep();

        SQLite::Statement insert(tokens, R"(INSERT INTO tokens VALUES (?, ?,  DATETIME(DATETIME('now'), '+1 hours') ))");
        insert.bind(1, login);
        insert.bind(2, gen_token);
        insert.tryExecuteStep();

        return gen_token;
    }
    else
    {
        return "";
    }
}

std::string Tokens::get_login(const std::string& token)
{
    SQLite::Statement getTokenInfo(tokens, "SELECT * FROM tokens WHERE token=\"" + token + "\"");
    if(getTokenInfo.executeStep())
    {
        std::string login = getTokenInfo.getColumn("login").getString();
        return login;
    }
    else
    {
        return "";
    }
}
