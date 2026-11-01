#include "synth.h"
#include <string.h>
#include "driver/rmt_encoder.h"
#include "driver/rmt_tx.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "portmacro.h"

#define SIGNAL_RESOLUTION_HZ      100000U
#define SCHEDULER_REFRESH_RATE_HZ 625U
#define SCHEDULER_PERIOD_TICK     (SIGNAL_RESOLUTION_HZ / SCHEDULER_REFRESH_RATE_HZ) // 160

#define CHUNK_BITS                SCHEDULER_PERIOD_TICK
#define UINT32_PER_CHUNK          ((CHUNK_BITS + 31) / 32) // 5
#define MAX_CHANNEL               8

#define FP_SHIFT                  16
#define FP_ONE                    (1UL << FP_SHIFT)

#define RING_BUF_SIZE             3

#define TAG                       "synth"

/* --------------------------------------------------------------------------
 * Type Definitions
 * -------------------------------------------------------------------------- */
typedef struct
{
    uint32_t words[UINT32_PER_CHUNK];
} chunk_t;

typedef struct
{
    uint64_t period_fp;
    uint64_t phase_acc_fp;
    uint8_t pulse_width;
    uint32_t remaining_highs;
} channel_state_t;

typedef struct
{
    float freq_hz;
    int width_us;
    uint8_t channel;
} queue_data_t;

typedef struct
{
    channel_state_t channels[MAX_CHANNEL];
    uint32_t next_chunk_idx;
} gen_t;

struct synth_internal_s
{
    synth_config_t cfg;

    gen_t gen;

    struct
    {
        chunk_t chunk;

        enum
        {
            SLOT_FREE = 0,
            SLOT_TRANSMITTING,
            SLOT_FILLLED,
        } state;
    } chunk_ring_buf[RING_BUF_SIZE];

    uint8_t chunk_ring_buf_idx;

    TaskHandle_t gen_task;
    QueueHandle_t freq_update_queue;
    rmt_channel_handle_t tx_chan;
    rmt_encoder_handle_t encoder;
};

/* --------------------------------------------------------------------------
 * Static Function Declarations
 * -------------------------------------------------------------------------- */
static bool rmt_tx_done_callback(
    rmt_channel_handle_t tx_chan, const rmt_tx_done_event_data_t *edata, void *user_ctx
);
static void gen_task_handler(void *args);
static inline void gen_next_chunk(gen_t *gen, chunk_t *chunk);
static inline void set_channel(gen_t *gen, int chan_id, float freq_hz, int width_us);
static inline void transmit_slot(synth_t *synth, int idx);

/* --------------------------------------------------------------------------
 * Function Definitions
 * -------------------------------------------------------------------------- */
int
synth_create(synth_config_t *config, synth_t **synth_out)
{
    if (config == NULL || synth_out == NULL)
    {
        return -1;
    }

    int ret = 0;
    TaskHandle_t gen_task = NULL;
    QueueHandle_t freq_update_queue = NULL;
    rmt_tx_channel_config_t tx_cfg = {0};
    rmt_channel_handle_t tx_chan = NULL;
    rmt_tx_event_callbacks_t cbs = {0};
    rmt_encoder_handle_t encoder = NULL;
    rmt_bytes_encoder_config_t bytes_encoder_cfg = {0};
    synth_t *synth;

    synth = malloc(sizeof(*synth));
    if (synth == NULL)
    {
        goto cleanup;
        ret = -1;
    }

    freq_update_queue = xQueueCreate(8, sizeof(queue_data_t));
    if (freq_update_queue == NULL)
    {
        goto cleanup;
        ret = -1;
    }

    xTaskCreate(gen_task_handler, "gen_task", 1024 * 8, synth, configMAX_PRIORITIES - 1, &gen_task);
    if (gen_task == NULL)
    {
        goto cleanup;
        ret = -1;
    }

    tx_cfg.clk_src = RMT_CLK_SRC_DEFAULT;
    tx_cfg.gpio_num = config->output_gpio;
    tx_cfg.mem_block_symbols = 160;
    tx_cfg.resolution_hz = 1000000;
    tx_cfg.trans_queue_depth = 3;
    if (rmt_new_tx_channel(&tx_cfg, &tx_chan) != ESP_OK)
    {
        goto cleanup;
        ret = -1;
    }

    // Register the event handler for TX_DONE
    cbs.on_trans_done = rmt_tx_done_callback;
    if (rmt_tx_register_event_callbacks(tx_chan, &cbs, synth) != ESP_OK)
    {
        goto cleanup;
        ret = -1;
    }

    bytes_encoder_cfg.bit0.duration0 = 5;
    bytes_encoder_cfg.bit0.level0 = 0;
    bytes_encoder_cfg.bit0.duration1 = 5;
    bytes_encoder_cfg.bit0.level1 = 0;
    bytes_encoder_cfg.bit1.duration0 = 5;
    bytes_encoder_cfg.bit1.level0 = 1;
    bytes_encoder_cfg.bit1.duration1 = 5;
    bytes_encoder_cfg.bit1.level1 = 1;
    if (rmt_new_bytes_encoder(&bytes_encoder_cfg, &encoder) != ESP_OK)
    {
        goto cleanup;
        ret = -1;
    }

    memset(synth->chunk_ring_buf, 0, sizeof(synth->chunk_ring_buf[0]));
    memset(&synth->gen, 0, sizeof(synth->gen));

    synth->cfg = *config;
    synth->tx_chan = tx_chan;
    synth->gen_task = gen_task;
    synth->encoder = encoder;
    synth->freq_update_queue = freq_update_queue;

    *synth_out = synth;

cleanup:
    if (ret != 0)
    {
        rmt_del_channel(tx_chan);
        vTaskDelete(gen_task);
        vQueueDelete(freq_update_queue);
        free(synth);
    }
    return ret;
}

int
synth_enable(synth_t *synth)
{
    if (rmt_enable(synth->tx_chan) != ESP_OK)
    {
        return -1;
    }

    xQueueReset(synth->freq_update_queue);

    synth->chunk_ring_buf_idx = 0;
    transmit_slot(synth, 0);

    return 0;
}

int
synth_disable(synth_t *synth)
{
    if (rmt_disable(synth->tx_chan) != ESP_OK)
    {
        return -1;
    }

    return 0;
}

int
synth_set_channel(synth_t *synth, int chan_id, float freq_hz, int width_us)
{
    if (chan_id >= MAX_CHANNEL)
    {
        return -1;
    }

    queue_data_t data;

    data.channel = chan_id;
    data.freq_hz = freq_hz;
    data.width_us = width_us;
    if (xQueueSend(synth->freq_update_queue, &data, 0) != pdPASS)
    {
        return -1;
    }

    return 0;
}

// void
// print_chunk(const chunk_t *chunk, uint32_t index)
// {
//     for (int i = 0; i < CHUNK_BITS; i++)
//     {
//         int w = i / 32;
//         int b = i % 32;
//         printf("%i", (int)((chunk->words[w] >> b) & 1));
//     }
//     printf("\n");
// }

/* --------------------------------------------------------------------------
 * Static Function Definitions
 * -------------------------------------------------------------------------- */
static bool
rmt_tx_done_callback(
    rmt_channel_handle_t tx_chan, const rmt_tx_done_event_data_t *edata, void *user_ctx
)
{
    synth_t *synth = user_ctx;
    BaseType_t high_task_wakeup = pdFALSE;

    vTaskNotifyGiveFromISR(synth->gen_task, &high_task_wakeup);

    return (high_task_wakeup == pdTRUE);
}

static inline void
transmit_slot(synth_t *synth, int idx)
{
    for (int i = 0; i < RING_BUF_SIZE; ++i)
    {
        if (synth->chunk_ring_buf[i].state == SLOT_FREE)
        {
            gen_next_chunk(&synth->gen, &synth->chunk_ring_buf[i].chunk);
            synth->chunk_ring_buf[i].state = SLOT_FILLLED;
        }
    }

    rmt_transmit_config_t tx_config = {
        .loop_count = 0,
    };

    if (rmt_transmit(
            synth->tx_chan, synth->encoder, &synth->chunk_ring_buf[synth->chunk_ring_buf_idx].chunk,
            sizeof(chunk_t), &tx_config
        )
        != ESP_OK)
    {}
    synth->chunk_ring_buf[synth->chunk_ring_buf_idx].state = SLOT_TRANSMITTING;
}

static inline void
gen_next_chunk(gen_t *gen, chunk_t *chunk)
{
    memset(chunk, 0, sizeof(chunk_t));

    for (int c = 0; c < MAX_CHANNEL; ++c)
    {
        channel_state_t *ch = &gen->channels[c];

        // Skip inactive channels
        if (ch->period_fp == 0)
        {
            continue;
        }

        uint64_t period = ch->period_fp;
        uint64_t current_phase = ch->phase_acc_fp;

        if (ch->remaining_highs > 0)
        {
            chunk->words[0] |= (1UL << ch->remaining_highs) - 1;

            ch->remaining_highs = 0;
        }

        while (1)
        {
            uint32_t tick_pos = (uint32_t)(current_phase >> FP_SHIFT);

            if (tick_pos >= CHUNK_BITS)
            {
                break;
            }

            // --- Bit Packing Logic (Handles cross-word boundaries safely) ---
            int remaining_width = ch->pulse_width;
            int current_bit_idx = (int)tick_pos;

            while (remaining_width > 0)
            {
                if (current_bit_idx >= CHUNK_BITS)
                {
                    break;
                }

                int word_idx = current_bit_idx / 32;
                int bit_offset = current_bit_idx % 32;

                int bits_available_in_word = 32 - bit_offset;
                int bits_to_write = (remaining_width < bits_available_in_word)
                                        ? remaining_width
                                        : bits_available_in_word;

                uint32_t mask = (bits_to_write == 32) ? UINT32_MAX : ((1u << bits_to_write) - 1u);
                chunk->words[word_idx] |= (mask << bit_offset);

                remaining_width -= bits_to_write;
                current_bit_idx += bits_to_write;
            }

            ch->remaining_highs = remaining_width;
            current_phase += period;
        }

        // Update phase accumulator for next chunk
        ch->phase_acc_fp = current_phase - ((uint64_t)CHUNK_BITS << FP_SHIFT);
    }

    ++gen->next_chunk_idx;
}

static void
gen_task_handler(void *args)
{
    synth_t *synth = args;

    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        queue_data_t data;
        while (xQueueReceive(synth->freq_update_queue, &data, 0) == pdTRUE)
        {
            set_channel(&synth->gen, data.channel, data.freq_hz, data.width_us);
        }

        synth->chunk_ring_buf[synth->chunk_ring_buf_idx].state = SLOT_FREE;
        synth->chunk_ring_buf_idx = (synth->chunk_ring_buf_idx + 1) % RING_BUF_SIZE;

        transmit_slot(synth, synth->chunk_ring_buf_idx);
    }
}

static inline void
set_channel(gen_t *gen, int chan_id, float freq_hz, int width_us)
{
    channel_state_t *ch = &gen->channels[chan_id];

    if (freq_hz <= 0.0f || width_us <= 0)
    {
        ch->period_fp = 0;
        ch->phase_acc_fp = 0;
        ch->remaining_highs = 0;
        return;
    }

    double period_ticks = (double)SIGNAL_RESOLUTION_HZ / freq_hz;
    ch->period_fp = (uint64_t)(period_ticks * (double)FP_ONE);

    int pulse_width = ((uint64_t)width_us * SIGNAL_RESOLUTION_HZ) / 1000000ULL;
    if (pulse_width > 16)
    {
        pulse_width = 16;
    }
    ch->pulse_width = pulse_width;
}
