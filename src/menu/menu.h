#ifndef MENU_H
#define MENU_H

#include <Arduino.h>
#include "U8g2lib.h"
#include <vector>
#include <variant>
#include <functional>
#include <string>

#include "FH_Font.h"

// 定义屏幕宽度和高度
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
//定义屏幕关闭时间
//#define CLOSE_SCREEN_COUNT ( 30 * 10 )
//定义绘制起始位置
#define BASE_X 2

extern int CLOSE_SCREEN_COUNT;

// 前向声明MenuItem结构体
struct MenuItem;

// 枚举定义菜单项类型
enum MenuType { 
    SUBMENU,  // 子菜单类型
    SWITCH,   // 开关类型
    NUMBER,   // 数字类型
    CUSTOM,   // 自定义类型
    INT,      // 整数类型
    STRING,   // 字符串类型
    BUTTON    // 新增按钮类型
};

// 子菜单数据结构
struct SubMenuData {
    MenuItem** subMenu;  // 子菜单项指针数组
    int count;           // 子菜单项数量
};

// 开关数据结构
struct SwitchData {
    std::function<bool()> getter;  // 获取开关状态的函数
    std::function<void(bool)> setter;  // 设置开关状态的函数
};

// 数字数据结构
struct NumberData {
    std::function<float()> getter;  // 获取数值的函数
    std::function<void(float)> setter;  // 设置数值的函数
    float min, max, step, decimal_places;  // 最小值、最大值、步长和小数位数
    String unit;
};

// 自定义渲染数据结构
struct CustomData {
    std::function<void()> customRender;  // 自定义渲染函数
};

// 整数数据结构
struct IntData {
    std::function<int()> getter;  // 获取整数值的函数
    std::function<void(int)> setter;  // 设置整数值的函数
    int min, max, step;  // 最小值、最大值和步长
    String unit;
};

// 字符串数据结构
struct StringData {
    std::function<String()> getter;  // 获取字符串值的函数
};

// 按钮类型数据结构
struct ButtonData {
    std::function<void()> callback;  // 回调函数
};

// 菜单项结构体
struct MenuItem {
    MenuType type;  // 菜单项类型
    String name;    // 菜单项名称
    String description;  // 菜单项描述
    std::variant<SubMenuData, SwitchData, NumberData, CustomData, IntData, StringData, ButtonData> data;  // 菜单项数据

    ~MenuItem() {
        // 如果是子菜单类型，释放子菜单内存
        if (std::holds_alternative<SubMenuData>(data)) {
            auto& subMenuData = std::get<SubMenuData>(data);
            delete[] subMenuData.subMenu;
        }
    }
};

// 菜单类
class Menu {
private:
    // 菜单层级结构
    struct MenuLevel {
        std::vector<MenuItem*> items;  // 当前层级的菜单项
        int currentItem;               // 当前选中的菜单项索引
    };

    std::vector<MenuLevel> levels;  // 菜单层级列表
    int currentLevelIndex = 0;      // 当前层级索引

    bool upPressed = false;         // 上键是否按下
    bool downPressed = false;       // 下键是否按下
    bool confirmPressed = false;    // 确认键是否按下
    bool backPressed = false;       // 返回键是否按下

    bool editMode = false;          // 是否处于编辑模式
    int editItemIndex = -1;         // 当前编辑的菜单项索引
    float editValue = 0.0;          // 当前编辑的值
    bool editBool = false;
    int closeScreenCount = 0;

public:
    // 按键事件处理函数
    void pressUp() {
        closeScreenCount = 0;
        upPressed = true;
    }  // 上键按下
    void pressDown() {
        closeScreenCount = 0;
        downPressed = true;
    }  // 下键按下
    void pressConfirm() {
        closeScreenCount = 0;
        confirmPressed = true;
    }  // 确认键按下
    void pressBack() {
        closeScreenCount = 0;
        backPressed = true;
    }  // 返回键按下

    // 构造函数，初始化菜单
    Menu(MenuItem** initialItems, int count) {
        MenuLevel level;
        for (int i = 0; i < count; i++) level.items.push_back(initialItems[i]);
        level.currentItem = 0;
        levels.push_back(level);
    }

    // 析构函数，释放菜单项内存
    ~Menu() {
        for (auto& level : levels) {
            for (auto item : level.items) {
                delete item;
            }
        }
    }

    // 创建子菜单项
    static MenuItem* createSubMenu(String name, String description, MenuItem** subItems, int count) {
        MenuItem* item = new MenuItem();
        item->type = SUBMENU;
        item->name = name;
        item->description = description;

        SubMenuData data;
        data.subMenu = new MenuItem*[count];
        data.count = count;
        for (int i = 0; i < count; i++) {
            data.subMenu[i] = subItems[i];
        }
        item->data = data;

        return item;
    }

    // 创建开关菜单项
    static MenuItem* createSwitch(String name, std::function<bool()> getter, std::function<void(bool)> setter) {
        MenuItem* item = new MenuItem();
        item->type = SWITCH;
        item->name = name;
        item->description = "";

        SwitchData data;
        data.getter = getter;
        data.setter = setter;
        item->data = data;

        return item;
    }

    // 创建数字菜单项
    static MenuItem* createNumber(String name, std::function<float()> getter, std::function<void(float)> setter, float min, float max, float step ,int decimal_places) {
        MenuItem* item = new MenuItem();
        item->type = NUMBER;
        item->name = name;
        item->description = "";

        NumberData data;
        data.getter = getter;
        data.setter = setter;
        data.min = min;
        data.max = max;
        data.step = step;
        data.decimal_places = decimal_places;
        data.unit = "";
        item->data = data;

        return item;
    }
    // 创建数字菜单项(包括单位)
    static MenuItem* createNumber(String name, std::function<float()> getter, std::function<void(float)> setter, float min, float max, float step ,int decimal_places,String unit) {
        MenuItem* item = new MenuItem();
        item->type = NUMBER;
        item->name = name;
        item->description = "";

        NumberData data;
        data.getter = getter;
        data.setter = setter;
        data.min = min;
        data.max = max;
        data.step = step;
        data.decimal_places = decimal_places;
        data.unit = unit;
        item->data = data;

        return item;
    }

    // 创建自定义菜单项
    static MenuItem* createCustom(String name, std::function<void()> customRender) {
        MenuItem* item = new MenuItem();
        item->type = CUSTOM;
        item->name = name;
        item->description = "";

        CustomData data;
        data.customRender = customRender;
        item->data = data;

        return item;
    }

    // 创建整数菜单项
    static MenuItem* createInt(String name, std::function<int()> getter, std::function<void(int)> setter, int min, int max, int step) {
        MenuItem* item = new MenuItem();
        item->type = INT;
        item->name = name;
        item->description = "";

        IntData data;
        data.getter = getter;
        data.setter = setter;
        data.min = min;
        data.max = max;
        data.step = step;
        data.unit = "";
        item->data = data;

        return item;
    }
    // 创建整数菜单项(包括单位)
    static MenuItem* createInt(String name, std::function<int()> getter, std::function<void(int)> setter, int min, int max, int step,String unit) {
        MenuItem* item = new MenuItem();
        item->type = INT;
        item->name = name;
        item->description = "";

        IntData data;
        data.getter = getter;
        data.setter = setter;
        data.min = min;
        data.max = max;
        data.step = step;
        data.unit = unit;
        item->data = data;

        return item;
    }

    // 创建字符串菜单项
    static MenuItem* createString(String name, std::function<String()> getter) {
        MenuItem* item = new MenuItem();
        item->type = STRING;
        item->name = name;
        item->description = "";

        StringData data;
        data.getter = getter;
        item->data = data;

        return item;
    }

    // 创建固定字符串菜单项
    static MenuItem* createString(String name, const String& value) {
        MenuItem* item = new MenuItem();
        item->type = STRING;
        item->name = name;
        item->description = "";

        StringData data;
        data.getter = [value]()->auto{return value;};
        item->data = data;

        return item;
    }

    // 新增按钮类型创建函数
    static MenuItem* createButton(String name, std::function<void()> callback) {
        MenuItem* item = new MenuItem();
        item->type = BUTTON;
        item->name = name;
        item->description = "";  // 固定第二行文字

        ButtonData data;
        data.callback = callback;
        item->data = data;

        return item;
    }

    // 处理子菜单逻辑
    void handleSubMenu(MenuItem* item) {
        auto& subMenuData = std::get<SubMenuData>(item->data);
        MenuLevel newLevel;
        newLevel.items = std::vector<MenuItem*>(subMenuData.subMenu, subMenuData.subMenu + subMenuData.count);
        newLevel.currentItem = 0;
        levels.push_back(newLevel);
        currentLevelIndex = levels.size() - 1;
    }

    // 逻辑更新函数
    void logic_tick() {
        if (closeScreenCount<CLOSE_SCREEN_COUNT) {
            closeScreenCount++;
        }
        if (editMode) {
            auto& item = levels[currentLevelIndex].items[editItemIndex];
            switch (item->type) {
                case NUMBER: {
                    auto& numberData = std::get<NumberData>(item->data);
                    if (upPressed) {
                        editValue += numberData.step;
                        if (editValue > numberData.max) editValue = numberData.max;
                        //numberData.setter(editValue);
                        upPressed = false;
                    }
                    if (downPressed) {
                        editValue -= numberData.step;
                        if (editValue < numberData.min) editValue = numberData.min;
                        //numberData.setter(editValue);
                        downPressed = false;
                    }
                    if (confirmPressed) {
                        numberData.setter(editValue);
                        editMode = false;
                        editItemIndex = -1;
                        confirmPressed = false;
                    }
                    if (backPressed) {
                        editMode = false;
                        editItemIndex = -1;
                        backPressed = false;
                    }
                    break;
                }
                case INT: {
                    auto& intData = std::get<IntData>(item->data);
                    if (upPressed) {
                        editValue += intData.step;
                        if (editValue > intData.max) editValue = intData.max;
                        //intData.setter(static_cast<int>(editValue));
                        upPressed = false;
                    }
                    if (downPressed) {
                        editValue -= intData.step;
                        if (editValue < intData.min) editValue = intData.min;
                        //intData.setter(static_cast<int>(editValue));
                        downPressed = false;
                    }
                    if (confirmPressed) {
                        intData.setter(static_cast<int>(editValue));
                        editMode = false;
                        editItemIndex = -1;
                        confirmPressed = false;
                    }
                    if (backPressed) {
                        editMode = false;
                        editItemIndex = -1;
                        backPressed = false;
                    }
                    break;
                }
                case SWITCH: {
                    auto& switchData = std::get<SwitchData>(item->data);
                    if (upPressed) {
                        //editValue += intData.step;
                        editBool = !editBool;
                        //if (editValue > intData.max) editValue = intData.max;
                        //intData.setter(static_cast<int>(editValue));
                        upPressed = false;
                    }
                    if (downPressed) {
                        //editValue -= intData.step;
                        editBool = !editBool;
                        //if (editValue < intData.min) editValue = intData.min;
                        //intData.setter(static_cast<int>(editValue));
                        downPressed = false;
                    }
                    if (confirmPressed) {
                        switchData.setter(editBool);
                        editMode = false;
                        editItemIndex = -1;
                        confirmPressed = false;
                    }
                    if (backPressed) {
                        editMode = false;
                        editItemIndex = -1;
                        backPressed = false;
                    }
                    break;
                }
            }
        } else {
            if (upPressed) {
                levels[currentLevelIndex].currentItem--;
                if (levels[currentLevelIndex].currentItem < 0) levels[currentLevelIndex].currentItem = levels[currentLevelIndex].items.size() - 1;
                upPressed = false;
            }
            if (downPressed) {
                levels[currentLevelIndex].currentItem++;
                if (levels[currentLevelIndex].currentItem >= levels[currentLevelIndex].items.size()) levels[currentLevelIndex].currentItem = 0;
                downPressed = false;
            }

            if (confirmPressed) {
                auto& item = levels[currentLevelIndex].items[levels[currentLevelIndex].currentItem];
                switch (item->type) {
                    case SUBMENU: {
                        handleSubMenu(item);
                        break;
                    }
                    case SWITCH: {
                        editMode = true;
                        //auto& switchData = std::get<SwitchData>(item->data);
                        //switchData.setter(!switchData.getter());
                        editItemIndex = levels[currentLevelIndex].currentItem;
                        editBool = std::get<SwitchData>(item->data).getter();
                        break;
                    }
                    case NUMBER: {
                        editMode = true;
                        editItemIndex = levels[currentLevelIndex].currentItem;
                        editValue = std::get<NumberData>(item->data).getter();
                        break;
                    }
                    case INT: {
                        editMode = true;
                        editItemIndex = levels[currentLevelIndex].currentItem;
                        editValue = static_cast<float>(std::get<IntData>(item->data).getter());
                        break;
                    }
                    case CUSTOM:
                        break;
                    case STRING:
                        break;
                    case BUTTON: {  // 新增按钮类型处理
                        auto& buttonData = std::get<ButtonData>(item->data);
                        buttonData.callback();
                        break;
                    }
                }
                confirmPressed = false;
            }

            if (backPressed) {
                if (levels.size() > 1) {
                    levels.pop_back();
                    currentLevelIndex = levels.size() - 1;
                }
                backPressed = false;
            }
        }
    }

    const uint8_t lookup_table[29] = {
        0,   // 索引0无效
        28,  // 28/1
        14,  // 28/2
        9,   // 28/3
        7,   // 28/4
        5,   // 28/5
        4,   // 28/6
        4,   // 28/7
        3,   // 28/8
        3,   // 28/9
        2,   // 28/10
        2,   // 28/11
        2,   // 28/12
        2,   // 28/13
        1,   // 28/14
        1,   // 28/15
        1,   // 28/16
        1,   // 28/17
        1,   // 28/18
        1,   // 28/19
        1,   // 28/20
        1,   // 28/21
        1,   // 28/22
        1,   // 28/23
        1,   // 28/24
        1,   // 28/25
        1,   // 28/26
        1    // 28/27 或 28/28（均为1）
        };

    // 渲染更新函数，负责在屏幕上绘制当前菜单项的内容
    void render_tick(u8g2_t* u8g2) {
        static bool lastStatus = false;
        if (lastStatus != (closeScreenCount >= CLOSE_SCREEN_COUNT) ) {
            u8g2_InitDisplay(u8g2);
        }
        if (closeScreenCount >= CLOSE_SCREEN_COUNT) {
            u8g2_SetPowerSave(u8g2, true);
            lastStatus = true;
        }else {
            lastStatus = false;
            u8g2_SetPowerSave(u8g2, false);
        }
        // 清除屏幕缓冲区，准备绘制新内容
        u8g2_ClearBuffer(u8g2);
        u8g2_SetFontMode(u8g2, 0);  // 设置字体模式为正常模式
        u8g2_SetFont(u8g2, u8g2_font_fh);  // 设置适合的字体
        u8g2_SetDrawColor(u8g2, 1);  // 设置绘制颜色为白色

        // 获取当前菜单层级和选中的菜单项
        auto& current = levels[currentLevelIndex];
        auto& item = current.items[current.currentItem];

        // 绘制当前菜单项的名称
        u8g2_DrawUTF8(u8g2, BASE_X, 14, item->name.c_str());
        uint8_t str_width =  14 + 14 + 7;//u8g2_GetStrWidth(u8g2,"编辑: ");

        // 根据菜单项类型绘制不同的内容
        switch (item->type) {
            case SUBMENU: {
                // 子菜单类型：显示提示信息"Enter"
                u8g2_SetDrawColor(u8g2, 1);
                u8g2_DrawBox(u8g2, 95, 17, 28, 16);
                u8g2_SetDrawColor(u8g2, 0);
                u8g2_DrawUTF8(u8g2, 96, 29, "-->");
                u8g2_SetDrawColor(u8g2, 1);
                break;
            }
            case SWITCH: {
                // 开关类型：显示开关的当前状态（On/Off）
                auto& switchData = std::get<SwitchData>(item->data);
                u8g2_SetDrawColor(u8g2, 1);
                if (editMode) {
                    u8g2_DrawBox(u8g2, 0, 17, str_width , 16);
                    u8g2_SetDrawColor(u8g2, 0);
                    u8g2_DrawUTF8(u8g2, BASE_X , 29, "编辑: ");
                    u8g2_SetDrawColor(u8g2, 1);
                    u8g2_DrawUTF8(u8g2, BASE_X + str_width , 29, editBool ? "开" : "关");
                }else {
                    u8g2_DrawUTF8(u8g2, BASE_X , 29, switchData.getter() ? "开" : "关");
                }
                u8g2_SetDrawColor(u8g2, 1);
                break;
            }
            case NUMBER: {
                // 数字类型：显示当前数值，编辑模式下前缀显示"Edit: "
                auto& numberData = std::get<NumberData>(item->data);
                u8g2_SetDrawColor(u8g2, 1);
                uint8_t draw_base_x = 0;
                if (editMode) {
                    draw_base_x = str_width;
                    u8g2_DrawBox(u8g2, 0, 17, str_width, 16);
                    u8g2_SetDrawColor(u8g2, 0);
                    u8g2_DrawUTF8(u8g2, BASE_X , 29, "编辑: ");
                    u8g2_SetDrawColor(u8g2, 1);
                }
                u8g2_DrawUTF8(u8g2, BASE_X + draw_base_x, 29,
                     (String(editMode ? editValue:numberData.getter(),
                         numberData.decimal_places) + numberData.unit).c_str());
                u8g2_SetDrawColor(u8g2, 1);
                break;
            }
            case INT: {
                // 整数类型：显示当前整数值，编辑模式下前缀显示"Edit: "
                auto& intData = std::get<IntData>(item->data);
                int currentValue = intData.getter();
                u8g2_SetDrawColor(u8g2, 1);
                uint8_t draw_base_x = 0;
                if (editMode) {
                    draw_base_x = str_width;
                    u8g2_DrawBox(u8g2, 0, 17, str_width, 16);
                    u8g2_SetDrawColor(u8g2, 0);
                    u8g2_DrawUTF8(u8g2, BASE_X , 29, "编辑: ");
                    u8g2_SetDrawColor(u8g2, 1);
                }
                u8g2_DrawUTF8(u8g2, BASE_X + draw_base_x, 29,
                     (String(editMode?editValue :  static_cast<float>(currentValue),0) + intData.unit).c_str());
                u8g2_SetDrawColor(u8g2, 1);
                break;
            }
            case CUSTOM: {
                // 自定义类型：调用自定义渲染函数绘制内容
                u8g2_SetDrawColor(u8g2, 1);
                auto& customData = std::get<CustomData>(item->data);
                customData.customRender();
                break;
            }
            case STRING: {
                // 字符串类型：显示字符串值
                u8g2_SetDrawColor(u8g2, 1);
                auto& stringData = std::get<StringData>(item->data);
                String value = stringData.getter();
                u8g2_DrawUTF8(u8g2, BASE_X, 29, value.c_str());
                break;
            }
            case BUTTON: {  // 新增按钮类型渲染
                u8g2_SetDrawColor(u8g2, 1);
                u8g2_DrawBox(u8g2, 49, 17, 32, 32);
                u8g2_SetDrawColor(u8g2, 0);
                u8g2_DrawUTF8(u8g2, 52, 29, "确认");
                u8g2_SetDrawColor(u8g2, 1);
                break;
            }
        }

        //渲染边框
        u8g2_DrawFrame(u8g2, 0,0,128,32);
        u8g2_DrawLine(u8g2, 0, 16, 128, 16);

        if (current.items.size() > 1) {
            //绘制右侧滚动条
            uint8_t select_index = current.currentItem;
            size_t select_size = current.items.size();
            static size_t last_size = 0;
            static float once_height = 1;

            if (select_size != last_size) {
                last_size = select_size;
                once_height = static_cast<float>(28) / static_cast<float>(select_size);//lookup_table[select_size];
            }

            // 后续计算保持不变
            auto draw_y_up = static_cast<uint8_t>(once_height * static_cast<float>(select_index) + 2.0);
            auto draw_y_down = static_cast<uint8_t>(once_height * (select_index + 1.0) + 2);

            u8g2_DrawBox(u8g2, 125, 0, 3, 32);
            u8g2_SetDrawColor(u8g2, 0);
            u8g2_DrawLine(u8g2, 126, draw_y_up, 126, draw_y_down);
        }

        // 将缓冲区内容发送到屏幕进行显示
        u8g2_SendBuffer(u8g2);
    }
};

#endif
