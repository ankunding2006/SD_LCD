# 轻量级菜单框架(C语言)

## 介绍

### 多级菜单

同级菜单以数组的方式体现，父菜单和子菜单的关联则使用链表实现。

> 数组元素内容有：
>
> * 菜单选项描述方式可选字符串或文本ID（多国语言时，可以统一管理翻译文件，而文本ID可以适用于数组管理的翻译文件）
> * 菜单选项进入回调函数：当前菜单选项进入时(从父菜单进入)需要执行一次的函数
> * 菜单选项退出回调函数：当前菜单选项进入后退出时(退出至父菜单)需要执行一次的函数
> * 菜单选项重加载回调函数：当前菜单选项每次加载时(从父菜单进入或子菜单退出)需要执行一次的函数
> * 菜单选项周期调度回调函数：当前菜单选项的周期调度函数
> * 菜单选项的扩展数据
>
> 链表内存可以选择采用动态内存分配或者数组实现

方便对不同菜单界面功能解耦

> 大部分菜单采用的都是数组中包含了所有不同级别的菜单选项内容实现，无法做到很好的解耦方式；
>
> 该模块通过动态绑定子菜单和链表的方式可以达到较好的解耦状态

### 显示效果

该框架只负责菜单选项控制操作，不负责在图像界面显示效果，需要在对应的回调函数中实现菜单显示效果。

> 设置对应的效果显示函数，即可为不同的菜单设置不同的菜单显示效果，比如图标式、列表式或右侧弹窗式等。
>
> 可以在不同显示平台体现，比如LCD、OLED或终端界面等。

### 多语种

没有直接的语种设置，而是通过联合体提供选择多种选择。

> 文本ID: 适用于多语种采用数组定义的翻译内容
> 字符串: 适用于单语种或者多语种，采用映射的方式实现的翻译内容

### 可扩展

每级菜单选项都可以设置自定义数据，用来实现更多的菜单操作或者显示效果等。

> 不同级别的菜单可以设置自定义数据（比如菜单选项隐藏/图标数据等）

### 可配置

| 配置选项                    | 描述                                                          |
| --------------------------- | ------------------------------------------------------------- |
| `_COT_MENU_USE_MALLOC_`   | 定义则采用 malloc/free 的方式实现多级菜单, 否则通过数组的形式 |
| `_COT_MENU_USE_SHORTCUT_` | 定义则启用快捷菜单选项进入功能                                |
| `COT_MENU_MAX_DEPTH`      | 多级菜单深度                                                  |
| `COT_MENU_MAX_NUM`        | 菜单支持的最大选项数目                                        |

### 功能多样化

* [X] 支持快速进入指定菜单界面。

  > - 可以通过相对选项索引或者绝对选项索引路径实现
  >
* [X] 可以实现有限界面内显示少量的菜单选项内容。

  > - 有现成的函数可用，无需担心使用不同尺寸重新实现菜单选项部分可见
  >

## 软件设计

略

## 使用说明

### 菜单初始化和使用

```c

// 定义菜单信息，函数由主菜单模块定义并提供
static cotMainMenuCfg_t sg_tMainMenu = {"主菜单", Hmi_EnterMainHmi, NULL, NULL, NULL};

int main(void)
{
    cotMenu_Init(&sg_tMainMenu);

    while (1)
    {
        ...

        if (timeFlag)
        {
            timeFlag = 0;
            cotMenu_Task(); // 周期调度
        }
    }
}

```

### 主菜单定义和绑定

定义一个主菜单选项内容、主菜单显示效果函数和主菜单进入函数等

```c

// 扩展数据为图标文件名字，每个菜单选项描述为文本ID
cotMenuList_t sg_MainMenuTable[] = 
{
    COT_MENU_ITEM_BIND(TEXT_MUSIC,  Hmi_MusicEnter, Hmi_MusicExit, Hmi_MusicLoad, Hmi_MusicTask, (MenuImage_t *)&sgc_MusicImage),
  
    COT_MENU_ITEM_BIND(TEXT_VIDEO,  NULL, Hmi_VideoExit, Hmi_VideoLoad, Hmi_VideoTask, (MenuImage_t *)&sgc_VideoImage),
  
    COT_MENU_ITEM_BIND(TEXT_CAMERA,  Hmi_CameraEnter, Hmi_CameraExit, Hmi_CameraLoad, Hmi_CameraTask, (MenuImage_t *)&sgc_CameraImage),
  
    COT_MENU_ITEM_BIND(TEXT_SETTING,  Hmi_SetEnter, Hmi_SetExit, Hmi_SetLoad,   Hmi_SetTask, (MenuImage_t *)&sgc_SettingImage),
};

/* 主菜单显示效果 */
static void ShowMainMenu(cotMenuShow_t *ptShowInfo)
{
    char *pszSelectDesc = get_text(ptShowInfo->pszItemsDesc[ptShowInfo->selectItem].textId); // 根据文本ID找到对应的字符串（多语言）
    oledsize_t idx = (128 - 6 * strlen(pszSelectDesc)) / 2;

    cotOled_DrawGraphic(40, 0, (const char *)ptShowInfo->pItemsListExtendData[ptShowInfo->selectItem], 1);
  
    cotOled_SetText(0, 50, "                ", 0, FONT_12X12);
    cotOled_SetText(idx, 50, pszSelectDesc, 0, FONT_12X12);
}

void Hmi_EnterMainHmi(const cotMenuItemInfo_t *pItemInfo)
{
    cotMenu_Bind(sg_MainMenuTable, COT_GET_MENU_NUM(sg_MainMenuTable), ShowMainMenu);
}

```

### 子菜单定义和绑定

如果菜单选项有子菜单，则该菜单选项调用 `cotMenu_Enter`，进入回调函数**不能为NULL**，且该回调函数需调用 `cotMenu_Bind`进行绑定

```c

/* 设置的子菜单内容，每个菜单选项描述为字符串 */
cotMenuList_t sg_SetMenuTable[] = 
{
    COT_MENU_ITEM_BIND("语言", NULL, NULL, NULL, OnLanguageFunction, NULL),
    COT_MENU_ITEM_BIND("蓝牙", NULL, NULL, NULL, OnBluetoothFunction, NULL),
    COT_MENU_ITEM_BIND("电池", NULL, NULL, NULL, OnBatteryFunction, NULL),
    COT_MENU_ITEM_BIND("储存", NULL, NULL, NULL, OnStorageFunction, NULL),
    COT_MENU_ITEM_BIND("更多", Hmi_MoreSetEnter, Hmi_MoreSetExit, Hmi_MoreSetLoad, Hmi_MoreSetTask, NULL),
};

/* 设置菜单显示效果 */
static void ShowSetMenu(cotMenuShow_t *ptShowInfo)
{
    uint8_t showNum = 3;
    menusize_t  tmpselect;

    cotMenu_LimitShowListNum(ptShowInfo, &showNum);

    // 这里直接使用字符串
    printf("\e[0;30;46m ------------- %s ------------- \e[0m\n", tParentMenuShowInfo.uMenuDesc.pTextString);

    for (int i = 0; i < showNum; i++)
    {
        tmpselect = i + ptShowInfo->showBaseItem;

        if (tmpselect == ptShowInfo->selectItem)
        {
            printf("\e[0;30;47m %d. %-16s\e[0m |\n", tmpselect + 1, ptShowInfo->pszItemsDesc[tmpselect].pTextString);
        }
        else
        {
            printf("\e[7;30;47m %d. %-16s\e[0m |\n", tmpselect + 1, ptShowInfo->pszItemsDesc[tmpselect].pTextString);
        }
    }
}

void Hmi_SetEnter(const cotMenuItemInfo_t *pItemInfo)
{
    // 进入设置选项后绑定子菜单，同时为当前绑定的菜单设置显示效果函数
    cotMenu_Bind(sg_SetMenuTable, COT_GET_MENU_NUM(sg_SetMenuTable), ShowSetMenu); 
}

```

### 菜单控制

通过调用相关函数实现菜单选项选择、进入、退出等

```c

// 需要先进入主菜单
cotMenu_MainEnter();

// 选择上一个，支持循环选择（即第一个可跳转到最后一个）
cotMenu_SelectPrevious(true);

// 选择下一个，不支持循环选择（即最后一个不可跳转到第一个）
cotMenu_SelectNext(false);

// 进入，会执行菜单选项的 pfnEnterCallFun 回调函数
cotMenu_Enter();

// 退出，会执行父菜单该选项的 pfnExitCallFun 回调函数，并在退出后父菜单选项列表复位从头选择
cotMenu_Exit(true);

```

定义了多个菜单选项表后，用来区分上下级，在某个菜单选项进入时绑定

1. 使用前初始化函数 cotMenu_Init, 设置主菜单内容
2. 周期调用函数 cotMenu_Task, 用来处理菜单显示和执行相关回调函数
3. 使用其他函数对菜单界面控制

## demo样式

博客：

[轻量级多级菜单控制框架程序（C语言）](https://blog.csdn.net/qq_24130227/article/details/121167276?csdn_share_tail=%7B%22type%22%3A%22blog%22%2C%22rType%22%3A%22article%22%2C%22rId%22%3A%22121167276%22%2C%22source%22%3A%22qq_24130227%22%7D&ctrtid=VbyfV)

命令行效果图：

![](https://img-blog.csdnimg.cn/22d1476746f64b82ae8a614a47d9d7de.gif)

STM32 + OLED 效果图：

代码链接：[stm32 菜单效果demo](https://gitee.com/cot_package/demo_stm32)

![](https://img-blog.csdnimg.cn/721e44b87f634c7e9aabe3191a3876a1.gif)

## 关于作者

1. CSDN 博客 [大橙子疯](https://blog.csdn.net/qq_24130227?spm=1010.2135.3001.5343)
2. 联系邮箱 const_zpc@163.com
3. 了解更多可关注微信公众号

![大橙子疯嵌入式](微信公众号.jpg)
