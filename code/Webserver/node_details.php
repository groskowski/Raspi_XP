<?php
include 'php/dbh.php'; // Include your database connection file

// Get the node ID from the query parameter
$nodeId = isset($_GET['node']) ? $_GET['node'] : null;

if ($nodeId !== null) {
    // Fetch the sensor data for the selected node
    $sql = "SELECT * FROM sensor_data WHERE pid = $nodeId ORDER BY insertion_time DESC LIMIT 1";
    $result = $conn->query($sql);

    if ($result->num_rows > 0) {
        $row = $result->fetch_assoc();
        $pid = $row['pid'];
        $insertion_time = $row['insertion_time'];
        $created_at = $row['created_at'];
        $lat = $row['lat'];
        $lon = $row['lon'];
        $sat_quality = $row['sat_quality'];
        $sats = $row['sats'];
        $altitude = $row['altitude'];
        $temp = $row['temp'];
        $humidity = $row['humidity'];
        $battery1 = $row['battery1'];
        $battery2 = $row['battery2'];
        $battery3 = $row['battery3'];
        $pressure = $row['pressure'];
    } else {
        echo 'No data found for the selected node.';
    }

    // Fetch the node type for the selected node
    $sql_node_type = "SELECT type FROM node_table WHERE pid = $pid";
    $result_node_type = $conn->query($sql_node_type);
    $node_type = '';
    if ($result_node_type->num_rows > 0) {
        $row_node_type = $result_node_type->fetch_assoc();
        $node_type = $row_node_type['type'];
    }
}
?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Node Details</title>
    <link rel="stylesheet" href="css/node_details.css">
    <link rel="stylesheet" href="fonts/fa/css/font-awesome.min.css">
    <link rel="stylesheet" href="fonts/roboto/roboto.css">
</head>
<body>
    <header>
        <nav class="navigation">
            <ul class="navbuttons">
                <li><a href="index.php" class="navbar-buttons">Dashboard</a></li>
                <li><a href="map.php" class="navbar-buttons">Map</a></li>
                <li><button id="refreshButton" class="refresh-button">Collect Data</button></li>
            </ul>
        </nav>
    </header>

    <div class="main-content" <?php if ($node_type !== 'special'): ?>style="grid-template-columns: 40% 0 30%;"<?php endif; ?>>
        <?php if ($nodeId !== null && isset($pid)): ?>
            <div class="node-details-container">
                <h2>
                    Node <?php echo $pid; ?> Details 
                    <span class="last-update">  (Last Updated: <?php echo $insertion_time; ?>)</span>
                </h2>
                <br/>
                <div class="node-details">
                    <div class="node-detail-item">
                        <h3>Timestamps</h3>
                        <p><strong>Insertion Time:</strong> <?php echo $insertion_time; ?></p>
                        <p><strong>Created At:</strong> <?php echo $created_at; ?></p>
                    </div>
                    <div class="node-detail-item">
                        <h3>Location</h3>
                        <p><strong>Latitude:</strong> <?php echo $lat; ?></p>
                        <p><strong>Longitude:</strong> <?php echo $lon; ?></p>
                        <p><strong>Altitude:</strong> <?php echo $altitude; ?></p>
                    </div>
                    <div class="node-detail-item">
                        <h3>Satellite Info</h3>
                        <p><strong>Satellite Quality:</strong> <?php echo $sat_quality; ?></p>
                        <p><strong>Satellites:</strong> <?php echo $sats; ?></p>
                    </div>
                    <div class="node-detail-item">
                        <h3>Environmental Data</h3>
                        <p><strong>Temperature:</strong> <?php echo $temp; ?>°C</p>
                        <p><strong>Humidity:</strong> <?php echo $humidity; ?>%</p>
                        <p><strong>Pressure:</strong> <?php echo $pressure; ?> atm</p>
                    </div>
                    <div class="node-detail-item">
                        <h3>Battery Levels</h3>
                        <p><strong>Battery 1:</strong> <?php echo $battery1; ?> V</p>
                        <p><strong>Battery 2:</strong> <?php echo $battery2; ?> V</p>
                        <p><strong>Battery 3:</strong> <?php echo $battery3; ?> V</p>
                    </div>
                </div>
            </div>

            <?php if ($node_type === 'special'): ?>
                <div class="image-panel">
                    <div class="image-container">
                        <?php
                        $image_available = true;

                        if ($image_available) {
                            echo '<img src="img/node_' . $pid . '.jpg" alt="Node Image">';
                        } else {
                            echo 'No image available for this node.';
                        }
                        ?>  
                        <br/>
                        <button class="refresh-button" onclick="sendCommand('TEST_PHOTO')">Request Image</button>
                    </div>
                </div>
            <?php endif; ?>
           

            <div class="function-blocks">
                <div class="function-block">
                    <h4><i class="fa fa-share-alt"></i> Shared Functions</h4>
                    <div class="content">
                        <div class="pi-control" style="box-shadow: #e1e1e1 4px 4px 10px; padding: 10px 20px;">
                            <i class="fa fa-laptop"></i> Raspberry PI<br/>
                            <button class="tooltip pi-wifi" onclick="sendCommand('TEST')"><span style="font-size:0">Pi </span>Ping</button>
                            <button onclick="sendCommand('REBOOT')"><span style="font-size:0">Pi </span>Reboot</button>
                            <button class="tooltip pi-on" onclick="sendCommand('PI_ON')"><span style="font-size:0">Pi </span>On</button>
                            <button class="tooltip pi-off" onclick="sendCommand('PI_OFF')"><span style="font-size:0">Pi </span>Off</button>
                        </div>
                    </div>
                </div>

                <div class="function-block">
                    <h4><i class="fa fa-cogs"></i> Special Functions</h4>
                    <div class="content">
                        <div class="pi-control" style="box-shadow: #e1e1e1 4px 4px 10px; padding: 10px 20px;">
                            <i class="fa fa-camera"></i> Photo<br/>
                            <button onclick="sendCommand('TEST_PHOTO')">Setup</button>
                            <button class="tooltip pi-on" onclick="sendCommand('PHOTO_START')">Start</button>
                            <button class="tooltip pi-off" onclick="sendCommand('PHOTO_STOP')">Stop</button>
                        </div>
                        <br/>
                        <div class="pi-control" style="box-shadow: #e1e1e1 4px 4px 10px; padding: 10px 20px;">
                            Fan<br/>
                            <button class="tooltip pi-on" onclick="sendCommand('FAN_START')">On</button>
                            <button class="tooltip pi-off" onclick="sendCommand('FAN_STOP')">off</button>
                        </div>
                    </div>
                </div>
                
                <?php if ($node_type !== 'special'): ?>
                    <div class="function-block">
                        <h4><i class="fa fa-wifi"></i> Sensor Control Functions</h4>
                        <div class="content">
                            <div class="pi-control" style="box-shadow: #e1e1e1 4px 4px 10px; padding: 10px 20px;">
                                <i class="fa fa-volume-up"></i> Audio<br/>
                                <button class="tooltip pi-on" onclick="sendCommand('AUDIO_START')"><span style="font-size:0">Audio </span>Start</button>
                                <button class="tooltip pi-off" onclick="sendCommand('AUDIO_STOP')"><span style="font-size:0">Audio </span>Stop</button>
                            </div>
                            <div class="pi-control" style="box-shadow: #e1e1e1 4px 4px 10px; padding: 10px 20px;">
                                <i class="fa fa-gamepad"></i> Control<br/>
                                <button onclick="sendCommand('LATCH')"><i class="fa fa-lock-open"></i> Latch Release</button>
                                <button onclick="sendCommand('CUT')"><i class="fa fa-cut"></i> Cut</button>
                            </div>
                            <div class="pi-control" style="box-shadow: #e1e1e1 4px 4px 10px; padding: 10px 20px;">
                                <i class="fa fa-wrench"></i> Reel<br/>
                                <input type="number" id="zposInput" placeholder="Enter value in meters">
                                <button onclick="sendCommand('ZPOS;' + document.getElementById('zposInput').value)">
                                    <span style="font-size:0">Reel </span>Position
                                </button><br/>
                                <input type="number" id="setZposInput" placeholder="Enter value in meters">
                                <button onclick="sendCommand('SET_ZPOS;' + document.getElementById('setZposInput').value)">
                                    <span style="font-size:0">Reel </span>Calibrate
                                </button><br/>
                                <input type="number" id="zpowInput" placeholder="Enter value in %">
                                <button onclick="sendCommand('ZPOW;' + document.getElementById('zpowInput').value)">
                                    <span style="font-size:0">Reel </span>Power
                                </button><br/>
                            </div>
                        </div>
                    </div>
                <?php endif; ?>
            </div>
            
        <?php endif; ?>
    </div>

    <script>
        let command_global;
        function sendCommand(command) {
            command_global = command;
        }
 
        window.addEventListener('DOMContentLoaded', function() {

            document.querySelectorAll('.function-block h4').forEach(function(header) {
                header.addEventListener('click', function() {
                    this.parentElement.classList.toggle('active');
                });
            });

            const overlay = document.querySelector('.overlay');
            const confirmBtn = document.getElementById('confirmBtn');
            const cancelBtn = document.getElementById('cancelBtn');
            const confirmationText = document.getElementById('confirmationText');

            document.querySelectorAll('.function-block button').forEach(function(button) {
                button.addEventListener('click', function(event) {
                    event.stopPropagation();
                    const buttonText = button.textContent.trim();
                    confirmationText.textContent = `Are you sure you want to perform the action "${buttonText}"?`;
                    overlay.style.display = 'flex';
                });
            });

            confirmBtn.addEventListener('click', function() {
                var pid = <?php echo json_encode($pid); ?>;
                var url = "php/requests.php?node_pid=" + encodeURIComponent(pid) + "&command="+command_global;
                
                fetch(url)
                    .then(response => response.text())
                    .then(serviceId => {
                        console.log("Service ID received:", serviceId);
                    })
                    .catch(error => console.error('Error:', error));
                
                overlay.style.display = 'none';
            });

            cancelBtn.addEventListener('click', function() {
                overlay.style.display = 'none';
            });
        });
    </script>

    <div class="overlay">
        <div class="popup">
            <h3>Confirmation</h3>
            <p id="confirmationText"></p>
            <div class="popup-buttons">
                <button id="confirmBtn" class="refresh-button">Confirm</button>
                <button id="cancelBtn" class="cancel-button">Cancel</button>
            </div>
        </div>
    </div>
</body>
</html>