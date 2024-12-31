#include "epaper.h"


static const char *TAG = "SSD1680";
static spi_device_handle_t spi;


int bw_bufsize;         // size of the black and white buffer
int red_bufsize;        // size of the red buffer

uint8_t *bw_buf;        // the pointer to the black and white buffer if using on-chip ram
uint8_t *red_buf;       // the pointer to the red buffer if using on-chip ram

static void spi_cmd(const uint8_t cmd) {
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 8;
    t.tx_buffer = &cmd;
    t.user = (void*)0;
    gpio_set_level(PIN_DC, 0);
    ret = spi_device_polling_transmit(spi, &t);
    assert(ret == ESP_OK);
}

static void spi_data(const uint8_t data) {
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 8;
    t.tx_buffer = &data;
    t.user = (void*)1;
    gpio_set_level(PIN_DC, 1);
    ret = spi_device_polling_transmit(spi, &t);
    assert(ret == ESP_OK);
}

static void wait_busy(void) {
    while(gpio_get_level(PIN_BUSY) == 1) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void epaper_init(void){

    // GPIO yapılandırması
    gpio_set_direction(PIN_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_BUSY, GPIO_MODE_INPUT);
    gpio_set_pull_mode(PIN_BUSY, GPIO_PULLUP_ONLY);
    
    // SPI yapılandırması
    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_MOSI,
        .miso_io_num = PIN_MISO,
        .sclk_io_num = PIN_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = (DISPLAY_WIDTH * DISPLAY_HEIGHT / 8) + 8
    };
    
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 4*1000*1000,
        .mode = 0,
        .spics_io_num = PIN_CS,
        .queue_size = 7,
    };
    
    ESP_ERROR_CHECK(spi_bus_initialize(HSPI_HOST, &buscfg, 0));
    ESP_ERROR_CHECK(spi_bus_add_device(HSPI_HOST, &devcfg, &spi));

    bw_buf = (uint8_t *)malloc(DISPLAY_WIDTH * (DISPLAY_HEIGHT / 8 + 1) * 8 / 8);
    red_buf = (uint8_t *)malloc(DISPLAY_WIDTH * (DISPLAY_HEIGHT / 8 + 1) * 8 / 8);
    bw_bufsize = DISPLAY_WIDTH * (DISPLAY_HEIGHT / 8 + 1) * 8 / 8;
    red_bufsize = bw_bufsize;
    ESP_LOGI(TAG, "Buffer sizes - BW: %d, Red: %d", bw_bufsize, red_bufsize);  // Added logging
    memset(bw_buf, 0xFF, bw_bufsize);
    memset(red_buf, 0x00, red_bufsize);
    // Reset ekran
    gpio_set_level(PIN_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(PIN_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
    
    wait_busy();

    spi_cmd(0x12);  // Software reset
    vTaskDelay(pdMS_TO_TICKS(100));
    wait_busy();


    spi_cmd(0x01); //Driver output control
    spi_data(0xF9);
    spi_data(0x00);
    spi_data(0x00);
    
    
    spi_cmd(0x11); //data entry mode
    spi_data(0x03);

    spi_cmd(0x44); //set Ram-X address start/end position
    spi_data(0x00);
    spi_data(0x0F); //0x0F-->(15+1)*8=128

    spi_cmd(0x45);  //set Ram-Y address start/end position
    spi_data(0x00);
    spi_data(0x00);
    spi_data(0xF9); //0xF9-->(249+1)=250
    spi_data(0x00);


    spi_cmd(0x3C); //BorderWavefrom
    spi_data(0x05);

    spi_cmd(0x18); //Read built-in temperature sensor
    spi_data(0x80);

    spi_cmd(0x21); //  Display update control
    spi_data(0x00);
    spi_data(0x80);

    spi_cmd(0x4E); // set RAM x address count to 0;
    spi_data(0x00);
    spi_cmd(0x4F); // set RAM y address count to 0X199;
    spi_data(0xF9);
    spi_data(0x00);
    wait_busy();
    
}

void epaper_update(void){
    spi_cmd(0x22);
    spi_data(0xF7);
    spi_cmd(0x20);
    vTaskDelay(pdMS_TO_TICKS(100));
    wait_busy();
}
void epaper_clear(void){
    
    spi_cmd(0x4E); // set RAM x address count to 0;
    spi_data(0x00);
    spi_cmd(0x4F); // set RAM y address count to 0X199;
    spi_data(0xF9);
    spi_data(0x00);
    wait_busy();


    spi_cmd(0x24); //write RAM for black(0)/white (1)

    for (uint16_t i = 0; i < bw_bufsize; i++)
    {
        spi_data(bw_buf[i]);
    }

    spi_cmd(0x26); //write RAM for red(1)/white (0)

    for (uint16_t i = 0; i < red_bufsize; i++)
    {
        spi_data(red_buf[i]);
    }

    epaper_update();
    
}
void epaper_deep_sleep(void){
    spi_cmd(0x10);
    spi_data(0x01); // Deep sleep mode 1
}
void epaper_draw_blackBitmap(const unsigned char IMAGE[]){
    spi_cmd(0x4E); // set RAM x address count to 0;
    spi_data(0x00);
    spi_cmd(0x4F); // set RAM y address count to 0X199;
    spi_data(0xF9);
    spi_data(0x00);

    spi_cmd(0x24); //write RAM for black(0)/white (1)

    for (uint16_t i = 0; i < bw_bufsize; i++)
    {
        spi_data(~IMAGE[i]);
    }
}
void epaper_draw_redBitmap(const unsigned char IMAGE[]){
    spi_cmd(0x4E); // set RAM x address count to 0;
    spi_data(0x00);
    spi_cmd(0x4F); // set RAM y address count to 0X199;
    spi_data(0xF9);
    spi_data(0x00);

    spi_cmd(0x26); //write RAM for red(1)/white (0)

    for (uint16_t i = 0; i < red_bufsize; i++)
    {
        spi_data(IMAGE[i]);
    }
}
void epaper_draw_blackAndRedBitmaps(const unsigned char IMAGE_BLACK[], const unsigned char IMAGE_RED[]){
    
    spi_cmd(0x4E); // set RAM x address count to 0;
    spi_data(0x00);
    spi_cmd(0x4F); // set RAM y address count to 0X199;
    spi_data(0xF9);
    spi_data(0x00);
    wait_busy();


    spi_cmd(0x24); //write RAM for black(0)/white (1)

    for (uint16_t i = 0; i < bw_bufsize; i++)
    {
        spi_data(~IMAGE_BLACK[i]);
    }

    spi_cmd(0x26); //write RAM for red(1)/white (0)

    for (uint16_t i = 0; i < red_bufsize; i++)
    {
        spi_data(IMAGE_RED[i]);
    }
}