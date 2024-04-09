import pyaudio
import wave
from datetime import datetime
import os
import time  # Import for optional delay between recordings

def list_devices():
    p = pyaudio.PyAudio()
    print("Available audio devices:")
    for i in range(p.get_device_count()):
        dev = p.get_device_info_by_index(i)
        print(f"{i}: {dev['name']} (Input Channels: {dev['maxInputChannels']})")
    p.terminate()

def record_audio(record_seconds, base_filename="recording", directory="/home/pi/audiorec", device_index=None):
    # Ensure the directory exists
    if not os.path.exists(directory):
        os.makedirs(directory)
    
    # Configuration
    chunk = 1024  # Record in chunks of 1024 samples
    sample_format = pyaudio.paInt16  # 16 bits per sample
    channels = 1  # Mono audio
    fs = 384000  # High-quality sample rate, adjust as needed

    p = pyaudio.PyAudio()  # Create an interface to PortAudio

    if device_index is None:
        print("No device index specified, listing available devices:")
        list_devices()
        return

    print(f'Recording on device index {device_index}')
    
    current_time = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
    filename = f"{directory}/{base_filename}_{current_time}.wav"

    print('Recording', filename)

    try:
        stream = p.open(format=sample_format,
                        channels=channels,
                        rate=fs,
                        frames_per_buffer=chunk,
                        input=True,
                        input_device_index=device_index)

        frames = []  # Initialize array to store frames

        # Store data in chunks for the specified recording length
        for i in range(0, int(fs / chunk * record_seconds)):
            data = stream.read(chunk)
            frames.append(data)

        # Stop and close the stream
        stream.stop_stream()
        stream.close()

        print('Finished recording')

        # Save the recorded data as a WAV file
        wf = wave.open(filename, 'wb')
        wf.setnchannels(channels)
        wf.setsampwidth(p.get_sample_size(sample_format))
        wf.setframerate(fs)
        wf.writeframes(b''.join(frames))
        wf.close()
    except Exception as e:
        print(f"An error occurred: {e}")
    finally:
        p.terminate()

def continuous_recording():
    # Continuous recording setup
    record_seconds = 3  # Adjust the length of each recording segment
    base_filename = 'output'  # Base name for your files
    directory = "/home/pi/audiorec"  # Directory to save recordings
    device_index = 0  # Set to your microphone device index or None to list devices

    while True:  # Loop to continuously record audio
        record_audio(record_seconds, base_filename, directory, device_index)
        # Optional: Add a delay between recordings if needed
        # time.sleep(1)  # Sleep for 1 second before the next recording

if __name__ == "__main__":
    continuous_recording()