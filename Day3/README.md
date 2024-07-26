# 线程间同步和线程间通信

## 线程同步

### 信号量

#### 概念：通过信号量获得公共资源

#### 分类：

* 互斥信号量：一般不用，因为会引起优先级反转；
* 二值信号量：用于解决同步问题；（两个值要么零，要么一，用于两个线程之间的同步，例如当按钮按下，led灯亮起）
* 计数信号量：用于解决资源计数问题；（计数信号量用于控制系统中共享资源的多个实例的使用，允许多个线程同时访问同一种资 源的多个实例，计数信号量被初始化为n（非负整数），n为该种共享资源的数目）

#### 信号量API

``` c
//信号量控制块
struct rt_semaphore
{
   struct rt_ipc_object parent;  /* 继承自 ipc_object 类 */
   rt_uint16_t value;            /* 信号量的值 */
};
/* rt_sem_t 是指向 semaphore 结构体的指针类型 */
typedef struct rt_semaphore* rt_sem_t;

//创建信号量  动态获取
 rt_sem_t rt_sem_create(const char *name,
                        rt_uint32_t value,
                        rt_uint8_t flag);

//删除信号量
rt_err_t rt_sem_delete(rt_sem_t sem);

//初始化信号量  静态获取
rt_err_t rt_sem_init(rt_sem_t       sem,
                    const char     *name,
                    rt_uint32_t    value,
                    rt_uint8_t     flag)
//脱离信号量
rt_err_t rt_sem_detach(rt_sem_t sem);

//获取信号量
rt_err_t rt_sem_take (rt_sem_t sem, rt_int32_t time);


//无等待获取信号量
rt_err_t rt_sem_trytake(rt_sem_t sem);

//释放信号量
rt_err_t rt_sem_release(rt_sem_t sem);


```



示例：

```c
#include <board.h>
#include <rtthread.h>
#include <drv_gpio.h>
#include<drv_lcd.h>
#ifndef RT_USING_NANO
#include <rtdevice.h>
#endif /* RT_USING_NANO */

    // PC0:  KEY0         --> KEY
#define PIN_KEY1      GET_PIN(C, 1)

#define GPIO_LED_B    GET_PIN(F, 11)
#define GPIO_LED_R    GET_PIN(F, 12)

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       1024
#define THREAD_TIMESLICE        5

static rt_thread_t tid1 = RT_NULL;
static rt_thread_t tid2= RT_NULL;
static rt_thread_t tid3= RT_NULL;
static rt_sem_t dynamic_sem = RT_NULL;

static void key_name_entry(void *parameter);
static void led_name_entry(void *parameter);
static void lcd_name_entry(void *parameter);
int main(void)
{
    rt_pin_mode(GPIO_LED_R, PIN_MODE_OUTPUT);
    rt_pin_mode(GPIO_LED_B, PIN_MODE_OUTPUT);
    rt_pin_mode(PIN_KEY1, PIN_MODE_INPUT_PULLUP);
    dynamic_sem = rt_sem_create("dsem", 2, RT_IPC_FLAG_PRIO);
    if (dynamic_sem == RT_NULL)
    {
        rt_kprintf("create dynamic semaphore failed.\n");
        return -1;
    }
    else
    {
        rt_kprintf("create done. dynamic semaphore value = 0.\n");
    }

    tid1 = rt_thread_create("key_thread",
                            key_name_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);
    if (tid1 != RT_NULL)
        rt_thread_startup(tid1);

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

    // while (1)
    // {
    //     rt_pin_write(GPIO_LED_R, PIN_HIGH);
    //     rt_thread_mdelay(500);
    //     rt_pin_write(GPIO_LED_R, PIN_LOW);
    //     rt_thread_mdelay(500);
    // }
}

static void key_name_entry(void *parameter)
{

    while (1)
    {
        if (rt_pin_read(PIN_KEY1) == PIN_LOW)
        {
            rt_thread_mdelay(100);
            if (rt_pin_read(PIN_KEY1) == PIN_LOW)
            {
                rt_kprintf("KEY0 pressed!\r\n");
                rt_sem_release(dynamic_sem);
            }
        }
        rt_thread_mdelay(10);
    }
}

static void led_name_entry(void *parameter)
{
    rt_uint32_t result = 0;
    while (1)
    {
        result = rt_sem_take(dynamic_sem, RT_WAITING_FOREVER);
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
    }
}

static void lcd_name_entry(void *parameter)
{
    rt_uint32_t result = 0;
    while (1)
    {
        result = rt_sem_take(dynamic_sem, RT_WAITING_FOREVER);
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
    }
}
```

### 互斥量

#### 概念： 是一种特殊的二值信息量，具有防优先级翻转的特性

##### 优先级反转(Priority Inversion)： 所谓优先级翻转，即当一个高优先级线程试图通过信号量机制访问共享资源时，如果该信号量已被一低优 先级线程持有，而这个低优先级线程在运行过程中可能又被其它一些中等优先级的线程抢占，因此造成高 优先级线程被许多具有较低优先级的线程阻塞，实时性难以得到保证

#### API

```c
//控制块
struct rt_mutex
    {
        struct rt_ipc_object parent;                /* 继承自 ipc_object 类 */

        rt_uint16_t          value;                   /* 互斥量的值 */
        rt_uint8_t           original_priority;     /* 持有线程的原始优先级 */
        rt_uint8_t           hold;                     /* 持有线程的持有次数   */
        struct rt_thread    *owner;                 /* 当前拥有互斥量的线程 */
    };
    /* rt_mutext_t 为指向互斥量结构体的指针类型  */
    typedef struct rt_mutex* rt_mutex_t;
struct rt_mutex
    {
        struct rt_ipc_object parent;                /* 继承自 ipc_object 类 */

        rt_uint16_t          value;                   /* 互斥量的值 */
        rt_uint8_t           original_priority;     /* 持有线程的原始优先级 */
        rt_uint8_t           hold;                     /* 持有线程的持有次数   */
        struct rt_thread    *owner;                 /* 当前拥有互斥量的线程 */
    };
    /* rt_mutext_t 为指向互斥量结构体的指针类型  */
    typedef struct rt_mutex* rt_mutex_t;

//创建
rt_mutex_t rt_mutex_create (const char* name, rt_uint8_t flag);
//删除
rt_err_t rt_mutex_delete (rt_mutex_t mutex);
//初始化
rt_err_t rt_mutex_init (rt_mutex_t mutex, const char* name, rt_uint8_t flag);
//脱离
rt_err_t rt_mutex_detach (rt_mutex_t mutex);
//获取
rt_err_t rt_mutex_take (rt_mutex_t mutex, rt_int32_t time);
//无等待获取
rt_err_t rt_mutex_trytake(rt_mutex_t mutex);
//释放
rt_err_t rt_mutex_release(rt_mutex_t mutex);

```

#### 示例：

```c

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
```

### 事件集

#### 概念：事件集也是线程间同步的机制之一，一个事件集可以包含多个事件，利用事件集可以完成一对多，多对多的线程间同步。
#### 工作机制：事件集主要用于线程间的同步，与信号量不同，它的特点是可以实现一对多，多对多的同步。即一个线程与多个事件的关系可设置为：其中任意一个事件唤醒线程，或几个事件都到达后才唤醒线程进行后续的处理；同样，事件也可以是多个线程同步多个事件。这种多个事件的集合可以用一个 32 位无符号整型变量来表示，变量的每一位代表一个事件，线程通过 “逻辑与” 或“逻辑或”将一个或多个事件关联起来，形成事件组合。事件的 “逻辑或” 也称为是独立型同步，指的是线程与任何事件之一发生同步；事件 “逻辑与” 也称为是关联型同步，指的是线程与若干事件都发生同步。



### API

```C
//控制块
struct rt_event
{
    struct rt_ipc_object parent;    /* 继承自 ipc_object 类 */

    /* 事件集合，每一 bit 表示 1 个事件，bit 位的值可以标记某事件是否发生 */
    rt_uint32_t set;
};
/* rt_event_t 是指向事件结构体的指针类型  */
typedef struct rt_event* rt_event_t;

//创建
rt_event_t rt_event_create(const char* name, rt_uint8_t flag);

//删除
rt_err_t rt_event_delete(rt_event_t event);

//初始化
rt_err_t rt_event_init(rt_event_t event, const char* name, rt_uint8_t flag);

//脱离
rt_err_t rt_event_detach(rt_event_t event);

//发送
rt_err_t rt_event_send(rt_event_t event, rt_uint32_t set);

//接收
rt_err_t rt_event_recv(rt_event_t event,
                           rt_uint32_t set,
                           rt_uint8_t option,
                           rt_int32_t timeout,
                           rt_uint32_t* recved);

/* 选择 逻辑与 或 逻辑或 的方式接收事件 */
RT_EVENT_FLAG_OR
RT_EVENT_FLAG_AND

/* 选择清除重置事件标志位 */
RT_EVENT_FLAG_CLEAR


```

### 示例：

```c

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

```

## 线程间通信



### 消息邮箱

> RT-Thread 操作系统的邮箱用于线程间通信，特点是开销比较低，效率较高。邮箱中的每一封邮件 只能容纳固定的 4 字节内容（针对 32 位处理系统，指针的大小即为 4 个字节，所以一封邮件恰 好能够容纳一个指针）。典型的邮箱也称作交换消息，如下图所示，线程或中断服务例程把一封 4  字节长度的邮件发送到邮箱中，而一个或多个线程可以从邮箱中接收这些邮件并进行处理。



#### API

```c
//控制块
struct rt_mailbox
{
    struct rt_ipc_object parent;

    rt_uint32_t* msg_pool;                /* 邮箱缓冲区的开始地址 */
    rt_uint16_t size;                     /* 邮箱缓冲区的大小     */

    rt_uint16_t entry;                    /* 邮箱中邮件的数目     */
    rt_uint16_t in_offset, out_offset;    /* 邮箱缓冲的进出指针   */
    rt_list_t suspend_sender_thread;      /* 发送线程的挂起等待队列 */
};
typedef struct rt_mailbox* rt_mailbox_t;

//创建
rt_mailbox_t rt_mb_create (const char* name, rt_size_t size, rt_uint8_t flag);

//删除
rt_err_t rt_mb_delete (rt_mailbox_t mb);

//初始化
  rt_err_t rt_mb_init(rt_mailbox_t mb,
                    const char* name,
                    void* msgpool,
                    rt_size_t size,
                    rt_uint8_t flag)

//脱离
rt_err_t rt_mb_detach(rt_mailbox_t mb);

//发送
rt_err_t rt_mb_send (rt_mailbox_t mb, rt_uint32_t value);

//等待发送
rt_err_t rt_mb_send_wait (rt_mailbox_t mb,
                      rt_uint32_t value,
                      rt_int32_t timeout);

//发送紧急邮件
rt_err_t rt_mb_urgent (rt_mailbox_t mb, rt_ubase_t value);

//接受邮件
rt_err_t rt_mb_recv (rt_mailbox_t mb, rt_uint32_t* value, rt_int32_t timeout);

```



### 消息队列

> 消息队列，也就是将多条消息排成的队列形式，是一种常用的线程间通信 方式，可以应用在多种场合，线程间的消息交换，使用串口接收不定长数据等。线 程可以将一条或多条消息放到消息队列中，同样一个或多个线程可以从消息队列中 获得消息；同时消息队列提供异步处理机制可以起到缓冲消息的作用

遵循先进先出原则

### API

```c
//控制块
struct rt_messagequeue
{
    struct rt_ipc_object parent;

    void* msg_pool;                     /* 指向存放消息的缓冲区的指针 */

    rt_uint16_t msg_size;               /* 每个消息的长度 */
    rt_uint16_t max_msgs;               /* 最大能够容纳的消息数 */

    rt_uint16_t entry;                  /* 队列中已有的消息数 */

    void* msg_queue_head;               /* 消息链表头 */
    void* msg_queue_tail;               /* 消息链表尾 */
    void* msg_queue_free;               /* 空闲消息链表 */

    rt_list_t suspend_sender_thread;    /* 发送线程的挂起等待队列 */
};
typedef struct rt_messagequeue* rt_mq_t;

//创建
rt_mq_t rt_mq_create(const char* name, rt_size_t msg_size,
            rt_size_t max_msgs, rt_uint8_t flag);

//删除
rt_err_t rt_mq_delete(rt_mq_t mq);

//初始化
rt_err_t rt_mq_init(rt_mq_t mq, const char* name,
                        void *msgpool, rt_size_t msg_size,
                        rt_size_t pool_size, rt_uint8_t flag);

//脱离
rt_err_t rt_mq_detach(rt_mq_t mq);

//发送
rt_err_t rt_mq_send (rt_mq_t mq, void* buffer, rt_size_t size);

//等待发送
rt_err_t rt_mq_send_wait(rt_mq_t     mq,
                         const void *buffer,
                         rt_size_t   size,
                         rt_int32_t  timeout);

//发送紧急消息
rt_err_t rt_mq_urgent(rt_mq_t mq, void* buffer, rt_size_t size);

//接收消息
rt_ssize_t rt_mq_recv (rt_mq_t mq, void* buffer,
                    rt_size_t size, rt_int32_t timeout);


```

