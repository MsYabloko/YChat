#include <iostream>
#include "messages.h"

#include <SQLiteCpp/SQLiteCpp.h>
#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Statement.h>
#include <SQLiteCpp/VariadicBind.h>

#include <jsoncpp/json/json.h>

static SQLite::Database messages("messages.db3", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

void Messages::init()
{
    messages.exec("CREATE TABLE IF NOT EXISTS messages (login TINYTEXT, text TEXT, time TIMESTAMP)");
}

void Messages::add_message(const std::string& text, const std::string& login)
{
    SQLite::Statement insert(messages, R"(INSERT INTO messages VALUES (?, ?,  DATETIME('now') ))");
    insert.bind(1, login);
    insert.bind(2, text);
    insert.tryExecuteStep();
}

Json::Value Messages::get_messages(int stack_count, const std::string& login)
{
    SQLite::Statement get(messages, R"(SELECT oid as id, *, strftime('%s', time) as unixtime FROM 'messages' ORDER BY id DESC LIMIT ?,?)");
    get.bind(1, 0);
    get.bind(2, stack_count);
    Json::Value messagesJson = Json::arrayValue;
    while(get.executeStep())
    {
        Json::Value message;
        message["login"] = get.getColumn("login").getString();
        message["text"] = get.getColumn("text").getString();
        message["time"] = get.getColumn("unixtime").getInt();
        message["my"] = (get.getColumn("login").getString() == login);
        message["id"] = get.getColumn("id").getInt();
        messagesJson.append(message);
    }

    Json::Value result;
    result["messages"] = messagesJson;
    return result;
}

Json::Value Messages::get_new_messages(int last_message, const std::string& login)
{
    SQLite::Statement get(messages, R"(SELECT oid as id, *, strftime('%s', time) as unixtime FROM 'messages' ORDER BY id ASC LIMIT -1 OFFSET ?)");
    get.bind(1, last_message);
    Json::Value messagesJson = Json::arrayValue;
    while(get.executeStep())
    {
        Json::Value message;
        message["login"] = get.getColumn("login").getString();
        message["text"] = get.getColumn("text").getString();
        message["time"] = get.getColumn("unixtime").getInt();
        message["my"] = (get.getColumn("login").getString() == login);
        message["id"] = get.getColumn("id").getInt();
        messagesJson.append(message);
    }

    Json::Value result;
    result["messages"] = messagesJson;
    return result;
}
Json::Value Messages::get_old_messages(int last_message, const std::string& login)
{
    SQLite::Statement get(messages, R"(SELECT oid as id, *, strftime('%s', time) as unixtime FROM 'messages' ORDER BY id ASC LIMIT ? OFFSET ?)");
    int offset = last_message - 6;
    int limit = 5;
    if(last_message < 6)
    {
        offset = 0;
        limit = last_message - 1;
    }
    get.bind(1, limit);
    get.bind(2, offset);
    Json::Value messagesJson = Json::arrayValue;
    while(get.executeStep())
    {
        Json::Value message;
        message["login"] = get.getColumn("login").getString();
        message["text"] = get.getColumn("text").getString();
        message["time"] = get.getColumn("unixtime").getInt();
        message["my"] = (get.getColumn("login").getString() == login);
        message["id"] = get.getColumn("id").getInt();
        messagesJson.append(message);
    }

    Json::Value result;
    result["messages"] = messagesJson;
    return result;
}
