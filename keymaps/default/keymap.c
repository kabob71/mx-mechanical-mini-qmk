#include QMK_KEYBOARD_H
#include "i2c_master.h"
#include "wait.h"
#include "gpio.h"

#define XXX KC_NO
#define LED_ADDR (0x30 << 1)
#define NO_LED 255

static uint8_t led_brightness = 0x00;

enum custom_keycodes {
    DESK_LEFT = SAFE_RANGE,
    DESK_RIGHT,
    TASK_VIEW,
    EMOJI,
    SCREENSHOT,
    DICTATION,
    SEARCH,
    LOCK_PC
};

static const uint8_t led_map[MATRIX_ROWS][MATRIX_COLS] = {
    {  0, 16, 31, 46, 60, 74, 12, 41 },
    {  1, 17, 32, 47, 61, 75, 13, 42 },
    {  2, 18, 33, 48, 62, 76, 14, 56 },
    {  3, 19, 34, 49, 63, 77, 15, 57 },
    {  4, 20, 35, 50, 64, 78, 28, 58 },
    {  5, 21, 36, 51, 65, 79, 29, 59 },
    {  6, 22, 37, 52, 66, 80, 30, 70 },
    {  7, 23, 38, 53, 67, 81, 45, 71 },
    {  8, 24, 39, 54, 68, 82, 43, 72 },
    {  9, 25, 40, 55, 69, 83, 44, 73 },
    { 10, 26, NO_LED, NO_LED, NO_LED, NO_LED, NO_LED, NO_LED },
    { 11, 27, NO_LED, NO_LED, NO_LED, NO_LED, NO_LED, NO_LED }
};

static bool brid_held = false;
static bool briu_held = false;
static uint16_t brightness_timer = 0;

static void led_raw2(uint8_t reg, uint8_t val) {
    uint8_t data[2] = {reg, val};
    i2c_transmit(LED_ADDR, data, 2, 100);
}

static void led_page(uint8_t page) {
    led_raw2(0xFE, 0xC5);
    wait_ms(5);
    led_raw2(0xFD, page);
    wait_ms(5);
}

static void led_fill(uint8_t val) {
    uint8_t data[17];

    for (uint8_t start = 0x00; start < 0xC0; start += 0x10) {
        data[0] = start;
        for (uint8_t i = 1; i < 17; i++) {
            data[i] = val;
        }
        i2c_transmit(LED_ADDR, data, 17, 100);
        wait_ms(2);
    }
}

static uint8_t led_to_reg(uint8_t led) {
    return led + ((led / 14) * 16);
}

static void led_set_brightness(uint8_t val) {
    led_brightness = val;
    eeconfig_update_user((eeconfig_read_user() & 0xFFFFFF00) | led_brightness);

    led_page(0x00);
    led_fill(led_brightness);
}

static void led_brightness_up(void) {
    if (led_brightness <= 0xEF) {
        led_set_brightness(led_brightness + 0x10);
    } else {
        led_set_brightness(0xFF);
    }
}

static void led_brightness_down(void) {
    if (led_brightness >= 0x10) {
        led_set_brightness(led_brightness - 0x10);
    } else {
        led_set_brightness(0x00);
    }
}

static void led_set_key(uint8_t row, uint8_t col, uint8_t val) {
    uint8_t led = led_map[row][col];

    if (led == NO_LED) {
        return;
    }

    led_page(0x00);
    led_raw2(led_to_reg(led), val);
}

static void mxmini_led_init(void) {
    i2c_init();
    wait_ms(50);

    gpio_set_pin_output(GP0);
    gpio_write_pin_low(GP0);
    wait_ms(50);
    gpio_write_pin_high(GP0);
    wait_ms(250);

    led_page(0x02);
    led_fill(0xFF);

    led_page(0x00);
    led_fill(led_brightness);

    led_page(0x04);
    led_raw2(0x00, 0x31);
    wait_ms(2);
    led_raw2(0x01, 0x66);
    wait_ms(2);
    led_raw2(0x02, 0x60);
    wait_ms(2);

    led_page(0x00);
    led_fill(led_brightness);
}

void keyboard_post_init_user(void) {
    uint32_t saved = eeconfig_read_user();
    led_brightness = saved & 0xFF;
    mxmini_led_init();
}
static bool key_down[MATRIX_ROWS][MATRIX_COLS] = {0};
static bool ctrl_down = false;
static bool shift_down = false;


static bool led_dirty = false;
static uint16_t led_dirty_timer = 0;

static bool key_led_linger[MATRIX_ROWS][MATRIX_COLS] = {0};
static uint16_t key_led_linger_timer[MATRIX_ROWS][MATRIX_COLS] = {0};

static uint8_t pressed_key_led_value(void) {
    if (led_brightness == 0x00) {
        return 0x80;
    }

    return led_brightness > 0x80 ? 0x00 : 0xFF;
}

static bool is_pos(uint8_t row, uint8_t col, uint8_t r, uint8_t c) {
    return row == r && col == c;
}

static bool shift_pos(uint8_t row, uint8_t col) {
    return
        is_pos(row, col, 0, 4) ||  // Left Shift
        is_pos(row, col, 7, 7);    // Right Shift
}

static bool ctrl_pos(uint8_t row, uint8_t col) {
    return
        is_pos(row, col, 0, 5) ||  // Left Ctrl
        is_pos(row, col, 6, 5);    // Right Ctrl
}

static bool ctrl_combo_pos(uint8_t row, uint8_t col) {
    return
        is_pos(row, col, 3, 2) ||  // C
        is_pos(row, col, 4, 4) ||  // V
        is_pos(row, col, 2, 4) ||  // X
        is_pos(row, col, 1, 4) ||  // Z
        is_pos(row, col, 1, 3) ||  // A
        is_pos(row, col, 4, 3) ||  // F
        is_pos(row, col, 2, 3) ||  // S
        is_pos(row, col, 9, 2) ||  // O
        is_pos(row, col, 6, 4) ||  // N
        is_pos(row, col, 9, 3) ||  // L
        is_pos(row, col, 0, 2) ||  // Tab
        is_pos(row, col, 5, 6) ||  // Backspace
        is_pos(row, col, 3, 6) ||  // Delete
        is_pos(row, col, 0, 0);    // Esc
}

static bool ctrl_shift_combo_pos(uint8_t row, uint8_t col) {
    return
        is_pos(row, col, 0, 0) ||  // Esc, Ctrl+Shift+Esc
        is_pos(row, col, 1, 4) ||  // Z, redo
        is_pos(row, col, 5, 2) ||  // T, reopen tab
        is_pos(row, col, 0, 2);    // Tab, previous tab
}

static bool fn_combo_pos(uint8_t row, uint8_t col) {
    return
        col == 0 ||                // F1-F11 column
        is_pos(row, col, 0, 6) ||  // F12 / mute
        is_pos(row, col, 1, 6) ||  // F13 / vol down
        is_pos(row, col, 2, 6) ||  // F14 / vol up
        is_pos(row, col, 3, 6) ||  // Delete
        is_pos(row, col, 5, 7) ||  // PgUp / Insert
        is_pos(row, col, 6, 6) ||  // Home / Lock
        is_pos(row, col, 7, 6) ||  // End / Menu
        is_pos(row, col, 9, 7);    // PgDn
}

static uint8_t combo_led_brightness(void) {
    return led_brightness < 0x80 ? 0x80 : led_brightness;
}

static void led_refresh_all(void) {
    bool fn_down = key_down[5][5];

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            bool on = true;
            uint8_t val = led_brightness;

            if (fn_down) {
                on = key_down[row][col] || fn_combo_pos(row, col);
                val = combo_led_brightness();
            } else if (ctrl_down && shift_down) {
                on = key_down[row][col] || ctrl_pos(row, col) || shift_pos(row, col) || ctrl_shift_combo_pos(row, col);
                val = combo_led_brightness();
            } else if (ctrl_down) {
                on = key_down[row][col] || ctrl_pos(row, col) || shift_pos(row, col) || ctrl_combo_pos(row, col);
                val = combo_led_brightness();
            } else {
                on = true;
                val = led_brightness;
            }

            led_set_key(row, col, on ? val : 0x00);
        }
    }
}
static void schedule_led_refresh(void) {
    led_dirty = true;
    led_dirty_timer = timer_read();
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    uint8_t row = record->event.key.row;
    uint8_t col = record->event.key.col;

    key_down[row][col] = record->event.pressed;

    // Physical Fn key is row 5, col 5
    if (row == 5 && col == 5) {
        schedule_led_refresh();
        return true;
    }

    switch (keycode) {
        case KC_LCTL:
        case KC_RCTL:
            ctrl_down = record->event.pressed;
            schedule_led_refresh();
            return true;

        case KC_LSFT:
        case KC_RSFT:
            shift_down = record->event.pressed;
            schedule_led_refresh();
            return true;

        case KC_BRID:
            if (record->event.pressed) {
                led_brightness_down();
                brid_held = true;
                brightness_timer = timer_read();
            } else {
                brid_held = false;
            }
            return false;

        case KC_BRIU:
            if (record->event.pressed) {
                led_brightness_up();
                briu_held = true;
                brightness_timer = timer_read();
            } else {
                briu_held = false;
            }
            return false;

        case DESK_LEFT:
            if (record->event.pressed) tap_code16(LGUI(KC_LEFT));
            return false;

        case DESK_RIGHT:
            if (record->event.pressed) tap_code16(LGUI(KC_RIGHT));
            return false;

        case TASK_VIEW:
            if (record->event.pressed) tap_code16(LGUI(KC_TAB));
            return false;

        case EMOJI:
            if (record->event.pressed) tap_code16(C(S(KC_E)));
            return false;

        case SCREENSHOT:
            if (record->event.pressed) tap_code16(S(KC_PSCR));
            return false;

        case DICTATION:
            if (record->event.pressed) tap_code16(LGUI(KC_H));
            return false;

        case SEARCH:
            if (record->event.pressed) tap_code(KC_LGUI);
            return false;

        case LOCK_PC:
            if (record->event.pressed) tap_code16(LGUI(KC_L));
            return false;
    }

    // Normal key LED effect only in normal mode.
    // Press: show effect immediately.
    // Release: keep effect for about half a second.
    if (!key_down[5][5] && !ctrl_down && !shift_down) {
        uint8_t effect_val = pressed_key_led_value();

        if (record->event.pressed) {
            key_led_linger[row][col] = false;
            led_set_key(row, col, effect_val);
        } else {
            key_led_linger[row][col] = true;
            key_led_linger_timer[row][col] = timer_read();
            led_set_key(row, col, effect_val);
        }
    }

    return true;
}


void matrix_scan_user(void) {
    if (led_dirty && timer_elapsed(led_dirty_timer) >= 5) {
        led_dirty = false;
        led_refresh_all();
    }

    if ((brid_held || briu_held) && timer_elapsed(brightness_timer) >= 150) {
        brightness_timer = timer_read();

        if (brid_held) {
            led_brightness_down();
        }

        if (briu_held) {
            led_brightness_up();
        }
    }

    if (!key_down[5][5] && !ctrl_down && !shift_down) {
        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            for (uint8_t col = 0; col < MATRIX_COLS; col++) {
                if (key_led_linger[row][col] && timer_elapsed(key_led_linger_timer[row][col]) >= 200) {
                    key_led_linger[row][col] = false;
                    led_set_key(row, col, led_brightness);
                }
            }
        }
    }
}
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

/* ================= BASE ================= */
[0] = LAYOUT(
    /* r0 */  KC_ESC,  KC_GRV,  KC_TAB,  KC_CAPS, KC_LSFT, KC_LCTL, KC_F12,  KC_P,
    /* r1 */  KC_F1,   KC_1,    KC_Q,    KC_A,    KC_Z,    KC_LGUI, KC_F13,  KC_LBRC,
    /* r2 */  KC_F2,   KC_2,    KC_W,    KC_S,    KC_X,    KC_LALT, KC_F14,  KC_SCLN,
    /* r3 */  KC_F3,   KC_3,    KC_E,    KC_D,    KC_C,    KC_SPC,  KC_DEL,  KC_QUOT,

    /* r4 */  KC_F4,   KC_4,    KC_R,    KC_F,    KC_V,    KC_RALT, KC_EQL,  KC_ENT,
    /* r5 */  KC_F5,   KC_5,    KC_T,    KC_G,    KC_B,    LT(1, KC_SPC), KC_BSPC, KC_PGUP,
    /* r6 */  KC_F6,   KC_6,    KC_Y,    KC_H,    KC_N,    KC_RCTL, KC_HOME, KC_SLSH,

    /* r7 */  KC_F7,   KC_7,    KC_U,    KC_J,    KC_M,    KC_LEFT, KC_END,  KC_RSFT,
    /* r8 */  KC_F8,   KC_8,    KC_I,    KC_K,    KC_COMM, KC_DOWN, KC_RBRC, KC_UP,

    /* r9 */  KC_F9,   KC_9,    KC_O,    KC_L,    KC_DOT,  KC_RGHT, KC_BSLS, KC_PGDN,
    /* r10 */ KC_F10,  KC_0,    XXX,     XXX,     XXX,     XXX,     XXX,     XXX,
    /* r11 */ KC_F11,  KC_MINS, XXX,     XXX,     XXX,     XXX,     XXX,     XXX
),

/* ================= FN ================= */
[1] = LAYOUT(
    /* r0 */  KC_ESC,  KC_GRV,  KC_TAB,  KC_CAPS, KC_LSFT, KC_LCTL, KC_MUTE, KC_P,

    /* r1 */  DESK_LEFT,  KC_1, KC_Q, KC_A, KC_Z, KC_LGUI, KC_VOLD, KC_LBRC,
    /* r2 */  DESK_RIGHT, KC_2, KC_W, KC_S, KC_X, KC_LALT, KC_VOLU, KC_SCLN,
    /* r3 */  TASK_VIEW,  KC_3, KC_E, KC_D, KC_C, KC_SPC,  KC_DEL,  KC_QUOT,

    /* r4 */  KC_BRID, KC_4, KC_R, KC_F, KC_V, KC_RALT, KC_EQL,  KC_ENT,
    /* r5 */  KC_BRIU, KC_5, KC_T, KC_G, KC_B, LT(1, KC_SPC), KC_BSPC, KC_INS,
    /* r6 */  DICTATION, KC_6, KC_Y, KC_H, KC_N, KC_RCTL, LOCK_PC, KC_SLSH,

    /* r7 */  EMOJI,      KC_7, KC_U, KC_J, KC_M,    KC_LEFT, KC_APP,  KC_RSFT,
    /* r8 */  SCREENSHOT, KC_8, KC_I, KC_K, KC_COMM, KC_DOWN, KC_RBRC, KC_UP,

    /* r9 */  KC_MUTE, KC_9, KC_O, KC_L, KC_DOT, KC_RGHT, XXX, KC_PGDN,
    /* r10 */ SEARCH,  KC_0, XXX,  XXX,  XXX,    XXX,     XXX, XXX,
    /* r11 */ KC_MPLY, KC_MINS, XXX, XXX, XXX,   XXX,     XXX, XXX
)

};
