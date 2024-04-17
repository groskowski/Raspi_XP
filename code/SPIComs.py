import spidev
import random
import string
import RPi.GPIO as GPIO

class SPIComs:
    def __init__(self, spi_port=0, spi_device=1, irq_pin=40):
        # Setup SPI
        self.spi = spidev.SpiDev(spi_port, spi_device)
        self.spi.max_speed_hz = 400000
        
        # Setup GPIO
        self.irq_pin = irq_pin
        GPIO.setmode(GPIO.BOARD)
        GPIO.setup(self.irq_pin, GPIO.IN)

    def read_irq_pin(self):
        return GPIO.input(self.irq_pin)

    def crc16_ccitt(self, data, offset=0):
        crc = 0xFFFF  # Initial value
        for i in range(offset, len(data)):
            crc ^= data[i] << 8  # Mix the input byte into the CRC
            for _ in range(8):  # Process each bit
                if crc & 0x8000:  # If the high bit is set
                    crc = (crc << 1) ^ 0x1021  # Polynomial 0x1021
                else:
                    crc <<= 1
                crc &= 0xFFFF  # Keep CRC within 16 bits
        return crc

    def get_pkg(self, d_type, data):
        start_byte = "$".encode('utf-8')
        type_byte = bytes([d_type])
        length_byte = bytes([len(data)])
        bytes_to_send = start_byte + type_byte + length_byte + data
        crc = self.crc16_ccitt(bytes_to_send, 1)
        bytes_to_send += bytes([crc >> 8, crc & 0xFF])
        print(bytes_to_send)
        return bytes_to_send

    def send_pkg(self, bytes_to_send):
        return self.spi.xfer(bytes_to_send)

    def random_string(self, length):
        letters = string.ascii_letters
        return ''.join(random.choice(letters) for _ in range(length))
        
    def get_ranS_pkg(self, length):
        data = self.random_string(length).encode('utf-8')
        return self.get_pkg(1, data)

    def close(self):
        self.spi.close()

# Example usage
if __name__ == "__main__":
    coms = SPIComs()
    try:
        len_data = 2
        pkg = coms.get_ranS_pkg(len_data)
        print(coms.send_pkg(pkg))
    finally:
        coms.close()
