//Modified: Abhishek Malik <abhishek.malik@intel.com>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "led.h"

#include "upm_utilities.h"

int running = 1;

void sig_handler(int signo)
{
    running = 0;
}

int main(void)
{
    signal(SIGINT, sig_handler);

    led_context dev = led_init(100);

    while(running){
        if(led_on(dev) != UPM_SUCCESS){
            printf("problem turning the LED on\n");
        }
        upm_delay(5);
        if(led_off(dev) != UPM_SUCCESS){
            printf("problem turning the LED off\n");
        }
        upm_delay(5);
    }

    led_close(dev);
    return 0;
}
