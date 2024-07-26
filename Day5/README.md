# 软件包和组件

## 软件包

**概念**：具有特定的功能，用来完成特定任务的一个程序或一组程序。可以粗略地把他理解为帮助我们完成了底层驱动的编写，我们只需要使用里面提供的API就好了。

RT_Thread提供了很多软件包可以供我们选择，而且操作十分简单。

先在env输入 ```menuconfig``` 打开菜单就可以选择我们需要软件包了

**下载软件包** : 在env输入```pkgs -update``` ,就可以下载软件包了.



### AHT10 温湿度传感器

#### 打开板载的AHT外设驱动

1. 打开板上外设AHT32开关
2. 因为Kconfig里面关联了，所以不用再次打开软件包
3. 查看确认一下
4. 搜索rt_vsprintf_full软件包（支持浮点输出）



示例:

```c
#include<rtthread.h>
#include<aht10.h>

struct AHT20
{
    float humidity;
    float temperature;
}aht20;

aht10_device_t aht20_init(const char *i2c_bus_name)
{
    aht10_device_t dev ;


    int count=0;

    rt_thread_mdelay(1000);

    dev = aht10_init(i2c_bus_name);
    if (dev == RT_NULL)
    {
        rt_kprintf("aht10 init failed\n");
        return;
    }

    return dev;
}
void AHT20_THREAD(void)
{

    aht10_device_t dev = aht20_init("i2c3");
    while(1)
    {
        aht20.humidity = aht10_read_humidity(dev);
        aht20.temperature = aht10_read_temperature(dev);

        // rt_vsnprintf(RT_NULL, 128, "humidity: %.2f%%\n temperature: %.2fC\n", humidity, temperature);

        rt_kprintf("humidity: %.2f%%\n temperature: %.2fC\n", aht20.humidity, aht20.temperature);
        char buf[50];
        rt_sprintf(buf,"{\"params\":{\"temperature\":%.0f}}",aht20.temperature);
        rt_kprintf(buf);
        rt_thread_mdelay(1000);
    }

} 

static int ath_app(void)
{
    rt_thread_t thread;
    thread = rt_thread_create("aht10", AHT20_THREAD, RT_NULL, 1024, 25, 10); 
    if (thread == RT_NULL)
    {
        return -RT_ERROR;
    }
    rt_thread_startup(thread);
    return RT_EOK;
}

MSH_CMD_EXPORT(ath_app, aht10 test);
```

 ### MQTT协议

**MQTT**（Message Queuing Telemetry Transport）是一种轻量级、基于发布-订阅模式的消息传输协议，适用于资源受限的设备和低带宽、高延迟或不稳定的网络环境。它在物联网应用中广受欢迎，能够实现传感器、执行器和其它设备之间的高效通信。



需要用到阿里云的物联网服务器,网路模块,温湿度传感器.

示例:

```c

#include <board.h>
#include "rtthread.h"
#include "dev_sign_api.h"
#include "mqtt_api.h"
#include "aht10.h"
#include <drv_gpio.h>
#include <dfs_posix.h>

char DEMO_PRODUCT_KEY[IOTX_PRODUCT_KEY_LEN + 1] = {0};
char DEMO_DEVICE_NAME[IOTX_DEVICE_NAME_LEN + 1] = {0};
char DEMO_DEVICE_SECRET[IOTX_DEVICE_SECRET_LEN + 1] = {0};

int count =1;
char String[100];

void *HAL_Malloc(uint32_t size);
void HAL_Free(void *ptr);
void HAL_Printf(const char *fmt, ...);
int HAL_GetProductKey(char product_key[IOTX_PRODUCT_KEY_LEN + 1]);
int HAL_GetDeviceName(char device_name[IOTX_DEVICE_NAME_LEN + 1]);
int HAL_GetDeviceSecret(char device_secret[IOTX_DEVICE_SECRET_LEN]);
uint64_t HAL_UptimeMs(void);
int HAL_Snprintf(char *str, const int len, const char *fmt, ...);

#define EXAMPLE_TRACE(fmt, ...)  \
    do { \
        HAL_Printf("%s|%03d :: ", __func__, __LINE__); \
        HAL_Printf(fmt, ##__VA_ARGS__); \
        HAL_Printf("%s", "\r\n"); \
    } while(0)


struct AHT20
{
    float humidity;
    float temperature;
}aht20;

aht10_device_t aht20_init(const char *i2c_bus_name)
{
    aht10_device_t dev ;


    int count=0;

    rt_thread_mdelay(1000);

    dev = aht10_init(i2c_bus_name);
    if (dev == RT_NULL)
    {
        rt_kprintf("aht10 init failed\n");
        return;
    }

    return dev;
}

static void example_message_arrive(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    iotx_mqtt_topic_info_t     *topic_info = (iotx_mqtt_topic_info_pt) msg->msg;

    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_PUBLISH_RECEIVED:
            /* print topic name and topic message */
            EXAMPLE_TRACE("Message Arrived:");
            EXAMPLE_TRACE("Topic  : %.*s", topic_info->topic_len, topic_info->ptopic);
            EXAMPLE_TRACE("Payload: %.*s", topic_info->payload_len, topic_info->payload);
            EXAMPLE_TRACE("\n");
            break;
        default:
            break;
    }
}

static int example_subscribe(void *handle)
{
    int res = 0;
    const char *fmt = "/%s/%s/user/get";
    char *topic = NULL;
    int topic_len = 0;

    topic_len = strlen(fmt) + strlen(DEMO_PRODUCT_KEY) + strlen(DEMO_DEVICE_NAME) + 1;
    topic = HAL_Malloc(topic_len);
    if (topic == NULL) {
        EXAMPLE_TRACE("memory not enough");
        return -1;
    }
    memset(topic, 0, topic_len);
    HAL_Snprintf(topic, topic_len, fmt, DEMO_PRODUCT_KEY, DEMO_DEVICE_NAME);

    res = IOT_MQTT_Subscribe(handle, topic, IOTX_MQTT_QOS0, example_message_arrive, NULL);
    if (res < 0) {
        EXAMPLE_TRACE("subscribe failed");
        HAL_Free(topic);
        return -1;
    }

    HAL_Free(topic);
    return 0;
}

static int example_publish_temperature(void *handle)
{

    
    aht10_device_t dev = aht20_init("i2c3");
    aht20.temperature = aht10_read_temperature(dev);
    char payload[50];
    rt_sprintf(payload,"{\"params\":{\"temperature\":%.1f}}",aht20.temperature);



    int             res = 0;
    const char     *fmt = "/sys/%s/%s/thing/event/property/post";
    char           *topic = NULL;
    int             topic_len = 0;


    topic_len = strlen(fmt) + strlen(DEMO_PRODUCT_KEY) + strlen(DEMO_DEVICE_NAME) + 1;
    topic = HAL_Malloc(topic_len);
    if (topic == NULL) {
        EXAMPLE_TRACE("memory not enough");
        return -1;
    }
    memset(topic, 0, topic_len);
    HAL_Snprintf(topic, topic_len, fmt, DEMO_PRODUCT_KEY, DEMO_DEVICE_NAME);

    res = IOT_MQTT_Publish_Simple(0, topic, IOTX_MQTT_QOS0, payload, strlen(payload));
    if (res < 0) {
        EXAMPLE_TRACE("publish failed, res = %d", res);
        HAL_Free(topic);
        return -1;
    }

    HAL_Free(topic);
    return 0;
}

static void example_event_handle(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    EXAMPLE_TRACE("msg->event_type : %d", msg->event_type);
}
static int example_publish_Humidity(void *handle)
{

    
    aht10_device_t dev = aht20_init("i2c3");
    aht20.humidity = aht10_read_humidity(dev);
    char payload[50];
    rt_sprintf(payload,"{\"params\":{\"Humidity\":%.1f}}",aht20.humidity);



    int             res = 0;
    const char     *fmt = "/sys/%s/%s/thing/event/property/post";
    char           *topic = NULL;
    int             topic_len = 0;


    topic_len = strlen(fmt) + strlen(DEMO_PRODUCT_KEY) + strlen(DEMO_DEVICE_NAME) + 1;
    topic = HAL_Malloc(topic_len);
    if (topic == NULL) {
        EXAMPLE_TRACE("memory not enough");
        return -1;
    }
    memset(topic, 0, topic_len);
    HAL_Snprintf(topic, topic_len, fmt, DEMO_PRODUCT_KEY, DEMO_DEVICE_NAME);

    res = IOT_MQTT_Publish_Simple(0, topic, IOTX_MQTT_QOS0, payload, strlen(payload));
    if (res < 0) {
        EXAMPLE_TRACE("publish failed, res = %d", res);
        HAL_Free(topic);
        return -1;
    }

    HAL_Free(topic);
    return 0;
}


static int mqtt_example_main1(void)
{
    void                   *pclient = NULL;
    int                     res = 0;
    int                     loop_cnt = 0;
    iotx_mqtt_param_t       mqtt_params;

    HAL_GetProductKey(DEMO_PRODUCT_KEY);
    HAL_GetDeviceName(DEMO_DEVICE_NAME);
    HAL_GetDeviceSecret(DEMO_DEVICE_SECRET);

    EXAMPLE_TRACE("mqtt example");

    memset(&mqtt_params, 0x0, sizeof(mqtt_params));

    mqtt_params.handle_event.h_fp = example_event_handle;

    pclient = IOT_MQTT_Construct(&mqtt_params);
    if (NULL == pclient) {
        EXAMPLE_TRACE("MQTT construct failed");
        return -1;
    }

    res = example_subscribe(pclient);
    if (res < 0) {
        IOT_MQTT_Destroy(&pclient);
        return -1;
    }
    int fd;
    while (1) {
        if (0 == loop_cnt % 20) {
            example_publish_temperature(pclient);
            example_publish_Humidity(pclient);
        }

        IOT_MQTT_Yield(pclient, 200);

        rt_sprintf(String, "Temp:%.2f Humi:%.2f,count=%d",aht20.temperature,aht20.humidity,count);
        fd = open("/fal/Data.txt", O_WRONLY | O_CREAT);

    //如果打开成功
        if (fd >= 0)
        {
            //写入文件
            write(fd, String, sizeof(String));

            rt_kprintf("Write done.\n");

            //关闭文件
            close(fd);
        }
        else
        {
            rt_kprintf("File Open Fail.\n");
        }

        loop_cnt += 1;
        count++;
        rt_thread_mdelay(1000);
    }

    return 0;
}
// #ifdef FINSH_USING_MSH
// MSH_CMD_EXPORT_ALIAS(mqtt_example_main1, ali_mqtt_sample, ali coap sample);
// #endif

static int ali_app(void)
{
    rt_thread_t thread;
    thread = rt_thread_create("ali",mqtt_example_main1 , RT_NULL, 4096, 11, 10); 
    if (thread == RT_NULL)
    {
        return -RT_ERROR;
    }
    rt_thread_startup(thread);
    return RT_EOK;
}
MSH_CMD_EXPORT(ali_app,app ali );
```



## 组件

### 文件系统:类似于LInux的文件系统

![image-20240726203025029](C:\Users\kinghge\AppData\Roaming\Typora\typora-user-images\image-20240726203025029.png)

**定义**：指的是一个可以独立开发、测试、部署和维护的软件单元

#### 分类:

| 类型  | 特点                                                         |
| ----- | ------------------------------------------------------------ |
| FatFs | FatFS 是专为小型嵌入式设备开发的一个兼容微软 FAT 格式的文件系统，采用ANSI C编写，具有良好的硬件无关性以及可移植性，是 RT-Thread 中最常用的文件系统类型。我们今天使用到的elm_fat就是这个类型。 |
| RomFS | 传统型的 RomFS 文件系统是一种简单的、紧凑的、只读的文件系统，不支持动态擦写保存，按顺序存放数据，因而支持应用程序以 XIP(execute In Place，片内运行) 方式运行，在系统运行时, 节省 RAM 空间。我们一般拿其作为挂载根目录的文件系统 |
| DevFS | DevFS                                                        |
| UFFS  | UFFS 是 Ultra-low-cost Flash File System（超低功耗的闪存文件系统）的简称。它是国人开发的、专为嵌入式设备等小内存环境中使用 Nand Flash 的开源文件系统。与嵌入式中常使用的 Yaffs 文件系统相比具有资源占用少、启动速度快、免费等优势。 |
| NFS   | NFS 网络文件系统（Network File System）是一项在不同机器、不同操作系统之间通过网络共享文件的技术。在操作系统的开发调试阶段，可以利用该技术在主机上建立基于 NFS 的根文件系统，挂载到嵌入式设备上，可以很方便地修改根文件系统的内容。 |





#### 目录管理

用readdir,mkdir,opendir,closedir来管理我们的目录.



#### 文件管理

open,close,read,write等API来管理我们的文件.

![目录管理常用函数](https://www.rt-thread.org/document/site/rt-thread-version/rt-thread-standard/programming-manual/filesystem/figures/fs-dir-mg.png)





示例:(温湿度传感器数据写入文件里)

```c
//记得在menuconfig中开启支持旧版本功能（Support legacy version）
#include <board.h>
#include <rtthread.h>
#include <drv_gpio.h>
#include <dfs_posix.h>//需要添加软件包进这里
#include <aht10.h>


struct AHT20
{
    float humidity;
    float temperature;
}aht20;
 int count  =0;
aht10_device_t aht20_init(const char *i2c_bus_name)
{
    aht10_device_t dev ;


    int count=0;

    rt_thread_mdelay(1000);

    dev = aht10_init(i2c_bus_name);
    if (dev == RT_NULL)
    {
        rt_kprintf("aht10 init failed\n");
        return;
    }

    return dev;
}

//定义要写入的内容
char String[100];

//定义接受文件内容的缓冲区
char buffer[100] = {};

void FileSystem_Test(void *parameter)
{
    aht10_device_t dev=aht20_init("i2c3");

    aht20.humidity = aht10_read_humidity(dev);
    aht20.temperature = aht10_read_temperature(dev);
    rt_sprintf(String, "Temp:%.2f Humi:%.2f,count=%d",aht20.temperature,aht20.humidity,count);
    //文件描述符
    int fd;

    //用只写方式打开文件,如果没有该文件,则创建一个文件
    fd = open("/fal/Data.txt", O_WRONLY | O_CREAT);

    //如果打开成功
    if (fd >= 0)
    {
        //写入文件
        write(fd, String, sizeof(String));

        rt_kprintf("Write done.\n");

        //关闭文件
        close(fd);
    }
    else
    {
        rt_kprintf("File Open Fail.\n");
    }

    //用只读方式打开文件
    fd = open("/fal/Data.txt", O_RDONLY);

    if (fd>= 0)
    {
        //读取文件内容
        rt_uint32_t size = read(fd, buffer, sizeof(buffer));
    
        if (size < 0)
        {
            rt_kprintf("Read File Fail.\n");
            return ;
        }

        //输出文件内容
        rt_kprintf("Read from file test.txt : %s \n", buffer);

        //关闭文件
        close(fd);
    }
    else
    {
        rt_kprintf("File Open Fail.\n");
    }
    count++;
}
//导出命令
MSH_CMD_EXPORT(FileSystem_Test, FileSystem_Test);

static void readdir_sample(void)
{
    DIR *dirp;
    struct dirent *d;

    /* 打开 / dir_test 目录 */
    dirp = opendir("/fal");
    if (dirp == RT_NULL)
    {
        rt_kprintf("open directory error!\n");
    }
    else
    {
        /* 读取目录 */
        while ((d = readdir(dirp)) != RT_NULL)
        {
            rt_kprintf("found %s\n", d->d_name);
        }

        /* 关闭目录 */
        closedir(dirp);
    }
}
/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(readdir_sample, readdir sample);
```

