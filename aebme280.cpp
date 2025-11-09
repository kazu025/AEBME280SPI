/*
AE-BME280   温度・湿度・気圧
*/
#include <cstdio>
#include <string>
#include "hardware/spi.h"
#include "boards/pico.h"
#include "pico/config.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include "aebme280.h"

/* ---------------------------------------------------------------
    GPIO 16 (pin 21) MISO/spi0_rx-> SDO/SDO on bme280 board
    GPIO 17 (pin 22) Chip select -> CSB/!CS on bme280 board
    GPIO 18 (pin 24) SCK/spi0_sclk -> SCL/SCK on bme280 board
    GPIO 19 (pin 25) MOSI/spi0_tx -> SDA/SDI on bme280 board
    3.3v (pin 36) -> VCC on bme280 board
    GND (pin 38)  -> GND on bme280 board
    -------------------------------------------------------------- */
AEBME280::AEBME280(){
    uint8_t id;
    // SPI初期化（SPIインスタンス、ボーレート設定）
    spi_init(spi_default, SPI_BAUDRATE * 1000);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);

    gpio_init(PICO_DEFAULT_SPI_CSN_PIN);
    gpio_set_dir(PICO_DEFAULT_SPI_CSN_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 1); // CSをHigh（無効）に 
    read_registers(0xD0, &id, 1);       // chip id 読み出し
    printf("Chip ID = 0x%0x\n, id");    // 確認のため
    read_parameters();
    write_register(0xF2, 0x01);
    write_register(0xF4, 0x27);
}
void AEBME280::read(int32_t &temp, int32_t &humid, int32_t &press){
    uint8_t buff[8];
    int32_t t, h, p, x, y, z;
    read_registers(0xF7, buff, 8);
    t = ((uint32_t) buff[3] << 12) | ((uint32_t) buff[4] << 4) | (buff[5] >> 4);
    h = (uint32_t) buff[6] << 8 | buff[7];
    p = ((uint32_t) buff[0] << 12) | ((uint32_t) buff[1] << 4) | (buff[2] >> 4);
    /* 温度変換 */ 
    x  = ((((t >> 3) - ((int32_t) dig_T1 << 1))) * ((int32_t) dig_T2)) >> 11;
    x += (((((t >> 4) - ((int32_t) dig_T1)) * ((t >> 4) - ((int32_t) dig_T1))) >> 12) * ((int32_t) dig_T3)) >> 14;
    temp = (x * 5 + 128) >> 8;
    
    /* 気圧変換 */;
    y = (((int32_t) x) >> 1) - (int32_t) 64000;
    z = (((y >> 2) * (y >> 2)) >> 11) * ((int32_t) dig_P6);
    z = z + ((y * ((int32_t) dig_P5)) << 1);
    z = (z >> 2) + (((int32_t) dig_P4) << 16);
    y = (((dig_P3 * (((y >> 2) * (y >> 2)) >> 13)) >> 3) + ((((int32_t) dig_P2) * y) >> 1)) >> 18;
    y = ((((32768 + y)) * ((int32_t) dig_P1)) >> 15);
    if (y == 0) press = 0;
    else{
        press = (((uint32_t) (((int32_t) 1048576) - p) - (z >> 12))) * 3125;
        if (press < 0x80000000)
            press = (press << 1) / ((uint32_t) y);
        else
            press = (press / (uint32_t) y) * 2;
        y = (((int32_t) dig_P9) * ((int32_t) (((press >> 3) * (press >> 3)) >> 13))) >> 12;
        z = (((int32_t) (press >> 2)) * ((int32_t) dig_P8)) >> 13;
        press = (uint32_t) ((int32_t) press + ((y + z + dig_P7) >> 4));
    }
    /* 湿度変換 */
    x -= (int32_t)76800;
    y = (((((h<< 14) - (((int32_t) dig_H4) << 20) - (((int32_t) dig_H5) * x)) +
            ((int32_t) 16384)) >> 15) * (((((((x * ((int32_t) dig_H6)) >> 10) * (((x * ((int32_t) dig_H3)) >> 11) +
            ((int32_t) 32768))) >> 10) + ((int32_t) 2097152)) * ((int32_t) dig_H2) + 8192) >> 14));
            y =  (y - (((((y >> 15) * (y >> 15)) >> 7) * ((int32_t) dig_H1)) >> 4));
            y = y < 0 ? 0 : y;
            y = y > 419430400 ? 419430400 : y;
    humid = y >> 12;
    //printf("read :: t=%d(%d) h=%d(%d) p=%d(%d)\n", t, temp, h, humid, p, press);
}
void AEBME280::read_parameters(void){
    uint8_t buffer[26];
    read_registers(0x88, buffer, 26);
    dig_T1 = buffer[0] | (buffer[1] << 8);
    dig_T2 = buffer[2] | (buffer[3] << 8);
    dig_T3 = buffer[4] | (buffer[5] << 8);
    dig_P1 = buffer[6] | (buffer[7] << 8);
    dig_P2 = buffer[8] | (buffer[9] << 8);
    dig_P3 = buffer[10] | (buffer[11] << 8);
    dig_P4 = buffer[12] | (buffer[13] << 8);
    dig_P5 = buffer[14] | (buffer[15] << 8);
    dig_P6 = buffer[16] | (buffer[17] << 8);
    dig_P7 = buffer[18] | (buffer[19] << 8);
    dig_P8 = buffer[20] | (buffer[21] << 8);
    dig_P9 = buffer[22] | (buffer[23] << 8);
    dig_H1 = buffer[25]; // 0xA1
    read_registers(0xE1, buffer, 8);
    dig_H2 = buffer[0] | (buffer[1] << 8); // 0xE1 | 0xE2
    dig_H3 = (int8_t) buffer[2]; // 0xE3
    dig_H4 = buffer[3] << 4 | (buffer[4] & 0xf); // 0xE4 | 0xE5[3:0]
    dig_H5 = (buffer[4] >> 4) | (buffer[5] << 4); // 0xE5[7:4] | 0xE6
    dig_H6 = (int8_t) buffer[6]; // 0xE7
}
void AEBME280::read_registers(uint8_t reg, uint8_t buff[], uint16_t len){
    reg |= 0x80;
    cs_select();
    spi_write_blocking(spi_default, &reg, 1);
    sleep_ms(10);
    spi_read_blocking(spi_default, 0, buff, len);
    cs_deselect();
    sleep_ms(10);
}
void AEBME280::write_register(uint8_t reg, uint8_t data) {
    uint8_t buf[2];
    buf[0] = reg & 0x7f;
    buf[1] = data;
    cs_select();
    spi_write_blocking(spi_default, buf, 2);
    cs_deselect();
    sleep_ms(10);
}
inline void AEBME280::cs_select(){
    asm volatile("nop \n nop \n nop");
    gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 0);
    asm volatile("nop \n nop \n nop");
}
inline void AEBME280::cs_deselect(){
    asm volatile("nop \n nop \n nop");
    gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 1);
    asm volatile("nop \n nop \n nop");
}
