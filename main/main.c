#include <stdio.h>
#include "epaper.h"

//Image header
#include "image.h"

void app_main(void)
{
    epaper_init();
    epaper_clear();
    epaper_draw_blackAndRedBitmaps(IMAGE_BLACK,IMAGE_RED);
    epaper_update();
    epaper_deep_sleep();

}
