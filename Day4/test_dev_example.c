#include<rtthread.h>
#include<rtdevice.h>

#define LOG_TAG "drv.test"
#define LOG_LVL LOG_LVL_DBG
#include<ulog.h>


static int drv_test_app(void)
{
    rt_device_t test_dev = rt_device_find("test_drv");

    if(test_dev == RT_NULL)
    {
        LOG_E("can't find test drv .");
        return -RT_ERROR;
    }

    rt_device_open(test_dev,RT_DEVICE_OFLAG_RDWR);
    rt_device_control(test_dev,RT_DEVICE_CTRL_CONFIG,RT_NULL);
    rt_device_write(test_dev,100,RT_NULL,1024);
    rt_device_read(test_dev,20,RT_NULL,128);
    
    rt_device_close(test_dev);

    return RT_EOK;
}

MSH_CMD_EXPORT(drv_test_app,drv test app);