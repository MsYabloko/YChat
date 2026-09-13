function getCookie(cname)
{
    let name = cname + "=";
    let decodedCookie = decodeURIComponent(document.cookie);
    let ca = decodedCookie.split(';');
    for(let i = 0; i <ca.length; i++)
    {
        let c = ca[i];
        while (c.charAt(0) === ' ')
        {
            c = c.substring(1);
        }
        if (c.indexOf(name) === 0)
        {
            return c.substring(name.length, c.length);
        }
    }
    return "";
}
function setCookie(cname, value)
{
    document.cookie = encodeURIComponent(cname) + '=' + encodeURIComponent(value);
}
function sendHttp(name, data, post)
{
    const urlParams = new URLSearchParams(data);
    let xmlHttp = new XMLHttpRequest();
    xmlHttp.open(post ? "POST" : "GET", "/api/" + name + (post ? "" : "?" + urlParams.toString()), false);
    xmlHttp.send(post ? urlParams.toString() : null);
    return  JSON.parse(xmlHttp.responseText);
}

class Api
{
    set_refresh_token(refresh_token)
    {
        setCookie("repeat_token", refresh_token);
        this.refresh_token();
    }
    check_current_token()
    {
        let token = getCookie("token");
        if(token === "") return false;
        const data = { token: token }
        let code = sendHttp("check_auth", data, false)["code"];
        // 1 - failed
        // 0 - Success
        if(code === 0)
        {
            console.log("Login Successful");
            document.getElementById("login-background").style.display = "none";
        }
        return code === 0;
    }
    refresh_token()
    {
        let repeat_token = getCookie("repeat_token");
        const data = {token: repeat_token};
        let result = sendHttp("refresh_token", data, false);
        if(result.code !== 0) return false;
        setCookie("token", result.token);
        return this.check_current_token();
    }
    reauth()
    {
        let repeat_token = getCookie("repeat_token");
        if(repeat_token === "") return false;
        if(this.check_current_token()) return true;
        return this.refresh_token();
    }
    constructor()
    {
        this.reauth();
    }
    send_api(name, data, post)
    {
        data.token = getCookie("token");
        if(data.token === "") return;
        let result = sendHttp(name, data, post);
        if(result.code === 1 && this.reauth()) //Incorrect Token
        {
            return sendHttp(name, data, post);
        }
        return result;
    }
}

const api = new Api();