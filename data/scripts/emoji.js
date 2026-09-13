function getAllEmoji()
{
    let xmlHttp = new XMLHttpRequest();
    xmlHttp.open("GET", "/api/get_emojis", false);
    xmlHttp.send( null );
    return JSON.parse(xmlHttp.responseText);
}

const emoji_menu = document.getElementById("emoji-menu");
const chat_input = document.getElementById("chat-input");

let emojipackcounter = 0;
for(const emojipack of getAllEmoji()["array"])
{
    let emoji_name = document.createElement("div");
    emoji_name.classList.add("emoji-name")
    emoji_name.textContent = emojipack.name;

    let emoji_container = document.createElement("div");
    emoji_container.classList.add("emoji-container")

    let emojicounter = 0;
    for(const path of emojipack["array"])
    {
        let img = document.createElement("img");
        img.classList.add("emoji");
        img.src = path;

        img.setAttribute("emojipack", emojipackcounter);
        img.setAttribute("emoji", emojicounter);

        img.onclick = function () {
            let emoji = document.createElement("img");
            emoji.classList.add("emoji");
            emoji.src = img.src;

            emoji.setAttribute("emojipack", img.getAttribute("emojipack"));
            emoji.setAttribute("emoji", img.getAttribute("emoji"));

            chat_input.appendChild(emoji);
        }

        emoji_container.appendChild(img);

        emojicounter += 1;
    }

    emoji_menu.appendChild(emoji_name);
    emoji_menu.appendChild(emoji_container);

    emojipackcounter += 1;
}