/****************************************************************************
 *   Aug 3 12:17:11 2020
 *   Copyright  2020  Dirk Brosswick
 *   Email: dirk.brosswick@googlemail.com
 ****************************************************************************/
 
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include "config.h"
#include <TTGO.h>

#include "lua_rt_app.h"
#include "lua_rt_app_main.h"

#include "gui/mainbar/app_tile/app_tile.h"
#include "gui/mainbar/main_tile/main_tile.h"
#include "gui/mainbar/mainbar.h"
#include "gui/statusbar.h"

lv_obj_t *lua_rt_app_main_tile = NULL;
lv_style_t lua_rt_app_main_style;
lv_task_t * _lua_rt_app_task;

LV_IMG_DECLARE(exit_32px);
LV_IMG_DECLARE(setup_32px);
LV_IMG_DECLARE(refresh_32px);
LV_FONT_DECLARE(Ubuntu_16px);

static void exit_lua_rt_app_main_event_cb( lv_obj_t * obj, lv_event_t event );
static void enter_lua_rt_app_setup_event_cb( lv_obj_t * obj, lv_event_t event );
static void refresh_output_event_cb( lv_obj_t * obj, lv_event_t event );
void lua_rt_app_task( lv_task_t * task );

 //#define UseOutputLabel 

static lv_obj_t *lua_rt_cont;
static lv_style_t lua_rt_cont_main_style;
#ifdef UseOutputLabel
static lv_obj_t *lua_rt_output_label;
static lv_style_t lua_rt_output_style;
#endif 
struct mb_interpreter_t* bas = NULL;
char * buffer = NULL;
#define LuaRTThreads 4
const char *LuaFileName = "/spiffs/myfile.bas";
#define dbg(x) Serial.println(x)

void lua_rt_app_main_setup( uint32_t tile_num ) {

    lua_rt_app_main_tile = mainbar_get_tile_obj( tile_num );
    lv_style_copy( &lua_rt_app_main_style, mainbar_get_style() );

    lv_obj_t * exit_btn = lv_imgbtn_create( lua_rt_app_main_tile, NULL);
    lv_imgbtn_set_src(exit_btn, LV_BTN_STATE_RELEASED, &exit_32px);
    lv_imgbtn_set_src(exit_btn, LV_BTN_STATE_PRESSED, &exit_32px);
    lv_imgbtn_set_src(exit_btn, LV_BTN_STATE_CHECKED_RELEASED, &exit_32px);
    lv_imgbtn_set_src(exit_btn, LV_BTN_STATE_CHECKED_PRESSED, &exit_32px);
    lv_obj_add_style(exit_btn, LV_IMGBTN_PART_MAIN, &lua_rt_app_main_style );
    lv_obj_align(exit_btn, lua_rt_app_main_tile, LV_ALIGN_IN_BOTTOM_LEFT, 10, -10 );
    lv_obj_set_event_cb( exit_btn, exit_lua_rt_app_main_event_cb );

    lv_obj_t * setup_btn = lv_imgbtn_create( lua_rt_app_main_tile, NULL);
    lv_imgbtn_set_src(setup_btn, LV_BTN_STATE_RELEASED, &setup_32px);
    lv_imgbtn_set_src(setup_btn, LV_BTN_STATE_PRESSED, &setup_32px);
    lv_imgbtn_set_src(setup_btn, LV_BTN_STATE_CHECKED_RELEASED, &setup_32px);
    lv_imgbtn_set_src(setup_btn, LV_BTN_STATE_CHECKED_PRESSED, &setup_32px);
    lv_obj_add_style(setup_btn, LV_IMGBTN_PART_MAIN, &lua_rt_app_main_style );
    lv_obj_align(setup_btn, lua_rt_app_main_tile, LV_ALIGN_IN_BOTTOM_RIGHT, -10, -10 );
    lv_obj_set_event_cb( setup_btn, enter_lua_rt_app_setup_event_cb );




    /************ lua_rt_output_label label for  "PRINT" redirection *********/
#ifdef UseLuaOutputLabel    
    lv_style_copy( &lua_rt_output_style, &lua_rt_cont_main_style );
    lv_style_set_text_font( &lua_rt_output_style, LV_STATE_DEFAULT, &Ubuntu_16px);
    lv_style_set_text_color(&lua_rt_output_style, LV_STATE_DEFAULT, LV_COLOR_BLUE);
    lua_rt_output_label = lv_label_create(lua_rt_cont, NULL);
    lv_obj_add_style( lua_rt_output_label, LV_OBJ_PART_MAIN, &lua_rt_output_style );
    //lv_label_set_long_mode(lua_rt_output_label, LV_LABEL_LONG_SROLL);            /*Automatically scroll long lines*/
    lv_obj_set_width(lua_rt_output_label, lv_page_get_width_fit(lua_rt_cont));          /*Set the label width to max value to not show hor. scroll bars*/
    lv_label_set_text(lua_rt_output_label, "");
#endif


    lv_obj_t * reload_btn = lv_imgbtn_create( lua_rt_app_main_tile, NULL);
    lv_imgbtn_set_src(reload_btn, LV_BTN_STATE_RELEASED, &refresh_32px);
    lv_imgbtn_set_src(reload_btn, LV_BTN_STATE_PRESSED, &refresh_32px);
    lv_imgbtn_set_src(reload_btn, LV_BTN_STATE_CHECKED_RELEASED, &refresh_32px);
    lv_imgbtn_set_src(reload_btn, LV_BTN_STATE_CHECKED_PRESSED, &refresh_32px);
    lv_obj_add_style(reload_btn, LV_IMGBTN_PART_MAIN, &lua_rt_app_main_style );
    lv_obj_align(reload_btn, lua_rt_app_main_tile, LV_ALIGN_IN_BOTTOM_MID, 0 , -10 );
    lv_obj_set_event_cb( reload_btn, refresh_output_event_cb );


    // create an task that runs every secound
    _lua_rt_app_task = lv_task_create( lua_rt_app_task, 1000, LV_TASK_PRIO_MID, NULL );
}

static void enter_lua_rt_app_setup_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):       statusbar_hide( true );
                                        mainbar_jump_to_tilenumber( lua_rt_app_get_app_setup_tile_num(), LV_ANIM_ON );
                                        break;
    }
}

static void exit_lua_rt_app_main_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):       CloseBasic();
                                        mainbar_jump_to_maintile( LV_ANIM_OFF );
                                        break;
    }
}

static void refresh_output_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):       DoBasic();
                                        break;
    }
}

void lua_rt_app_task( lv_task_t * task ) {
    // put your code here
}




bool InitLua ( void ) {

    FILE * pFile;
    long lSize;

    size_t result;
    log_i("Loading %s\r\n", LuaFileName);
    pFile = fopen ( LuaFileName, "r" );
    if (pFile==NULL) {Serial.printf ("File error"); return false;}

    // obtain file size:
    fseek (pFile , 0 , SEEK_END);
    lSize = ftell (pFile);
    rewind (pFile);

    // allocate memory to contain the whole file:
    buffer = (char*) malloc (sizeof(char)*lSize+1);
    if (buffer == NULL) {Serial.printf ("Memory error"); return false;}

    // copy the file into the buffer:
    result = fread (buffer,1,lSize,pFile);
    buffer[lSize]=0;
    if (result != lSize) {Serial.printf ("Reading error"); return false;}

    // terminate
    fclose (pFile);
    log_i("Loaded %d bytes of code\r\n", lSize);
    //log_i("-------------Source----------\n%s-----------End------\n", buffer);

    log_i("Free heap: %d\r\n", ESP.getFreeHeap());
    log_i("Free PSRAM: %d\r\n", ESP.getFreePsram());
    log_i("Lua Runtime RUN\n");

    /************ lua_rt_cont main container (for lvgl integration) *********/
    lua_rt_cont = lv_obj_create(lua_rt_app_main_tile, NULL);
    lv_obj_set_size(lua_rt_cont, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL) - 64);
    lv_style_init(&lua_rt_cont_main_style); 
    lv_style_copy(&lua_rt_cont_main_style, &lua_rt_app_main_style);
    lv_style_set_bg_color(&lua_rt_cont_main_style, LV_OBJ_PART_MAIN, LV_COLOR_BLUE);
    lv_obj_add_style(lua_rt_cont, LV_OBJ_PART_MAIN, &lua_rt_cont_main_style);
    lv_obj_align(lua_rt_cont, lua_rt_app_main_tile, LV_ALIGN_IN_TOP_MID, 0, 32);

    // This is where the code that runs the lua interpreter begins.
    // Need to create a Lua context here then load the program we read into memory above.
    

    
  return true;
}

bool DoLua( void ) {

    // Get that Lua context which should contain the code by now, adn run it.
    
}

void CloseLua (void) {

    // Close the Lua context.
    
  log_i("Lua Runtime END\r\n");

  log_i("Free heap: %d\r\n", ESP.getFreeHeap());
  log_i("Free PSRAM: %d\r\n", ESP.getFreePsram());
  free (buffer);
}

// TODO: Follow patterns below to establish error handling.

#if 0
static void _on_error(struct mb_interpreter_t* s, mb_error_e e, const char* m, const char* f, int p, unsigned short row, unsigned short col, int abort_code);
static void _on_error(struct mb_interpreter_t* s, mb_error_e e, const char* m, const char* f, int p, unsigned short row, unsigned short col, int abort_code) {
    mb_unrefvar(s);
    mb_unrefvar(p);

    if (e != SE_NO_ERR) {
        if (f) {
            if (e == SE_RN_WRONG_FUNCTION_REACHED) {
                log_e(
                    "Error:\n    Ln %d, Col %d in Func: %s\n    Code %d, Abort Code %d\n    Message: %s.\n",
                    row, col, f,
                    e, abort_code,
                    m
                );
            }
            else {
                log_e(
                    "Error:\n    Ln %d, Col %d in File: %s\n    Code %d, Abort Code %d\n    Message: %s.\n",
                    row, col, f,
                    e, e == SE_EA_EXTENDED_ABORT ? abort_code - MB_EXTENDED_ABORT : abort_code,
                    m
                );
            }
        }
        else {
            log_e(
                "Error:\n    Ln %d, Col %d\n    Code %d, Abort Code %d\n    Message: %s.\n",
                row, col,
                e, e == SE_EA_EXTENDED_ABORT ? abort_code - MB_EXTENDED_ABORT : abort_code,
                m
            );
        }
    }
}
#endif
