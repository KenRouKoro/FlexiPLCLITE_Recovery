#include <Arduino.h>
#include "broad/FlexiPLCLITE.h"
#include "U8g2lib.h"
#include "stm32g0xx.h"
#include "tick_scheduler.h"
#include "menu/menu.h"
#include <EEPROM.h>


HardwareTimer timer(TIM3); //逻辑周期定时器
HardwareTimer aimtimer(TIM6); //动画逻辑周期定时器

HardwareSerial *IN_Serial;
HardwareSerial *OUT_Serial;

HardwareSerial S1(RX1,TX1);
HardwareSerial S2(RX2,TX2);
HardwareSerial S3(RX3,TX3);
HardwareSerial S4(RX4,TX4);
struct SaveData {
    uint8_t MAGIC_VALUE = 0xA5;
    uint8_t tag_num = 1; // 标签号
    uint8_t num_485 = 1; // 485站号
    uint8_t relay_num = 12; // 继电器数量
    bool use_switch = false; // 使用两档开关模式
    int long_time1 = 100; // 长延时1
    int short_time1 = 10; // 短延时1
    int long_time2 = 200; // 长延时2
    int short_time2 = 20; // 短延时2
    int close_screen_sec = 30;// 关闭屏幕时间
}nowData;

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R2);

int CLOSE_SCREEN_COUNT = 60;

int use_long_time = 100;
int use_short_time = 10;
int running_tick = 0;

bool switch_status = false;

void tick();

void screenTick();

void initFlashStorage();
bool checkFlashStorage();

void renderStr(const char *string1, const char *string2);

void writeSaveData();

void init_menu();

void init_main();

void close_IRQ();

void main_logic_tick();
void init_main_logic_tick();

MenuItem* settingMenuItem[]={

    Menu::createInt("标签号",
        []()->auto{return nowData.tag_num;},
        [](auto val)->auto {
            nowData.tag_num = val;
        },0,255,1,"号"
    ),
    Menu::createInt("继电器路数(1-12)",
        []()->auto{return nowData.relay_num;},
        [](auto val)->auto {
            nowData.relay_num = val;
        },1,12,1,"路"
    ),
    Menu::createNumber("导通时间1",
        []()->auto {
            return  static_cast<float>(nowData.short_time1) / 10.0;
        },
        [](auto val)->auto {
            nowData.short_time1 = static_cast<int>(val * 10);
        },0.5,120,0.5,1,"秒"
    ),
    Menu::createNumber("关断时间1",
        []()->auto {
            return static_cast<float>(nowData.long_time1) / 10.0;
        },
        [](auto val)->auto {
            nowData.long_time1 = static_cast<int>(val * 10);
        },0.5,120,0.5,1, "秒"
    ),

    Menu::createSwitch("使用两档开关模式",
        []()->auto {
            return nowData.use_switch;
        },
        [](auto val)->auto {
            nowData.use_switch = val;
        }),

    Menu::createNumber("导通时间2",
        []()->auto {
            return static_cast<float>(nowData.short_time2) / 10.0;
        },
        [](auto val)->auto {
            nowData.short_time2 = static_cast<int>(val * 10);
        },0.5,120,0.5,1,"秒"
    ),
    Menu::createNumber("关断时间2",
        []()->auto {
            return static_cast<float>(nowData.long_time2) / 10.0;
        },
        [](auto val)->auto {
            nowData.long_time2 = static_cast<int>(val * 10);
        },0.5,120,0.5,1,"秒"
    ),

    Menu::createInt("屏幕关闭时间",
        []()->auto{return nowData.close_screen_sec;},
        [](auto val)->auto {
            nowData.close_screen_sec = val;
        },2,600,1,"秒"
    ),
    Menu::createButton("保存",[]()->auto {
        register_callback([]()->auto {
            writeSaveData();
            delay(3000);
            HAL_NVIC_SystemReset();
        },5);
    }),
    Menu::createButton("复位",[]()->auto {
        close_IRQ();
        HAL_NVIC_SystemReset();
    }),
    Menu::createButton("恢复出厂设置",[]()->auto {
        register_callback([]()->auto {
            initFlashStorage();
            delay(3000);
            HAL_NVIC_SystemReset();
        },5);
    }),
};
MenuItem* aboutMenuItem[] = {
    Menu::createString("类型: ","智能脉冲控制器"),
    Menu::createString("昊明喷涂设备","版本:1.0-NONE")

};

MenuItem* mainMenuItems[] = {
    Menu::createString("状态",[]()->auto {
        static char str[64];
        static char fstr[10];
        dtostrf( static_cast<float>(running_tick) / 10.0f,5,1,fstr);
        sprintf(str, "%03d %02d %s%1d%sS", nowData.tag_num, nowData.relay_num, nowData.use_switch ? "S" : "N",digitalRead(sw1),fstr);
        return String(str);
    }),
    Menu::createSubMenu("设置","",settingMenuItem,11),
    Menu::createSubMenu("关于","",aboutMenuItem,2),
};

Menu myMenu(mainMenuItems, 3);



void setup() {
    initBSP();
    //Flash_ReadStruct(FLASH_USER_START_ADDR, &nowData);
    nowData = EEPROM.get(0, nowData);
    if (nowData.MAGIC_VALUE != 0xA5) {
        initFlashStorage();
    }

    if (checkFlashStorage()) {
        initFlashStorage();
    }

    CLOSE_SCREEN_COUNT = nowData.close_screen_sec * 30;

    attachInterrupt(digitalPinToInterrupt(BTN_OK), onClickOK, LOW);
    attachInterrupt(digitalPinToInterrupt(BTN_CANCEL), onClickCANCEL, LOW);
    attachInterrupt(digitalPinToInterrupt(BTN_UP), onClickUP, LOW);
    attachInterrupt(digitalPinToInterrupt(BTN_DOWN), onClickDOWN, LOW);

    u8g2.setBusClock(1000000);
    Wire.setClock(1000000);
    u8g2.begin();
    u8g2.enableUTF8Print();


    timer.setPrescaleFactor(6400);
    timer.setOverflow(1001); // 100ms 主周期
    timer.attachInterrupt(tick);

    // 设置通道 2，触发 16.6ms ，60hz//30hz
    aimtimer.setPrescaleFactor(6400);
    aimtimer.setOverflow(334);
    aimtimer.attachInterrupt(screenTick);

    NVIC_SetPriority(TIM3_IRQn, 0);
    NVIC_SetPriority(TIM6_IRQn, 5);

    init_menu();

    S1.begin(115200);
    S2.begin(115200);
    S3.begin(115200);
    S4.begin(115200);

    init_main();
    init_main_logic_tick();

    timer.resume(); // 启动定时器
    aimtimer.resume();
}
uint8_t now_relay = 1;
void change_relay_status() {
    now_relay++;
    if (now_relay >= nowData.relay_num) {
        now_relay = 1;
    }
}

void main_logic_tick() {
    init_main_logic_tick();
    static bool on_short = false;
    running_tick++;

    if (on_short) {
        if (running_tick >= use_short_time) {
            running_tick = 0;
            on_short = false;
            outputRelay(now_relay,false);
            change_relay_status();
        }else {
            outputRelay(now_relay,true);
        }
    }else {
        if (running_tick >= use_long_time) {
            running_tick = 0;
            on_short = true;
            outputRelay(now_relay,true);
            //change_relay_status();
        }else {
            outputRelay(now_relay,false);
        }
    }

}
void init_main_logic_tick() {
    if (!nowData.use_switch) {
        use_long_time = nowData.long_time1;
        use_short_time = nowData.short_time1;
        return;
    }
    if (digitalRead(sw1) == LOW) {
        use_long_time = nowData.long_time2;
        use_short_time = nowData.short_time2;
    }else{
        use_long_time = nowData.long_time1;
        use_short_time = nowData.short_time1;
    }
}


void tick() {
    main_logic_tick();
}

void screenTick() {
    myMenu.logic_tick();
}

void initFlashStorage() {
    nowData.MAGIC_VALUE = 0xA5;
    nowData.tag_num = 1; // 标签号
    nowData.num_485 = 1; // 485站号
    nowData.relay_num = 12; // 继电器数量
    nowData.use_switch = false; // 使用两档开关模式
    nowData.long_time1 = 100; // 长延时1
    nowData.short_time1 = 10; // 短延时1
    nowData.long_time2 = 200; // 长延时2
    nowData.short_time2 = 20; // 短延时2
    nowData.close_screen_sec = 30;// 关闭屏幕时间
    writeSaveData();
}
bool checkFlashStorage() {
    if ((nowData.tag_num < 1) || (nowData.tag_num >255)) {
        return true;
    }
    if ((nowData.relay_num < 1) || (nowData.relay_num >12)) {
        return true;
    }
    if ((nowData.close_screen_sec < 1) || (nowData.close_screen_sec >600)) {
        return true;
    }
    if ((nowData.short_time1 < 5) || (nowData.short_time1 >600)) {
        return true;
    }
    if ((nowData.long_time1 < 5) || (nowData.long_time1 >600)) {
        return true;
    }
    if ((nowData.short_time2 < 5) || (nowData.short_time2 >600)) {
        return true;
    }
    if ((nowData.long_time2 < 5) || (nowData.long_time2 >600)) {
        return true;
    }

    return false;
}

void close_IRQ() {
    aimtimer.pause();
    timer.pause();
    __disable_irq();
}


void writeSaveData() {
    //EEPROM.put(0, nowData);

    //Flash_WriteStruct(FLASH_USER_START_ADDR, &nowData);

    const auto *data = reinterpret_cast<uint8_t *>(&nowData);
    constexpr uint32_t size = sizeof(nowData);
    for (int i = 0; i < size; i++) {
        EEPROM.write(i, data[i]);
    }
}

HardwareSerial getHardwareSerial(int n) {
    switch (n) {
        case 1:
            return S1;
        case 2:
            return S2;
        case 3:
            return S3;
        case 4:
            return S4;
        default:
            return S1;
    }
}

void loop() {
    tick_increment();
    myMenu.render_tick(u8g2.getU8g2());
}

void init_menu() {
}

void onClickOK() {
    myMenu.pressConfirm();
}

void onClickCANCEL() {
    myMenu.pressBack();
}

void onClickUP() {
    myMenu.pressUp();
}

void onClickDOWN() {
    myMenu.pressDown();
}


void init_main() {
}
