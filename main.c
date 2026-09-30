#include "liquidcrystal_i2c.h"

#include <stdio.h>

#define SET_BLINK_SETTITNG 11U

#define ORDER_BLINK_SETTITNG 3U

#define NOT_TURN_OFF_LIGHT 24

typedef enum

{

    FALSE = 0,

    TRUE = 1

} Boolean;

typedef enum

{

    AUTO = 0,

    MANU = 1

} ModeOfWatering;

typedef enum

{

    SETTING = 0U,

    UP,

    DOWN,

    NOT_PRESS

} ButonStatusType;

typedef enum

{

    TIME_HOURS = 0U,

    TIME_MINUTES,

    TIME_SECONDS,

    DRYING_MODE,

    DRYING_HOURS_START,

    DRYING_MIN_START,

    DRYING_SEC_START,

    DRYING_HOURS_STOP,

    DRYING_MIN_STOP,

    DRYING_SEC_STOP,

    NOT_BLINK

} SetBlinkType;

typedef enum

{

    LINE_LCD_0 = 0U,

    LINE_LCD_1,

    LINE_LCD_2,

    LINE_LCD_3,

    LINE_LCD_NONE

} LineLCDType;

typedef enum

{

    ALL_LINES_LCD_0 = 0U,

    ALL_LINES_LCD_1,

    ALL_LINES_LCD_2

} AllLinesLCDType;

typedef struct

{

    uint8_t Time_Hours;

    uint8_t Time_Minutes;

    uint8_t Time_Seconds;

    uint8_t Drying_Mode;

    uint8_t Drying_Hours_Start;

    uint8_t Drying_Minutes_Start;

    uint8_t Drying_Seconds_Start;

    uint8_t Drying_Hours_Stop;

    uint8_t Drying_Minutes_Stop;

    uint8_t Drying_Seconds_Stop;

    uint8_t Drying_Time;

} TimeAndDryingType;

void RTC_Configuration(void);

void NVIC_Configuration(void);

void RTC_IRQHandler(void);

void I2C_LCD_Configuration(void);

void ShowInforScreen(TimeAndDryingType *TimeAlarm, uint8_t *TempSetBlink);

ButonStatusType GetButon(void);

void RTC_Setup(void);

void UpdateTimeAndAlarm(void);

void GPIO_Config(void);

void PressSettingButton(TimeAndDryingType *TmAndAl);

void SettingUpDownShowMode(void);

void CheckSetting(void);

void ShowStatusAndTimeAlarm(void);

void HandleAlarm(void);

void TurnOffONBacklight(void);

void ShowTimeTurnOffOnBacklight(void);

void ShowTimeAndAlarm(TimeAndDryingType *TimeAlarm);

void HandleDryingSoil(void);

void ADC_DMA_Config(void);

static TimeAndDryingType TimeAndDry;

static TimeAndDryingType TimeAndDrySetting;

static ButonStatusType ButonStatus = NOT_PRESS;

static uint8_t SetBlink[SET_BLINK_SETTITNG][ORDER_BLINK_SETTITNG] =

    {

        {TIME_HOURS, LINE_LCD_0, 9U},

        {TIME_MINUTES, LINE_LCD_0, 12U},

        {TIME_SECONDS, LINE_LCD_0, 15U},

        {DRYING_MODE, LINE_LCD_1, 14U},

        {DRYING_HOURS_START, LINE_LCD_2, 12U},

        {DRYING_MIN_START, LINE_LCD_2, 15U},

        {DRYING_SEC_START, LINE_LCD_2, 18U},

        {DRYING_HOURS_STOP, LINE_LCD_3, 12U},

        {DRYING_MIN_STOP, LINE_LCD_3, 15U},

        {DRYING_SEC_STOP, LINE_LCD_3, 18U},

        {NOT_BLINK, LINE_LCD_NONE, LINE_LCD_NONE}

};

static int8_t *PointerToSetting;

static int8_t SettingCount = -1;

volatile uint16_t u16AdcValues[2U] = {0};

int main(void)

{

    static TimeAndDryingType TempTimeAndDrySetting;

    GPIO_Config();

    ADC_DMA_Config();

    I2C_LCD_Configuration();

    HD44780_Init(4);

    HD44780_Clear();

    NVIC_Configuration();

    RTC_Configuration();

    /* Default to dry clothes */

    TimeAndDry.Drying_Mode = AUTO;

    TimeAndDry.Drying_Hours_Start = 18;

    TimeAndDry.Drying_Minutes_Start = 0;

    TimeAndDry.Drying_Seconds_Start = 0;

    TimeAndDry.Drying_Hours_Stop = 18;

    TimeAndDry.Drying_Minutes_Stop = 10;

    TimeAndDry.Drying_Seconds_Stop = 0;

    TimeAndDry.Drying_Time = 10;

    while (1)

    {

        PressSettingButton(&TempTimeAndDrySetting);

        if (ButonStatus == SETTING)

        {

            SettingUpDownShowMode();
        }

        else

        {

            ShowTimeAndAlarm(&TimeAndDry);
        }

        HandleDryingSoil();
    }
}

void HandleDryingSoil(void)

{

    if (TimeAndDry.Drying_Mode == AUTO)

    {

        if (u16AdcValues[1] > 3200)

        {

            GPIO_SetBits(GPIOB, GPIO_Pin_5);
        }

        else

        {

            GPIO_ResetBits(GPIOB, GPIO_Pin_5);
        }
    }

    else

    {

        if (TimeAndDry.Drying_Hours_Start != TimeAndDry.Drying_Hours_Stop)

        {

            if ((TimeAndDry.Drying_Hours_Start <= TimeAndDry.Time_Hours) && (TimeAndDry.Time_Hours <= TimeAndDry.Drying_Hours_Stop))

            {

                GPIO_SetBits(GPIOB, GPIO_Pin_5);
            }

            else

            {

                GPIO_ResetBits(GPIOB, GPIO_Pin_5);
            }
        }

        else

        {

            if (TimeAndDry.Drying_Minutes_Start != TimeAndDry.Drying_Minutes_Stop)

            {

                if ((TimeAndDry.Drying_Hours_Start == TimeAndDry.Time_Hours) && (TimeAndDry.Drying_Minutes_Start <= TimeAndDry.Time_Minutes) && (TimeAndDry.Time_Minutes < TimeAndDry.Drying_Minutes_Stop))

                {

                    GPIO_SetBits(GPIOB, GPIO_Pin_5);
                }

                else

                {

                    GPIO_ResetBits(GPIOB, GPIO_Pin_5);
                }
            }

            else

            {

                if ((TimeAndDry.Drying_Hours_Start == TimeAndDry.Time_Hours) && (TimeAndDry.Drying_Minutes_Start == TimeAndDry.Time_Minutes) && (TimeAndDry.Drying_Seconds_Start <= TimeAndDry.Time_Seconds) && (TimeAndDry.Time_Seconds < TimeAndDry.Drying_Seconds_Stop))

                {

                    GPIO_SetBits(GPIOB, GPIO_Pin_5);
                }

                else

                {

                    GPIO_ResetBits(GPIOB, GPIO_Pin_5);
                }
            }
        }
    }
}

void CheckSetting(void)

{

    if ((SetBlink[SettingCount][0] == TIME_HOURS) || (SetBlink[SettingCount][0] == DRYING_HOURS_START) || (SetBlink[SettingCount][0] == DRYING_HOURS_STOP))

    {

        if (*PointerToSetting > 23)

        {

            *PointerToSetting = 0U;
        }

        if (*PointerToSetting < 0)

        {

            *PointerToSetting = 23U;
        }
    }

    else if ((SetBlink[SettingCount][0] == TIME_MINUTES) || (SetBlink[SettingCount][0] == TIME_SECONDS) || (SetBlink[SettingCount][0] == DRYING_MIN_START) ||

             (SetBlink[SettingCount][0] == DRYING_SEC_START) || (SetBlink[SettingCount][0] == DRYING_MIN_STOP) || (SetBlink[SettingCount][0] == DRYING_SEC_STOP)

    )

    {

        if (*PointerToSetting > 59)

        {

            *PointerToSetting = 0U;
        }

        if (*PointerToSetting < 0)

        {

            *PointerToSetting = 59U;
        }
    }

    else if (SetBlink[SettingCount][0] == DRYING_MODE)

    {

        if (*PointerToSetting > 1)

        {

            *PointerToSetting = 0U;
        }

        if (*PointerToSetting < 0)

        {

            *PointerToSetting = 1;
        }
    }

    else

    {

        /* Do nothing */
    }

    if ((SetBlink[SettingCount][0] == DRYING_HOURS_START) && (TimeAndDrySetting.Drying_Hours_Stop < TimeAndDrySetting.Drying_Hours_Start))

    {

        TimeAndDrySetting.Drying_Hours_Stop = TimeAndDrySetting.Drying_Hours_Start;
    }

    if ((SetBlink[SettingCount][0] == DRYING_MIN_START) && (TimeAndDrySetting.Drying_Hours_Stop == TimeAndDrySetting.Drying_Hours_Start) && (TimeAndDrySetting.Drying_Minutes_Stop < TimeAndDrySetting.Drying_Minutes_Start))

    {

        TimeAndDrySetting.Drying_Minutes_Stop = TimeAndDrySetting.Drying_Minutes_Start;
    }

    if ((SetBlink[SettingCount][0] == DRYING_SEC_START) && (TimeAndDrySetting.Drying_Hours_Stop == TimeAndDrySetting.Drying_Hours_Start) &&

        (TimeAndDrySetting.Drying_Seconds_Stop < TimeAndDrySetting.Drying_Seconds_Start) && (TimeAndDrySetting.Drying_Minutes_Stop == TimeAndDrySetting.Drying_Minutes_Start))

    {

        TimeAndDrySetting.Drying_Seconds_Stop = TimeAndDrySetting.Drying_Seconds_Start;
    }

    if ((SetBlink[SettingCount][0] == DRYING_HOURS_STOP) && (TimeAndDrySetting.Drying_Hours_Stop < TimeAndDrySetting.Drying_Hours_Start))

    {

        TimeAndDrySetting.Drying_Hours_Stop = TimeAndDrySetting.Drying_Hours_Start;
    }

    if ((SetBlink[SettingCount][0] == DRYING_MIN_STOP) && (TimeAndDrySetting.Drying_Hours_Stop == TimeAndDrySetting.Drying_Hours_Start) && (TimeAndDrySetting.Drying_Minutes_Stop < TimeAndDrySetting.Drying_Minutes_Start))

    {

        TimeAndDrySetting.Drying_Minutes_Stop = TimeAndDrySetting.Drying_Minutes_Start;
    }

    if ((SetBlink[SettingCount][0] == DRYING_SEC_STOP) && (TimeAndDrySetting.Drying_Hours_Stop == TimeAndDrySetting.Drying_Hours_Start) &&

        (TimeAndDrySetting.Drying_Seconds_Stop < TimeAndDrySetting.Drying_Seconds_Start) && (TimeAndDrySetting.Drying_Minutes_Stop == TimeAndDrySetting.Drying_Minutes_Start))

    {

        TimeAndDrySetting.Drying_Seconds_Stop = TimeAndDrySetting.Drying_Seconds_Start;
    }
}

void SettingUpDownShowMode(void)

{

    if (UP == GetButon())

    {

        *PointerToSetting += 1U;

        CheckSetting();

        ShowInforScreen(&TimeAndDrySetting, &SetBlink[SettingCount][0]);
    }

    if (DOWN == GetButon())

    {

        *PointerToSetting -= 1U;

        CheckSetting();

        ShowInforScreen(&TimeAndDrySetting, &SetBlink[SettingCount][0]);
    }
}

void PressSettingButton(TimeAndDryingType *TmAndAl)

{

    if (SETTING == GetButon())

    {

        if (SettingCount == -1)

        {

            ButonStatus = SETTING;

            TimeAndDrySetting.Time_Hours = TimeAndDry.Time_Hours;

            TimeAndDrySetting.Time_Minutes = TimeAndDry.Time_Minutes;

            TimeAndDrySetting.Time_Seconds = TimeAndDry.Time_Seconds;

            TimeAndDrySetting.Drying_Mode = TimeAndDry.Drying_Mode;

            TimeAndDrySetting.Drying_Hours_Start = TimeAndDry.Drying_Hours_Start;

            TimeAndDrySetting.Drying_Minutes_Start = TimeAndDry.Drying_Minutes_Start;

            TimeAndDrySetting.Drying_Seconds_Start = TimeAndDry.Drying_Seconds_Start;

            TimeAndDrySetting.Drying_Hours_Stop = TimeAndDry.Drying_Hours_Stop;

            TimeAndDrySetting.Drying_Minutes_Stop = TimeAndDry.Drying_Minutes_Stop;

            TimeAndDrySetting.Drying_Seconds_Stop = TimeAndDry.Drying_Seconds_Stop;

            TmAndAl->Time_Hours = TimeAndDrySetting.Time_Hours;

            TmAndAl->Time_Minutes = TimeAndDrySetting.Time_Minutes;

            TmAndAl->Time_Seconds = TimeAndDrySetting.Time_Seconds;

            PointerToSetting = (int8_t *)&TimeAndDrySetting - 1U;
        }

        SettingCount++;

        PointerToSetting = PointerToSetting + 1U;

        ShowInforScreen(&TimeAndDrySetting, &SetBlink[SettingCount][0]);

        if ((SetBlink[SettingCount][0] == NOT_BLINK) || ((SetBlink[SettingCount][0] == DRYING_HOURS_START) && (TimeAndDrySetting.Drying_Mode == AUTO)))

        {

            SettingCount = -1;

            ButonStatus = NOT_PRESS;

            /* Update Time */

            if ((TmAndAl->Time_Hours != TimeAndDrySetting.Time_Hours) || (TmAndAl->Time_Minutes != TimeAndDrySetting.Time_Minutes) ||

                (TmAndAl->Time_Seconds != TimeAndDrySetting.Time_Seconds)

            )

            {

                UpdateTimeAndAlarm();
            }

            HD44780_NoBlink();

            TimeAndDry.Drying_Hours_Start = TimeAndDrySetting.Drying_Hours_Start;

            TimeAndDry.Drying_Minutes_Start = TimeAndDrySetting.Drying_Minutes_Start;

            TimeAndDry.Drying_Seconds_Start = TimeAndDrySetting.Drying_Seconds_Start;

            TimeAndDry.Drying_Hours_Stop = TimeAndDrySetting.Drying_Hours_Stop;

            TimeAndDry.Drying_Minutes_Stop = TimeAndDrySetting.Drying_Minutes_Stop;

            TimeAndDry.Drying_Seconds_Stop = TimeAndDrySetting.Drying_Seconds_Stop;

            TimeAndDry.Drying_Mode = TimeAndDrySetting.Drying_Mode;
        }
    }
}

void UpdateTimeAndAlarm(void)

{

    RTC_Setup();

    RTC_SetCounter((TimeAndDrySetting.Time_Hours * 3600 + TimeAndDrySetting.Time_Minutes * 60 + TimeAndDrySetting.Time_Seconds));

    BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
}

ButonStatusType GetButon(void)

{

    ButonStatusType Buton = NOT_PRESS;

    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0U)

    {

        Delay_Ms(50);

        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0U)

        {

            Buton = SETTING;
        }
    }

    else if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) == 0U)

    {

        Delay_Ms(50);

        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) == 0U)

        {

            Buton = UP;
        }
    }

    else if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2) == 0U)

    {

        Delay_Ms(50);

        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2) == 0U)

        {

            Buton = DOWN;
        }
    }

    else

    {

        Buton = NOT_PRESS;
    }

    return Buton;
}

void ShowInforScreen(TimeAndDryingType *TimeAlarm, uint8_t *TempSetBlink)

{

    ShowTimeAndAlarm(TimeAlarm);

    if (*TempSetBlink != NOT_BLINK)

    {

        HD44780_SetCursor(*(TempSetBlink + 2), *(TempSetBlink + 1));

        HD44780_Blink();
    }
}

void ShowTimeAndAlarm(TimeAndDryingType *TimeAlarm)

{

    char Str[16];

    /************************************ Show Time ***********************************/

    HD44780_SetCursor(0, 0);

    HD44780_PrintStr("Time  ");

    HD44780_SetCursor(8, 0);

    sprintf(&Str[0], "%0.2d:", TimeAlarm->Time_Hours);

    HD44780_PrintStr(&Str[0]);

    HD44780_SetCursor(11, 0);

    sprintf(&Str[0], "%0.2d:", TimeAlarm->Time_Minutes);

    HD44780_PrintStr(&Str[0]);

    HD44780_SetCursor(14, 0);

    sprintf(&Str[0], "%0.2d", TimeAlarm->Time_Seconds);

    HD44780_PrintStr(&Str[0]);

    /************************************ Show Alarm  ***********************************/

    if (TimeAlarm->Drying_Mode == AUTO)

    {

        HD44780_SetCursor(0, 1);

        HD44780_PrintStr("Pump Mode  Auto");

        HD44780_SetCursor(0, 2);

        HD44780_PrintStr("Pump Start ");

        HD44780_SetCursor(11, 2);

        HD44780_PrintStr("--:");

        HD44780_SetCursor(14, 2);

        HD44780_PrintStr("--:");

        HD44780_SetCursor(17, 2);

        HD44780_PrintStr("--");

        HD44780_SetCursor(0, 3);

        HD44780_PrintStr("Pump Stop ");

        HD44780_SetCursor(11, 3);

        HD44780_PrintStr("--:");

        HD44780_SetCursor(14, 3);

        HD44780_PrintStr("--:");

        HD44780_SetCursor(17, 3);

        HD44780_PrintStr("--");
    }

    else

    {

        HD44780_SetCursor(0, 1);

        HD44780_PrintStr("Pump Mode  Manu");

        HD44780_SetCursor(0, 2);

        HD44780_PrintStr("Pump Start ");

        HD44780_SetCursor(11, 2);

        sprintf(&Str[0], "%0.2d:", TimeAlarm->Drying_Hours_Start);

        HD44780_PrintStr(&Str[0]);

        HD44780_SetCursor(14, 2);

        sprintf(&Str[0], "%0.2d:", TimeAlarm->Drying_Minutes_Start);

        HD44780_PrintStr(&Str[0]);

        HD44780_SetCursor(17, 2);

        sprintf(&Str[0], "%0.2d", TimeAlarm->Drying_Seconds_Start);

        HD44780_PrintStr(&Str[0]);

        HD44780_SetCursor(0, 3);

        HD44780_PrintStr("Pump Stop ");

        HD44780_SetCursor(11, 3);

        sprintf(&Str[0], "%0.2d:", TimeAlarm->Drying_Hours_Stop);

        HD44780_PrintStr(&Str[0]);

        HD44780_SetCursor(14, 3);

        sprintf(&Str[0], "%0.2d:", TimeAlarm->Drying_Minutes_Stop);

        HD44780_PrintStr(&Str[0]);

        HD44780_SetCursor(17, 3);

        sprintf(&Str[0], "%0.2d", TimeAlarm->Drying_Seconds_Stop);

        HD44780_PrintStr(&Str[0]);
    }
}

void NVIC_Configuration(void)

{

    NVIC_InitTypeDef NVIC_InitStructure;

    /* Config Priority interrupt */

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

    /* Enable Interrupt for RTC */

    NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;

    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;

    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;

    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStructure);
}

void RTC_Configuration(void)

{

    /* Check RTC already configured or not by checking backup data register */

    if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)

    {

        RTC_Setup();

        BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
    }

    else

    {

        RTC_WaitForSynchro();

        RTC_ITConfig(RTC_IT_SEC, ENABLE);

        RTC_WaitForLastTask();

        RCC_ClearFlag();
    }
}

void RTC_Setup(void)

{

    /* Enable clock for PWR va BKP */

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

    PWR_BackupAccessCmd(ENABLE);

    BKP_DeInit();

    RCC_LSEConfig(RCC_LSE_ON);

    while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)

    {
    }

    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

    RCC_RTCCLKCmd(ENABLE);

    RTC_WaitForSynchro();

    RTC_WaitForLastTask();

    RTC_ITConfig(RTC_IT_SEC, ENABLE);

    RTC_WaitForLastTask();

    RTC_SetPrescaler(32767); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */

    RTC_WaitForLastTask();

    RTC_SetCounter(0);

    RTC_WaitForLastTask();
}

void RTC_IRQHandler(void)

{

    uint32_t TimeVar = 0;

    if (RTC_GetITStatus(RTC_IT_SEC) != RESET)

    {

        /* Clear the RTC Second interrupt */

        RTC_ClearITPendingBit(RTC_IT_SEC);

        /* Wait until last write operation on RTC registers has finished */

        RTC_WaitForLastTask();

        TimeVar = RTC_GetCounter();

        /* when it was over

              0x0001517F = 23H*3600 + 59M*60 + 59S

        */

        TimeVar = TimeVar % 86400;

        /* Caclute time */

        TimeAndDry.Time_Hours = (uint8_t)(TimeVar / 3600);

        TimeAndDry.Time_Minutes = (TimeVar % 3600) / 60;

        TimeAndDry.Time_Seconds = (TimeVar % 3600) % 60;
    }
}

void I2C_LCD_Configuration(void)

{

    GPIO_InitTypeDef GPIO_InitStructure;

    I2C_InitTypeDef I2C_InitStructure;

    /* PortB clock enable */

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    /* I2C1 clock enable */

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    GPIO_StructInit(&GPIO_InitStructure);

    /* SCL (PB6) */

    /* SDA (PB7) */

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;

    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Configure I2Cx */

    I2C_StructInit(&I2C_InitStructure);

    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;

    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;

    I2C_InitStructure.I2C_OwnAddress1 = 0x00;

    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;

    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;

    I2C_InitStructure.I2C_ClockSpeed = 100000;

    I2C_Init(I2C_Chanel, &I2C_InitStructure);

    I2C_Cmd(I2C_Chanel, ENABLE);
}

void GPIO_Config(void)

{

    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Turn Off */

    GPIO_ResetBits(GPIOB, GPIO_Pin_5);
}

void ADC_DMA_Config(void)

{

    GPIO_InitTypeDef GPIO_InitStructure;

    ADC_InitTypeDef ADC_InitStructure;

    DMA_InitTypeDef DMA_InitStructure;

    RCC_ADCCLKConfig(RCC_PCLK2_Div6);

    /* Enable ADC1 and GPIOA clock */

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    DMA_InitStructure.DMA_BufferSize = 2;

    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;

    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)u16AdcValues;

    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;

    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;

    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;

    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;

    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;

    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;

    DMA_InitStructure.DMA_Priority = DMA_Priority_High;

    DMA_Init(DMA1_Channel1, &DMA_InitStructure);

    DMA_Cmd(DMA1_Channel1, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;

    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;

    GPIO_Init(GPIOB, &GPIO_InitStructure);

    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;

    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;

    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;

    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;

    ADC_InitStructure.ADC_NbrOfChannel = 2;

    ADC_InitStructure.ADC_ScanConvMode = ENABLE;

    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_7Cycles5);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 2, ADC_SampleTime_7Cycles5);

    ADC_Cmd(ADC1, ENABLE);

    ADC_DMACmd(ADC1, ENABLE);

    ADC_ResetCalibration(ADC1);

    while (ADC_GetResetCalibrationStatus(ADC1))
        ;

    ADC_StartCalibration(ADC1);

    while (ADC_GetCalibrationStatus(ADC1))
        ;

    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}
