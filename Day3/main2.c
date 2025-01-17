
#include <board.h>
#include <rtthread.h>
#include <drv_gpio.h>
#include<drv_lcd.h>
#ifndef RT_USING_NANO
#include <rtdevice.h>
#endif /* RT_USING_NANO */

// #define PIN_KEY1      GET_PIN(C, 1)

#define GPIO_LED_B    GET_PIN(F, 11)
#define GPIO_LED_R    GET_PIN(F, 12)

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       1024
#define THREAD_TIMESLICE        5

// static rt_thread_t tid1 = RT_NULL;
static rt_thread_t tid2= RT_NULL;
static rt_thread_t tid3= RT_NULL;
static rt_mutex_t dynamic_mutex = RT_NULL;

// static void key_name_entry(void *parameter);
static void led_name_entry(void *parameter);
static void lcd_name_entry(void *parameter);


int main(void)
{
    rt_pin_mode(GPIO_LED_R, PIN_MODE_OUTPUT);
    rt_pin_mode(GPIO_LED_B, PIN_MODE_OUTPUT);
    // rt_pin_mode(PIN_KEY1, PIN_MODE_INPUT_PULLUP);
    dynamic_mutex = rt_mutex_create("dynamic_mutex", RT_IPC_FLAG_PRIO);
    if (dynamic_mutex == RT_NULL)
    {
        rt_kprintf("create dynamic semaphore failed.\n");
        return -1;
    }
    else
    {
        rt_kprintf("create done. dynamic semaphore value = 0.\n");
    }

    // tid1 = rt_thread_create("key_thread",
    //                         key_name_entry, RT_NULL,
    //                         THREAD_STACK_SIZE,
    //                         THREAD_PRIORITY, THREAD_TIMESLICE);
    // if (tid1 != RT_NULL)
    //     rt_thread_startup(tid1);

    tid2 = rt_thread_create("led_thread",
                            led_name_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);
    if (tid2 != RT_NULL)
        rt_thread_startup(tid2);

    tid3 = rt_thread_create("lcd_thread",
                            lcd_name_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);
    if (tid3 != RT_NULL)
        rt_thread_startup(tid3);




}


static void led_name_entry(void *parameter)
{
    rt_uint32_t result = 0;
    while (1)
    {
        result = rt_mutex_take(dynamic_mutex, RT_WAITING_FOREVER);
        if (result == RT_EOK)
        {
            rt_kprintf("LED HIGH\r\n");
            rt_pin_write(GPIO_LED_R, PIN_HIGH);
            rt_thread_mdelay(500);
            rt_pin_write(GPIO_LED_B, PIN_HIGH);
            rt_thread_mdelay(500);
            rt_pin_write(GPIO_LED_R, PIN_LOW);
            rt_pin_write(GPIO_LED_B, PIN_LOW);
        }
        else
        {
            rt_kprintf("LED HLOW\r\n");
        }
        rt_mutex_release(dynamic_mutex);
    }
}

static void lcd_name_entry(void *parameter)
{
    rt_uint32_t result = 0;
    while (1)
    {
        result = rt_mutex_take(dynamic_mutex, RT_WAITING_FOREVER);
        if (result == RT_EOK)
        {   
            LCD_DisplayOn();
            rt_thread_mdelay(1000);
            rt_kprintf("OPEN LCD\r\n");
            lcd_clear(WHITE);
            lcd_set_color(WHITE, BLACK);
            lcd_show_string(10, 10, 32, "RT-Thread");
            lcd_show_string(10, 50, 32, "STM32F104");
            LCD_DisplayOff();
        }
        else
        {
            rt_kprintf("NOT OPEN LCD");
        }
        rt_mutex_release(dynamic_mutex);
    }
}