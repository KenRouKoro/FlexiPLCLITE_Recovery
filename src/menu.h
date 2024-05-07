//
// Created by koro on 24-4-26.
//
#pragma once

#include "U8g2lib.h"

//动画长度，默认30帧一次
#define TIME_LENGTH 30

enum BTN_Type {
    UP,
    DOWN,
    OK,
    CANCEL
};

int easeInOut(int t);

extern U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R2);
using CallbackType = std::function<const char *()>;
using BTNCallbackType = std::function<void(BTN_Type)>;

struct Menu {
    CallbackType formate_callback = nullptr;
    const char *menu_str = nullptr;
    Menu *children_menu = nullptr;
    Menu *father_menu = nullptr;
    BTNCallbackType btn_callback = nullptr;
    Menu *last_menu = nullptr;
    Menu *next_menu = nullptr;
};

static int halfTime = TIME_LENGTH / 2;

static Menu *nowMenu = nullptr;
static int nowIndex = 0;

static short base_y = 0;
static short base_x = 0;
static bool renderUP = false;
static bool renderDOWN = false;
static bool renderLEFT = false;
static bool renderRIGHT = false;
static bool inAnimation = false;
static BTN_Type animationType = UP;
static int NowAnimationTick = 0;

void setMenu(Menu *m) {
    nowMenu = m;
}

void startAnimation(BTN_Type animation_type) {
    if (inAnimation == true)return;
    animationType = animation_type;
    if (animationType == UP) {
        renderDOWN = true;
        renderUP = true;
    }
    if (animationType == DOWN) {
        renderUP = true;
        renderDOWN = true;
    }
    if (animationType == CANCEL) {
        if (nowMenu->father_menu == nullptr)return;
        renderLEFT = true;
    }
    if (animationType == OK) {
        if (nowMenu->children_menu == nullptr)return;
        renderRIGHT = true;
    }
    NowAnimationTick = 0;
    inAnimation = true;
}
//动画逻辑tick，建议间隔16.7ms执行一次
void animationTick() {
    if (!inAnimation)return;
    //base_y min = 0,max = 32
    if (animationType == UP) {
        base_y = -(32 * easeInOut(NowAnimationTick)) / TIME_LENGTH;
    } else if (animationType == DOWN) {
        base_y = (32 * easeInOut(NowAnimationTick)) / TIME_LENGTH;
    }
    if (animationType == OK) {
        base_x = -(128 * easeInOut(NowAnimationTick)) / TIME_LENGTH;
    }
    if (animationType == CANCEL) {
        base_x = (128 * easeInOut(NowAnimationTick)) / TIME_LENGTH;
    }


    if (NowAnimationTick >= TIME_LENGTH) {
        inAnimation = false;
        NowAnimationTick = 0;
        renderUP = false;
        renderDOWN = false;
        renderLEFT = false;
        renderRIGHT = false;
        base_y = 0;
        base_x = 0;
        if (animationType == UP) {
            nowMenu = nowMenu->last_menu;
        } else if (animationType == DOWN) {
            nowMenu = nowMenu->next_menu;
        } else if (animationType == CANCEL) {
            nowMenu = nowMenu->father_menu;
        } else if (animationType == OK) {
            nowMenu = nowMenu->children_menu;
        }
    }
    NowAnimationTick++;
}
//在loop中调用，优先级最低不影响其他逻辑
void renderTick() {
    if (nowMenu == nullptr) return;
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_fh);
    //u8g2.setCursor(0, 30);
    //u8g2.printf("%d,%d,%d", base_x, base_y, inAnimation);

    // Render current menu
    u8g2.setCursor(base_x, base_y + 14);
    u8g2.print(nowMenu[nowIndex].menu_str);
    if (nowMenu[nowIndex].formate_callback != nullptr) {
        u8g2.setCursor(base_x, base_y + 30);
        u8g2.print(nowMenu[nowIndex].formate_callback());
    }

    /**
 * 渲染左右额外菜单
 **/
    if (renderLEFT) {
        if (nowMenu->father_menu != nullptr) {
            Menu *render_menu = nowMenu->father_menu;
            u8g2.setCursor(base_x - 128, 14);
            u8g2.print(render_menu->menu_str);
            if (render_menu->formate_callback != nullptr) {
                u8g2.setCursor(base_x - 128, base_y + 30);
                u8g2.print((render_menu->formate_callback()));
            }
        }
    }

    if (renderRIGHT) {
        if (nowMenu->children_menu != nullptr) {
            Menu *render_menu = nowMenu->children_menu;
            u8g2.setCursor(base_x + 128, 14);
            u8g2.print(render_menu->menu_str);
            if (render_menu->formate_callback != nullptr) {
                u8g2.setCursor(base_x + 128, base_y + 30);
                u8g2.print((render_menu->formate_callback()));
            }
        }
    }

    // Render next and last menu
    if (renderUP && (nowMenu->next_menu != nullptr)) {
        Menu *render_menu = nowMenu->next_menu;
        u8g2.setCursor(0, base_y + 14 - 32);
        u8g2.print(render_menu->menu_str);
        if (render_menu->formate_callback != nullptr) {
            u8g2.setCursor(0, base_y + 30 - 32);
            u8g2.print((render_menu->formate_callback()));
        }
    }
    /**
     * 渲染底部额外菜单
     **/
    if (renderDOWN && (nowMenu->last_menu != nullptr)) {
        Menu *render_menu = nowMenu->last_menu;
        u8g2.setCursor(0, base_y + 14 + 32);
        u8g2.print(render_menu->menu_str);
        if (render_menu->formate_callback != nullptr) {
            u8g2.setCursor(0, base_y + 30 + 32);
            u8g2.print((render_menu->formate_callback()));
        }
    }


    u8g2.sendBuffer();
}

void onClickOK() {
    if (nowMenu != nullptr && nowMenu->btn_callback != nullptr)nowMenu->btn_callback(OK);
    if (nowMenu->children_menu == nullptr) return;
    startAnimation(OK);
}

void onClickCANCEL() {
    if (nowMenu != nullptr && nowMenu->btn_callback != nullptr)nowMenu->btn_callback(CANCEL);
    if (nowMenu->father_menu == nullptr) return;
    startAnimation(CANCEL);
}

void onClickUP() {
    if (nowMenu != nullptr && nowMenu->btn_callback != nullptr)nowMenu->btn_callback(UP);
    if (nowMenu->last_menu == nullptr) return;
    startAnimation(UP);
}

void onClickDOWN() {
    if (nowMenu != nullptr && nowMenu->btn_callback != nullptr)nowMenu->btn_callback(DOWN);
    if (nowMenu->next_menu == nullptr) return;
    startAnimation(DOWN);
}

int easeInOut(int t) {
    int tSquared;
    int result;

    if (t < halfTime) {
        tSquared = t * t;
        result = tSquared / halfTime;
    } else {
        t = TIME_LENGTH - t;
        tSquared = t * t;
        result = TIME_LENGTH - (tSquared / halfTime);
    }

    return result;
}

// 动态创建一个新的菜单项
Menu *createMenu(const char *str) {
    Menu *m = new Menu;
    m->menu_str = str;
    return m;
}

// 创建一个双向循环链表
void createCircularMenuChain(Menu *menus[], int count) {
    if (count < 2) return; // 需要至少两个菜单来形成循环

    for (int i = 0; i < count; ++i) {
        menus[i]->next_menu = menus[(i + 1) % count]; // 循环链接
        menus[(i + 1) % count]->last_menu = menus[i]; // 设置上一个菜单
    }
}

// 为父菜单创建并绑定子菜单
Menu **bindChildrenToMenu(Menu *parentMenu, const char **childrenStrs, int numChildren) {
    if (parentMenu == nullptr || childrenStrs == nullptr) return nullptr;


    Menu **childrenMenus = new Menu *[numChildren];
    for (int i = 0; i < numChildren; i++) {
        childrenMenus[i] = createMenu(childrenStrs[i]);
        childrenMenus[i]->father_menu = parentMenu;
    }

    createCircularMenuChain(childrenMenus, numChildren);

    parentMenu->children_menu = *childrenMenus;

    return childrenMenus;
}
