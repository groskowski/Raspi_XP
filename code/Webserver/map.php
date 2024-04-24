<!DOCTYPE html>
<html>
<head>
    <title>Interactive Map</title>
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.15.3/css/all.min.css" />
    <link rel="stylesheet" href="https://unpkg.com/leaflet@1.9.3/dist/leaflet.css" />
    <link rel="stylesheet" href="css/style.css">
    <link rel="stylesheet" href="fonts/fa/css/font-awesome.min.css">
    <link rel="stylesheet" href="fonts/roboto/roboto.css">
    <script src="https://unpkg.com/leaflet@1.9.3/dist/leaflet.js"></script>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body, html {
            height: 100%;
            margin: 0;
            padding: 0;
        }

        #map {
            height: 100%;
            width: 100%;
        }

        .battery-indicator.low {
            color: #ff4d4d;
        }

        .battery-indicator.medium {
            color: #ffa500;
        }

        .battery-indicator.high {
            color: #4CAF50;
        }
    </style>
</head>
<body>
    <?php include "navbar.php"?>
    <div id="map"></div>

    <?php
    // Database connection details
    $servername = "localhost";
    $username = "root";
    $password = "raspberry";
    $dbname = "ipro497";

    // Create connection
    $conn = new mysqli($servername, $username, $password, $dbname);

    // Check connection
    if ($conn->connect_error) {
        die("Connection failed: " . $conn->connect_error);
    }

    // SQL query to fetch data
    $sql = "SELECT lat, lon, temp, humidity, battery1, battery2, battery3, pressure FROM sensor_data ORDER BY temp DESC";
    $result = $conn->query($sql);

    // Store the data in an array
    $data = array();
    if ($result->num_rows > 0) {
        while($row = $result->fetch_assoc()) {
            $data[] = $row;
        }
    }

    // Close the database connection
    $conn->close();

    // Function to get battery level class based on percentage
    function getBatteryLevelClass($batteryLevel) {
        if ($batteryLevel < 3.2) {
            return 'low';
        } else if ($batteryLevel < 3.4) {
            return 'medium';
        } else {
            return 'high';
        }
    }
    ?>

    <script>
        // Initialize the map
        var map = L.map('map').setView([0, 0], 2);

        // Add a tile layer (e.g., OpenStreetMap)
        L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
            attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
        }).addTo(map);

        // Loop through the data and create markers
        <?php foreach ($data as $row): ?>
            var batteryClass = '<?php echo getBatteryLevelClass(max($row['battery1'], $row['battery2'], $row['battery3'])); ?>';
            var marker = L.marker([<?php echo $row['lat']; ?>, <?php echo $row['lon']; ?>], {
                icon: L.divIcon({
                    className: 'battery-indicator ' + batteryClass,
                    iconAnchor: [8, -5],
                    html: '<i class="fas fa-map-marker-alt fa-2x"></i>'
                })
            })
            .bindPopup('<b>Location:</b> <?php echo $row['lat']; ?>, <?php echo $row['lon']; ?><br>' +
                    '<b>Temperature:</b> <?php echo $row['temp']; ?>°C<br>' +
                    '<b>Humidity:</b> <?php echo $row['humidity']; ?>%<br>' +
                    '<b>Pressure:</b> <?php echo $row['pressure']; ?> hPa<br>' +
                    '<b>Battery Levels:</b> <?php echo $row['battery1']; ?>, <?php echo $row['battery2']; ?>, <?php echo $row['battery3']; ?>');
            marker.on('mouseover', function(e) {
                this.openPopup();
            });
            marker.on('mouseout', function(e) {
                this.closePopup();
            });
            marker.addTo(map);

            // Add random lines between markers
            <?php $randomIndex = array_rand($data, 1); ?>

            if(Math.random() < 0.0){
                var randomLat = <?php echo $data[$randomIndex]['lat']; ?>;
                var randomLon = <?php echo $data[$randomIndex]['lon']; ?>;

                var line = L.polyline([[<?php echo $row['lat']; ?>, <?php echo $row['lon']; ?>], [randomLat, randomLon]], {
                    color: 'red',
                    weight: 2
                }).addTo(map);
            }
        <?php endforeach; ?>
    </script>
</body>
</html>