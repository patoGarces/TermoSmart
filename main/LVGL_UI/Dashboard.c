#include "Dashboard.h"

/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
    DISP_SMALL,
    DISP_MEDIUM,
    DISP_LARGE,
} disp_size_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void Onboard_create(lv_obj_t * parent);

// void dashboard_increase_lvgl_tick(lv_timer_t * t);
/**********************
 *  STATIC VARIABLES
 **********************/
static disp_size_t disp_size;

static lv_obj_t * tv;
lv_style_t style_title;
static lv_style_t style_bullet;
static lv_style_t style_temp;

lv_obj_t *wifiLabel;
lv_obj_t *mqttLabel;

static lv_style_t style_status_ok;
static lv_style_t style_status_error;
static lv_style_t style_status_normal;

static const lv_font_t * font_large;
static const lv_font_t * font_normal;
static const lv_font_t * font_temp;

static lv_timer_t * meter2_timer;

lv_obj_t * tempCard;
lv_obj_t * heatingIcon;
lv_obj_t * tempValue;
lv_obj_t * tempDecimalsLabel;

void IRAM_ATTR auto_switch(lv_timer_t * t)
{
  uint16_t page = lv_tabview_get_tab_act(tv);

  if (page == 0) { 
    lv_tabview_set_act(tv, 1, LV_ANIM_ON); 
  } else if (page == 3) {
    lv_tabview_set_act(tv, 2, LV_ANIM_ON); 
  }
}

void updateTabColor(bool heating)
{
    lv_obj_t * tab_bar = lv_obj_get_child(tv, 0);

    lv_color_t color = heating
        ? TEMP_COLOR_HOT
        : TEMP_COLOR_COLD;

    lv_obj_set_style_bg_color(
      tab_bar,
      color,
      LV_PART_ITEMS | LV_STATE_CHECKED
    );

    lv_obj_set_style_border_color(
      tab_bar,
      color,
      LV_PART_ITEMS | LV_STATE_CHECKED
    );

    lv_obj_set_style_border_width(
      tab_bar,
      2,
      LV_PART_ITEMS | LV_STATE_CHECKED
    );

    lv_obj_set_style_text_color(
      tab_bar,
      lv_color_white(),
      LV_PART_ITEMS
    );

    lv_obj_set_style_text_color(
      tab_bar,
      lv_color_white(),
      LV_PART_ITEMS | LV_STATE_CHECKED
    );
}

void dashboardInit(void) {
  disp_size = DISP_SMALL;                            

  font_large = LV_FONT_DEFAULT;                             
  font_normal = LV_FONT_DEFAULT;    
  font_temp = LV_FONT_DEFAULT;                     
  
  lv_coord_t tab_h;
  tab_h = 45;

  #if LV_FONT_MONTSERRAT_48
    font_temp     = &lv_font_montserrat_48;
  #else
    LV_LOG_WARN("LV_FONT_MONTSERRAT_48 is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.");
  #endif
  #if LV_FONT_MONTSERRAT_18
    font_large     = &lv_font_montserrat_18;
  #else
    LV_LOG_WARN("LV_FONT_MONTSERRAT_18 is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.");
  #endif
  #if LV_FONT_MONTSERRAT_12
    font_normal    = &lv_font_montserrat_12;
  #else
    LV_LOG_WARN("LV_FONT_MONTSERRAT_12 is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.");
  #endif

  lv_style_init(&style_title);
  lv_style_set_text_font(&style_title, font_large);
  lv_style_set_text_color(&style_title, lv_color_white());

  lv_style_init(&style_bullet);
  lv_style_set_border_width(&style_bullet, 0);
  lv_style_set_radius(&style_bullet, LV_RADIUS_CIRCLE);

  lv_style_init(&style_temp);
  lv_style_set_text_font(&style_temp, font_temp);
  lv_style_set_text_color(&style_temp, lv_color_white());

  tv = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, tab_h);

  lv_obj_set_style_text_font(lv_scr_act(), font_large, 0);

  lv_obj_t * t1 = lv_tabview_add_tab(tv, "TermoSmart");
  lv_obj_set_style_bg_color(tv, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(tv, LV_OPA_COVER, LV_PART_MAIN);

  updateTabColor(false);  // relay OFF
  
  Onboard_create(t1);
}

void Lvgl_Example1_close(void) {
  /*Delete all animation*/
  lv_anim_del(NULL, NULL);

  lv_timer_del(meter2_timer);
  meter2_timer = NULL;

  lv_obj_clean(lv_scr_act());

  lv_style_reset(&style_title);
  lv_style_reset(&style_bullet);
  lv_style_reset(&style_temp);
}

/**********************
*   STATIC FUNCTIONS
**********************/

static void Onboard_create(lv_obj_t * parent) {

    tempCard = lv_obj_create(parent);
    lv_obj_set_size(tempCard, 140, 100);

    lv_obj_set_style_bg_color(tempCard, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(tempCard, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(tempCard, 2, 0);
    lv_obj_set_style_radius(tempCard, 8, 0);

    lv_obj_set_style_pad_all(tempCard, 5, 0);

    lv_obj_clear_flag(tempCard, LV_OBJ_FLAG_SCROLLABLE);

    heatingIcon = lv_label_create(tempCard);
    lv_label_set_text(heatingIcon, LV_SYMBOL_CHARGE);
    lv_obj_add_style(heatingIcon, &style_title, 0);
    lv_obj_set_style_text_color(heatingIcon, lv_color_white(), 0);
    lv_obj_set_style_translate_x(heatingIcon, 10, 0);
    lv_obj_add_flag(heatingIcon, LV_OBJ_FLAG_HIDDEN);       // arranca oculto

    tempValue = lv_label_create(tempCard);
    lv_obj_set_width(tempValue, 70);
    lv_label_set_text(tempValue, "--");

    lv_obj_add_style(tempValue, &style_temp, 0);
    lv_obj_set_style_text_align(tempValue, LV_TEXT_ALIGN_RIGHT, 0);

    tempDecimalsLabel = lv_label_create(tempCard);
    lv_label_set_text(tempDecimalsLabel, ".-°C");
    lv_obj_add_style(tempDecimalsLabel, &style_title, 0);
    lv_obj_set_style_translate_y(tempDecimalsLabel, -8, 0);

    lv_obj_set_flex_flow(tempCard, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(
        tempCard,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER
    );

    lv_obj_set_style_pad_column(tempCard, 6, 0);            // margen entre labels de temperatura

    lv_style_init(&style_status_ok);
    lv_style_set_text_color(&style_status_ok, lv_color_hex(0x00FF00));

    lv_style_init(&style_status_error);
    lv_style_set_text_color(&style_status_error, lv_color_hex(0xFF0000));

    lv_style_init(&style_status_normal);
    lv_style_set_text_color(&style_status_normal, lv_color_white());

    wifiLabel = lv_label_create(parent);
    lv_label_set_text(wifiLabel, "WiFi: --");
    lv_obj_add_style(wifiLabel, &style_status_normal, 0);


    mqttLabel = lv_label_create(parent);
    lv_label_set_text(mqttLabel, "MQTT: --");
    lv_obj_add_style(mqttLabel, &style_status_normal, 0);

    static lv_coord_t col_dsc[] = {
        LV_GRID_FR(1),
        LV_GRID_TEMPLATE_LAST
    };

    static lv_coord_t row_dsc[] = {
      LV_GRID_CONTENT, // header
      LV_GRID_FR(1),   // espacio disponible para temperatura
      LV_GRID_CONTENT, // wifi
      LV_GRID_CONTENT, // mqtt
      LV_GRID_TEMPLATE_LAST
    };

    lv_obj_set_grid_dsc_array(parent, col_dsc, row_dsc);

    // temperatura centrada arriba
    lv_obj_set_grid_cell(
        tempCard,
        LV_GRID_ALIGN_CENTER, 0, 1,
        LV_GRID_ALIGN_CENTER, 1, 1
    );

    // estados abajo
    lv_obj_set_grid_cell(
        wifiLabel,
        LV_GRID_ALIGN_CENTER, 0, 1,
        LV_GRID_ALIGN_CENTER, 2, 1
    );

    lv_obj_set_grid_cell(
        mqttLabel,
        LV_GRID_ALIGN_CENTER, 0, 1,
        LV_GRID_ALIGN_CENTER, 3, 1
    );
}

void updateDashboardTemp(float newTemp) {
    char buf[100]={0}; 
    snprintf(buf, sizeof(buf), "%d", (uint8_t)newTemp);
    lv_label_set_text(tempValue, buf);

    snprintf(buf, sizeof(buf), ".%d°C", ((int)(newTemp * 10)) % 10);
    lv_label_set_text(tempDecimalsLabel, buf);
}

void updateDashboardWifiStatus(bool connected) {
    lv_label_set_text(wifiLabel, connected ? "WiFi: OK" : "WiFi: OFF");

    lv_obj_remove_style(wifiLabel, &style_status_ok, 0);
    lv_obj_remove_style(wifiLabel, &style_status_error, 0);

    lv_obj_add_style(wifiLabel, connected ? &style_status_ok : &style_status_error, 0);

    if (!connected) {
      lv_obj_set_style_bg_color(tempCard, lv_color_black(), 0);
    }
}

void updateDashboardMqttStatus(bool connected) {
    lv_label_set_text(mqttLabel, connected ? "MQTT: OK" : "MQTT: OFF");

    lv_obj_remove_style(mqttLabel, &style_status_ok, 0);
    lv_obj_remove_style(mqttLabel, &style_status_error, 0);

    lv_obj_add_style(mqttLabel, connected ? &style_status_ok : &style_status_error, 0);

    if (!connected) {
      lv_obj_set_style_bg_color(tempCard, lv_color_black(), 0);
    }
}

void updateDashboardRelayStatus(bool on) {
    // updateTabColor(on);

    if (on)
      lv_obj_clear_flag(heatingIcon, LV_OBJ_FLAG_HIDDEN);
    else
      lv_obj_add_flag(heatingIcon, LV_OBJ_FLAG_HIDDEN);
}

void updateDashboardTempColor(lv_color_t newColor) {
  lv_obj_set_style_bg_color(tempCard, newColor, 0);
}