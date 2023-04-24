// @description:                           //

#include <string.h>
#include "core.Range.h"
#include "gpio_common.h"
#include "meta_obj.h"
#include "syscalls.h"
#include "utils.h"
#include "class.h"
#include "core.h"

#include "Led.h"

/*****************************HEAR-FILE************************************/
#include "fpioa.h"
#include "gpio.h"

/*****************************HARDWARE-PIN*********************************/
// 硬件IO口，与原理图对应
#define PIN_LED_0             (0)
#define PIN_LED_1             (17)

/*****************************SOFTWARE-GPIO********************************/
// 软件GPIO口，与程序对应
#define LED0_GPIONUM          (0)
#define LED1_GPIONUM          (1)

/*****************************FUNC-GPIO************************************/
// GPIO口的功能，绑定到硬件IO口
#define FUNC_LED0             (FUNC_GPIO0 + LED0_GPIONUM)
#define FUNC_LED1             (FUNC_GPIO0 + LED1_GPIONUM)

bool primLedInit(VM *vm, Value *args)
{
    fpioa_set_function(PIN_LED_0, FUNC_LED0);
    fpioa_set_function(PIN_LED_1, FUNC_LED1);

    gpio_init();    // 使能GPIO的时钟
    
    // 设置LED0和LED1的GPIO模式为输出
    gpio_set_drive_mode(LED0_GPIONUM, GPIO_DM_OUTPUT);
    gpio_set_drive_mode(LED1_GPIONUM, GPIO_DM_OUTPUT);

    // 先关闭LED0和LED1
    gpio_pin_value_t value = GPIO_PV_HIGH;
    gpio_set_pin(LED0_GPIONUM, value);
    gpio_set_pin(LED1_GPIONUM, value);

    RET_NULL

}

bool primLedSet(VM *vm, Value *args){
    
    int pin = (int)VALUE_TO_NUM(args[1]);
    gpio_pin_value_t value = (gpio_pin_value_t)VALUE_TO_NUM(args[2]);

    gpio_set_pin(pin, value);

    RET_NULL
}

void extenLedBind(VM *vm, ObjModule *coreModule){
    Class *templatClass = VALUE_TO_CLASS(getCoreClassValue(coreModule, "Led"));
    PRIM_METHOD_BIND(templatClass->objHeader.class, "ledInit_()", primLedInit);
    PRIM_METHOD_BIND(templatClass->objHeader.class, "ledSet_(_,_)", primLedSet);
}