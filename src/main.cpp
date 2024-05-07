#include <Arduino.h>
#include "broad/FlexiPLCLITE.h"
#include "U8g2lib.h"
#include "FH_Font.h"
#include "menu.h"
#include "EEPROM.h"
#include "stm32g0xx.h"
#include "SerialTransfer.h"
#include "tick_scheduler.h"

enum COMData {
    COM1, COM2, COM3, COM4
};

struct SaveData {
    byte MAGIC_VALUE = 0xA5;
    uint8_t tag_num = 1;
    COMData in_com = COM1;
    COMData out_com = COM2;
    bool use_wifi = false;
} nowData;

HardwareTimer timer(TIM3); //逻辑周期定时器
HardwareTimer aimtimer(TIM6); //动画逻辑周期定时器

HardwareSerial *IN_Serial;
HardwareSerial *OUT_Serial;

HardwareSerial S1(RX1,TX1);
HardwareSerial S2(RX2,TX2);
HardwareSerial S3(RX3,TX3);
HardwareSerial S4(RX4,TX4);

SerialTransfer INTransfer;
SerialTransfer OUTTransfer;

static uint8_t in_buffer[MAX_PACKET_SIZE];
static uint8_t out_buffer[MAX_PACKET_SIZE];

void tick();

void screentick();

void initFlashStorage();

void renderStr(const char *string1, const char *string2);

void writeSaveData();

void init_menu();

void init_main();

Menu *testMenu;
bool renderStatus = true;


void setup() {
    initBSP();

    nowData = EEPROM.get(0, nowData);
    if (nowData.MAGIC_VALUE != 0xA5) {
        initFlashStorage();
    }

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

    // 设置通道 2，触发 16.6ms ，60hz
    aimtimer.setPrescaleFactor(6400);
    aimtimer.setOverflow(167);
    aimtimer.attachInterrupt(screentick);

    NVIC_SetPriority(TIM3_IRQn, 0);
    NVIC_SetPriority(TIM6_IRQn, 5);

    init_menu();

    S1.begin(115200);
    S2.begin(115200);
    S3.begin(115200);
    S4.begin(115200);

    init_main();


    timer.resume(); // 启动定时器
    aimtimer.resume();
}

void tick() {
    tick_increment();
    INTransfer.tick();
    OUTTransfer.tick();
}

void screentick() {
    animationTick();
}

void initFlashStorage() {
    nowData.MAGIC_VALUE = 0xA5;
    nowData.tag_num = 1;
    nowData.in_com = COM1;
    nowData.out_com = COM2;
    nowData.use_wifi = false;
    writeSaveData();
}


void renderStr(const char *string1, const char *string2) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_fh);
    u8g2.setCursor(0, 14);
    u8g2.println(string1);
    u8g2.setCursor(0, 30);
    u8g2.println(string2);
    u8g2.sendBuffer();
}

void writeSaveData() {
    auto *data = (uint8_t *) &nowData;
    uint32_t size = sizeof(nowData);
    for (int i = 0; i < size; i++) {
        EEPROM.write(i, data[i]);
    }
}


void loop() {
    if (renderStatus) {
        renderTick();
    }
}

void init_menu() {
    Menu *menus[3];
    menus[0] = createMenu("状态");
    menus[1] = createMenu("设置");
    menus[2] = createMenu("关于");

    createCircularMenuChain(menus, 3);

    menus[0]->formate_callback = []()-> auto {
        static char line2buffer[30];
        sprintf(line2buffer, "机位:%3d|I:%1d|O:%1d", nowData.tag_num, nowData.in_com + 1, nowData.out_com + 1);
        return line2buffer;
    };

    const char *setting_str[] = {"机位号设置", "通讯口设置", "通讯模式设置", "保存", "恢复出厂设置"};
    Menu **setting_menu = bindChildrenToMenu(menus[1], setting_str, 5);

    const char *tag_str[] = {"设置机位号"};
    Menu **tag_menu = bindChildrenToMenu(setting_menu[0], tag_str, 1);
    tag_menu[0]->formate_callback = []()-> auto {
        static char line2buffer[30];
        sprintf(line2buffer, "当前机位号: %3d", nowData.tag_num);
        return line2buffer;
    };
    tag_menu[0]->btn_callback = [](auto type)-> auto {
        if (type == UP) {
            if (nowData.tag_num == 255) {
                nowData.tag_num = 0;
            } else {
                nowData.tag_num++;
            }
        } else if (type == DOWN) {
            if (nowData.tag_num == 0) {
                nowData.tag_num = 255;
            } else {
                nowData.tag_num--;
            }
        }
    };

    const char *com_str[] = {"输入通讯口设置", "输出通讯口设置"};
    Menu **com_menu = bindChildrenToMenu(setting_menu[1], com_str, 2);

    const char *in_str[] = {"输入串口号："};
    Menu **in_menu = bindChildrenToMenu(com_menu[0], in_str, 1);
    in_menu[0]->formate_callback = []()-> auto {
        static char line2buffer[30];
        sprintf(line2buffer, "COM%1d", nowData.in_com + 1);
        return line2buffer;
    };
    in_menu[0]->btn_callback = [](auto type)-> auto {
        if (type == UP) {
            if (nowData.in_com == 3) {
                nowData.in_com = COM1;
            } else {
                nowData.in_com = static_cast<COMData>(nowData.in_com + 1);
            }
        } else if (type == DOWN) {
            if (nowData.in_com == 0) {
                nowData.in_com = COM4;
            } else {
                nowData.in_com = static_cast<COMData>(nowData.in_com - 1);
            }
        }
    };

    const char *out_str[] = {"输出串口号："};
    Menu **out_menu = bindChildrenToMenu(com_menu[1], out_str, 1);
    out_menu[0]->formate_callback = []()-> auto {
        static char line2buffer[30];
        sprintf(line2buffer, "COM%1d", nowData.out_com + 1);
        return line2buffer;
    };
    out_menu[0]->btn_callback = [](auto type)-> auto {
        if (type == UP) {
            if (nowData.out_com == 3) {
                nowData.out_com = COM1;
            } else {
                nowData.out_com = static_cast<COMData>(nowData.out_com + 1);
            }
        } else if (type == DOWN) {
            if (nowData.out_com == 0) {
                nowData.out_com = COM4;
            } else {
                nowData.out_com = static_cast<COMData>(nowData.out_com - 1);
            }
        }
    };

    const char *wifi_str[] = {"通讯模式："};
    Menu **wifi_menu = bindChildrenToMenu(setting_menu[2], wifi_str, 1);
    wifi_menu[0]->formate_callback = []()-> auto {
        if (nowData.use_wifi) {
            return "ESP_NOW";
        } else {
            return "UART_LINE";
        }
    };
    wifi_menu[0]->btn_callback = [](auto type)-> auto {
        if (type == UP) {
            nowData.use_wifi = true;
        } else if (type == DOWN) {
            nowData.use_wifi = false;
        }
    };
    setting_menu[3]->btn_callback = [](BTN_Type type)-> auto {
        if (type == OK) {
            writeSaveData();
            renderStatus = false;
            renderStr("完成", "请等待复位...");
            //NVIC_SystemReset();
            register_callback([]()-> auto { NVIC_SystemReset(); }, 20);
        } else {
        }
    };

    setting_menu[4]->btn_callback = [](BTN_Type type)-> auto {
        if (type == OK) {
            initFlashStorage();
            renderStatus = false;
            renderStr("完成", "请等待复位...");
            //NVIC_SystemReset();
            register_callback([]()-> auto { NVIC_SystemReset(); }, 20);
        } else {
        }
    };

    const char *about_str[] = {"昊明喷涂设备"};
    Menu **about_menu = bindChildrenToMenu(menus[2], about_str, 1);
    about_menu[0]->formate_callback = []()-> auto { return "版本:1.0.0-232"; };

    setMenu(*menus);
}

void handleMessage() {
    uint8_t formID = in_buffer[1];
    uint8_t command = in_buffer[2];
    uint16_t data = *reinterpret_cast<const uint16_t *>(&(in_buffer[3]));

    switch (command) {
        case 0: {
            //设置输出
            for (short i = 0; i < 16; i++) {
                bool bit = (data >> i) & 1;
                outputRelay(i, bit);
            }
        }
        break;
        case 1: {
            //获取输出状态
            uint16_t out = 0000000000000000;
            for (short i = 0; i < 16; i++) {
                // 使用提供的方法设置当前位
                if (readRelay(i)) {
                    out |= (1 << i); // 设置为1
                } else {
                    out &= ~(1 << i); // 设置为0
                }
            }
            auto *sendData = new uint8_t[5];
            sendData[0] = formID;
            sendData[1] = nowData.tag_num;
            sendData[2] = static_cast<uint8_t>(1);
            sendData[3] =  static_cast<uint8_t>(out & 0x00FF);
            sendData[4] =  static_cast<uint8_t>((out >> 8) & 0x00FF);

            INTransfer.sendData(INTransfer.txObj(sendData,0,sizeof(uint8_t) * 5));
            delete sendData;
        }
        break;
        case 2: {//获取输入寄存器状态
            uint8_t out = 00000000;
            for (short i = 0; i < 4; i++) {
                // 使用提供的方法设置当前位
                if (readINPUT(i)) {
                    out |= (1 << i); // 设置为1
                } else {
                    out &= ~(1 << i); // 设置为0
                }
            }
            auto *sendData = new uint8_t[5];
            sendData[0] = formID;
            sendData[1] = nowData.tag_num;
            sendData[2] = static_cast<uint8_t>(2);
            sendData[3] = out;
            sendData[4] = 00000000;

            INTransfer.sendData(INTransfer.txObj(sendData,0,sizeof(uint8_t) * 5));
            delete sendData;
        }
        break;
        default: {
        }
        break;
    }
}

void in_callback() {
    INTransfer.rxObj(in_buffer, 0, INTransfer.bytesRead);
    uint8_t id = in_buffer[0];
    if (id != nowData.tag_num) {
        if (!nowData.use_wifi) {
            OUTTransfer.sendData(OUTTransfer.txObj(in_buffer, 0, INTransfer.bytesRead));
        }
    } else {
        handleMessage();
    }

}

void out_callback() {
    OUTTransfer.rxObj(out_buffer, 0, OUTTransfer.bytesRead);
    uint8_t id = out_buffer[0];
    if (id != nowData.tag_num) {
        INTransfer.sendData(INTransfer.txObj(out_buffer, 0, OUTTransfer.bytesRead));
    }else {
        memcpy(in_buffer,out_buffer,OUTTransfer.bytesRead);
        handleMessage();
    }
}

static const functionPtr in_callbackArr[] = {in_callback};
static const functionPtr out_callbackArr[] = {out_callback};

void onInput(short pinNum) {
    auto *sendData = new uint8_t[5];
    sendData[0] = 0;
    sendData[1] = nowData.tag_num;
    sendData[2] = static_cast<uint8_t>(3);
    sendData[3] =  static_cast<uint8_t>(pinNum & 0x00FF);
    sendData[4] =  static_cast<uint8_t>((pinNum >> 8) & 0x00FF);

    INTransfer.sendData(INTransfer.txObj(sendData,0,sizeof(uint8_t) * 5));
    delete sendData;
}

void init_main() {
    switch (nowData.in_com) {
        case COM1:
            IN_Serial = &S1;
            break;
        case COM2:
            IN_Serial = &S2;
            break;
        case COM3:
            IN_Serial = &S3;
            break;
        case COM4:
            IN_Serial = &S4;
            break;
        default:
            IN_Serial = &S1;
            break;
    }
    switch (nowData.out_com) {
        case COM1:
            OUT_Serial = &S1;
            break;
        case COM2:
            OUT_Serial = &S2;
            break;
        case COM3:
            OUT_Serial = &S3;
            break;
        case COM4:
            OUT_Serial = &S4;
            break;
        default:
            OUT_Serial = &S2;
            break;
    }

    configST INConfig;
    INConfig.callbacks = in_callbackArr;
    INConfig.callbacksLen = sizeof(in_callbackArr) / sizeof(functionPtr);

    INTransfer.begin(*IN_Serial, INConfig);

    if (nowData.use_wifi) {
        return;
    }

    configST OUTConfig;
    OUTConfig.callbacks = out_callbackArr;
    OUTConfig.callbacksLen = sizeof(out_callbackArr) / sizeof(functionPtr);

    OUTTransfer.begin(*OUT_Serial, OUTConfig);
}
