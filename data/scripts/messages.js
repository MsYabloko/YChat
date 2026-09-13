const messages_container = document.getElementById("messages-container");

let last_message_id = 0;
let oldest_message_id = undefined;

function add_message(login, text, unixtime, my, first)
{
    let account_info = sendHttp("get_info", {login: login}, false);

    let message_base = document.createElement("div");
    message_base.classList.add("message", my ? "my" : "other");

    let message_sender = document.createElement("div");
    message_sender.classList.add("message-sender");
    let avatar = document.createElement("img");
    avatar.setAttribute("src", account_info.imgsrc);
    message_sender.appendChild(avatar);
    let nameText = document.createTextNode(account_info.username);
    message_sender.appendChild(nameText);
    message_base.appendChild(message_sender);

    let message_text = document.createElement("div");
    message_text.classList.add("message-text");

    message_text.innerText = text;

    message_base.appendChild(message_text);

    let message_date = document.createElement("div");
    message_date.classList.add("message-timestamp");
    let date = new Date(unixtime * 1000);
    let message_date_node = document.createTextNode(date.toLocaleTimeString());
    message_date.appendChild(message_date_node);
    message_base.appendChild(message_date);

    if(!first)
    {
        messages_container.appendChild(message_base);
    }
    else
    {
        messages_container.insertBefore(message_base, messages_container.firstChild);
    }
}

function add_message_json(message, first)
{
    if(message.id > last_message_id) last_message_id = message.id;
    if(oldest_message_id === undefined || oldest_message_id > message.id) oldest_message_id = message.id;
    add_message(message.login, message.text, message.time, message.my, first);
}

const send_button = document.getElementById("send-button");

send_button.onclick = function ()
{
    const data = {
        text: chat_input.innerText
    }
    api.send_api('send_message', data, true);
    chat_input.replaceChildren();
}

let messages = api.send_api("get_messages", {}, false);
if(messages.code === 0)
{
    for(const message of messages.messages)
    {
        add_message_json(message, true);
    }
}
let end_of_top = false;
function check_top()
{
    if(messages_container.scrollTop < 30 && !end_of_top)
    {
        let messages = api.send_api("get_old_messages", {last_message: oldest_message_id}, false);
        console.log(messages);
        console.log(oldest_message_id);
        if(messages.code === 0)
        {
            for(const message of messages.messages.reverse())
            {
                add_message_json(message, true);
            }
            if(messages.messages.length === 0) end_of_top = true;
        }
    }
}
messages_container.addEventListener("scroll", check_top);

function fetch_new_messages()
{
    let messages = api.send_api("get_new_messages", {last_message: last_message_id}, false);
    if(messages.code !== 0) return;
    for(const message of messages.messages)
    {
        add_message_json(message, false);
    }
}
window.setInterval(fetch_new_messages, 1000);
