/*
  ks-clock-face.c
  sorgente per watchface rotondo
  
  Deve essere integrato l'accesso a OpenWeather per ottenere le effemeridi del giorno
  a supporto del disegno della durata della giornata e dell'avanzamento del sole
  20170305 aggiunto js per la gestione della chiamata a openWeather, calcolo della durata
    e visualizzazione condizioni e durata
  
*/
#include <pebble.h>

#define COLORS PBL_IF_COLOR_ELSE(true, false)
#define ANTIALIASING true
#define HAND_MARGIN 10
#define ANIMATION_DURATION 500
#define ANIMATION_DELAY 600

// Persistent storage key
#define SETTINGS_KEY 1

// Define our settings struct
typedef struct ClaySettings {
  GColor backgroundColor;
  GColor backgroundBTColor;
  GColor textColor;
  GColor batteryColor;
  GColor hourColor;
  GColor quadrantColor;
  GColor innerFillColor;
  GColor hourhandColor;
  GColor minhandColor;
} ClaySettings;

// An instance of the struct
static ClaySettings settings;

typedef struct {
  uint8_t hours;
  uint8_t minutes;
} Time;

static Window *s_main_window;
static Layer *s_canvas_layer;

static GPoint s_center;
static Time s_last_time, s_anim_time;
static uint8_t s_radius = 0, s_anim_hours_60 = 0, s_color_channels[3];
static uint8_t s_radius_final;
static bool s_animating = false;

static TextLayer *s_time_layer, *s_date_layer, *s_steps_layer, *s_duration_layer;
static GFont s_time_font, s_date_font, s_steps_font;
static int s_battery_level;
static GColor s_background_color;
static char duration_layer_buffer[32];
// dirata per il disegno del quadrante
static double durataGiorno;


// Vibe pattern: ON for 200ms, OFF for 100ms, ON for 400ms:
static uint32_t const segments[] = { 200, 100, 200, 100, 200, 400, 500 };
VibePattern pat = {
  .durations = segments,
  .num_segments = ARRAY_LENGTH(segments),
};
/************************************ UI **************************************/
static void update_time() {
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer, and show the time
  static char buffer[] = "00:00";
  if(clock_is_24h_style()) {
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }
  text_layer_set_text(s_time_layer, buffer);
  
  // Show the date
  static char date_buffer[16];
  strftime(date_buffer, sizeof(date_buffer), "%a %d %b", tick_time);
  text_layer_set_text(s_date_layer, date_buffer);
  
  text_layer_set_text(s_duration_layer, duration_layer_buffer);
  
  text_layer_set_text_color(s_time_layer, settings.hourColor);
  text_layer_set_text_color(s_date_layer, settings.textColor);
  text_layer_set_text_color(s_steps_layer, settings.textColor);
  text_layer_set_text_color(s_duration_layer, settings.hourColor);
}

/************************************ configuration ***************************/
static void prv_save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
  // Update the display based on new settings
  // prv_update_display();
  if (s_canvas_layer) {
    layer_mark_dirty(s_canvas_layer);
  }
}

/* Configuration received
*/
static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
  // Read color preferences
  Tuple *bg_color_t = dict_find(iter, MESSAGE_KEY_backgroundColor);
  if(bg_color_t) {
    settings.backgroundColor = GColorFromHEX(bg_color_t->value->int32);
  }

  Tuple *bt_color_t = dict_find(iter, MESSAGE_KEY_backgroundBTColor);
  if(bt_color_t) {
    settings.backgroundBTColor = GColorFromHEX(bt_color_t->value->int32);
  }

  // Read boolean preferences
  Tuple *text_color_t = dict_find(iter, MESSAGE_KEY_textColor);
  if(text_color_t) {
    settings.textColor = GColorFromHEX(text_color_t->value->int32);
  }

  Tuple *battery_color_t = dict_find(iter, MESSAGE_KEY_batteryColor);
  if(battery_color_t) {
    settings.batteryColor = GColorFromHEX(battery_color_t->value->int32);
  }
  Tuple *hour_color_t = dict_find(iter, MESSAGE_KEY_hourColor);
  if(hour_color_t) {
    settings.hourColor = GColorFromHEX(hour_color_t->value->int32);
  }
  Tuple *quadrant_color_t = dict_find(iter, MESSAGE_KEY_quadrantColor);
  if(quadrant_color_t) {
    settings.quadrantColor = GColorFromHEX(quadrant_color_t->value->int32);
  }
  Tuple *innerFill_color_t = dict_find(iter, MESSAGE_KEY_innerFillColor);
  if(innerFill_color_t) {
    settings.innerFillColor = GColorFromHEX(innerFill_color_t->value->int32);
  }
  Tuple *hourhand_color_t = dict_find(iter, MESSAGE_KEY_hourhandColor);
  if(hourhand_color_t) {
    settings.hourhandColor = GColorFromHEX(hourhand_color_t->value->int32);
  }
  Tuple *minhand_color_t = dict_find(iter, MESSAGE_KEY_minhandColor);
  if(minhand_color_t) {
    settings.minhandColor = GColorFromHEX(minhand_color_t->value->int32);
  }
  prv_save_settings();
  s_background_color = settings.backgroundColor;
  update_time();
}

/************************************ Weather *********************************/

  static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char temperature_buffer[8];
  static char conditions_buffer[32];
  static char sunrise_buffer[32];
  static char sunset_buffer[32];
  static char weather_layer_buffer[32];
  static char sun_layer_buffer[32];
  static int ore, minuti;
  static float tmin;
  

  // Read tuples for data
  Tuple *type_tuple = dict_find(iterator, MESSAGE_KEY_type);  
  if( type_tuple){
    Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_temperature);
    Tuple *conditions_tuple = dict_find(iterator, MESSAGE_KEY_conditions);
    Tuple *sunrise_tuple = dict_find(iterator, MESSAGE_KEY_sunrise);
    Tuple *sunset_tuple = dict_find(iterator, MESSAGE_KEY_sunset);
  
    // If all data is available, use it
    if(temp_tuple && conditions_tuple) {
      snprintf(temperature_buffer, sizeof(temperature_buffer), "%dC", (int)temp_tuple->value->int32);
      snprintf(conditions_buffer, sizeof(conditions_buffer), "%d", (int)conditions_tuple->value->cstring);
  
      // used to translate the result to TIME
      time_t temp = (uint)sunrise_tuple->value->uint32;
      struct tm *tick_time = localtime( &temp );
      strftime(sunrise_buffer, sizeof("00:00"), "%H:%M", tick_time);
  
      temp = (uint)sunset_tuple->value->uint32;
      tick_time = localtime( &temp );
      strftime(sunset_buffer, sizeof("00:00"), "%H:%M", tick_time);
  
  //    snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", conditions_tuple->value->cstring);
  
      // Assemble full string and display
      snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", sunrise_buffer, sunset_buffer);
      // snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
      // text_layer_set_text(s_weather_layer, weather_layer_buffer);
  
      temp = (uint)sunset_tuple->value->uint32 - (uint)sunrise_tuple->value->uint32;
      ore = temp / 3600;
      tmin = temp / 3600.;
      minuti = (tmin - ore) * 60.;
      
      durataGiorno = temp/3600.;
      
      APP_LOG(APP_LOG_LEVEL_INFO, "seconds: %d h %d min %d", (uint)temp, ore, minuti);
      
      tick_time = localtime( &temp );
  
      strftime(sunset_buffer, sizeof("00:00"), "%H:%M", tick_time);
  
      snprintf(duration_layer_buffer, sizeof(duration_layer_buffer), "%s Dur.: %d:%d", conditions_tuple->value->cstring, ore, minuti);
      // snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
      // text_layer_set_text(s_sun_layer, sun_layer_buffer);
    }
    update_time();
  }else{
    APP_LOG(APP_LOG_LEVEL_ERROR, "Configurazione arrivata");
     prv_inbox_received_handler(iterator, &context); 
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}



/* ********************************* Services **********************************/
static void battery_callback(BatteryChargeState state) {
  // Record the new battery level
  s_battery_level = state.charge_percent; 
  if (s_canvas_layer) {
    layer_mark_dirty(s_canvas_layer);
  }
}

static void bluetooth_callback(bool connected) {
  // in case of state change vibes and sets the background color
  if(!connected) {
//    vibes_double_pulse();
    vibes_enqueue_custom_pattern(pat);
    s_background_color = settings.backgroundBTColor;
//    s_text_color = GColorWhite;
  }else{
    vibes_double_pulse();
    s_background_color = settings.backgroundColor;
//    s_text_color = GColorBlue;
  }
  window_set_background_color(s_main_window, s_background_color);

  // Redraw
  if (s_canvas_layer) {
    layer_mark_dirty(s_canvas_layer);
    update_time();
  }
}

/*************************** Health System ************************************/
void updateSteps(){
  HealthMetric metric = HealthMetricStepCount;

  time_t end = time(NULL);
  time_t start = time_start_of_today();
  time_t oneHour = end - SECONDS_PER_HOUR;

  static char date_buffer[26];

  // Check data is available
  HealthServiceAccessibilityMask result = health_service_metric_accessible(HealthMetricStepCount, oneHour, end);
  if(result & HealthServiceAccessibilityMaskAvailable) {
    // Data is available! Read it
    APP_LOG(APP_LOG_LEVEL_INFO, "Steps in the last hour: %d", (int)health_service_sum(metric, oneHour, end));
    // if the # of steps in the last hour is less then 500
    // change the color of the layer
/*
    if( (int)health_service_sum(metric, oneHour, end) < 180 ){
      layer_set_hidden(bitmap_layer_get_layer(s_walking_man_layer), false);

//      text_layer_set_background_color(s_steps_layer, GColorRed);
//      text_layer_set_text_color(s_steps_layer, GColorBlack);
    }else{
      layer_set_hidden(bitmap_layer_get_layer(s_walking_man_layer), true);
//      text_layer_set_background_color(s_steps_layer, GColorClear);      
//      text_layer_set_text_color(s_steps_layer, GColorPastelYellow);
    }
*/
    text_layer_set_background_color(s_steps_layer, GColorClear);      
    text_layer_set_text_color(s_steps_layer, settings.textColor);

    snprintf( date_buffer, sizeof(date_buffer), "2day %d:%d", (int)health_service_sum_today(metric), 
                                                             (int)health_service_sum(metric, oneHour, end) );
    text_layer_set_text(s_steps_layer, date_buffer);
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "No data available!");
  }
  
}
/*
  health system 20170212
  */
static void health_handler(HealthEventType event, void *context) {
  
  // Which type of event occurred?
  switch(event) {
    case HealthEventMetricAlert:
      APP_LOG(APP_LOG_LEVEL_INFO, 
              "New HealthService HealthEventMetricAlert event");
      break;
    case HealthEventSignificantUpdate:
      APP_LOG(APP_LOG_LEVEL_INFO, 
              "New HealthService HealthEventSignificantUpdate event");
      updateSteps();
    break;
    case HealthEventMovementUpdate:
      APP_LOG(APP_LOG_LEVEL_INFO, 
              "New HealthService HealthEventMovementUpdate event");
      updateSteps();
      break;
    case HealthEventSleepUpdate:
      APP_LOG(APP_LOG_LEVEL_INFO, 
              "New HealthService HealthEventSleepUpdate event");
      break;
    case HealthEventHeartRateUpdate:
      APP_LOG(APP_LOG_LEVEL_INFO,
              "New HealthService HealthEventHeartRateUpdate event");
      break;
  }
}

/*************************** AnimationImplementation **************************/

static void prv_animation_started(Animation *anim, void *context) {
  s_animating = true;
}

static void prv_animation_stopped(Animation *anim, bool stopped, void *context) {
  s_animating = false;
}

static void prv_animate(int duration, int delay, AnimationImplementation *implementation, bool handlers) {
  Animation *anim = animation_create();
  animation_set_duration(anim, duration);
  animation_set_delay(anim, delay);
  animation_set_curve(anim, AnimationCurveEaseInOut);
  animation_set_implementation(anim, implementation);
  if (handlers) {
    animation_set_handlers(anim, (AnimationHandlers) {
      .started = prv_animation_started,
      .stopped = prv_animation_stopped
    }, NULL);
  }
  animation_schedule(anim);
}

/************************************ UI **************************************/

static void prv_tick_handler(struct tm *tick_time, TimeUnits changed) {
  // Store time
  s_last_time.hours = tick_time->tm_hour;
  s_last_time.hours -= (s_last_time.hours > 12) ? 12 : 0;
  s_last_time.minutes = tick_time->tm_min;

  for (int i = 0; i < 3; i++) {
    s_color_channels[i] = rand() % 256;
  }

  // Redraw
  if (s_canvas_layer) {
    update_time();
    layer_mark_dirty(s_canvas_layer);
  }
}

static int prv_hours_to_minutes(int hours_out_of_12) {
  return hours_out_of_12 * 60 / 12;
}

static void prv_update_proc(Layer *layer, GContext *ctx) {
  GRect full_bounds = layer_get_bounds(layer);
  GRect bounds = layer_get_unobstructed_bounds(layer);
  s_center = grect_center_point(&bounds);

  // Color background?
  if (COLORS) {
    graphics_context_set_fill_color(ctx, s_background_color );
  } else {
    graphics_context_set_fill_color(ctx, GColorDarkGray);
  }
  graphics_fill_rect(ctx, full_bounds, 0, GCornerNone);

  graphics_context_set_stroke_color(ctx, settings.quadrantColor);
  graphics_context_set_stroke_width(ctx, 4);

  graphics_context_set_antialiased(ctx, ANTIALIASING);

  // White clockface
  graphics_context_set_fill_color(ctx, settings.innerFillColor);
  graphics_fill_circle(ctx, s_center, s_radius);

  // Draw outline
  // graphics_draw_circle(ctx, s_center, s_radius);

  // Don't use current time while animating
  Time mode_time = (s_animating) ? s_anim_time : s_last_time;

  // Adjust for minutes through the hour
  float minute_angle = TRIG_MAX_ANGLE * mode_time.minutes / 60;
  float hour_angle;
  if (s_animating) {
    // Hours out of 60 for smoothness
    hour_angle = TRIG_MAX_ANGLE * mode_time.hours / 60;
  } else {
    hour_angle = TRIG_MAX_ANGLE * mode_time.hours / 12;
  }
  hour_angle += (minute_angle / TRIG_MAX_ANGLE) * (TRIG_MAX_ANGLE / 12);

  // Plot hands
  GPoint minute_hand = (GPoint) {
    .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * mode_time.minutes / 60) * (int32_t)(s_radius - HAND_MARGIN) / TRIG_MAX_RATIO) + s_center.x,
    .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * mode_time.minutes / 60) * (int32_t)(s_radius - HAND_MARGIN) / TRIG_MAX_RATIO) + s_center.y,
  };
  GPoint hour_hand = (GPoint) {
    .x = (int16_t)(sin_lookup(hour_angle) * (int32_t)(s_radius - (2 * HAND_MARGIN)) / TRIG_MAX_RATIO) + s_center.x,
    .y = (int16_t)(-cos_lookup(hour_angle) * (int32_t)(s_radius - (2 * HAND_MARGIN)) / TRIG_MAX_RATIO) + s_center.y,
  };

  graphics_context_set_stroke_color(ctx, settings.hourhandColor);

  // Draw hands with positive length only
  if (s_radius > 2 * HAND_MARGIN) {
    graphics_draw_line(ctx, s_center, hour_hand);
  }

  graphics_context_set_stroke_color(ctx, settings.minhandColor);
  if (s_radius > HAND_MARGIN) {
    graphics_draw_line(ctx, s_center, minute_hand);
  }
  // Battery clockface  
  graphics_context_set_stroke_color(ctx, settings.batteryColor);
  graphics_context_set_stroke_width(ctx, 5);  
//  graphics_draw_circle(ctx, s_center, s_radius / 100. * s_battery_level);

//  GRect battRect = GRect(144/2 - s_radius -5, 168/2 - s_radius -5, s_radius * 2+11, s_radius * 2 +11);
  GRect battRect = GRect( s_center.x - s_radius -5, s_center.y - s_radius -5, s_radius * 2+11, s_radius * 2 +11);
  graphics_draw_arc( ctx, battRect, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(360 / 100. * s_battery_level));

  graphics_context_set_stroke_color(ctx, settings.quadrantColor);
  graphics_context_set_stroke_width(ctx, 7);  
//  GRect battRect = GRect(144/2 - s_radius -5, 168/2 - s_radius -5, s_radius * 2+11, s_radius * 2 +11);
  GRect quadrantRect = GRect( s_center.x - s_radius+3, s_center.y - s_radius+3, s_radius * 2 -5 , s_radius * 2 -5 );
  graphics_draw_arc( ctx, quadrantRect, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(90), DEG_TO_TRIGANGLE(90+360 / 24. * durataGiorno));
//  graphics_fill_radial( ctx, quadrantRect, GOvalScaleModeFitCircle, 5, DEG_TO_TRIGANGLE(90), DEG_TO_TRIGANGLE(90+360 / 24. * durataGiorno));
}

static int prv_anim_percentage(AnimationProgress dist_normalized, int max) {
  return (int)(dist_normalized * max / ANIMATION_NORMALIZED_MAX);
}

static void prv_radius_update(Animation *anim, AnimationProgress dist_normalized) {
  s_radius = prv_anim_percentage(dist_normalized, s_radius_final);

  layer_mark_dirty(s_canvas_layer);
}

static void prv_hands_update(Animation *anim, AnimationProgress dist_normalized) {
  s_anim_time.hours = prv_anim_percentage(dist_normalized, prv_hours_to_minutes(s_last_time.hours));
  s_anim_time.minutes = prv_anim_percentage(dist_normalized, s_last_time.minutes);

  layer_mark_dirty(s_canvas_layer);
}

static void prv_start_animation() {
  // Prepare animations
  static AnimationImplementation s_radius_impl = {
    .update = prv_radius_update
  };
  prv_animate(ANIMATION_DURATION, ANIMATION_DELAY, &s_radius_impl, false);

  static AnimationImplementation s_hands_impl = {
    .update = prv_hands_update
  };
  prv_animate(2 * ANIMATION_DURATION, ANIMATION_DELAY, &s_hands_impl, true);
}

static void prv_create_canvas() {
  Layer *window_layer = window_get_root_layer(s_main_window);
  GRect bounds = layer_get_unobstructed_bounds(window_layer);

  s_radius_final = (bounds.size.w - 30) / 2;

  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, prv_update_proc);
  layer_add_child(window_layer, s_canvas_layer);

  /* test dell'ora */
  s_time_font = fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS);
  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(0, 42, 144, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, settings.textColor);
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  s_date_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);  
  // Create date TextLayer
  s_date_layer = text_layer_create(GRect(0, 144, 144, 30));
  text_layer_set_text_color(s_date_layer, settings.textColor);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  text_layer_set_text(s_date_layer, "Sept 23");
  text_layer_set_font(s_date_layer, s_date_font);
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

  // Create duration TextLayer
  s_duration_layer = text_layer_create(GRect(0, 100, 144, 30));
  text_layer_set_text_color(s_duration_layer, settings.textColor);
  text_layer_set_background_color(s_duration_layer, GColorClear);
  text_layer_set_text_alignment(s_duration_layer, GTextAlignmentCenter);
  text_layer_set_text(s_date_layer, "duration");
  text_layer_set_font(s_date_layer, s_date_font);
  layer_add_child(window_layer, text_layer_get_layer(s_duration_layer));

  s_steps_font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  // Create steps TextLayer
  s_steps_layer = text_layer_create(GRect(10, -5, 144-20, 30));
  text_layer_set_text_color(s_steps_layer, settings.textColor);
  text_layer_set_background_color(s_steps_layer, GColorClear);
  text_layer_set_text_alignment(s_steps_layer, GTextAlignmentCenter);
  text_layer_set_text(s_steps_layer, "steps: --");
  text_layer_set_font(s_steps_layer, s_steps_font);
  layer_add_child(window_layer, text_layer_get_layer(s_steps_layer));
  
}

/*********************************** App **************************************/

// Event fires once, before the obstruction appears or disappears
static void prv_unobstructed_will_change(GRect final_unobstructed_screen_area, void *context) {
  if(s_animating) {
    return;
  }
  // Reset the clock animation
  s_radius = 0;
  s_anim_hours_60 = 0;
}

// Event fires once, after obstruction appears or disappears
static void prv_unobstructed_did_change(void *context) {
  if(s_animating) {
    return;
  }
  // Play the clock animation
  prv_start_animation();
}

static void prv_window_load(Window *window) {
  prv_create_canvas();

  prv_start_animation();

  tick_timer_service_subscribe(MINUTE_UNIT, prv_tick_handler);

  // Subscribe to the unobstructed area events
  UnobstructedAreaHandlers handlers = {
    .will_change = prv_unobstructed_will_change,
    .did_change = prv_unobstructed_did_change
  };
  unobstructed_area_service_subscribe(handlers, NULL);

  battery_callback(battery_state_service_peek());
  bluetooth_callback(connection_service_peek_pebble_app_connection());

  // Subscrive the health service
  #if defined(PBL_HEALTH)
    // Attempt to subscribe 
    if(!health_service_events_subscribe(health_handler, NULL)) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Health NOT available!");
    }
    #else
      APP_LOG(APP_LOG_LEVEL_ERROR, "Health available!");
  #endif


}

static void prv_window_unload(Window *window) {
  layer_destroy(s_canvas_layer);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
}

// Initialize the default settings
static void prv_default_settings() {
  settings.backgroundColor = GColorPictonBlue;
  settings.backgroundBTColor = GColorRed;
  settings.textColor = GColorBlack;
  settings.batteryColor = GColorChromeYellow;
}


// Read settings from persistent storage
static void prv_load_settings() {
  // Load the default settings
  prv_default_settings();
  // Read settings from persistent storage, if they exist
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}


static void prv_init() {
  srand(time(NULL));

  time_t t = time(NULL);
  struct tm *time_now = localtime(&t);
  prv_tick_handler(time_now, MINUTE_UNIT);

  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  
   // Open AppMessage connection
  app_message_register_inbox_received(prv_inbox_received_handler);
  app_message_open(128, 128);

  prv_load_settings();
  s_background_color = settings.backgroundColor;
  
  window_stack_push(s_main_window, true);

  battery_state_service_subscribe(battery_callback);

  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_callback
  });



  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
}

static void prv_deinit() {
  window_destroy(s_main_window);
}

int main() {
  prv_init();
  app_event_loop();
  prv_deinit();
}
