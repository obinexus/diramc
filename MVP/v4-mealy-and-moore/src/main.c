/*====================================================================*/
/*  v4-mealy-and-moore/src/main.c                                    */
/*  ---------------------------------------------------------------  */
/*  • Mealy  – output on transition arrow                           */
/*  • Moore  – output inside state bubble                           */
/*  • One binary, switch with -DMEALY or -DMOORE                     */
/*  • Functor-style transform for the direction-bit toggle          */
/*  • ODTS trace hook for formal verification                       */
/*====================================================================*/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

/* ------------------------------------------------------------------ */
/*  ODTS – tiny stub that mimics the trace ODTS would emit            */
/* ------------------------------------------------------------------ */
#ifdef ENABLE_ODTS
#define ODTS_TRACE(fmt, ...)  printf("[ODTS] " fmt "\n", ##__VA_ARGS__)
#else
#define ODTS_TRACE(fmt, ...)  ((void)0)
#endif

/* ------------------------------------------------------------------ */
/*  Functor Framework – direction-bit toggle (lossy part)             */
/* ------------------------------------------------------------------ */
typedef struct { bool dir; } DirBit;

static inline DirBit functor_toggle(DirBit d)
{
    ODTS_TRACE("Functor.toggle: %d → %d", d.dir, !d.dir);
    return (DirBit){ .dir = !d.dir };
}

/* ------------------------------------------------------------------ */
/*  Common helpers                                                    */
/* ------------------------------------------------------------------ */
static void print_leds(uint8_t leds)
{
    printf("LEDs: %c%c%c%c\n",
           (leds & 1) ? '1' : '0',
           (leds & 2) ? '1' : '0',
           (leds & 4) ? '1' : '0',
           (leds & 8) ? '1' : '0');
}

/* ------------------------------------------------------------------ */
/*  1. Light-dimmer (4 states) – Mealy vs Moore                       */
/* ------------------------------------------------------------------ */
typedef enum { OFF, DIM, MED, BRIGHT } LightState;

#ifdef MEALY
/* Mealy – output lives on the edge */
static LightState light_mealy(LightState s, bool click, uint8_t *out)
{
    *out = 0;
    switch (s) {
        case OFF:   *out = 0; return click ? DIM   : OFF;
        case DIM:   *out = 1; return click ? MED   : DIM;
        case MED:   *out = 2; return click ? BRIGHT: MED;
        case BRIGHT:*out = 3; return click ? OFF   : BRIGHT;
    }
    return OFF; /* unreachable */
}
#else
/* Moore – output lives inside the state */
static LightState light_moore(LightState s, bool click)
{
    switch (s) {
        case OFF:   return click ? DIM   : OFF;
        case DIM:   return click ? MED   : DIM;
        case MED:   return click ? BRIGHT: MED;
        case BRIGHT:return click ? OFF   : BRIGHT;
    }
    return OFF;
}
static uint8_t light_moore_output(LightState s)
{
    return (uint8_t)s;   /* 0-OFF,1-DIM,2-MED,3-BRIGHT */
}
#endif

/* ------------------------------------------------------------------ */
/*  2. 011-detector (Mealy needs 3 states, Moore needs 4)             */
/* ------------------------------------------------------------------ */
typedef enum { S0, S01, S011, S011_DONE } SeqState;

#ifdef MEALY
static SeqState seq_mealy(SeqState s, bool bit, bool *found)
{
    *found = false;
    switch (s) {
        case S0:      return bit ? S0   : S01;                /* 0 → S01 */
        case S01:     return bit ? S011 : S01;                /* 1 → S011, 0 → S01 */
        case S011:    *found = bit; return bit ? S0 : S01;    /* 1 → FOUND, 0 → S01 */
        case S011_DONE: return S0;
    }
    return S0;
}
#else
static SeqState seq_moore(SeqState s, bool bit)
{
    switch (s) {
        case S0:      return bit ? S0   : S01;
        case S01:     return bit ? S011 : S01;
        case S011:    return bit ? S011_DONE : S01;
        case S011_DONE:return S0;
    }
    return S0;
}
static bool seq_moore_found(SeqState s) { return s == S011_DONE; }
#endif

/* ------------------------------------------------------------------ */
/*  3. LED sequencer 1-2-3-4-3-2-1 (toggle direction)                 */
/* ------------------------------------------------------------------ */
typedef enum {
    L0, L1, L2, L3, L4
} LedSeqState;

typedef struct {
    LedSeqState  state;
    DirBit       dir;
} LedSeq;

static LedSeq ledseq_step(LedSeq ls, bool clk)
{
    ODTS_TRACE("LEDSEQ step: state=%d dir=%d", ls.state, ls.dir.dir);

    if (ls.state == L0 && ls.dir.dir) {
        /* error – wrong direction at start */
        ls.dir = functor_toggle(ls.dir);
        return ls;
    }

    if (ls.state == L4 && !ls.dir.dir) {
        /* end of forward run */
        ls.dir = functor_toggle(ls.dir);
        return ls;
    }

    if (ls.state == L0 && ls.dir.dir == false) {
        /* end of reverse run */
        ls.dir = functor_toggle(ls.dir);
        return ls;
    }

    /* normal movement */
    if (ls.dir.dir == false) {               /* forward */
        if (ls.state < L4) ls.state++;
    } else {                                 /* reverse */
        if (ls.state > L0) ls.state--;
    }
    return ls;
}

static uint8_t ledseq_output(LedSeq ls)
{
    return 1 << ls.state;      /* LED 0-3 */
}

/* ------------------------------------------------------------------ */
/*  Demo driver                                                       */
/* ------------------------------------------------------------------ */
int main(void)
{
    printf("=== v4 Mealy/Moore Demo (compiled as %s) ===\n",
           #ifdef MEALY
           "MEALY"
           #else
           "MOORE"
           #endif
          );

    /* ---- Light dimmer demo ---- */
    LightState ls = OFF;
    bool click = true;
    for (int i = 0; i < 6; ++i) {
        #ifdef MEALY
        uint8_t out;
        ls = light_mealy(ls, click, &out);
        printf("Light click → state=%d out=%u\n", ls, out);
        #else
        ls = light_moore(ls, click);
        printf("Light click → state=%d out=%u\n", ls, light_moore_output(ls));
        #endif
        click = !click;   /* alternate */
    }

    /* ---- 011 detector demo ---- */
    SeqState ss = S0;
    bool found;
    const bool stream[] = {0,1,1,0,0,1,1,1,0,1,1};
    printf("\n011 detector input stream: ");
    for (size_t i = 0; i < sizeof(stream)/sizeof(stream[0]); ++i) {
        printf("%d", stream[i]);
        #ifdef MEALY
        ss = seq_mealy(ss, stream[i], &found);
        #else
        ss = seq_moore(ss, stream[i]);
        found = seq_moore_found(ss);
        #endif
        if (found) printf(" **FOUND**");
    }
    printf("\n");

    /* ---- LED sequencer demo ---- */
    LedSeq leds = { .state = L0, .dir = { .dir = false } };
    printf("\nLED sequencer (clock ticks):\n");
    for (int t = 0; t < 20; ++t) {
        leds = ledseq_step(leds, true);
        print_leds(ledseq_output(leds));
    }

    /* ODTS verification hook – in a real build this would call the
       formal prover; here we just emit the trace. */
    #ifdef ENABLE_ODTS
    printf("\n[ODTS] Verification trace emitted – feed to ODTS prover.\n");
    #endif

    return 0;
}

/*====================================================================*/
/*  Build instructions                                                */
/*====================================================================*/
/*
   gcc -std=c11 -Wall -Wextra -O2 -DMEALY   -DENABLE_ODTS -o demo_mealy   main.c
   gcc -std=c11 -Wall -Wextra -O2 -DMOORE   -DENABLE_ODTS -o demo_moore   main.c
   gcc -std=c11 -Wall -Wextra -O2 -DMEALY                  -o demo_mealy_no_odts main.c
   gcc -std=c11 -Wall -Wextra -O2 -DMOORE                  -o demo_moore_no_odts main.c
*/
