#ifndef EPAPER_H
#define EPAPER_H

#include <stdio.h>
#include <string.h>
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"



// Pin Definitions
#define PIN_MOSI    23
#define PIN_MISO    -1
#define PIN_CLK     18
#define PIN_CS      5
#define PIN_DC      0
#define PIN_RST     2
#define PIN_BUSY    15

// Screen Definitions
#define DISPLAY_WIDTH   250
#define DISPLAY_HEIGHT  122




void epaper_init(void);
void epaper_update(void);
void epaper_clear(void);
void epaper_deep_sleep(void);
void epaper_draw_blackBitmap(const unsigned char IMAGE[]);
void epaper_draw_redBitmap(const unsigned char IMAGE[]);
void epaper_draw_blackAndRedBitmaps(const unsigned char IMAGE_BLACK[], const unsigned char IMAGE_RED[]);


#endif 