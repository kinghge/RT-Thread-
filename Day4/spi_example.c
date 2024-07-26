#include<rtthread.h>
#include<rtdevice.h>

#include<drv_spi.h>
#include<drv_gpio.h>

static int spi_attach(void)
{
    return rt_hw_spi_device_attach("spi2","spi120",GET_PIN(B,12));
}
MSH_CMD_EXPORT(spi_attach,spi attach);
//INIT_DEVICE_EXPORT(spi_attach);


static int spi_transfer_one_data(void)
{
    rt_err_t ret = RT_EOK;
    struct rt_spi_device *spi120 = (struct rt_spi_device *)rt_device_find("spi120");

    struct rt_spi_configuration cfg;
    cfg.data_width = 8;
    cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
    cfg.max_hz = 1000000;
    rt_spi_configure(spi120, &cfg);

    rt_uint8_t sendBuff = 0xDA;
    rt_uint8_t recvBuff = 0xF1;

    ret = rt_spi_transfer(spi120, &sendBuff, &recvBuff, 1);
    rt_kprintf("ret = %d\n",ret);

    return ret;
}
MSH_CMD_EXPORT(spi_transfer_one_data,spi treansfer one data);


static int spi_example(void)
{
    rt_err_t ret = RT_EOK;
    struct rt_spi_device *spi120 = (struct rt_spi_device *)rt_device_find("spi120");

    struct rt_spi_configuration cfg;
    cfg.data_width = 8;
    cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
    cfg.max_hz = 1000000;
    rt_spi_configure(spi120, &cfg);

    rt_uint8_t sendBuff = 0xDA;
    rt_uint8_t recvBuff = 0xF1;

    ret = rt_spi_transfer(spi120, &sendBuff, &recvBuff, 1);
    rt_kprintf("ret = %d\n",ret);
    rt_kprintf("sendBuff = %x,recvBuff = %x\n",sendBuff,recvBuff);

    return ret;
}

MSH_CMD_EXPORT(spi_example,a example for spi);

static int spi_send_one_data(void)
{
    rt_err_t ret = RT_EOK;
    struct rt_spi_device *spi120 = (struct rt_spi_device *)rt_device_find("spi120");

    struct rt_spi_configuration cfg;
    cfg.data_width = 8;
    cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
    cfg.max_hz = 1000000;
    rt_spi_configure(spi120, &cfg);

    rt_uint8_t sendBuff = 0x1A;
    ret = rt_spi_send(spi120, &sendBuff, 1);
    rt_kprintf("ret = %d\n",ret);
    rt_kprintf("sendBuff = %x\n",sendBuff);

    return ret;
}
MSH_CMD_EXPORT(spi_send_one_data,spi send one data);

static int spi_recv_one_data(void)
{
    rt_err_t ret = RT_EOK;
    struct rt_spi_device *spi120 = (struct rt_spi_device *)rt_device_find("spi120");

    struct rt_spi_configuration cfg;
    cfg.data_width = 8;
    cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
    cfg.max_hz = 1000000;
    rt_spi_configure(spi120, &cfg);

    rt_uint8_t recvBuff = 0x1A;
    ret = rt_spi_recv(spi120, &recvBuff, 1);
    rt_kprintf("ret = %d\n",ret);
    rt_kprintf("recvBuff = %x\n",recvBuff);

    return ret;
}
MSH_CMD_EXPORT(spi_recv_one_data,spi recvice one data);

static int spi_send_then_send_data(void)
{
    rt_err_t ret = RT_EOK;
    struct rt_spi_device *spi120 = (struct rt_spi_device *)rt_device_find("spi120");

    struct rt_spi_configuration cfg;
    cfg.data_width = 8;
    cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
    cfg.max_hz = 1000000;
    rt_spi_configure(spi120, &cfg);

    rt_uint8_t sendBuff1[2]={0x1A,0x2B};
    rt_uint8_t sendBuff2[2]={0x12,0x39};

    ret = rt_spi_send_then_send(spi120, &sendBuff1,2,&sendBuff2, 2);

    rt_kprintf("ret = %d\n",ret);
    rt_kprintf("sendBuff1 = %x,%x\n",sendBuff1[0],sendBuff1[1]);
    rt_kprintf("sendBuff2 = %x,%x\n",sendBuff2[0],sendBuff2[1]);

    return ret;
}
MSH_CMD_EXPORT(spi_send_then_send_data,spi send two data);

static int spi_send_then_recv_data(void)
{
    rt_err_t ret = RT_EOK;
    struct rt_spi_device * spi120 = (struct rt_spi_device *)rt_device_find("spi120");

    struct rt_spi_configuration cfg;
    cfg.data_width = 8;
    cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
    cfg.max_hz = 1000000;
    rt_spi_configure(spi120, &cfg);

    rt_uint8_t sendBuff[2]={0x1A,0x2B};
    rt_uint8_t recvBuff[2]={0x1A,0x2B};

    ret = rt_spi_send_then_recv(spi120, &sendBuff,2,&recvBuff, 2);
    rt_kprintf("ret = %d\n",ret);
    rt_kprintf("sendBuff = %x,%x\n",sendBuff[0],sendBuff[1]);
    rt_kprintf("recvBuff = %x,%x\n",recvBuff[0],recvBuff[1]);

    return ret;

}

MSH_CMD_EXPORT(spi_send_then_recv_data,spi send  data and recvice data );
