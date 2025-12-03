/*
 * Pico W USB Audio Class Microphone with Wi-Fi Audio Reception
 * 
 * This firmware makes the Pico W appear as a USB microphone to the Quest 2,
 * while receiving audio data over Wi-Fi from a PC.
 * 
 * Requirements:
 * - Pico SDK with TinyUSB
 * - lwIP for network stack
 * - CMake build system
 */

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "tusb.h"
#include "lwip/udp.h"
#include "hardware/clocks.h"

// ==============================
// Configuration
// ==============================
#define WIFI_SSID "ChangeMe" // WIFI SSID
#define WIFI_PASSWORD "ChangeMe" // WIFI PASSWORD
#define UDP_PORT 5005

#define AUDIO_SAMPLE_RATE 16000
#define AUDIO_CHANNELS 1
#define AUDIO_BUFFER_SIZE 512
#define AUDIO_RING_BUFFER_SIZE (AUDIO_BUFFER_SIZE * 8)

// ==============================
// Audio Ring Buffer
// ==============================
typedef struct {
    int16_t buffer[AUDIO_RING_BUFFER_SIZE];
    volatile uint32_t write_pos;
    volatile uint32_t read_pos;
    volatile uint32_t available;
} audio_ring_buffer_t;

static audio_ring_buffer_t audio_buffer = {0};

// Silence buffer for when no data is available
static int16_t silence_buffer[AUDIO_BUFFER_SIZE] = {0};

// ==============================
// Ring Buffer Functions
// ==============================
static inline void ring_buffer_write(int16_t *data, uint32_t length) {
    for (uint32_t i = 0; i < length; i++) {
        audio_buffer.buffer[audio_buffer.write_pos] = data[i];
        audio_buffer.write_pos = (audio_buffer.write_pos + 1) % AUDIO_RING_BUFFER_SIZE;
        
        if (audio_buffer.available < AUDIO_RING_BUFFER_SIZE) {
            audio_buffer.available++;
        }
    }
}

static inline uint32_t ring_buffer_read(int16_t *data, uint32_t length) {
    uint32_t samples_read = 0;
    
    while (samples_read < length && audio_buffer.available > 0) {
        data[samples_read] = audio_buffer.buffer[audio_buffer.read_pos];
        audio_buffer.read_pos = (audio_buffer.read_pos + 1) % AUDIO_RING_BUFFER_SIZE;
        audio_buffer.available--;
        samples_read++;
    }
    
    // Fill remainder with silence if buffer underrun
    while (samples_read < length) {
        data[samples_read++] = 0;
    }
    
    return samples_read;
}

// ==============================
// UDP Audio Reception
// ==============================
static struct udp_pcb *audio_pcb;
static uint32_t packets_received = 0;

void udp_receive_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                          const ip_addr_t *addr, u16_t port) {
    if (p != NULL) {
        // Receive audio data and write to ring buffer
        int16_t *audio_data = (int16_t *)p->payload;
        uint32_t sample_count = p->len / sizeof(int16_t);
        
        ring_buffer_write(audio_data, sample_count);
        
        packets_received++;
        if (packets_received % 100 == 0) {
            printf("Received %lu packets, buffer: %lu/%d\n", 
                   packets_received, audio_buffer.available, AUDIO_RING_BUFFER_SIZE);
        }
        
        pbuf_free(p);
    }
}

bool setup_udp_server(void) {
    audio_pcb = udp_new();
    if (audio_pcb == NULL) {
        printf("Failed to create UDP PCB\n");
        return false;
    }
    
    err_t err = udp_bind(audio_pcb, IP_ADDR_ANY, UDP_PORT);
    if (err != ERR_OK) {
        printf("Failed to bind UDP port %d: %d\n", UDP_PORT, err);
        return false;
    }
    
    udp_recv(audio_pcb, udp_receive_callback, NULL);
    printf("UDP server listening on port %d\n", UDP_PORT);
    
    return true;
}

// ==============================
// TinyUSB Audio Callbacks
// ==============================

// Called when host (Quest 2) requests audio data
bool tud_audio_tx_done_pre_load_cb(uint8_t rhport, uint8_t itf, 
                                    uint8_t ep_in, uint8_t cur_alt_setting) {
    (void) rhport;
    (void) itf;
    (void) ep_in;
    (void) cur_alt_setting;
    
    int16_t audio_data[AUDIO_BUFFER_SIZE];
    
    // Read from ring buffer or use silence
    if (audio_buffer.available >= AUDIO_BUFFER_SIZE) {
        ring_buffer_read(audio_data, AUDIO_BUFFER_SIZE);
    } else {
        // Buffer underrun - send silence
        memcpy(audio_data, silence_buffer, sizeof(silence_buffer));
    }
    
    // Write to USB
    return tud_audio_write(audio_data, AUDIO_BUFFER_SIZE * sizeof(int16_t));
}

// Called when host opens/closes audio stream
bool tud_audio_set_itf_cb(uint8_t rhport, tusb_control_request_t const *p_request) {
    (void) rhport;
    uint8_t const itf = tu_u16_low(tu_le16toh(p_request->wIndex));
    uint8_t const alt = tu_u16_low(tu_le16toh(p_request->wValue));
    
    printf("Audio interface %d set to alt %d\n", itf, alt);
    return true;
}

// ==============================
// USB Descriptors
// ==============================
tusb_desc_device_t const desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = 0xCAFE,
    .idProduct          = 0x4001,
    .bcdDevice          = 0x0100,
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    .bNumConfigurations = 0x01
};

uint8_t const *tud_descriptor_device_cb(void) {
    return (uint8_t const *)&desc_device;
}

// ==============================
// Main Program
// ==============================
int main() {
    stdio_init_all();
    
    printf("\n\n=================================\n");
    printf("Pico W USB Audio Microphone\n");
    printf("=================================\n");
    
    // Initialize LED
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed\n");
        return -1;
    }
    
    // Connect to Wi-Fi
    cyw43_arch_enable_sta_mode();
    printf("Connecting to Wi-Fi: %s\n", WIFI_SSID);
    
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, 
                                            CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("Failed to connect to Wi-Fi\n");
        return -1;
    }
    
    printf("Connected to Wi-Fi\n");
    printf("IP Address: %s\n", ip4addr_ntoa(netif_ip4_addr(netif_list)));
    
    // Setup UDP server
    if (!setup_udp_server()) {
        return -1;
    }
    
    // Initialize USB
    tusb_init();
    printf("USB initialized\n");
    
    printf("\n✓ Ready! Connect to Quest 2 via USB-C\n");
    printf("✓ Start streaming from PC to this IP address\n\n");
    
    // Main loop
    while (true) {
        tud_task(); // TinyUSB device task
        cyw43_arch_poll(); // Wi-Fi stack poll
        
        // Blink LED to show activity
        static absolute_time_t last_blink = 0;
        if (absolute_time_diff_us(last_blink, get_absolute_time()) > 500000) {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 
                               !cyw43_arch_gpio_get(CYW43_WL_GPIO_LED_PIN));
            last_blink = get_absolute_time();
        }
    }
    
    return 0;
}