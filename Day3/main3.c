
#include <board.h>
#include <rtthread.h>
#include <drv_gpio.h>
#include<drv_lcd.h>
#ifndef RT_USING_NANO
#include <rtdevice.h>
#endif /* RT_USING_NANO */

#define PIN_KEY_DOWN      GET_PIN(C, 1)
#define PIN_KEY_LEFT      GET_PIN(C, 0)
#define GPIO_LED_B    GET_PIN(F, 11)
#define GPIO_LED_R    GET_PIN(F, 12)

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       1024
#define THREAD_TIMESLICE        5

#define EVENT_FLAG3 (1 << 3)  
#define EVENT_FLAG5 (1 << 5)

static struct rt_thread tid1 ;
static struct rt_thread tid2;
static struct rt_thread tid3;
static struct rt_thread tid4;
static struct rt_event event;

static void key_DOWN_name_entry(void *parameter);
static void key_LEFT_name_entry(void *parameter);
static void led_name_entry(void *parameter);
static void lcd_name_entry(void *parameter);

static char thread1_stack[1024];
static char thread2_stack[1024];
static char thread3_stack[1024];
static char thread4_stack[1024];

int main(void)
{
    rt_uint32_t result = 0;
    rt_pin_mode(GPIO_LED_R, PIN_MODE_OUTPUT);
    rt_pin_mode(GPIO_LED_B, PIN_MODE_OUTPUT);
    rt_pin_mode(PIN_KEY_LEFT, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(PIN_KEY_DOWN, PIN_MODE_INPUT_PULLUP);
    result = rt_event_init(&event, "event", RT_IPC_FLAG_PRIO);
    if (result != RT_EOK)
    {
        rt_kprintf("init event failed.\n");
        return -1;
    }

    /* initiate the thread #1 (statically) */
    rt_thread_init(&tid1,
                   "key_left",
                   key_LEFT_name_entry,
                   RT_NULL,
                   &thread1_stack[0],
                   sizeof(thread1_stack),
                   THREAD_PRIORITY - 1, THREAD_TIMESLICE);
#ifdef RT_USING_SMP
    /* Bind threads to the same core to avoid messy log output when multiple cores are enabled */
    rt_thread_control(&thread1, RT_THREAD_CTRL_BIND_CPU, (void*)0);
#endif
    rt_thread_startup(&tid1); /* start thread #1 */

    /* initiate the thread #2 (statically) */
    rt_thread_init(&tid2,
                   "key  down",
                   key_DOWN_name_entry,
                   RT_NULL,
                   &thread2_stack[0],
                   sizeof(thread2_stack),
                   THREAD_PRIORITY, THREAD_TIMESLICE);
#ifdef RT_USING_SMP
    /* Bind threads to the same core to avoid messy log output when multiple cores are enabled */
    rt_thread_control(&thread2, RT_THREAD_CTRL_BIND_CPU, (void*)0);
#endif
    rt_thread_startup(&tid2); /* start thread #2 */

    rt_thread_init(&tid3,
                   "led",
                   led_name_entry,
                   RT_NULL,
                   &thread3_stack[0],
                   sizeof(thread3_stack),
                   THREAD_PRIORITY, THREAD_TIMESLICE);
#ifdef RT_USING_SMP
    /* Bind threads to the same core to avoid messy log output when multiple cores are enabled */
    rt_thread_control(&thread2, RT_THREAD_CTRL_BIND_CPU, (void*)0);
#endif
    rt_thread_startup(&tid3); /* start thread #2 */


    rt_thread_init(&tid4,
                   "lcd",
                   lcd_name_entry,
                   RT_NULL,
                   &thread4_stack[0],
                   sizeof(thread4_stack),
                   THREAD_PRIORITY, THREAD_TIMESLICE);
#ifdef RT_USING_SMP
    /* Bind threads to the same core to avoid messy log output when multiple cores are enabled */
    rt_thread_control(&thread2, RT_THREAD_CTRL_BIND_CPU, (void*)0);
#endif
    rt_thread_startup(&tid4); /* start thread #2 */

}


static void led_name_entry(void *parameter)
{
    rt_uint32_t result = 0;
    rt_uint32_t e;
    while (1)
    {
        if (rt_event_recv(&event, (EVENT_FLAG3 | EVENT_FLAG5),
                      RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
                      RT_WAITING_FOREVER, &e) == RT_EOK)
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
    }
}

static void lcd_name_entry(void *parameter)
{
    rt_uint32_t result = 0;
    rt_uint32_t e;
    while (1)
    {

        if (rt_event_recv(&event, (EVENT_FLAG3 | EVENT_FLAG5),
                      RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,
                      RT_WAITING_FOREVER, &e) == RT_EOK)
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

    }
}

static void key_LEFT_name_entry(void *parameter)
{
    while (1)
    {
        if (rt_pin_read(PIN_KEY_LEFT) == PIN_LOW)
        {
            rt_thread_mdelay(100);
            if (rt_pin_read(PIN_KEY_LEFT) == PIN_LOW)
            {
                rt_event_send(&event, EVENT_FLAG3);
            }
        }
        rt_thread_mdelay(10);
    }
}

static void key_DOWN_name_entry(void *parameter)
{
    while (1)
    {
        if (rt_pin_read(PIN_KEY_DOWN) == PIN_LOW)
        {
            rt_thread_mdelay(100);
            if (rt_pin_read(PIN_KEY_DOWN) == PIN_LOW)
            {
                rt_event_send(&event, EVENT_FLAG5);
            }
        }
        rt_thread_mdelay(10);
    }
}
