/**
 * \file midi_synth.c
 * \brief 
 *
 * \author Stanley Arnaud
 * \date 2026-10-31
 */

/* --------------------------------------------------------------------------
 * Includes
 * -------------------------------------------------------------------------- */
#include "midi_synth.h"
#include "driver/synth.h"
#include "service/gui.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>

/* --------------------------------------------------------------------------
 * Macros and Constants
 * -------------------------------------------------------------------------- */
#define SYNTH_VOICES SYNTH_MAX_CHANNEL

/* --------------------------------------------------------------------------
 * Type Definitions
 * -------------------------------------------------------------------------- */
typedef struct
{
    bool active;
    uint8_t midi_channel;
    uint8_t note;
} synth_voice_t;

/* --------------------------------------------------------------------------
 * Static Variables
 * -------------------------------------------------------------------------- */
static synth_voice_t voices[SYNTH_VOICES] = {0};

/* --------------------------------------------------------------------------
 * Static Function Declarations
 * -------------------------------------------------------------------------- */
static int alloc_voice(uint8_t midi_channel, uint8_t note);
static int find_voice(uint8_t midi_channel, uint8_t note);
static void free_voice(int voice);

/* --------------------------------------------------------------------------
 * Function Definitions
 * -------------------------------------------------------------------------- */
static void
midi_note_to_string(uint8_t note, char *buf, size_t size)
{
    static const char *names[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

    unsigned pitch = note % 12;
    int octave = note / 12 - 1;

    snprintf(buf, size, "%s%d", names[pitch], octave);
}

void
midi_interpret_and_play_message(synth_t *synth, const midi_message_t *message, unsigned max_width)
{
    uint8_t command = message->status & 0xF0;
    uint8_t channel = message->status & 0x0F;
    uint8_t note = message->data1;
    uint8_t velocity = message->data2;

    bool note_on = (command == 0x90 && velocity > 0);

    bool note_off = (command == 0x80) || (command == 0x90 && velocity == 0);

    char *label_buf;
    size_t label_buf_sz;

    gui_get_home_label_buf(&label_buf, &label_buf_sz);

    if (note_on)
    {
        int voice = alloc_voice(channel, note);

        if (voice >= 0)
        {
            float freq = 440.0f * powf(2.0f, ((float)note - 69.0f) / 12.0f);

            int width_us = ((float)velocity / 127.0f) * (max_width - 0) + 0;

            synth_set_channel(synth, voice, freq, width_us);
        }

        /* Write note on the home label */
        midi_note_to_string(note, label_buf, label_buf_sz);
        gui_send_command(&(gui_cmd_t){.code = GUI_CMD_UPDATE_HOME_LABEL});
    }
    else if (note_off)
    {
        int voice = find_voice(channel, note);

        if (voice >= 0)
        {
            synth_set_channel(synth, voice, 0, 0);

            free_voice(voice);
        }

        /* Clear home label */
        label_buf[0] = '\0';
        gui_send_command(&(gui_cmd_t){.code = GUI_CMD_UPDATE_HOME_LABEL});
    }
}

/* Find a free synth voice */
static int
alloc_voice(uint8_t midi_channel, uint8_t note)
{
    for (int i = 0; i < SYNTH_VOICES; i++)
    {
        if (!voices[i].active)
        {
            voices[i].active = true;
            voices[i].midi_channel = midi_channel;
            voices[i].note = note;
            return i;
        }
    }

    /* No voice available */
    return -1;
}

/* Find which synth voice is playing this MIDI note */
static int
find_voice(uint8_t midi_channel, uint8_t note)
{
    for (int i = 0; i < SYNTH_VOICES; i++)
    {
        if (voices[i].active && voices[i].midi_channel == midi_channel && voices[i].note == note)
        {
            return i;
        }
    }

    return -1;
}

static void
free_voice(int voice)
{ voices[voice].active = false; }

/* --------------------------------------------------------------------------
 * Static Function Definitions
 * -------------------------------------------------------------------------- */
