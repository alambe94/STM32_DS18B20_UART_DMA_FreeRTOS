/*
*********************************************************************************************************
*                                            EXAMPLE CODE
*
*               This file is provided as an example on how to use Micrium products.
*
*               Please feel free to use any application code labeled as 'EXAMPLE CODE' in
*               your application products.  Example code may be used as is, in whole or in
*               part, or may be used as a reference only. This file can be modified as
*               required to meet the end-product requirements.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*
*               You can find our product's user manual, API reference, release notes and
*               more information at https://doc.micrium.com.
*               You can contact us at www.micrium.com.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                    MICRIUM BOARD SUPPORT PACKAGE
*
* Filename : bsp_int.h
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                               MODULE
*
* Note(s) : (1) This header file is protected from multiple pre-processor inclusion through use of the
*               BSP present pre-processor macro definition.
*********************************************************************************************************
*/

#ifndef  BSP_INT_PRESENT
#define  BSP_INT_PRESENT


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#define    CPU_BSP_MODULE
#include  <cpu_core.h>
#include  "bsp_clk.h"


/*
*********************************************************************************************************
*                                     EXTERNAL C LANGUAGE LINKAGE
*
* Note(s) : (1) C++ compilers MUST 'extern'ally declare ALL C function prototypes & variable/object
*               declarations for correct C language linkage.
*********************************************************************************************************
*/

#ifdef __cplusplus
extern  "C" {                                                   /* See Note #1.                                         */
#endif


/*
*********************************************************************************************************
*                                               DEFINES
*
* Note(s) : (1) The Cortex-M "Vector Table Offset Register" section states the following:
*
*               You must align the offset to the number of exception entries in the vector table. The
*               minimum alignment is 32 words, enough for up to 16 interrupts. For more interrupts,
*               adjust the alignment by rounding up to the next power of two. For example, if you require
*               21 interrupts, the alignment must be on a 64-word boundary because the required table
*               size is 37 words, and the next power of two is 64. SEE YOUR VENDOR DOCUMENTATION FOR THE
*               ALIGNMENT DETAILS FOR YOUR DEVICE.
*********************************************************************************************************
*/

#define  INT_ID_MAX_NBR               95u                       /* Max. number of ext. interrupt sources. (Check MCU RM)*/

                                                                /* 111 VTOR entries; next power of 2 is 128             */
#define  INT_VTOR_TBL_SIZE         (INT_ID_MAX_NBR + CPU_INT_EXT0)
#define  INT_VTOR_TBL_ALIGNMENT    (0x200uL)                    /* 128 * 4 = 512 words. See note 1                      */


/*
*********************************************************************************************************
*                                              DATA TYPES
*********************************************************************************************************
*/
                                                                /* -------- STM32L4x5/STM32L4x6 Specific Intr. -------- */
typedef  enum  bsp_int_id {
    INT_ID_WWDG                   = 0u,                         /* Window WatchDog interrupt                            */
    INT_ID_PVD_PVM                = 1u,                         /* PVD/PVM1/PVM2/PVM3/PVM4 through EXTI Line detection  */
    INT_ID_TAMP_STAMP             = 2u,                         /* Tamper or TimeStamp interrupts through the EXTI line */
    INT_ID_RTC_WKUP               = 3u,                         /* RTC Wakeup interrupt through the EXTI line           */
    INT_ID_FLASH                  = 4u,                         /* FLASH global interrupt                               */
    INT_ID_RCC                    = 5u,                         /* RCC global interrupt                                 */
    INT_ID_EXTI0                  = 6u,                         /* EXTI Line0 interrupt                                 */
    INT_ID_EXTI1                  = 7u,                         /* EXTI Line1 interrupt                                 */
    INT_ID_EXTI2                  = 8u,                         /* EXTI Line2 interrupt                                 */
    INT_ID_EXTI3                  = 9u,                         /* EXTI Line3 interrupt                                 */
    INT_ID_EXTI4                  = 10u,                        /* EXTI Line4 interrupt                                 */
    INT_ID_DMA1_Channel1          = 11u,                        /* DMA1 Channel 1 global interrupt                      */
    INT_ID_DMA1_Channel2          = 12u,                        /* DMA1 Channel 2 global interrupt                      */
    INT_ID_DMA1_Channel3          = 13u,                        /* DMA1 Channel 3 global interrupt                      */
    INT_ID_DMA1_Channel4          = 14u,                        /* DMA1 Channel 4 global interrupt                      */
    INT_ID_DMA1_Channel5          = 15u,                        /* DMA1 Channel 5 global interrupt                      */
    INT_ID_DMA1_Channel6          = 16u,                        /* DMA1 Channel 6 global interrupt                      */
    INT_ID_DMA1_Channel7          = 17u,                        /* DMA1 Channel 7 global interrupt                      */
    INT_ID_ADC1                   = 18u,                        /* ADC1 global interrupt                                */
    INT_ID_CAN1_TX                = 19u,                        /* CAN1 TX interrupt                                    */
    INT_ID_CAN1_RX0               = 20u,                        /* CAN1 RX0 interrupt                                   */
    INT_ID_CAN1_RX1               = 21u,                        /* CAN1 RX1 interrupt                                   */
    INT_ID_CAN1_SCE               = 22u,                        /* CAN1 SCE interrupt                                   */
    INT_ID_EXTI9_5                = 23u,                        /* External Line[9:5] interrupts                        */
    INT_ID_TIM1_BRK_TIM15         = 24u,                        /* TIM1 Break interrupt and TIM15 global interrupt      */
    INT_ID_TIM1_UP_TIM16          = 25u,                        /* TIM1 Update interrupt and TIM16 global interrupt     */
    INT_ID_TIM1_TRG_COM_TIM17     = 26u,                        /* TIM1 Trigger & Commutation/TIM17 interrupt           */
    INT_ID_TIM1_CC                = 27u,                        /* TIM1 Capture Compare interrupt                       */
    INT_ID_TIM2                   = 28u,                        /* TIM2 global interrupt                                */
    INT_ID_TIM3                   = 29u,                        /* TIM3 global interrupt                                */
    INT_ID_TIM4                   = 30u,                        /* TIM4 global interrupt                                */
    INT_ID_I2C1_EV                = 31u,                        /* I2C1 Event interrupt                                 */
    INT_ID_I2C1_ER                = 32u,                        /* I2C1 Error interrupt                                 */
    INT_ID_I2C2_EV                = 33u,                        /* I2C2 Event interrupt                                 */
    INT_ID_I2C2_ER                = 34u,                        /* I2C2 Error interrupt                                 */
    INT_ID_SPI1                   = 35u,                        /* SPI1 global interrupt                                */
    INT_ID_SPI2                   = 36u,                        /* SPI2 global interrupt                                */
    INT_ID_USART1                 = 37u,                        /* USART1 global interrupt                              */
    INT_ID_USART2                 = 38u,                        /* USART2 global interrupt                              */
    INT_ID_USART3                 = 39u,                        /* USART3 global interrupt                              */
    INT_ID_EXTI15_10              = 40u,                        /* External Line[15:10] interrupts                      */
    INT_ID_RTC_Alarm              = 41u,                        /* RTC Alarm (A and B) through EXTI Line interrupt      */
    INT_ID_DFSDM1_FLT3            = 42u,                        /* DFSDM1 Filter 3 global interrupt                     */
    INT_ID_TIM8_BRK               = 43u,                        /* TIM8 Break interrupt                                 */
    INT_ID_TIM8_UP                = 44u,                        /* TIM8 Update interrupt                                */
    INT_ID_TIM8_TRG_COM           = 45u,                        /* TIM8 Trigger and Commutation interrupt               */
    INT_ID_TIM8_CC                = 46u,                        /* TIM8 Capture Compare interrupt                       */
    INT_ID_RSVD_47                = 47u,                        /* Reserved                                             */
    INT_ID_FMC                    = 48u,                        /* FMC global interrupt                                 */
    INT_ID_SDMMC1                 = 49u,                        /* SDMMC1 global interrupt                              */
    INT_ID_TIM5                   = 50u,                        /* TIM5 global interrupt                                */
    INT_ID_SPI3                   = 51u,                        /* SPI3 global interrupt                                */
    INT_ID_UART4                  = 52u,                        /* UART4 global interrupt                               */
    INT_ID_UART5                  = 53u,                        /* UART5 global interrupt                               */
    INT_ID_TIM6_DAC               = 54u,                        /* TIM6 global and DAC1&2 underrun error  interrupts    */
    INT_ID_TIM7                   = 55u,                        /* TIM7 global interrupt                                */
    INT_ID_DMA2_Channel1          = 56u,                        /* DMA2 Channel 1 global interrupt                      */
    INT_ID_DMA2_Channel2          = 57u,                        /* DMA2 Channel 2 global interrupt                      */
    INT_ID_DMA2_Channel3          = 58u,                        /* DMA2 Channel 3 global interrupt                      */
    INT_ID_DMA2_Channel4          = 59u,                        /* DMA2 Channel 4 global interrupt                      */
    INT_ID_DMA2_Channel5          = 60u,                        /* DMA2 Channel 5 global interrupt                      */
    INT_ID_DFSDM1_FLT0            = 61u,                        /* DFSDM1 Filter 0 global interrupt                     */
    INT_ID_DFSDM1_FLT1            = 62u,                        /* DFSDM1 Filter 1 global interrupt                     */
    INT_ID_DFSDM1_FLT2            = 63u,                        /* DFSDM1 Filter 2 global interrupt                     */
    INT_ID_COMP                   = 64u,                        /* COMP1 and COMP2 interrupts                           */
    INT_ID_LPTIM1                 = 65u,                        /* LP TIM1 interrupt                                    */
    INT_ID_LPTIM2                 = 66u,                        /* LP TIM2 interrupt                                    */
    INT_ID_OTG_FS                 = 67u,                        /* USB OTG FS global interrupt                          */
    INT_ID_DMA2_Channel6          = 68u,                        /* DMA2 Channel 6 global interrupt                      */
    INT_ID_DMA2_Channel7          = 69u,                        /* DMA2 Channel 7 global interrupt                      */
    INT_ID_LPUART1                = 70u,                        /* LP UART1 interrupt                                   */
    INT_ID_OCTOSPI1               = 71u,                        /* Octo SPI 1 global interrupt                          */
    INT_ID_I2C3_EV                = 72u,                        /* I2C3 event interrupt                                 */
    INT_ID_I2C3_ER                = 73u,                        /* I2C3 error interrupt                                 */
    INT_ID_SAI1                   = 74u,                        /* Serial Audio Interface 1 global interrupt            */
    INT_ID_SAI2                   = 75u,                        /* Serial Audio Interface 2 global interrupt            */
    INT_ID_OCTOSPI2               = 76u,                        /* Octo SPI 2 global interrupt                          */
    INT_ID_TSC                    = 77u,                        /* Touch Sense Controller global interrupt              */
    INT_ID_DSIHSOT                = 78u,                        /* DSI global interrupt                                 */
    INT_ID_AES                    = 79u,                        /* AES global interrupt                                 */
    INT_ID_RNG                    = 80u,                        /* RNG global interrupt                                 */
    INT_ID_FPU                    = 81u,                        /* FPU global interrupt                                 */
    INT_ID_HASH_CRS               = 82u,                        /* HASH and CRS interrupt                               */
    INT_ID_I2C4_EV                = 83u,                        /* I2C4 event interrupt                                 */
    INT_ID_I2C4_ER                = 84u,                        /* I2C4 error interrupt                                 */
    INT_ID_DCMI                   = 85u,                        /* DCMI global interrupt                                */
    INT_ID_RSVD_86                = 86u,                        /* Reserved                                             */
    INT_ID_RSVD_87                = 87u,                        /* Reserved                                             */
    INT_ID_RSVD_88                = 88u,                        /* Reserved                                             */
    INT_ID_RSVD_89                = 89u,                        /* Reserved                                             */
    INT_ID_DMA2D                  = 90u,                        /* DMA2D global interrupt                               */
    INT_ID_LCD_TFT                = 91u,                        /* LTDC global interrupt                                */
    INT_ID_LCD_TFT_ER             = 92u,                        /* LTDC global error interrupt                          */
    INT_ID_GFXMMU                 = 93u,                        /* GFXMMU global error interrupt                        */
    INT_ID_DMAMUX1_OVR            = 94u                         /* DMAMUX Overrung interrupt                            */

} BSP_INT_ID;


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void  BSP_IntInit   (void);

void  BSP_IntEnable (BSP_INT_ID     int_id);

void  BSP_IntDisable(BSP_INT_ID     int_id);

void  BSP_IntClear  (BSP_INT_ID     int_id);

void  BSP_IntVectSet(BSP_INT_ID     int_id,
                     CPU_INT08U     int_prio,
                     CPU_INT08U     int_type,
                     CPU_FNCT_VOID  isr_handler);


/*
*********************************************************************************************************
*                                   EXTERNAL C LANGUAGE LINKAGE END
*********************************************************************************************************
*/

#ifdef __cplusplus
}                                                               /* End of 'extern'al C lang linkage.                    */
#endif


/*
*********************************************************************************************************
*                                              MODULE END
*********************************************************************************************************
*/

#endif                                                          /* End of module include.                               */
