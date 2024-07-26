#include<rtthread.h>
#include<rtdevice.h>

static int rt_dev_test_init(void)
{
    rt_device_t test_dev =rt_device_create(RT_Device_Class_Char,0);

    if(!test_dev)
    {
        rt_kprintf("create device failed!\n");
        return -RT_ERROR;
    }

    if(rt_device_register(test_dev,"test_dev",RT_DEVICE_FLAG_RDWR)!=RT_EOK)
    {
        rt_kprintf("register device failed!\n");
        return -RT_ERROR;
    }
    return RT_EOK;
}

INIT_COMPONENT_EXPORT(rt_dev_test_init);
