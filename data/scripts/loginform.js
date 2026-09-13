const login_menu = document.getElementById("login");
const register_menu = document.getElementById("register");

const goto_login = document.getElementById("goto-login");
const goto_register = document.getElementById("goto-register");

const login_form = document.getElementById("login-form");

register_menu.style["display"] = "none";

goto_login.onclick = function () { login_menu.style["display"] = ""; register_menu.style["display"] = "none"; }
goto_register.onclick = function () { register_menu.style["display"] = ""; login_menu.style["display"] = "none"; }

const wrong_login  = document.getElementById("wrong-login");
const wrong_name  = document.getElementById("wrong-name");
const wrong_password  = document.getElementById("wrong-pass");
const occ_login = document.getElementById("occ-login");
const password_mismatch = document.getElementById("password-mismatch");
const empty_name = document.getElementById("empty-name");
const empty_password = document.getElementById("empty-password");
const empty_login = document.getElementById("empty-login");

const login_reg_input = document.getElementById("login-reg-input");
const name_input = document.getElementById("name-input");
const password_reg_input = document.getElementById("password-reg-input");
const repeat_password_input = document.getElementById("repeat-password-input");

const reg_errors = document.getElementById("reg-errors");

const login_error = document.getElementById("login-error");
const auth_login = document.getElementById("auth-login");
const auth_password = document.getElementById("auth-password");

function check_register()
{
    empty_name.style.display = name_input.value.trim() === "" ? "" : "none";
    empty_password.style.display = password_reg_input.value.trim() === "" ? "" : "none";
    empty_login.style.display = login_reg_input.value.trim() === "" ? "" : "none";
    password_mismatch.style.display = password_reg_input.value === repeat_password_input.value ? "none" : "";

    occ_login.style.display = "none";
    wrong_login.style.display = /^[a-z0-9_-]+$/.test(login_reg_input.value) ? "none" : "";
    wrong_name.style.display = /^[\s\u0400-\u04FFa-zA-Z0-9!@#$%^&*()_+\-=\[\]{};':"\\|,.<>\/?]+$/.test(name_input.value) ? "none" : "";
    wrong_password.style.display = /^[a-zA-Z0-9!@#$%^&*()_+\-=\[\]{};':"\\|,.<>\/?]+$/.test(password_reg_input.value) ? "none" : "";


    let everything_ok = empty_name.style.display === "none" && empty_password.style.display === "none"
        && empty_login.style.display === "none" && password_mismatch.style.display === "none"
        && occ_login.style.display === "none" && wrong_login.style.display === "none"
        && wrong_name.style.display === "none" && wrong_password.style.display === "none";

    reg_errors.style.display = everything_ok ? "none" : "";

    return everything_ok;
}

function register()
{
    if(!check_register()) return 0;
    const data = {
        login: login_reg_input.value.trim().toLocaleLowerCase(),
        username: name_input.value.trim(),
        password: password_reg_input.value.trim()
    }
    let code = sendHttp("register", data, true)["code"];
    if(code === 0)
    {
        login_menu.style["display"] = "";
        register_menu.style["display"] = "none";
    }
    if(code === 1)
    {
        reg_errors.style["display"] = "";
        occ_login.style["display"] = "";
    }
}

function auth()
{
    const data = {
        login: auth_login.value.trim().toLocaleLowerCase(),
        password: auth_password.value.trim()
    }
    let result = sendHttp("auth", data, false);
    let code = result.code;
    if(code === 0)
    {
        api.set_refresh_token(result.refresh_token);
        login_error.textContent = "";
        return;
    }
    login_error.textContent = code === 1 ? "Пользователя не существует" : "Неправильный пароль";
}

check_register();

