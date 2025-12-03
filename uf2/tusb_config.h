#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif



//--------------------------------------------------------------------
// COMMON CONFIGURATION
//--------------------------------------------------------------------

// defined by board.mk
#ifndef CFG_TUSB_MCU
#define CFG_TUSB_MCU OPT_MCU_RP2040
#endif

#ifndef CFG_TUSB_OS
#define CFG_TUSB_OS OPT_OS_NONE
#endif

// RHPort number used for device
#ifndef BOARD_DEVICE_RHPORT_NUM
#define BOARD_DEVICE_RHPORT_NUM 0
#endif

#ifndef BOARD_DEVICE_RHPORT_SPEED
#define BOARD_DEVICE_RHPORT_SPEED OPT_MODE_FULL_SPEED
#endif

// Device mode with rhport and speed defined by board.mk
#if BOARD_DEVICE_RHPORT_NUM == 0
#define CFG_TUSB_RHPORT0_MODE (OPT_MODE_DEVICE | BOARD_DEVICE_RHPORT_SPEED)
#endif

// Use DMA for RP2040
#ifndef CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_SECTION
#endif

#ifndef CFG_TUSB_MEM_ALIGN
#define CFG_TUSB_MEM_ALIGN __attribute__ ((aligned(4)))
#endif

//--------------------------------------------------------------------
// DEVICE CONFIGURATION
//--------------------------------------------------------------------

#ifndef CFG_TUD_ENDPOINT0_SIZE
#define CFG_TUD_ENDPOINT0_SIZE 64
#endif

//------------- CLASS -------------//
#define CFG_TUD_CDC 0
#define CFG_TUD_MSC 0
#define CFG_TUD_HID 0
#define CFG_TUD_MIDI 0
#define CFG_TUD_VENDOR 0
#define CFG_TUD_AUDIO 1  // Enable Audio Class
// ...existing code...
#define CFG_TUD_AUDIO_FUNC_1_N_AS_INT (1)
#define CFG_TUD_AUDIO_FUNC_1_CTRL_BUF_SZ 64
// ...existing code...

//--------------------------------------------------------------------
// AUDIO CLASS DRIVER CONFIGURATION
//--------------------------------------------------------------------

// Audio format: 16-bit PCM
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_TYPE (AUDIO_FORMAT_TYPE_I)
#define CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX (2)
#define CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_RX (2)

// Number of channels
#define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX (1)
#define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX (1)

// Audio resolution (16-bit)
#define CFG_TUD_AUDIO_FUNC_1_RESOLUTION_TX (16)
#define CFG_TUD_AUDIO_FUNC_1_RESOLUTION_RX (16)

// Sample rate: 16 kHz (Quest 2 compatible)
#define CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE (16000)

// EP and buffer size
#define CFG_TUD_AUDIO_ENABLE_EP_IN 1
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SW_BUF_SZ (512)
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SZ_MAX (512)

// Number of audio functions
#define CFG_TUD_AUDIO_FUNC_1_N_AS_INT (1)

#ifdef __cplusplus
}
#endif

#endif /* _TUSB_CONFIG_H_ */