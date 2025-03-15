//
// Created by koro on 24-4-28.
//
#include "FlexiPLCLITE.h"
WEAK void onClickOK() {
    outputRelay(5,true);
}

WEAK void onClickCANCEL() {
    outputRelay(5,true);
}

WEAK void onClickUP() {
    outputRelay(5,true);
}

WEAK void onClickDOWN() {
    outputRelay(5,true);
}

WEAK void onSW1() {
}

WEAK void onSW2() {
}

WEAK void onInput(uint8_t pinNum) {
}

WEAK void onIn0() {
    onInput(0);
}

WEAK void onIn1() {
    onInput(1);
}

WEAK void onIn2() {
    onInput(2);
}

WEAK void onIn3() {
    onInput(3);
}
bool readINPUT(uint8_t num) {
    switch (num) {
        case 1:
            return (digitalRead(IN0)==HIGH);
        case 2:
            return (digitalRead(IN1)==HIGH);
        case 3:
            return (digitalRead(IN2)==HIGH);
        case 4:
            return (digitalRead(IN3)==HIGH);
        default:
            return false;
    }
}
bool readRelay(uint8_t num) {
    switch (num) {
        case 1:
            return (digitalRead(OUT0)==HIGH);
        case 2:
            return (digitalRead(OUT1)==HIGH);
        case 3:
            return (digitalRead(OUT2)==HIGH);
        case 4:
            return (digitalRead(OUT3)==HIGH);
        case 5:
            return (digitalRead(OUT4)==HIGH);
        case 6:
            return (digitalRead(OUT5)==HIGH);
        case 7:
            return (digitalRead(OUT6)==HIGH);
        case 8:
            return (digitalRead(OUT7)==HIGH);
        case 9:
            return (digitalRead(OUT8)==HIGH);
        case 10:
            return (digitalRead(OUT9)==HIGH);
        case 11:
            return (digitalRead(OUT10)==HIGH);
        case 12:
            return (digitalRead(OUT11)==HIGH);
        default:
            return false;
    }
}
void outputRelay(uint8_t num, bool status) {
    if (status) {
        switch (num) {
            case 1:
                digitalWrite(OUT0, HIGH);
                break;
            case 2:
                digitalWrite(OUT1, HIGH);
                break;
            case 3:
                digitalWrite(OUT2, HIGH);
                break;
            case 4:
                digitalWrite(OUT3, HIGH);
                break;
            case 5:
                digitalWrite(OUT4, HIGH);
                break;
            case 6:
                digitalWrite(OUT5, HIGH);
                break;
            case 7:
                digitalWrite(OUT6, HIGH);
                break;
            case 8:
                digitalWrite(OUT7, HIGH);
                break;
            case 9:
                digitalWrite(OUT8, HIGH);
                break;
            case 10:
                digitalWrite(OUT9, HIGH);
                break;
            case 11:
                digitalWrite(OUT10, HIGH);
                break;
            case 12:
                digitalWrite(OUT11, HIGH);
                break;
            default:
                break;
        }
    } else {
        switch (num) {
            case 1:
                digitalWrite(OUT0, LOW);
                break;
            case 2:
                digitalWrite(OUT1, LOW);
                break;
            case 3:
                digitalWrite(OUT2, LOW);
                break;
            case 4:
                digitalWrite(OUT3, LOW);
                break;
            case 5:
                digitalWrite(OUT4, LOW);
                break;
            case 6:
                digitalWrite(OUT5, LOW);
                break;
            case 7:
                digitalWrite(OUT6, LOW);
                break;
            case 8:
                digitalWrite(OUT7, LOW);
                break;
            case 9:
                digitalWrite(OUT8, LOW);
                break;
            case 10:
                digitalWrite(OUT9, LOW);
                break;
            case 11:
                digitalWrite(OUT10, LOW);
                break;
            case 12:
                digitalWrite(OUT11, LOW);
                break;
            default:
                break;
        }
    }
}

extern "C" void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
    */
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

    /** Configure LSE Drive Capability
    */
    HAL_PWR_EnableBkUpAccess();
    __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
    RCC_OscInitStruct.PLL.PLLN = 8;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
}
