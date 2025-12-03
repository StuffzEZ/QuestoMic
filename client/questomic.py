import sounddevice as sd
import socket
import numpy as np
import sys

# ==============================
# Configuration
# ==============================
PICO_IP = "192.168.1.50"  # Change to your Pico W's IP address
PICO_PORT = 5005
SAMPLE_RATE = 16000  # Hz - Quest 2 compatible
BLOCK_SIZE = 512     # Frames per packet (adjust for latency/stability)
CHANNELS = 1         # Mono

# ==============================
# List All Audio Devices
# ==============================
def list_devices():
    print("\n" + "="*60)
    print("AVAILABLE AUDIO DEVICES")
    print("="*60)
    devices = sd.query_devices()
    
    for i, dev in enumerate(devices):
        # Determine device type
        if dev['max_input_channels'] > 0:
            dev_type = "INPUT"
        elif dev['max_output_channels'] > 0:
            dev_type = "OUTPUT (can use loopback)"
        else:
            dev_type = "UNKNOWN"
        
        # Print device info
        print(f"\n[{i}] {dev['name']}")
        print(f"    Type: {dev_type}")
        print(f"    Channels: In={dev['max_input_channels']}, Out={dev['max_output_channels']}")
        print(f"    Sample Rate: {dev['default_samplerate']} Hz")
    
    print("\n" + "="*60)

# ==============================
# Setup UDP Socket
# ==============================
print("Initializing UDP socket...")
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
print(f"Target: {PICO_IP}:{PICO_PORT}")

# ==============================
# Audio Statistics
# ==============================
packet_count = 0
error_count = 0

def callback(indata, frames, time_info, status):
    """
    Called by sounddevice when audio data is available
    Converts float32 audio to int16 PCM and sends via UDP
    """
    global packet_count, error_count
    
    if status:
        print(f"Status: {status}", file=sys.stderr)
        error_count += 1
    
    try:
        # Convert float32 [-1.0, 1.0] to int16 [-32768, 32767]
        # Handle both mono and stereo by taking first channel
        if indata.ndim > 1:
            audio_data = indata[:, 0]  # Take first channel
        else:
            audio_data = indata
        
        audio_int16 = np.int16(audio_data * 32767)
        
        # Send to Pico W
        sock.sendto(audio_int16.tobytes(), (PICO_IP, PICO_PORT))
        
        packet_count += 1
        if packet_count % 100 == 0:
            print(f"Packets sent: {packet_count} | Errors: {error_count}", end='\r')
    
    except Exception as e:
        print(f"\nError in callback: {e}", file=sys.stderr)
        error_count += 1

# ==============================
# Main Program
# ==============================
def main():
    global PICO_IP
    
    # List available devices
    list_devices()
    
    # Device selection
    try:
        device_index = int(input("\nEnter device index to capture: "))
    except ValueError:
        print("Invalid input. Exiting.")
        return
    
    # Optional: Update Pico IP
    ip_input = input(f"\nPico W IP address (press Enter for {PICO_IP}): ").strip()
    if ip_input:
        PICO_IP = ip_input
    
    # Determine if we should use loopback (for output devices)
    device_info = sd.query_devices(device_index)
    use_loopback = device_info['max_input_channels'] == 0
    
    print("\n" + "="*60)
    print("STREAMING CONFIGURATION")
    print("="*60)
    print(f"Device: {device_info['name']}")
    print(f"Mode: {'LOOPBACK (output capture)' if use_loopback else 'DIRECT INPUT'}")
    print(f"Sample Rate: {SAMPLE_RATE} Hz")
    print(f"Block Size: {BLOCK_SIZE} frames")
    print(f"Target: {PICO_IP}:{PICO_PORT}")
    print("="*60)
    
    # Configure stream parameters
    stream_params = {
        'samplerate': SAMPLE_RATE,
        'blocksize': BLOCK_SIZE,
        'dtype': 'float32',
        'device': device_index,
        'channels': CHANNELS,
        'callback': callback,
        'latency': 'low',
    }
    
    # Add loopback for output devices (Windows WASAPI)
    if use_loopback:
        try:
            stream_params['extra_settings'] = sd.WasapiSettings(loopback=True)
            print("\n✓ Loopback mode enabled (capturing output)")
        except Exception as e:
            print(f"\n⚠ Warning: Could not enable loopback: {e}")
            print("Loopback capture requires Windows with WASAPI support")
    
    # Start streaming
    try:
        print("\n🎙️  Streaming audio to Pico W...")
        print("Press Ctrl+C to stop.\n")
        
        with sd.InputStream(**stream_params):
            sd.sleep(-1)  # Sleep indefinitely
            
    except KeyboardInterrupt:
        print(f"\n\n✓ Stopped streaming")
        print(f"Total packets sent: {packet_count}")
        print(f"Total errors: {error_count}")
    except Exception as e:
        print(f"\n✗ Error: {e}")
        print("\nTroubleshooting:")
        print("- Check that the device index is valid")
        print("- Verify your audio device is not in use")
        print("- Try a different sample rate (44100 or 48000)")
        print("- Check Pico W is on the same network")

if __name__ == "__main__":
    main()