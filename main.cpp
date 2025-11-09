/*
    AE-BME280 温湿度・気圧計
    SPIプログラム
*/
#include <cstdio>
#include <string>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/config.h"
#include "pico/cyw43_arch.h"
#include "pico/time.h"
#include "hardware/spi.h"
#include "boards/pico.h"
#include "aebme280.h"
int main(void){
    bool f = true;
    stdio_init_all();
    if(cyw43_arch_init()){
        printf("WiFi init failed");
        return -1;
    }
    int32_t temperature, humidity, pressure;
    AEBME280 bme280 = AEBME280(); 
    while(true){
        bme280.read(temperature, humidity, pressure);
        printf(" Temperature = %.2f[C] Humdidity = %.2f[%%] Pressure = %d[Pa](%f[Atmos press])\n",
            temperature/100.0, humidity/1024.0, pressure, pressure/101325.0);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, f);
        f = !f;
        sleep_ms(1000);
    }

}