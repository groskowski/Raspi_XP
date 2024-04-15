import serial
import time

print("This is in fact running...")

# Helper function to convert raw GPS data to DMS format
def to_dms(raw_value, direction):
    # Extract the degrees and the raw minutes
    degrees = int(raw_value) // 100
    raw_minutes = raw_value - (degrees * 100)
    
    # Convert the minutes to seconds (60 seconds = 1 minute)
    minutes = int(raw_minutes)
    seconds = (raw_minutes - minutes) * 60
    
    # Format the output
    dms = f"{degrees}°{minutes}'{seconds:.1f}\"{direction}"
    return dms

# Function to parse GPGGA sentences for GPS information
def parse_gpgga(sentence):
    fields = sentence.split(',')
    try:
        latitude = to_dms(float(fields[2]), fields[3]) if fields[2] else 'N/A'
        longitude = to_dms(float(fields[4]), fields[5]) if fields[4] else 'N/A'
    except ValueError:
        print("Error converting GPS data. Skipping sentence.")
        return None

    gps_data = {
        'time': fields[1],
        'latitude': latitude,
        'longitude': longitude,
        'quality': fields[6],
        'satellites': fields[7],
        'altitude': f"{fields[9]} {fields[10]}",
    }
    return gps_data

# Function to format GPS data for printing
def format_gps_data(gps_data):
    # Extract and format the time
    hours, minutes, seconds = gps_data['time'][:2], gps_data['time'][2:4], gps_data['time'][4:]
    formatted_time = f"{hours}:{minutes}:{seconds}"
    
    # Construct the formatted GPS data string
    return f"Time: {formatted_time}, Latitude: {gps_data['latitude']}, Longitude: {gps_data['longitude']}, Quality: {gps_data['quality']}, Satellites: {gps_data['satellites']}, Altitude: {gps_data['altitude']}"

# Open serial connection default for BE-220 is 38400bps
ser = serial.Serial('/dev/ttyS0', 38400, timeout=1)

# Continuously prints data read
while True:
    line = ser.readline().decode('utf-8').rstrip()
    if line.startswith('$GNGGA'):  # Process only GNGGA sentences
        gps_data = parse_gpgga(line)
        if gps_data is not None:  # Check if gps_data is not None
            formatted_gps_data = format_gps_data(gps_data)
            print(formatted_gps_data)  # Print formatted GPS data
    time.sleep(0.5)  # Adjust the sleep time as needed

# Prints once
#try:
#    found_gpgga = False
#    while not found_gpgga:
#        line = ser.readline().decode('utf-8').rstrip()
#        if line.startswith('$GNGGA'):  # Process only GNGGA sentences
#            gps_data = parse_gpgga(line)
#            formatted_gps_data = format_gps_data(gps_data)
#            print(formatted_gps_data)  # Print formatted GPS data
#            found_gpgga = True  # Mark as found and exit loop
#        time.sleep(0.5)  # Adjust the sleep time as needed
#except KeyboardInterrupt:
#    print("Program terminated!")
#finally:
#    ser.close()  # Ensure the serial connection is closed
