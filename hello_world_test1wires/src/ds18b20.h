#define DQ_PIN 10
static const struct device *gpio_dev;


void Init_GPIO(){

    gpio_dev = device_get_binding("GPIO_0");
    if (!gpio_dev) {
	    printk("Error getting GPIO_0: failed.\n");
    }

}


void ds18b20_read(void * data_ptr, unsigned int num_bytes){
  char  ch; //Current reading byte buffer
  char * data_buf = data_ptr;
  
  int i=0,u=0;
  for (i=0;i<num_bytes;i++){
    ch=0;
    for(u=0;u<8;u++){
      //Form read slot
      gpio_pin_configure(gpio_dev, DQ_PIN, GPIO_OUTPUT);
      gpio_pin_set(gpio_dev, DQ_PIN, 0);
      k_busy_wait(5);
      gpio_pin_configure(gpio_dev, DQ_PIN, GPIO_INPUT);
      k_busy_wait(10);
      if(gpio_pin_get_raw(gpio_dev, DQ_PIN)!=0){
        ch |= 1 << u; //There is 1 on the bus
      }
      else{
        ch &= ~(1 << u); //There us 0 on the bus
      }

      //Apply "long" timer for make sure that timeslot is end
     k_busy_wait(60);
    }
    data_buf[i]=ch;
  }
 	gpio_pin_configure(gpio_dev, DQ_PIN, GPIO_INPUT);
}

/* Function to write byte*/
void ds18b20_write(void * data_ptr, unsigned int num_bytes){
  char  ch; //Current reading byte buffer
  char * data_buf = data_ptr;
  
  int i=0,u=0;
  for (i=0;i<num_bytes;i++){
    ch=data_buf[i];
    for(u=0;u<8;u++){
      //Form write slot
      gpio_pin_configure(gpio_dev, DQ_PIN, GPIO_OUTPUT);
	    gpio_pin_set(gpio_dev, DQ_PIN, 0);
      k_busy_wait(1);;
      //write 1 - pull bus to HIGH just after short timer
      if(ch&(1<<u)){
		    gpio_pin_configure(gpio_dev, DQ_PIN, GPIO_INPUT);
      }
      //Apply "long" timer for make sure that timeslot is end
      k_busy_wait(60);
      //Release bus, if this wasn't done before
	    gpio_pin_configure(gpio_dev, DQ_PIN, GPIO_INPUT);
      data_buf[i]=ch;
    }
  }
  gpio_pin_configure(gpio_dev, DQ_PIN, GPIO_INPUT);
}


//Perform reset of the bus, and then wait for the presence pulse
bool ds18b20_reset_and_check(void){
  int res=0;
  //Form reset pulse
  
  gpio_pin_configure(gpio_dev, DQ_PIN, GPIO_OUTPUT);
  gpio_pin_set(gpio_dev, DQ_PIN, 0);
  k_busy_wait(500);
  //Release bus and wait 15-60MS
  gpio_pin_configure(gpio_dev, DQ_PIN, GPIO_INPUT);
  k_busy_wait(60);

  //Read from bus
  res=gpio_pin_get_raw(gpio_dev, DQ_PIN);
  if(res==0){
    k_busy_wait(500);
    return true;
  }
  return false;
}


float ds18b20_read_temp(void){
   char buf[16];
    double temp;
        //int i;
     int16_t raw_temp=0;

    ds18b20_reset_and_check();

//Read ROM
    buf[0]=0x33;
    ds18b20_write(&buf,1);

//Read the results
    
    ds18b20_read(&buf,8);
    memset(&buf,0,16);

    //Send convert TX cmd
    buf[0]=0x44; //Convert temp
    ds18b20_write(&buf,1);
    k_busy_wait(1000000);//Wait for finishing of the conversion

    ds18b20_reset_and_check();
    buf[0]=0x33; //Read ROM
    ds18b20_write(&buf,1);
    //Read the results
    ds18b20_read(&buf,8);
    memset(&buf,0,16);
    buf[0]=0xBE;  //Read scratchpad
    ds18b20_write(&buf,1);
    //Read the results
    ds18b20_read(&buf,12);
    raw_temp = (buf[1] << 8) | buf[0];

    //memcpy(&f,&buf,8);
    temp = (float)raw_temp / 16.0;
    return(temp);
}
