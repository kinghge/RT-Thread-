#include<rtthread.h>
#include<rtdevice.h>

static int re_device_test_init(void)
{
    rt_device_t test_dev = rt_device_create(RT_Device_Class_Char,0);
    if(!test_dev)
    {
        rt_kprintf("test_dev create failed\n");
        return -RT_ERROR;
    }

    if(rt_device_register(test_dev,"test_dev",RT_DEVICE_FLAG_RDWR)!= RT_EOK)
    {
        rt_kprintf("test_dev register failed\n");
        return -RT_ERROR;
    }
    rt_kprintf("test_dev register success\n");
    return RT_EOK;
}

MSH_CMD_EXPORT(re_device_test_init, test);
