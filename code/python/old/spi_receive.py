from spi_comms import SPIComs
spi_coms = SPIComs()

while True:
    try:
        if spi_coms.read_irq_pin():
            message = spi_coms.read_pkg()
            print("Received message:", message)
            # Process the received message as needed
    except ValueError as e:
        print("Error:", str(e))