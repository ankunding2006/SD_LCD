#ifndef LANGUAGE_H
#define LANGUAGE_H

typedef enum
{
    SYSTEM_LANGUAGE_CHINESE = 0,
    SYSTEM_LANGUAGE_ENGLISH,

    SYSTEM_LANGUAGE_ALL,
} SystemLanguage_e;


typedef enum
{
    TEXT_MENU = 0,              // 菜单
    TEXT_MAIN_MENU,             // 主菜单
    TEXT_SELECT_OPTION,         // 选择操作
    TEXT_ENTER,                 // 进入
    TEXT_EXIT,                  // 退出
    TEXT_RETURN,                // 返回
    TEXT_MUSIC,                 // 音乐
    TEXT_VIDEO,                 // 视频
    TEXT_CAMERA,                // 摄像机
    TEXT_SETTING,               // 设置
    TEXT_SHORTCUT,              // 快捷
    TEXT_MORE,                  // 更多
    TEXT_LANGUAGE,              // 语言
    TEXT_NEXT,                  // 下一个
    TEXT_PREVIOUS,              // 上一个
    TEXT_FUNCTION_TEST,         // 功能测试
    TEXT_TEST,                  // 测试
    TEXT_BLUETOOTH,             // 蓝牙
    TEXT_BATTERY,               // 电池
    TEXT_STORE,                 // 储存
    TEXT_UPGRADE,               // 升级
    TEXT_ABOUT,                 // 关于
    TEXT_PHOTO,                 // 拍照
    TEXT_PHOTOGRAPHY,           // 摄影
    TEXT_LOADING,               // 加载中
    TEXT_STOPPING_PLAYBACK,     // 正在停止播放
    TEXT_EXIT_MAIN_MENU,        // 退出主菜单
    TEXT_RETURN_MAIN_MENU,      // 返回主菜单
    TEXT_MORE_SETTING,          // 更多设置

    TEXT_ALL,
} TextId_e;

void set_language(SystemLanguage_e lang);
const char *get_text(TextId_e id);
const char *get_text_by_language(SystemLanguage_e lang, TextId_e id);

#endif
