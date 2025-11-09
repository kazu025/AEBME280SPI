/*
    AE-BME820 header
*/

#define PICO_DEFAULT_SPI 0      // spi_default = spi0
//#define PICO_DEFAULT_SPI 1    // spi_default = spi1
#define SPI_BAUDRATE 500        // SPI ボーレート[kHz]　
class AEBME280{
    private:
        int32_t t_fine;
        uint16_t dig_T1, dig_P1;
        int16_t dig_T2, dig_T3;
        int16_t dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
        int16_t dig_H2, dig_H4, dig_H5;
        uint8_t dig_H1, dig_H3;
        int8_t dig_H6;
        void read_parameters(void);
        void read_registers(uint8_t reg, uint8_t buff[], uint16_t len);
        void write_register(uint8_t reg, uint8_t data);
        inline void cs_select();
        inline void cs_deselect();
    public:
        AEBME280();
        void read(int32_t &temp, int32_t &humid, int32_t &press);
};