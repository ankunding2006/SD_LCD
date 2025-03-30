/**
  **********************************************************************************************************************
  * @file    cot_menu.h
  * @brief   该文件提供菜单框架所有函数原型
  * @author  const_zpc    any question please send mail to const_zpc@163.com
  * @date    2024-06-09
  **********************************************************************************************************************
  *
  **********************************************************************************************************************
  */

/* Define to prevent recursive inclusion -----------------------------------------------------------------------------*/
#ifndef COT_MENU_H
#define COT_MENU_H


/* Includes ----------------------------------------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
 extern "C" {
#endif 


/******************************************* 配置项 ********************************************************************/

/* 定义 _COT_MENU_USE_MALLOC_ 则采用 malloc/free 的方式实现多级菜单, 否则通过数组的形式 */
// #define _COT_MENU_USE_MALLOC_

/* 定义 _COT_MENU_USE_SHORTCUT_ 则启用快捷菜单选项进入功能 */
#define _COT_MENU_USE_SHORTCUT_

/* 多级菜单深度 */
#define COT_MENU_MAX_DEPTH              10

/* 菜单支持的最大选项数目 */
#define COT_MENU_MAX_NUM                20

/******************************************* 配置项 ********************************************************************/


/* Exported types ----------------------------------------------------------------------------------------------------*/

#if COT_MENU_MAX_NUM < 255
typedef uint8_t menusize_t;
#else
typedef uint16_t menusize_t;
#endif

typedef union
{
    void     *pVoid;                   /*!< 通用指针(目的消除编译警告) */
    
    size_t    textId;                  /*!< 文本ID */

    char     *pTextString;             /*!< 文本字符串 */
} cotMenuDsecStr_u;

/**
  * @brief      单个选项信息
  * 
  */
typedef struct
{
    cotMenuDsecStr_u uMenuDesc;         /*!< 该选项的描述 */

    void     *pExtendData;              /*!< 该选项注册时的扩展数据 */
} cotMenuItemInfo_t;

typedef void (*cotMenuCallFun_f)(const cotMenuItemInfo_t *pItemInfo);

/**
  * @brief      单个选项带菜单列表等信息
  * 
  */
typedef struct
{
    menusize_t itemsNum;                /*!< 当前菜单中选项的总数目 */

    menusize_t selectItem;              /*!< 当前菜单中被选中的选项 */

    menusize_t showBaseItem;            /*!< 当前菜单首个显示的选项 */

    cotMenuDsecStr_u uMenuDesc;         /*!< 当前菜单的描述 */

    void     *pExtendData;              /*!< 当前菜单注册时的扩展数据 */

    cotMenuDsecStr_u uItemsListDesc[COT_MENU_MAX_NUM];/*!< 当前菜单中选项列表的描述 */

    void *pItemsListExtendData[COT_MENU_MAX_NUM];   /*!< 当前菜单中选项列表注册时的扩展数据 */
} cotMenuShow_t;

typedef void (*cotShowMenuCallFun_f)(cotMenuShow_t *ptShowInfo);

/**
  * @brief 菜单信息注册结构体
  * 
  */
typedef struct
{
    cotMenuDsecStr_u     uMenuDesc;        /*!< 当前菜单的描述 */

    cotMenuCallFun_f     pfnEnterCallFun;  /*!< 当前菜单选项进入时(从父菜单进入)需要执行一次的函数, 为NULL不执行 */

    cotMenuCallFun_f     pfnExitCallFun;   /*!< 当前菜单选项进入后退出时(退出至父菜单)需要执行一次的函数, 为NULL不执行 */
    
    cotMenuCallFun_f     pfnLoadCallFun;   /*!< 当前菜单选项每次加载时(从父菜单进入或子菜单退出)需要执行一次的函数, 为NULL不执行 */

    cotMenuCallFun_f     pfnRunCallFun;    /*!< 当前菜单选项的周期调度函数 */

    void                *pExtendData;      /*!< 当前选项的菜单显示效果函数扩展数据入参, 可自行设置该内容 */
} cotMenuList_t, cotMainMenuCfg_t;

/* Exported constants ------------------------------------------------------------------------------------------------*/
/* Exported macro ----------------------------------------------------------------------------------------------------*/

/**
  * @brief  单个菜单选项定义
  * 
  * @param  title    菜单选项描述
  * @param  enterFun 当前菜单选项进入时(从父菜单进入)需要执行一次的函数, 为NULL不执行
  * @param  exitFun  当前菜单选项进入后退出时(退出至父菜单)需要执行一次的函数, 为NULL不执行
  * @param  loadFun  当前菜单选项每次加载时(从父菜单进入或子菜单退出)需要执行一次的函数, 为NULL不执行
  * @param  runFun   当前菜单选项的周期调度函数
  * @param  extendData 当前选项的菜单显示效果函数扩展数据入参, 可自行设置该内容
  */
#define COT_MENU_ITEM_BIND(title, enterFun, exitFun, loadFun, runFun, extendData) \
    { \
        .uMenuDesc = {.pVoid = (void *)title}, \
        .pfnEnterCallFun = enterFun, \
        .pfnExitCallFun = exitFun, \
        .pfnLoadCallFun = loadFun, \
        .pfnRunCallFun = runFun, \
        .pExtendData = extendData \
    }

// 计算得到菜单列表元素数目
#define COT_GET_MENU_NUM(X)    (sizeof(X) / sizeof(cotMenuList_t))

/* Exported functions ------------------------------------------------------------------------------------------------*/

/* 菜单初始化和反初始化 */

extern int cotMenu_Init(cotMainMenuCfg_t *pMainMenu);
extern int cotMenu_DeInit(void);

extern int cotMenu_Bind(cotMenuList_t *pMenuList, menusize_t menuNum, cotShowMenuCallFun_f pfnShowMenuFun);

/* 菜单选项显示时需要使用的功能扩展函数 */

extern int cotMenu_LimitShowListNum(cotMenuShow_t *ptMenuShow, menusize_t *pShowNum);
extern int cotMenu_QueryParentMenu(cotMenuShow_t *ptMenuShow, uint8_t level);

/* 菜单操作 */

extern int cotMenu_MainEnter(void);
extern int cotMenu_MainExit(void);

extern int cotMenu_Reset(void);
extern int cotMenu_Enter(void);
extern int cotMenu_Exit(bool isReset);
extern int cotMenu_SelectPrevious(bool isAllowRoll);
extern int cotMenu_SelectNext(bool isAllowRoll);
extern int cotMenu_Select(menusize_t selectItem);

extern int cotMenu_ShortcutEnter(bool isAbsolute, uint8_t deep, ...);

/* 菜单轮询处理任务 */

extern int cotMenu_Task(void);

#ifdef __cplusplus
 }
#endif

#endif // MENU_H
