/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef _BOARD_H_
#define _BOARD_H_

/*
 * Board identifier.
 */
#define BOARD_SMARTY
#define BOARD_NAME              "Smarty"

/*
 * Board supply voltage.
 */
#define STM32_VDD               330

/*
 * Board frequencies.
 */
#define STM32_LSECLK            32768
#define STM32_HSECLK            25000000

/*
 * MCU type as defined in the ST header file stm32f4xx.h.
 */
#define STM32F40_41xxx

/*
 * IO pins assignments.
 */

#define GPIOA_TXD4              0
#define GPIOA_RXD4              1
#define GPIOA_BUT5              2
#define GPIOA_BUT6              3
#define GPIOA_DAC1              4
#define GPIOA_DAC2              5
#define GPIOA_MP3               6
#define GPIOA_MP2               7
#define GPIOA_USB_VB            9
#define GPIOA_USB_ID           10
#define GPIOA_USB_DM           11
#define GPIOA_USB_DP           12
#define GPIOA_J_TMS            13
#define GPIOA_J_TCK            14
#define GPIOA_J_TDI            15

#define GPIOB_AIN8              0
#define GPIOB_BUT7              1
#define GPIOB_J_TDO             3
#define GPIOB_J_RST             4
#define GPIOB_TXD1              6
#define GPIOB_RXD1              7
#define GPIOB_I2C1_C            8
#define GPIOB_I2C1_D            9
#define GPIOB_I2C2_C           10
#define GPIOB_I2C2D            11

#define GPIOC_AIN1              0
#define GPIOC_AIN2              1
#define GPIOC_AIN3              2
#define GPIOC_AIN4              3
#define GPIOC_UBAT_DIV          4
#define GPIOC_AIN7              5
#define GPIOC_SD_D0             8
#define GPIOC_SD_D1             9
#define GPIOC_SD_D2            10
#define GPIOC_SD_D3            11
#define GPIOC_SD_CLK           12

#define GPIOD_CAN_RX            0
#define GPIOD_CAN_TX            1
#define GPIOD_SD_CMD            2
#define GPIOD_SD_CARD_IN        3
#define GPIOD_TXD2              5
#define GPIOD_RXD2              6
#define GPIOD_TXD3              8
#define GPIOD_RXD3              6
#define GPIOD_POX_ST           11
#define GPIOD_PO5              12
#define GPIOD_PO6              13
#define GPIOD_PO7              14
#define GPIOD_PO8              15

#define GPIOE_ERR_USB_TO_5V     7
#define GPIOE_ERR_5V_TO_USB     8
#define GPIOE_BUT8              9
#define GPIOE_EN_USB_TO_5V     10
#define GPIOE_EN1_5V_TO_USB    11
#define GPIOE_EN2_5V_TO_USB    12
#define GPIOE_5V_LOAD          13
#define GPIOE_X5V_LOAD         14
#define GPIOE_X5V_EN           15

#define GPIOF_ACCX              3
#define GPIOF_ACCY              4
#define GPIOF_ACCZ              5
#define GPIOF_BUT1              6
#define GPIOF_BUT2              7
#define GPIOF_BUT3              8
#define GPIOF_BUT4              9
#define GPIOF_YAW              10
#define GPIOF_UBAT_FEED        11
#define GPIOF_SPKR1_UD         12
#define GPIOF_SPKR1_UD_STP     13
#define GPIOF_SPKR2_UD         14
#define GPIOF_SPKR2_UD_STP     15

#define GPIOG_SPKR1_ON          0
#define GPIOG_SOKR2_ON          1
#define GPIOG_PO1               2
#define GPIOG_PO2               3
#define GPIOG_PO3               4
#define GPIOG_PO4               5
#define GPIOG_RXD_WLAN          9
#define GPIOG_RST_WLAN         12
#define GPIOG_ADHOC_WLAN       13
#define GPIOG_TX_WLAN          14

#define GPIOH_YAW_ST            3
#define GPIOH_I2C_C             7
#define GPIOH_I2C_D             8
#define GPIOH_LS_1              9
#define GPIOH_LS_2             10
#define GPIOH_LS_3             11
#define GPIOH_LS_4             12
#define GPIOH_CS_WLAN          13

#define GPIOI_SPI1_CLK          1
#define GPIOI_SPI2_MISO         2
#define GPIOI_SPI2_MOSI         3

/*
 * I/O ports initial setup, this configuration is established soon after reset
 * in the initialization code.
 * Please refer to the STM32 Reference Manual for details.
 */
#define PIN_MODE_INPUT(n)           (0U << ((n) * 2))
#define PIN_MODE_OUTPUT(n)          (1U << ((n) * 2))
#define PIN_MODE_ALTERNATE(n)       (2U << ((n) * 2))
#define PIN_MODE_ANALOG(n)          (3U << ((n) * 2))
#define PIN_ODR_LOW(n)              (0U << (n))
#define PIN_ODR_HIGH(n)             (1U << (n))
#define PIN_OTYPE_PUSHPULL(n)       (0U << (n))
#define PIN_OTYPE_OPENDRAIN(n)      (1U << (n))
#define PIN_OSPEED_2M(n)            (0U << ((n) * 2))
#define PIN_OSPEED_25M(n)           (1U << ((n) * 2))
#define PIN_OSPEED_50M(n)           (2U << ((n) * 2))
#define PIN_OSPEED_100M(n)          (3U << ((n) * 2))
#define PIN_PUPDR_FLOATING(n)       (0U << ((n) * 2))
#define PIN_PUPDR_PULLUP(n)         (1U << ((n) * 2))
#define PIN_PUPDR_PULLDOWN(n)       (2U << ((n) * 2))
#define PIN_AFIO_AF(n, v)           ((v##U) << ((n % 8) * 4))

/*
 * Port A setup.
 * All input with pull-up except:
 * PA0  - TXD4         (output) This is used as a normal LED.
 * PA13 - J_TMS        (alternate 0)
 * PA14 - J_TCK        (alternate 0)
 * PA15 - J_TDI        (alternate 0)
 */
#define VAL_GPIOA_MODER             (PIN_MODE_OUTPUT(GPIOA_TXD4)       | \
                                     PIN_MODE_ALTERNATE(GPIOA_J_TMS)   | \
                                     PIN_MODE_ALTERNATE(GPIOA_J_TCK)   | \
                                     PIN_MODE_ALTERNATE(GPIOA_J_TDI))
#define VAL_GPIOA_OTYPER            0x00000000
#define VAL_GPIOA_OSPEEDR           0xFFFFFFFF
#define VAL_GPIOA_PUPDR             (~(PIN_PUPDR_FLOATING(GPIOA_TXD4)   | \
                                       PIN_PUPDR_FLOATING(GPIOA_J_TMS)  | \
                                       PIN_PUPDR_FLOATING(GPIOA_J_TCK)  | \
                                       PIN_PUPDR_FLOATING(GPIOA_J_TDI)))
#define VAL_GPIOA_ODR               0xFFFFFFFF
#define VAL_GPIOA_AFRL              0x00000000
#define VAL_GPIOA_AFRH              (PIN_AFIO_AF(GPIOA_J_TMS, 0)       | \
                                     PIN_AFIO_AF(GPIOA_J_TCK, 0)       | \
                                     PIN_AFIO_AF(GPIOA_J_TDI, 0))
/*
 * Port B setup.
 * All input with pull-up except:
 * PB0 - AIN8           (analog - SEN5)
 * PB3 - J_TDO          (alternate 0)
 * PB4 - J_RST          (alternate 0)
 * PB6 - Serial1 TX     (alternate 7)
 * PB7 - Serial1 RX     (alternate 7)
 * PB8 - I2C SCL        (alternate 4)
 * PB9 - I2C SDA        (alternate 4)
 */
#define VAL_GPIOB_MODER             (PIN_MODE_ANALOG(GPIOB_AIN8)        | \
                                     PIN_MODE_ALTERNATE(GPIOB_TXD1)     | \
                                     PIN_MODE_ALTERNATE(GPIOB_RXD1)     | \
                                     PIN_MODE_ALTERNATE(GPIOB_I2C1_C)   | \
                                     PIN_MODE_ALTERNATE(GPIOB_I2C1_D)   | \
                                     PIN_MODE_ALTERNATE(GPIOB_J_TDO)    | \
                                     PIN_MODE_ALTERNATE(GPIOB_J_RST))
#define VAL_GPIOB_OTYPER            (PIN_OTYPE_OPENDRAIN(GPIOB_I2C1_C)  | \
                                     PIN_OTYPE_OPENDRAIN(GPIOB_I2C1_D))
#define VAL_GPIOB_OSPEEDR           0xFFFFFFFF
#define VAL_GPIOB_PUPDR             (~(PIN_PUPDR_FLOATING(GPIOB_J_TDO)             | \
                                       PIN_PUPDR_FLOATING(GPIOB_J_RST)))
#define VAL_GPIOB_ODR               0xFFFFFFFF
#define VAL_GPIOB_AFRL              (PIN_AFIO_AF(GPIOB_J_TDO, 0)        | \
                                     PIN_AFIO_AF(GPIOB_J_RST, 0)        | \
                                     PIN_AFIO_AF(GPIOB_TXD1, 7)         | \
                                     PIN_AFIO_AF(GPIOB_RXD1, 7))
#define VAL_GPIOB_AFRH              (PIN_AFIO_AF(GPIOB_I2C1_C, 4)       | \
                                     PIN_AFIO_AF(GPIOB_I2C1_D, 4))
/*
 * Port C setup.
 * All input with pull-up except:
 * PC0  - AIN1          (analog - BRAKE_PRESSURE1).
 * PC1  - AIN2          (analog - BRAKE_PRESSURE2).
 * PC2  - AIN3          (analog - SEN2).
 * PC3  - AIN4          (analog - SEN3).
 * PC4  - UBAT_DIV      (analog - UBAT).
 * PC5  - AIN7          (analog - SEN4).
 * PC8  - SDIO_D0       (alternate 12).
 * PC9  - SDIO_D1       (alternate 12).
 * PC10 - SDIO_D2       (alternate 12).
 * PC11 - SDIO_D3       (alternate 12).
 * PC12 - SDIO_CLK      (alternate 12).
 * PC14 - OSC32_IN      (input floating).
 * PC15 - OSC32_OUT     (input floating).
 */
#define VAL_GPIOC_MODER             (PIN_MODE_ANALOG(GPIOC_AIN1)        | \
                                     PIN_MODE_ANALOG(GPIOC_AIN2)        | \
                                     PIN_MODE_ANALOG(GPIOC_AIN3)        | \
                                     PIN_MODE_ANALOG(GPIOC_AIN4)        | \
                                     PIN_MODE_ANALOG(GPIOC_UBAT_DIV)    | \
                                     PIN_MODE_ANALOG(GPIOC_AIN7)        | \
                                     PIN_MODE_ALTERNATE(GPIOC_SD_D0)    | \
                                     PIN_MODE_ALTERNATE(GPIOC_SD_D1)    | \
                                     PIN_MODE_ALTERNATE(GPIOC_SD_D2)    | \
                                     PIN_MODE_ALTERNATE(GPIOC_SD_D3)    | \
                                     PIN_MODE_ALTERNATE(GPIOC_SD_CLK))
#define VAL_GPIOC_OTYPER            0x00000000
#define VAL_GPIOC_OSPEEDR           (PIN_OSPEED_25M(GPIOC_SD_D0)    | \
                                     PIN_OSPEED_25M(GPIOC_SD_D1)    | \
                                     PIN_OSPEED_25M(GPIOC_SD_D2)    | \
                                     PIN_OSPEED_25M(GPIOC_SD_D3)    | \
                                     PIN_OSPEED_25M(GPIOC_SD_CLK))
#define VAL_GPIOC_PUPDR             (~(PIN_PUPDR_FLOATING(14)  | \
                                       PIN_PUPDR_FLOATING(15)))
#define VAL_GPIOC_ODR               0xFFFFFFFF
#define VAL_GPIOC_AFRL              0x00000000
#define VAL_GPIOC_AFRH              (PIN_AFIO_AF(GPIOC_SD_D0, 12)     | \
                                     PIN_AFIO_AF(GPIOC_SD_D1, 12)     | \
                                     PIN_AFIO_AF(GPIOC_SD_D2, 12)     | \
                                     PIN_AFIO_AF(GPIOC_SD_D3, 12)     | \
                                     PIN_AFIO_AF(GPIOC_SD_CLK, 12))


/*
 * Port D setup.
 * All input with pull-up except:
 * PD0 - CAN_RX                 (alternate 9)
 * PD1 - CAN_TX                 (alternate 9)
 * PD2 - SDIO_CLK               (alternate 12).
 * PD3 - SD Card installed      (input) 
 * PD5 - TXD2                   (alternate 7) 
 * PD6 - RXD2                   (alternate 7)
 * PD12 - PO5 Horn              (alternate 2)
 * PD13 - PO6 Right front light (alternate 2)
 * PD14 - PO7 Left front light  (alternate 2)
 * PD15 - PO8 Front light       (alternate 2)
 */
#define VAL_GPIOD_MODER             (PIN_MODE_ALTERNATE(GPIOD_CAN_RX)    | \
                                     PIN_MODE_ALTERNATE(GPIOD_CAN_TX)    | \
                                     PIN_MODE_INPUT(GPIOD_SD_CARD_IN)    | \
                                     PIN_MODE_ALTERNATE(GPIOD_SD_CMD)    | \
                                     PIN_MODE_ALTERNATE(GPIOD_TXD2)      | \
                                     PIN_MODE_ALTERNATE(GPIOD_RXD2)      | \
                                     PIN_MODE_ALTERNATE(GPIOD_PO5)       | \
                                     PIN_MODE_ALTERNATE(GPIOD_PO6)       | \
                                     PIN_MODE_ALTERNATE(GPIOD_PO7)       | \
                                     PIN_MODE_ALTERNATE(GPIOD_PO8))
#define VAL_GPIOD_OTYPER            0x00000000
#define VAL_GPIOD_OSPEEDR           (PIN_OSPEED_25M(GPIOD_SD_CMD))
#define VAL_GPIOD_PUPDR             0xFFFFFFFF
#define VAL_GPIOD_ODR               0xFFFFFFFF
#define VAL_GPIOD_AFRL              (PIN_AFIO_AF(GPIOD_CAN_RX, 9)     | \
                                     PIN_AFIO_AF(GPIOD_CAN_TX, 9)     | \
                                     PIN_AFIO_AF(GPIOD_SD_CMD, 12)    | \
                                     PIN_AFIO_AF(GPIOD_TXD2, 7)       | \
                                     PIN_AFIO_AF(GPIOD_RXD2,7 ))

#define VAL_GPIOD_AFRH              (PIN_AFIO_AF(GPIOD_PO5, 2)        | \
                                     PIN_AFIO_AF(GPIOD_PO6, 2)        | \
                                     PIN_AFIO_AF(GPIOD_PO7, 2)        | \
                                     PIN_AFIO_AF(GPIOD_PO8, 2))

/*
 * Port E setup.
 * All input with pull-up.
 * PE9  - BUT8                  (alternate 1 - SPEED)
 * PE15 - 5V Enable             (output)
 */
#define VAL_GPIOE_MODER             (PIN_MODE_ALTERNATE(GPIOE_BUT8)    | \
                                     PIN_MODE_OUTPUT(GPIOE_X5V_EN))
#define VAL_GPIOE_OTYPER            0x00000000
#define VAL_GPIOE_OSPEEDR           0xFFFFFFFF
#define VAL_GPIOE_PUPDR             0xFFFFFFFF
#define VAL_GPIOE_ODR               0xFFFFFFFF
#define VAL_GPIOE_AFRL              0x00000000
#define VAL_GPIOE_AFRH              (PIN_AFIO_AF(GPIOE_BUT8, 1))

/*
 * Port F setup.
 * All input with pull-up except:
 * PF6  - BUT1                  (analog - CURR1)
 * PF7  - BUT2                  (analog - CURR2)
 * PF8  - BUT3                  (analog - CURR2)
 */
#define VAL_GPIOF_MODER             (PIN_MODE_ANALOG(GPIOF_BUT1)        | \
                                     PIN_MODE_ANALOG(GPIOF_BUT2)        | \
                                     PIN_MODE_ANALOG(GPIOF_BUT3))
#define VAL_GPIOF_OTYPER            0x00000000
#define VAL_GPIOF_OSPEEDR           0xFFFFFFFF
#define VAL_GPIOF_PUPDR             0xFFFFFFFF
#define VAL_GPIOF_ODR               0xFFFFFFFF
#define VAL_GPIOF_AFRL              0x00000000
#define VAL_GPIOF_AFRH              0x00000000

/*
 * Port G setup.
 * All input with pull-up.
 */
#define VAL_GPIOG_MODER             0x00000000
#define VAL_GPIOG_OTYPER            0x00000000
#define VAL_GPIOG_OSPEEDR           0xFFFFFFFF
#define VAL_GPIOG_PUPDR             0xFFFFFFFF
#define VAL_GPIOG_ODR               0xFFFFFFFF
#define VAL_GPIOG_AFRL              0x00000000
#define VAL_GPIOG_AFRH              0x00000000

/*
 * Port H setup.
 * All input with pull-up except:
 * PH0 - OSC_IN                     (input floating).
 * PH1 - OSC_OUT                    (input floating).
 * PH12 - LS_4 Throttle             (alternate 2)
 * PH11 - LS_3 TRegenerative brake  (alternate 2)
 */
#define VAL_GPIOH_MODER             (PIN_MODE_ALTERNATE(GPIOH_LS_4)  | \
                                     PIN_MODE_ALTERNATE(GPIOH_LS_3))
#define VAL_GPIOH_OTYPER            0x00000000
#define VAL_GPIOH_OSPEEDR           0xFFFFFFFF
#define VAL_GPIOH_PUPDR             (~(PIN_PUPDR_FLOATING(0)  | \
                                       PIN_PUPDR_FLOATING(1)))
#define VAL_GPIOH_ODR               0xFFFFFFFF
#define VAL_GPIOH_AFRL              0x00000000
#define VAL_GPIOH_AFRH              (PIN_AFIO_AF(GPIOH_LS_4, 2)  | \
                                     PIN_AFIO_AF(GPIOH_LS_3, 2))

/*
 * Port I setup.
 * All input with pull-up.
 */
#define VAL_GPIOI_MODER             0x00000000
#define VAL_GPIOI_OTYPER            0x00000000
#define VAL_GPIOI_OSPEEDR           0xFFFFFFFF
#define VAL_GPIOI_PUPDR             0xFFFFFFFF
#define VAL_GPIOI_ODR               0xFFFFFFFF
#define VAL_GPIOI_AFRL              0x00000000
#define VAL_GPIOI_AFRH              0x00000000

#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
  void boardInit(void);
#ifdef __cplusplus
}
#endif
#endif /* _FROM_ASM_ */

#endif /* _BOARD_H_ */
