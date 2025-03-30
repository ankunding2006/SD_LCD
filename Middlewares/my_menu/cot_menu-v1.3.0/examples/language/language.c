#include "language.h"

static SystemLanguage_e sg_eSystemLanguage = SYSTEM_LANGUAGE_CHINESE;

const char *(sg_kSystemLanguage[TEXT_ALL])[SYSTEM_LANGUAGE_ALL] =
{
    [TEXT_MENU] = {"菜单", "menu"},
    [TEXT_MAIN_MENU] = {"主菜单", "main menu"},
    [TEXT_SELECT_OPTION] = {"选择操作", "select option"},
    [TEXT_ENTER] = {"进入", "enter"},
    [TEXT_EXIT] = {"退出", "exit"},
    [TEXT_RETURN] = {"返回", "return"},
    [TEXT_MUSIC] = {"音乐", "music"},
    [TEXT_VIDEO] = {"视频", "video"},
    [TEXT_CAMERA] = {"摄像机", "camera"},
    [TEXT_SETTING] = {"设置", "setting"},
    [TEXT_SHORTCUT] = {"快捷", "shortcut"},
    [TEXT_MORE] = {"更多", "more"},
    [TEXT_LANGUAGE] = {"语言", "language"},
    [TEXT_NEXT] = {"下一个", "next"},
    [TEXT_PREVIOUS] = {"上一个", "previous"},
    [TEXT_FUNCTION_TEST] = {"功能测试", "functional testing"},
    [TEXT_TEST] = {"测试", "test"},
    [TEXT_BLUETOOTH] = {"蓝牙", "bluetooth"},
    [TEXT_BATTERY] = {"电池", "battery"},
    [TEXT_STORE] = {"储存", "store"},
    [TEXT_UPGRADE] = {"升级", "upgrade"},
    [TEXT_ABOUT] = {"关于", "about"},
    [TEXT_PHOTO] = {"拍照", "Photo"},
    [TEXT_PHOTOGRAPHY] = {"摄影", "photography"},
    [TEXT_LOADING] = {"加载中", "loading"},
    [TEXT_STOPPING_PLAYBACK] = {"正在停止播放", "stopping playback"},
    [TEXT_EXIT_MAIN_MENU] = {"退出主菜单", "exit main menu"},
    [TEXT_RETURN_MAIN_MENU] = {"返回主菜单", "return main menu"},
    [TEXT_MORE_SETTING] = {"更多设置", "more setting"},

};

void set_language(SystemLanguage_e lang)
{
    if (lang >= 0 && lang < SYSTEM_LANGUAGE_ALL)
    {
        sg_eSystemLanguage = lang;
    }
}


const char *get_text(TextId_e id)
{
    static const char *pszNullString = "N/A";

    if (id >= 0 && id < TEXT_ALL)
    {
        return sg_kSystemLanguage[id][sg_eSystemLanguage];
    }

    return pszNullString; // 未找到对应的文本
}

const char *get_text_by_language(SystemLanguage_e lang, TextId_e id)
{
    static const char *pszNullString = "N/A";

    if (id >= 0 && id < TEXT_ALL)
    {
        return sg_kSystemLanguage[id][lang];
    }

    return pszNullString; // 未找到对应的文本
}
