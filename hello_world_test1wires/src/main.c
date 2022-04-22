#include <zephyr.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <device.h>
#include <devicetree.h>
#include <sys/printk.h>
#include <drivers/gpio.h>
#include <drivers/sensor.h>
#include <kernel.h>
#include <logging/log.h>
#include <sys/__assert.h>

#include "ds18b20.h"



void main(void){

	printk("DS18b20 sensor test program started on %s-ns\n", CONFIG_BOARD);
	//k_msleep(5000);

	Init_GPIO();
  gpio_pin_configure(gpio_dev, 11, GPIO_OUTPUT);
	
	while(true){
		printf("read temp: %0.8f \r\n", ds18b20_read_temp());
    
		gpio_pin_set(gpio_dev, 11, 1);
		k_msleep(500);
		gpio_pin_set(gpio_dev, 11, 0);
    k_msleep(500);

		
	}
}
