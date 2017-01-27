#include <pebble.h>
#include "math-sll.h"

#define ANIM_SPEED 5
static Window *window;
static TextLayer *TimeLayer;
static TextLayer *AMPMLayer;
static TextLayer *DayLayer;
static Layer *ChaosLayer;
static char TimeText[] = "00:00";
static char AMPM[] = "XX";
static char DayText[] = "Xxx";
AppTimer *timer_handle;

#ifdef PBL_BW
uint8_t fb_data[18][138], pixel_x, pixel_y, pixel_byte, pixel_bit;
uint8_t powers[8] = {1,2,4,8,16,32,64,128};
#else
#ifdef PBL_ROUND
uint8_t fb_data[180][144], pixel_x, pixel_y;
#else
#ifdef PBL_PLATFORM_EMERY
uint8_t fb_data[200][168], pixel_x, pixel_y;
#else
uint8_t fb_data[144][138], pixel_x, pixel_y;
#endif
#endif
#endif
uint8_t x, y, x1, y1, roll, move, filled_dot, new_hour, current_minute, minute_count, battery_saver, finish_up, completed, update_display = 0, pattern = 0, x_offset = 0, y_offset = 0, jzoom = 1;
static sll fp1, fp2, fp3, fp4;
int move_x, move_y, const_real, const_img;

#ifdef PBL_COLOR
int red, green, blue, red_di, green_di, blue_di;
#endif

int Draw_Pixel() {
    
#ifdef PBL_COLOR
    if (fb_data[x1][y1] == 0) {
        fb_data[x1][y1] = roll;
        return 0;
    } else {
        return 1;
    }
#else
    pixel_byte = x1 / 8;
    pixel_bit = x1 % 8;
    if ((fb_data[pixel_byte][y1] & powers[pixel_bit]) == 0) {
        fb_data[pixel_byte][y1] = fb_data[pixel_byte][y1] | powers[pixel_bit];
        return 0;
    } else {
        return 1;
    }
#endif
}

int isSierpinskiCarpetPixelFilled(int x_coord, int y_coord)
{
    while((x_coord > 0) || (y_coord > 0)) // when either of these reaches zero the pixel is determined to be on the edge
        // at that square level and must be filled
    {
        if((x_coord % 3 == 1) && (y_coord % 3 == 1)) //checks if the pixel is in the center for the current square level
            return 0;
        x_coord = x_coord / 3; //x and y are decremented to check the next larger square level
        y_coord = y_coord / 3;
    }
    return 1; // if all possible square levels are checked and the pixel is not determined
    // to be open it must be filled
}

void julia_set() {

  fp1 = slladd(slldiv(sllmul(int2sll(1.5), int2sll(x - (144 + (x_offset * 2)) / 2)), sllmul(int2sll(0.5), int2sll(jzoom * (144 + (x_offset * 2))))), slldiv(int2sll(move_x), int2sll(10000)));
   
  fp4 = slladd(slldiv(int2sll(y - (138 + y_offset) / 2), sllmul(int2sll(0.5), int2sll(jzoom * (138 + y_offset)))), slldiv(int2sll(move_y), int2sll(10000)));
  
  for (roll = 0; roll < 255; roll++) {
    fp2 = fp1;
    fp3 = fp4;
    fp1 = slladd(sllsub(sllmul(fp2, fp2), sllmul(fp3, fp3)), slldiv(int2sll(const_real), int2sll(10000)));
    fp4 = slladd(sllmul(sllmul(int2sll(2), fp2), fp3), slldiv(int2sll(const_img), int2sll(10000)));
    if (slladd(sllmul(fp1, fp1), sllmul(fp4, fp4)) > int2sll(4)) break;
  }
}

void mandelbrot() {
  
  fp1 = int2sll(0);
  fp2 = int2sll(0);
  fp3 = int2sll(0);
  fp4 = int2sll(0);
  
  for (roll = 0; roll < 255; roll++) {
    fp2 = fp1;
    fp3 = fp4;
	
    fp1 = slladd(slladd(sllsub(sllmul(fp2, fp2), sllmul(fp3, fp3)), slldiv(sllmul(int2sll(1.5), int2sll(x - (144 + (x_offset * 2)) / 2)), sllmul(int2sll(0.5), int2sll(jzoom * (144 + (x_offset * 2)))))), slldiv(int2sll(move_x), int2sll(10000)));
    fp4 = slladd(slladd(sllmul(int2sll(2), sllmul(fp2, fp3)), slldiv(int2sll(y - (138 + y_offset) / 2), sllmul(int2sll(0.5), int2sll(jzoom * (138 + y_offset))))), slldiv(int2sll(move_y), int2sll(10000)));
  
	
	if (slladd(sllmul(fp1, fp1), sllmul(fp4, fp4)) > int2sll(4)) break;
  }
}

void newton () {
  
  static sll tmp;
  
  fp1 = slladd(slldiv(int2sll(x - (144 + (x_offset * 2))), sllmul(int2sll(0.5), int2sll(jzoom * (144 + (x_offset * 2))))), slldiv(int2sll(move_x), int2sll(10000)));
  fp2 = slladd(slldiv(int2sll(y - (138 + y_offset)), sllmul(int2sll(0.5), int2sll(jzoom * (138 + y_offset)))), slldiv(int2sll(move_y), int2sll(10000)));
  
  for (roll = 0; roll < 255; roll++) {
    fp3 = fp1;
    fp4 = fp2;

    tmp = sllmul(slladd(sllmul(fp1, fp1), sllmul(fp2, fp2)), slladd(sllmul(fp1, fp1), sllmul(fp2, fp2)));
    fp1 = slldiv(sllsub(slladd(sllmul(sllmul(int2sll(2), fp1), tmp), sllmul(fp1, fp1)), sllmul(fp2, fp2)), sllmul(int2sll(3), tmp));
    fp2 = slldiv(sllmul(sllmul(int2sll(2), fp2), sllsub(tmp, fp3)),  sllmul(int2sll(3), tmp));
    tmp = slladd(sllmul(sllsub(fp1, fp3), sllsub(fp1, fp3)), sllmul(sllsub(fp2, fp4), sllsub(fp2, fp4)));
    if (tmp <= int2sll(0.000001)) break;
  }
}

void pick_pattern() {
  
    pattern = rand() % 9;
    completed = 0;
	  finish_up = 0;
	
    if (pattern == 0) {
        x = 50;
        y = 25;
        battery_saver = 5;
    }
    if (pattern == 1) {
        x = 0;
        y = 0;
        battery_saver = 5;
    }
	  if (pattern == 2) {
		  fp1 = int2sll(0);
		  fp2 = int2sll(0);
		  fp3 = int2sll(0);
		  fp4 = int2sll(0);
		  battery_saver = 2;
	  }
    if ((pattern >= 3) && (pattern < 5)) {
        pattern = 3;
        roll = 0;
        while ((roll < 100) || (roll == 255)) {
          
          x = (144 + (x_offset * 2)) / 2;
          y = (138 + (y_offset)) / 2;
          move_x = (rand() % 30000) - 15000;
          move_y = (rand() % 30000) - 15000;
          const_real = (rand() % 16000) - 8000;
          const_img = (rand() % 16000) - 8000;
          jzoom = (rand() % 800) + 1;
          julia_set();
		  APP_LOG(APP_LOG_LEVEL_DEBUG, "Julia center point value = %i     Real = %i    Imaginary = %i", roll, const_real, const_img);
        }
		battery_saver = 10;
    }
    if ((pattern >= 5) && (pattern < 7)) {
        x = 0;
        y = 0;
        pattern = 4;
        roll = 0;
        while ((roll < 100) || (roll == 255)) {

          move_x = (rand() % 30000) - 15000;
          move_y = (rand() % 30000) - 15000;
          jzoom = (rand() % 8000) + 1;
          x = (144 + (x_offset * 2)) / 2;
          y = (138 + (y_offset)) / 2;
          mandelbrot();
          APP_LOG(APP_LOG_LEVEL_DEBUG, "Mandelbrot center point value = %i", roll);
        }
		battery_saver = 10;
    }
    if (pattern >= 7) {
        pattern = 5;
        roll = 0;
        while (roll < 20) {

          move_x = (rand() % 30000) - 15000;
          move_y = (rand() % 30000) - 15000;
#ifdef PBL_BW
          jzoom = (rand() % 7) + 1;
#else
          jzoom = (rand() % 100) + 1;
#endif
          x = (144 + (x_offset * 2)) / 2;
          y = (138 + (y_offset)) / 2;
          newton();
          APP_LOG(APP_LOG_LEVEL_DEBUG, "Newton center point value = %i", roll);
        }
		battery_saver = 10;
    }
}
#ifdef PBL_COLOR
void color_cycle(uint8_t color_shift) {
  
        if ((rand() % color_shift) == 1) {
            
            if ((red == 0) || (red == 255)) red_di = -red_di;
            red = red + red_di;
        }
        
        if ((rand() % color_shift) == 1) {
            
            if ((green == 0) || (green == 255)) green_di = -green_di;
            green = green + green_di;
        }
        
        if ((rand() % color_shift) == 1) {
            
            if ((blue == 0) || (blue == 255)) blue_di = -blue_di;
            blue = blue + blue_di;
        }
}
#endif
void chaoslayer_update_callback(Layer *layer, GContext* ctx) {
    
#ifdef PBL_BW
    graphics_context_set_stroke_color(ctx, GColorWhite);
#endif
    
    for (pixel_x=0; pixel_x <144 + (x_offset * 2); pixel_x++) {
        for (pixel_y=0; pixel_y <138 + y_offset; pixel_y++) {
#ifdef PBL_BW
            pixel_byte = pixel_x / 8;
            pixel_bit = pixel_x % 8;
            
            if ((fb_data[pixel_byte][pixel_y] & powers[pixel_bit]) > 0) {
#else
                if (fb_data[pixel_x][pixel_y] > 0) {
                    
                    
                    graphics_context_set_stroke_color(ctx, GColorFromRGB(red - (255 - fb_data[pixel_x][pixel_y]), green - (255 - fb_data[pixel_x][pixel_y]), blue - (255 - fb_data[pixel_x][pixel_y])));
#endif
                    graphics_draw_pixel(ctx, GPoint(pixel_x, pixel_y));
                }
            }
        }
        
    }
    
    void handle_timer() {
        
#ifdef PBL_COLOR
        color_cycle(50);
#endif
        
        if (pattern == 0) { //Sierpinski Triangle
            
            roll = (rand() % 3) + 1;
#ifdef PBL_PLATFORM_EMERY
            if (roll == 1) {
                x1 = x/2;
                y1 = y/2;
            }
            if (roll == 2) {
                x1 = (200 - x)/2 + x;
                y1 = y/2;
            }
            if (roll == 3) {
                if (x < 100) {
                    x1 = (99 - x)/2 + x;
                    y1 = (167 - y)/2 + y;
                } else {
                    x1 = (x - 100)/2 + 100;
                    y1 = (167 - y)/2 + y;
                }
            }
#else
            
            if (roll == 1) {
                x1 = x/2;
                y1 = y/2;
            }
            if (roll == 2) {
                x1 = (144 - x)/2 + x;
                y1 = y/2;
            }
            if (roll == 3) {
                if (x < 72) {
                    x1 = (71 - x)/2 + x;
                    y1 = (137 - y)/2 + y;
                } else {
                    x1 = (x - 72)/2 + 72;
                    y1 = (137 - y)/2 + y;
                }
            }
#endif
            x = x1;
            y = y1;
#ifdef PBL_ROUND
            x1 = x1 * .83 + 32;
            y1 = y1 * .83 + 24;
#endif
            roll = roll * 63;
            filled_dot = Draw_Pixel();
			if (current_minute > minute_count) completed = 1;
            
        }
        if (pattern == 1) { //Sierpinski Carpet

          if (finish_up == 0) {
            x = rand() % 243;
            y = rand() % 243;
          }
            
            
            if (isSierpinskiCarpetPixelFilled(x,y)) {
                
                x1 = (x / 2) + 11 + (x_offset);

#ifndef PBL_ROUND
          y1 = (y / 2) + 8 + (y_offset);
          if (y1 <= ((138 + (y_offset * 2)) / 2)) {
            
            if (x1 <= ((144 + (x_offset * 2)) / 2) ) {
              roll = (x1 + y1) / 2;
            } else {
              roll = (((144 + (x_offset * 2)) / 2) - (x1 - ((144 + (x_offset * 2)) / 2)) + y1) / 2;
            }
          } else {
              
            if (x1 <= ((144 + (x_offset * 2)) / 2) ) {
              roll = (((138 + (y_offset * 2)) / 2) - (y1 - ((138 + (y_offset * 2)) / 2)) + x1) / 2;
            } else {
              roll = (((138 + (y_offset * 2)) / 2) - (y1 - ((138 + (y_offset * 2)) / 2)) + ((144 + (x_offset * 2)) / 2) - (x1 - ((144 + (x_offset * 2)) / 2))) / 2;
            }
          }
#else
          y1 = (y / 2) + 8 + (y_offset * 2);
          if (y1 <= ((138 + (y_offset * 4)) / 2)) {
            
            if (x1 <= ((144 + (x_offset * 2)) / 2) ) {
              roll = (x1 + y1) / 2;
            } else {
              roll = (((144 + (x_offset * 2)) / 2) - (x1 - ((144 + (x_offset * 2)) / 2)) + y1) / 2;
            }
          } else {
              
            if (x1 <= ((144 + (x_offset * 2)) / 2) ) {
              roll = (((138 + (y_offset * 4)) / 2) - (y1 - ((138 + (y_offset * 4)) / 2)) + x1) / 2;
            } else {
              roll = (((138 + (y_offset * 4)) / 2) - (y1 - ((138 + (y_offset * 4)) / 2)) + ((144 + (x_offset * 2)) / 2) - (x1 - ((144 + (x_offset * 2)) / 2))) / 2;
            }
          }                            
#endif
              filled_dot = Draw_Pixel();
            }
          if (finish_up) {
			  
			  x++;
              if (x == 244) {
                x = 0;
                y++;
                if (y == 244) {
                  y = 0;
				          completed = 1;
                }
              }
		  }
	  
	  
	  
	  
	  }
        if (pattern == 2) { //Henon Attractor
            
            fp1 = fp2;
            fp4 = sllmul(int2sll(1.4), sllmul(fp2, fp2));
            fp2 = sllsub(slladd(fp3, int2sll(1)),fp4);
            fp3 = sllmul(int2sll(0.3), fp1);
#ifdef PBL_PLATFORM_EMERY
            x1 = sll2int(sllmul(slladd(fp2,int2sll(1)), int2sll(60))) + 40;
            y1 = sll2int(sllmul(slladd(fp3,int2sll(1)), int2sll(180))) - 90;
#else
            x1 = sll2int(sllmul(slladd(fp2,int2sll(1)), int2sll(50))) + 15 + (x_offset * 2);
            y1 = sll2int(sllmul(slladd(fp3,int2sll(1)), int2sll(150))) - 75 + (y_offset);
#endif

            roll = (x1 + y1) / 2;
            filled_dot = Draw_Pixel();
			if (current_minute > minute_count) completed = 1;
          
        }
        if (pattern == 3 ) { //Julia Set
          
          if (finish_up == 0) {
            
            x = rand() % (144 + (x_offset * 2));
            y = rand() % (138 + (y_offset));
          }
          
            julia_set();
            
            x1 = x;
            y1 = y;
          
          if (finish_up) {
            
            x++;
            if (x == (144 + (x_offset * 2))) {
              x = 0;
              y++;
              if (y == (138 + (y_offset))) {
                y = 0;
				        completed = 1;
              }
            }
          }         
#ifdef PBL_BW
            if ((roll > 100) && (roll < 255)) filled_dot = Draw_Pixel();
#else
            if (roll > 50) filled_dot = Draw_Pixel();
#endif
        }
        if (pattern == 4 ) { //Mandelbrot Set
          
          if (finish_up == 0) {
            
            x = rand() % (144 + (x_offset * 2));
            y = rand() % (138 + (y_offset));
          }
          
          mandelbrot();
            
            x1 = x;
            y1 = y;
          
          if (finish_up) {
            
            x++;
            if (x == (144 + (x_offset * 2))) {
              x = 0;
              y++;
              if (y == (138 + (y_offset))) {
                y = 0;
				        completed = 1;
              }
            }
          }         
#ifdef PBL_BW
            if ((roll > 100) && (roll < 255)) filled_dot = Draw_Pixel();
#else
            if (roll > 50) filled_dot = Draw_Pixel();
#endif
        }
          if (pattern ==5) { //Newton's Method
            
            if (finish_up == 0) {
              
              x = rand() % (144 + (x_offset * 2));
              y = rand() % (138 + (y_offset));
            }
            
            newton();
        
            x1 = x;
            y1 = y;
            roll = roll * 11;
            
            if (finish_up) {
              
              x++;
              if (x == (144 + (x_offset * 2))) {
                x = 0;
                y++;
                if (y == (138 + (y_offset))) {
                  y = 0;
				          completed = 1;
                }
              }
            }            
#ifdef PBL_BW
            if (roll > 110) filled_dot = Draw_Pixel();
#else
            filled_dot = Draw_Pixel();
#endif
      }
       
        if (filled_dot == 0) update_display++;
        if (update_display == 255) {
            layer_mark_dirty(ChaosLayer);
            update_display = 0;
        }
        if (completed == 0) {
        timer_handle = app_timer_register(ANIM_SPEED, handle_timer, NULL);
        }
    }
    
    void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
        
        char *TimeFormat;
        
        if (clock_is_24h_style()) {
            TimeFormat = "%R";
        } else {
            
            TimeFormat = "%I:%M";
        }
        
        strftime(TimeText, sizeof(TimeText), TimeFormat, tick_time);
        if (!clock_is_24h_style() && (TimeText[0] == '0')) {
            memmove(TimeText, &TimeText[1], sizeof(TimeText) - 1);
        }
        
        text_layer_set_text(TimeLayer, TimeText);
        
        if (!clock_is_24h_style()) {
            strftime(AMPM, sizeof(AMPM), "%p", tick_time);
            text_layer_set_text(AMPMLayer, AMPM);
        }
        
        strftime(DayText, sizeof(DayText), "%a", tick_time);
        text_layer_set_text(DayLayer, DayText);
        current_minute = tick_time->tm_min;
		if ((current_minute > minute_count) && (finish_up == 0)) {
			finish_up = 1;
			x = 0;
			y = 0;
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Finishing Drawing!");
		}
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Completed = %i", completed);
#ifdef PBL_COLOR
        if (completed) {
          
          red = rand() % 256;
          green = rand() % 256;
          blue = rand() % 256;          
        }
#endif
        
        if (new_hour != tick_time->tm_hour) {
#ifdef PBL_BW
            for (pixel_x=0; pixel_x <18; pixel_x++) {
#else
                for (pixel_x=0; pixel_x <144 + (x_offset * 2); pixel_x++) {
#endif
                    for (pixel_y=0; pixel_y <138 + y_offset; pixel_y++) {
                        fb_data[pixel_x][pixel_y] = 0;
                    }
                }
                pick_pattern();
                new_hour = tick_time->tm_hour;
                minute_count = current_minute + battery_saver;
                timer_handle = app_timer_register(ANIM_SPEED, handle_timer, NULL);
                fp1 = int2sll(0);
                fp2 = int2sll(0);
                fp3 = int2sll(0);
                fp4 = int2sll(0);
#ifdef PBL_COLOR
                red = rand() % 256;
                green = rand() % 256;
                blue = rand() % 256;
#endif
                update_display = 0;
                layer_mark_dirty(ChaosLayer);
            }
            
        }
        
        void handle_init(void) {
            
            window = window_create();
            window_stack_push(window, true /* Animated */);
            Layer *window_layer = window_get_root_layer(window);
            window_set_background_color(window, GColorBlack);
#ifdef PBL_ROUND
            x_offset = 18;
            y_offset = 6;
#endif
#ifdef PBL_PLATFORM_EMERY
            x_offset = 28;
            y_offset = 30;
#endif
            ChaosLayer = layer_create(GRect(0,0,(144 + (x_offset * 2)),(138 + y_offset)));
            layer_set_update_proc(ChaosLayer, chaoslayer_update_callback);
            layer_add_child(window_layer, ChaosLayer);
            
            
#ifdef PBL_PLATFORM_EMERY
            TimeLayer = text_layer_create(GRect(28, 180, 144, 33));
            text_layer_set_text_color(TimeLayer, GColorWhite);
            text_layer_set_background_color(TimeLayer, GColorBlack);
            text_layer_set_text_alignment(TimeLayer, GTextAlignmentCenter);
            text_layer_set_font(TimeLayer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_REGULAR_30)));
            layer_add_child(window_layer, text_layer_get_layer(TimeLayer));
            
            if (!clock_is_24h_style()) {
                AMPMLayer = text_layer_create(GRect(134, 184, 30, 14));
                text_layer_set_text_color(AMPMLayer, GColorWhite);
                text_layer_set_background_color(AMPMLayer, GColorBlack);
                text_layer_set_text_alignment(AMPMLayer, GTextAlignmentCenter);
                text_layer_set_font(AMPMLayer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_REGULAR_14)));
                layer_add_child(window_layer, text_layer_get_layer(AMPMLayer));
            }
            
            DayLayer = text_layer_create(GRect(22, 184, 50, 14));
            text_layer_set_text_color(DayLayer, GColorWhite);
            text_layer_set_background_color(DayLayer, GColorClear);
            text_layer_set_text_alignment(DayLayer, GTextAlignmentCenter);
            text_layer_set_font(DayLayer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_REGULAR_14)));
            layer_add_child(window_layer, text_layer_get_layer(DayLayer));
#else
            TimeLayer = text_layer_create(GRect(x_offset+0, y_offset+138, 144, 33));
            text_layer_set_text_color(TimeLayer, GColorWhite);
            text_layer_set_background_color(TimeLayer, GColorBlack);
            text_layer_set_text_alignment(TimeLayer, GTextAlignmentCenter);
            text_layer_set_font(TimeLayer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_REGULAR_30)));
            layer_add_child(window_layer, text_layer_get_layer(TimeLayer));
            
            if (!clock_is_24h_style()) {
                AMPMLayer = text_layer_create(GRect(x_offset+106, y_offset+142, 30, 14));
                text_layer_set_text_color(AMPMLayer, GColorWhite);
                text_layer_set_background_color(AMPMLayer, GColorBlack);
                text_layer_set_text_alignment(AMPMLayer, GTextAlignmentCenter);
                text_layer_set_font(AMPMLayer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_REGULAR_14)));
                layer_add_child(window_layer, text_layer_get_layer(AMPMLayer));
            }
            
            DayLayer = text_layer_create(GRect(x_offset+0, y_offset+142, 50, 14));
            text_layer_set_text_color(DayLayer, GColorWhite);
            text_layer_set_background_color(DayLayer, GColorClear);
            text_layer_set_text_alignment(DayLayer, GTextAlignmentCenter);
            text_layer_set_font(DayLayer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_REGULAR_14)));
            layer_add_child(window_layer, text_layer_get_layer(DayLayer));
#endif
            srand(time(NULL));
            pick_pattern();
#ifdef PBL_COLOR
            red = rand() % 256;
            green = rand() % 256;
            blue = rand() % 256;
            red_di = rand() % 2;
            green_di = rand() % 2;
            blue_di = rand() % 2;
            if (red_di == 0) red_di = -1;
            if (green_di == 0) green_di = -1;
            if (blue_di == 0) blue_di = -1;
#endif
            time_t now = time(NULL);
            struct tm *current_time = localtime(&now);
            new_hour = current_time->tm_hour;
            current_minute = current_time->tm_min;
            minute_count = current_minute + battery_saver;
            handle_minute_tick(current_time, MINUTE_UNIT);
            timer_handle = app_timer_register(ANIM_SPEED, handle_timer, NULL);
            tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
            
        }
        
        void handle_deinit(void) {
            
            tick_timer_service_unsubscribe();
            text_layer_destroy(TimeLayer);
            text_layer_destroy(AMPMLayer);
            text_layer_destroy(DayLayer);
            layer_destroy(ChaosLayer);
            window_destroy(window);
            
        }
        
        int main(void) {
            
            handle_init();
            app_event_loop();
            handle_deinit();
            
        }
