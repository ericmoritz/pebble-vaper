#include <pebble.h>
#include "model.h"
#define LABEL_FONT FONT_KEY_GOTHIC_28_BOLD
#define LABEL_FONT_HEIGHT 28

#define VALUE_FONT FONT_KEY_GOTHIC_28
#define VALUE_FONT_HEIGHT 28
#define VALUE_WIDTH 60
#define VALUE_BUFFER_SIZE 16  

#define LOCKED_WIDTH 20
/*******************************************************************
 ** UI
 *******************************************************************/
typedef struct Unit {
  char *label;
  char *units;
  char val_buffer[VALUE_BUFFER_SIZE];
  TextLayer *locked_layer;
  TextLayer *label_layer;
  TextLayer *value_layer;
} Unit;

static Window *window;

static Unit p_unit;
static Unit i_unit;
static Unit v_unit;
static Unit r_unit;


static Model model;

static int selected;



char* doubleToString(char* buffer, int bufferSize, double number)
{
  char decimalBuffer[4];

  snprintf(buffer, bufferSize, "%d", (int)number);
  strcat(buffer, ".");

  snprintf(decimalBuffer, 4, "%01d", (int)((double)(number - (int)number) * (double)10));
  strcat(buffer, decimalBuffer);

  return buffer;
}

static void format_value(char *buffer, int n, char *units, double value) {
  doubleToString(buffer, 16, value);
}

static Unit *find_unit(int index) {
  switch (index) {
  case WATTS:
    return &p_unit;
  case AMPS:
    return &i_unit;
  case VOLTS:
    return &v_unit;
  default:
    return &r_unit;
  }
}

static Unit *selected_unit() {
  return find_unit(selected);
}

static Unit *locked_unit() {
  return find_unit(model.locked);
}

static void clear_selected() {
  Unit *s = selected_unit();
  text_layer_set_background_color(s->label_layer, GColorClear);
  text_layer_set_text_color(s->label_layer, GColorBlack);
}


static void draw_selected() {
  Unit *s = selected_unit();
  text_layer_set_background_color(s->label_layer, GColorBlack);
  text_layer_set_text_color(s->label_layer, GColorWhite);
}

static void draw_locked() {
  Unit *s = locked_unit();
  text_layer_set_text(s->locked_layer, "*");
}

static void clear_locked() {
  Unit *s = locked_unit();
  text_layer_set_text(s->locked_layer, "");
}

static void unit_fill_value(Unit *unit, double value) {
  doubleToString(unit->val_buffer, VALUE_BUFFER_SIZE, value);
  text_layer_set_text(unit->value_layer, unit->val_buffer);
}

static void fill_values() {
  unit_fill_value(&p_unit, model.p);
  unit_fill_value(&i_unit, model.i);
  unit_fill_value(&v_unit, model.v);
  unit_fill_value(&r_unit, model.r);
}

static void window_redraw() {
  draw_locked();
  draw_selected();
  fill_values();
}

static TextLayer *text_layer_load(Layer *parent, GRect frame, GFont font, char *text, int y) {
  TextLayer *t = text_layer_create(frame);
  text_layer_set_font(t, fonts_get_system_font(font));
  text_layer_set_text(t, text);
  layer_add_child(parent, text_layer_get_layer(t));
  return t;
}
static TextLayer *locked_layer_load(GRect *bounds, Layer *parent, int y) {
  return text_layer_load(
			 parent,
			 (GRect) { .origin = {0, y }, .size = { LOCKED_WIDTH, LABEL_FONT_HEIGHT } },
			 LABEL_FONT,
			 "",
			 y
			 );
}
static TextLayer *label_layer_load(GRect *bounds, Layer *parent, char *label, int y) {
  return text_layer_load(
			 parent,
			 (GRect) { .origin = {LOCKED_WIDTH, y }, .size = { bounds->size.w, LABEL_FONT_HEIGHT } },
			 LABEL_FONT,
			 label,
			 y
			 );
}

static TextLayer *value_layer_load(GRect *bounds, Layer *parent, char *value, int y) {
  TextLayer *t;
  t = text_layer_load(
		      parent,
		      (GRect) { .origin = {bounds->size.w-VALUE_WIDTH, y }, .size = { VALUE_WIDTH, VALUE_FONT_HEIGHT } },
		      VALUE_FONT,
		      value,
		      y
		      );
  text_layer_set_text_alignment(t, GTextAlignmentRight);
  return t;
}

static void unit_load(Unit *unit, char *label, char *units, GRect *bounds, Layer *parent, int y) {
  unit->label = label;
  unit->units = units;
  unit->locked_layer = locked_layer_load(bounds, parent, y);
  unit->label_layer = label_layer_load(bounds, parent, label, y);
  unit->value_layer = value_layer_load(bounds, parent, "", y);
}

static void unit_destroy(Unit *unit) {
  text_layer_destroy(unit->label_layer);
  text_layer_destroy(unit->value_layer);
}

static void window_load(Window *window) {
  int y = 0;
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  unit_load(&p_unit, "Power", "w", &bounds, window_layer, y);
  unit_load(&i_unit, "Curr", "a", &bounds, window_layer, y+=LABEL_FONT_HEIGHT);
  unit_load(&v_unit, "Volts", "v", &bounds, window_layer, y+=LABEL_FONT_HEIGHT);
  unit_load(&r_unit, "Res", "o", &bounds, window_layer, y+=LABEL_FONT_HEIGHT);

  window_redraw();
}


static void window_unload(Window *window) {
  unit_destroy(&p_unit);
  unit_destroy(&i_unit);
  unit_destroy(&v_unit);
  unit_destroy(&r_unit);
}



static void up_10x_click_handler(ClickRecognizerRef rec, void *context) {
  model_incr(&model, selected, 1);
  window_redraw();
}

static void down_10x_click_handler(ClickRecognizerRef rec, void *context) {
  model_incr(&model, selected, -1);
  window_redraw();
}

static void up_click_handler(ClickRecognizerRef rec, void *context) {
  model_incr(&model, selected, 0.1);
  window_redraw();
}

static void down_click_handler(ClickRecognizerRef rec, void *context) {
  model_incr(&model, selected, -0.1);
  window_redraw();
}


static void select_click_handler(ClickRecognizerRef rec, void *context) {
  clear_selected();
  selected = selected << 1;
  if(selected > 8) 
    selected = 1;
  draw_selected();
}


static void lock_click_handler(ClickRecognizerRef rec, void *context) {
  clear_locked();
  model.locked = selected;
  draw_locked();  
}

static void window_click_config_provider(void *context) {
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 250, down_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_UP, 250, up_click_handler);

  window_long_click_subscribe(BUTTON_ID_UP, 500, lock_click_handler, NULL);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 500, lock_click_handler, NULL);
}
static void init(void) {
  model_init(&model);
  selected = VOLTS;
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload
  });
  window_set_click_config_provider(window, window_click_config_provider);
  window_stack_push(window, true);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);
  app_event_loop();
  deinit();
}
