/*********************************************************************
 *  v4 – Mealy / Moore demo with a fixed LED sequencer
 *  -------------------------------------------------
 *  • Light-click demo (unchanged)
 *  • 011 detector   (unchanged)
 *  • LED sequencer  → now correctly rotates 1000→0100→0010→0001
 *
 *  Switch between Moore and Mealy with the #define below.
 *********************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* ------------------------------------------------------------------
 *  Choose the machine type
 * ------------------------------------------------------------------ */
// #define MEALY          /* <-- comment this line for pure Moore */
#define MOORE

/* ------------------------------------------------------------------
 *  1. Light-click demo (Moore only)
 * ------------------------------------------------------------------ */
typedef enum { OFF = 0, DIM = 1, BRIGHT = 2 } light_state_t;

light_state_t light_next(light_state_t s, int click)
{
    if (!click) return s;                     /* no click → stay */
    return (s == BRIGHT) ? OFF : (light_state_t)(s + 1);
}

int light_output(light_state_t s) { return (int)s; }

/* ------------------------------------------------------------------
 *  2. 011 sequence detector (Moore, overlapping)
 * ------------------------------------------------------------------ */
typedef enum { S0, S1, S2, S3 } det_state_t;

det_state_t det_next(det_state_t s, int bit)
{
    switch (s) {
        case S0: return bit ? S1 : S0;
        case S1: return bit ? S2 : S0;
        case S2: return bit ? S2 : S3;   /* 01 1 → stay in S2, 01 0 → S3 */
        case S3: return bit ? S1 : S0;   /* after "011" restart */
        default: return S0;
    }
}

int det_output(det_state_t s) { return (s == S3) ? 1 : 0; }

/* ------------------------------------------------------------------
 *  3. LED sequencer – Moore (or Mealy with the #define)
 * ------------------------------------------------------------------ */
typedef enum { LED_1000 = 0, LED_0100 = 1, LED_0010 = 2, LED_0001 = 3 } led_state_t;

/* ---- transition table (Moore) ----------------------------------- */
led_state_t led_next_moore(led_state_t s, int dummy)   /* dummy = clock, always 1 */
{
    return (s == LED_0001) ? LED_1000 : (led_state_t)(s + 1);
}

/* ---- transition table (Mealy) ----------------------------------- */
#ifdef MEALY
led_state_t led_next_mealy(led_state_t s, int clk)
{
    return clk ? led_next_moore(s, clk) : s;   /* clk = 0 → stay */
}
#endif

/* ---- output function -------------------------------------------- */
int led_output(led_state_t s)
{
    switch (s) {
        case LED_1000: return 0b1000;
        case LED_0100: return 0b0100;
        case LED_0010: return 0b0010;
        case LED_0001: return 0b0001;
        default:       return 0;
    }
}

void print_led(int val)
{
    printf("LEDs: %d%d%d%d\n",
           !!(val & 0b1000),
           !!(val & 0b0100),
           !!(val & 0b0010),
           !!(val & 0b0001));
}

/* ------------------------------------------------------------------
 *  Helper – format a 4-bit word as a string "1000"
 * ------------------------------------------------------------------ */
void fmt_led(char buf[5], int val)
{
    for (int i = 0; i < 4; ++i)
        buf[3-i] = ((val >> i) & 1) ? '1' : '0';
    buf[4] = '\0';
}

/* ------------------------------------------------------------------
 *  Main demo
 * ------------------------------------------------------------------ */
int main(void)
{
    printf("=== v4 Mealy/Moore Demo (compiled as %s) ===\n",
           #ifdef MEALY
           "MEALY"
           #else
           "MOORE"
           #endif
    );

    /* ---------- Light-click demo ---------- */
    light_state_t light = OFF;
    int clicks[] = {1,1,1,1,1,1};
    for (size_t i = 0; i < sizeof clicks / sizeof clicks[0]; ++i) {
        light = light_next(light, clicks[i]);
        printf("Light click → state=%d out=%d\n",
               (int)light, light_output(light));
    }
    putchar('\n');

    /* ---------- 011 detector ---------- */
    det_state_t det = S0;
    const char *stream = "0110011011";   /* contains three overlapping "011" */
    printf("011 detector input stream: %s ", stream);
    for (size_t i = 0; i < strlen(stream); ++i) {
        int bit = stream[i] - '0';
        det = det_next(det, bit);
        if (det_output(det))
            printf("**FOUND**");
    }
    putchar('\n');
    putchar('\n');

    /* ---------- LED sequencer ---------- */
    printf("LED sequencer (clock ticks):\n");
    led_state_t led = LED_1000;

    for (int tick = 0; tick < 20; ++tick) {
        #ifdef MEALY
        led = led_next_mealy(led, 1);   /* clock = 1 */
        #else
        led = led_next_moore(led, 1);
        #endif

        int out = led_output(led);
        print_led(out);
    }

    return 0;
}
