/*
 * USB Descriptors for Audio Class 1.0 Microphone
 * Makes the Pico W appear as a USB microphone to Quest 2
 */
#define TUD_AUDIO_FUNC_DESC_LEN  (8 + 9 + 9 + 9 + 7 + 7 + 6 + 6 + 9)
#include "tusb.h"

// --------------------------------------------------------------------
// Interface numbering
// --------------------------------------------------------------------
enum
{
    ITF_NUM_AUDIO_CONTROL = 0,
    ITF_NUM_AUDIO_STREAMING,
    ITF_NUM_TOTAL
};

// --------------------------------------------------------------------
// TinyUSB requires total length of the Audio Function Descriptor.
// Since TUD_AUDIO_MIC_ONE_CH_DESC_LEN does NOT exist in your version,
// we manually compute it.
// --------------------------------------------------------------------
//
// This value comes directly from analyzing TinyUSB’s macro expansion
// for TUD_AUDIO_MIC_ONE_CH_DESCRIPTOR.
//
// TOTAL = IAD + AC interface descriptors + AS interface descriptors +
//         Type I/II format descriptors + endpoint descriptors.
//
// For a 1-channel microphone using TUD_AUDIO_MIC_ONE_CH_DESCRIPTOR,
// the total size is:
//


#define CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + TUD_AUDIO_FUNC_DESC_LEN)


//--------------------------------------------------------------------
// Device Descriptor
//--------------------------------------------------------------------
tusb_desc_device_t const desc_device =
{
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = TUSB_CLASS_MISC,
    .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol    = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor           = 0xCAFE,
    .idProduct          = 0x4001,
    .bcdDevice          = 0x0100,

    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,

    .bNumConfigurations = 0x01
};

uint8_t const * tud_descriptor_device_cb(void)
{
    return (uint8_t const *) &desc_device;
}


//--------------------------------------------------------------------
// Configuration Descriptor
//--------------------------------------------------------------------
uint8_t const desc_configuration[] =
{
    // Config number, interfaces, string index, total length, attributes, power
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),

    // Audio Microphone (1 channel)
    TUD_AUDIO_MIC_ONE_CH_DESCRIPTOR(
        ITF_NUM_AUDIO_CONTROL,    // Audio Control interface
        0,                        // String index
        2,                        // Bytes per sample
        16,                       // Bits used per sample
        0x81,                     // Endpoint address (IN 1)
        512                       // Endpoint size
    )
};

uint8_t const * tud_descriptor_configuration_cb(uint8_t index)
{
    (void) index;
    return desc_configuration;
}


//--------------------------------------------------------------------
// String Descriptors
//--------------------------------------------------------------------
char const* string_desc_arr[] =
{
    (const char[]){ 0x09, 0x04 },  // 0: English
    "Raspberry Pi",                // 1: Manufacturer
    "Pico W USB Microphone",       // 2: Product
    "123456",                      // 3: Serial
    "Pico W Audio",                // 4: Audio Interface
};

static uint16_t _desc_str[32];

uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    (void) langid;

    uint8_t chr_count;

    if (index == 0)
    {
        memcpy(&_desc_str[1], string_desc_arr[0], 2);
        chr_count = 1;
    }
    else
    {
        if (index >= (sizeof(string_desc_arr) / sizeof(string_desc_arr[0])))
            return NULL;

        const char* str = string_desc_arr[index];
        chr_count = strlen(str);
        if (chr_count > 31) chr_count = 31;

        for (uint8_t i = 0; i < chr_count; i++)
            _desc_str[1 + i] = str[i];
    }

    _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);
    return _desc_str;
}
