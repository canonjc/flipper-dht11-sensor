// dht11_sensor.c
// Version 0.1 - Placeholder for next update

#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <gui/elements.h>
#include <gui/view_port.h>
#include <input/input.h>
#include <stm32wb55xx.h>
#include <stdlib.h>
#include <string.h>
#include <storage/storage.h>

#define SETTINGS_PATH "/ext/dht11_sensor.cfg"

// Global variables to control app state
// 'volatile' ensures this flag is not optimized away by the compiler
// as it can be modified asynchronously, e.g., by interrupt or input event
volatile bool app_running = true;

float temperature = 0.0f;
float humidity = 0.0f;

// Configuration options and flags
uint8_t selected_pin_index = 0;
bool use_fahrenheit = false;
uint8_t read_interval = 0;
uint8_t selected_menu_item = 0;
bool in_menu = true;
bool in_config = false;
bool in_about = false;

// GPIO pin options and display strings
const GpioPin* pin_options[] = { &gpio_ext_pb3, &gpio_ext_pb2, &gpio_ext_pc3 };
const char* pin_names[] = { "PB3 (Pin 5)", "PB2 (Pin 6)", "PC3 (Pin 7)" };
const char* freq_names[] = { "1 sec", "1 min", "5 min" };
const GpioPin* dht11_pin = &gpio_ext_pb3;

// Initialize DWT (Data Watchpoint and Trace unit) for microsecond timing
// The DWT is a peripheral in ARM Cortex-M cores used for precise cycle counting
// Here, it's enabled and its cycle counter is reset and started
void dwt_init(void) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

// Get current time in microseconds
uint32_t dwt_get_us(void) { return DWT->CYCCNT / (SystemCoreClock / 1000000); }

// Microsecond delay using DWT
void delay_us(uint32_t us) { uint32_t start = dwt_get_us(); while((dwt_get_us() - start) < us); }

// Send start signal to DHT11 sensor
static void dht11_send_start_signal(void) {
    furi_hal_gpio_init_simple(dht11_pin, GpioModeOutputOpenDrain);
    furi_hal_gpio_write(dht11_pin, false);
    furi_delay_ms(18);
    furi_hal_gpio_write(dht11_pin, true);
    delay_us(30);
}

// Wait for DHT11 response signal
static bool dht11_wait_response(void) {
    furi_hal_gpio_init_ex(dht11_pin, GpioModeInput, GpioPullUp, GpioSpeedLow, 0);
    delay_us(10);

    uint32_t timeout = dwt_get_us();
    while(furi_hal_gpio_read(dht11_pin)) if((dwt_get_us() - timeout) > 100) return false;
    timeout = dwt_get_us();
    while(!furi_hal_gpio_read(dht11_pin)) if((dwt_get_us() - timeout) > 100) return false;
    timeout = dwt_get_us();
    while(furi_hal_gpio_read(dht11_pin)) if((dwt_get_us() - timeout) > 100) return false;

    return true;
}

// Read 40 bits of data from the DHT11 sensor
static bool dht11_read_data(uint8_t data[5]) {
    memset(data, 0, 5);
    uint32_t timeout;
    for(int i = 0; i < 40; i++) {
        timeout = dwt_get_us();
        while(!furi_hal_gpio_read(dht11_pin)) if((dwt_get_us() - timeout) > 70) return false;
        uint32_t bit_start = dwt_get_us();
        timeout = bit_start;
        while(furi_hal_gpio_read(dht11_pin)) if((dwt_get_us() - timeout) > 120) break;
        uint32_t pulse_length = dwt_get_us() - bit_start;
        if(pulse_length > 50) data[i/8] |= (1 << (7 - (i%8)));
    }
    return true;
}

// Validate checksum of the received data
static bool dht11_validate_checksum(uint8_t data[5]) {
    return data[4] == ((data[0]+data[1]+data[2]+data[3]) & 0xFF);
}

// Read temperature and humidity from DHT11
bool dht11_read(float* temperature, float* humidity) {
    uint8_t data[5];

    dht11_send_start_signal();

    if(!dht11_wait_response()) return false;

    if(!dht11_read_data(data)) return false;

    furi_hal_gpio_init_simple(dht11_pin, GpioModeOutputOpenDrain);
    furi_hal_gpio_write(dht11_pin, true);

    if(!dht11_validate_checksum(data)) return false;

    *humidity = (float)data[0];
    *temperature = (float)data[2];
    return true;
}

// Save configuration settings to file
void save_settings(void) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);
    if(storage_file_open(file, SETTINGS_PATH, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        char line[64];
        snprintf(line, sizeof(line), "%d %d %d", selected_pin_index, use_fahrenheit, read_interval);
        storage_file_write(file, line, strlen(line));
        storage_file_close(file);
    }
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
}

// Load configuration settings from file
void load_settings(void) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);
    if(storage_file_open(file, SETTINGS_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
        char line[64] = {0};
        storage_file_read(file, line, sizeof(line) - 1);
        sscanf(line, "%hhu %hhu %hhu", &selected_pin_index, (uint8_t*)&use_fahrenheit, &read_interval);
        if(selected_pin_index > 2) selected_pin_index = 0;
        if(read_interval > 2) read_interval = 0;
        dht11_pin = pin_options[selected_pin_index];
        storage_file_close(file);
    }
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
}

// Handle input when in the main menu
static void handle_menu_input(InputKey key) {
    if(key == InputKeyUp) selected_menu_item = (selected_menu_item + 3) % 4;
    if(key == InputKeyDown) selected_menu_item = (selected_menu_item + 1) % 4;
    if(key == InputKeyOk) {
        if(selected_menu_item == 0) in_menu = false;
        else if(selected_menu_item == 1) { in_menu = false; in_config = true; selected_menu_item = 0; }
        else if(selected_menu_item == 2) { in_menu = false; in_about = true; }
        else if(selected_menu_item == 3) app_running = false;
    }
}

// Handle input when in configuration screen
static void handle_config_input(InputKey key) {
    if(key == InputKeyBack) { in_config = false; in_menu = true; save_settings(); }
    else if(key == InputKeyUp) selected_menu_item = (selected_menu_item + 2) % 3;
    else if(key == InputKeyDown) selected_menu_item = (selected_menu_item + 1) % 3;
    else if(key == InputKeyRight || key == InputKeyLeft) {
        if(selected_menu_item == 0)
            selected_pin_index = (selected_pin_index + 1) % 3;
        else if(selected_menu_item == 1)
            use_fahrenheit = !use_fahrenheit;
        else if(selected_menu_item == 2)
            read_interval = (read_interval + 1) % 3;
        dht11_pin = pin_options[selected_pin_index];
    }
}

// Handle input when in about screen or reading screen
static void handle_simple_back(InputKey key) {
    if(key == InputKeyBack) {
        if(in_about) { in_about = false; in_menu = true; }
        else in_menu = true;
    }
}

// Unified input handler
void input_callback(InputEvent* event, void* context) {
    UNUSED(context);
    if(event->type == InputTypeShort) {
        if(in_menu) handle_menu_input(event->key);
        else if(in_config) handle_config_input(event->key);
        else handle_simple_back(event->key);
    }
}

// Render current screen on the canvas
void render_gui(Canvas* canvas, void* context) {
    UNUSED(context);
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    if(in_menu) {
        canvas_draw_str(canvas, 24, 10, "DHT11 Sensor");
        const char* menu[] = {"Acquire Data", "Configuration", "About", "Exit"};
        for(int i = 0; i < 4; i++) {
            if(i == selected_menu_item) canvas_draw_box(canvas, 2, 14 + i * 12, 120, 12);
            canvas_set_color(canvas, i == selected_menu_item ? ColorWhite : ColorBlack);
            canvas_draw_str(canvas, 6, 24 + i * 12, menu[i]);
            canvas_set_color(canvas, ColorBlack);
        }
    } else if(in_config) {
        canvas_draw_str(canvas, 24, 12, "Configuration");
        char line[32];
        for(int i = 0; i < 3; i++) {
            if(i == selected_menu_item) {
                canvas_set_color(canvas, ColorBlack);
                canvas_draw_box(canvas, 2, 18 + i * 14, 120, 14);
                canvas_set_color(canvas, ColorWhite);
            } else {
                canvas_set_color(canvas, ColorBlack);
            }
            if(i == 0) snprintf(line, sizeof(line), "Pin: %s", pin_names[selected_pin_index]);
            else if(i == 1) snprintf(line, sizeof(line), "Units: %s", use_fahrenheit ? "F" : "C");
            else snprintf(line, sizeof(line), "Interval: %s", freq_names[read_interval]);
            canvas_draw_str(canvas, 6, 28 + i * 14, line);
        }
    } else if(in_about) {
        canvas_draw_str(canvas, 0, 14, ".DHT11 Sensor v0.1");
        canvas_draw_str(canvas, 0, 28, ".Designed by Javier Canon");
        canvas_draw_str(canvas, 0, 42, ".https://github.com/");
        canvas_draw_str(canvas, 0, 56, "canonjc/flipper-dht11");
    } else {
        canvas_draw_str(canvas, 32, 12, "R e a d i n g . . .");
        char line[32];
        float display_temp = use_fahrenheit ? (temperature * 9.0 / 5.0 + 32.0) : temperature;
        snprintf(line, sizeof(line), "Temp: %.1f %c", (double)display_temp, use_fahrenheit ? 'F' : 'C');
        canvas_draw_str(canvas, 6, 28, line);
        snprintf(line, sizeof(line), "Humidity: %.1f%%", (double)humidity);
        canvas_draw_str(canvas, 6, 42, line);
    }
}  // unchanged

// Main application loop
int32_t dht11_sensor_app(void* p) {
    UNUSED(p);

    // Setup: initialize microsecond timing and load saved settings
    dwt_init();
    load_settings();

    // GUI setup
    Gui* gui = furi_record_open(RECORD_GUI);
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, render_gui, NULL);
    view_port_input_callback_set(view_port, input_callback, NULL);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    // Timing setup for periodic reading
    uint32_t last_read_time = furi_get_tick();
    uint32_t interval_ms[] = {1000, 60000, 300000};

    // Main loop: handles sensor reading and GUI update
    while(app_running) {
        if(!in_menu && !in_config && !in_about) {
            if(furi_get_tick() - last_read_time >= interval_ms[read_interval]) {
                dht11_read(&temperature, &humidity);
                last_read_time = furi_get_tick();
            }
        }
        view_port_update(view_port);
        furi_delay_ms(100);
    }

    // Cleanup: remove GUI view and free resources
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close(RECORD_GUI);

    return 0;
}  // unchanged
