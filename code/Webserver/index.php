<?php
  include 'php/dbh.php';

  // Function to get relative time format
  function getRelativeTime($timeDiff) {
    $seconds = floor($timeDiff);
    $minutes = floor($seconds / 60);
    $hours = floor($minutes / 60);
    $days = floor($hours / 24);

    if ($seconds < 60) {
      return 'Updated ' . $seconds . ' seconds ago';
    } else if ($minutes < 60) {
      return 'Updated ' . $minutes . ' minutes ago';
    } else if ($hours < 24) {
      return 'Updated ' . $hours . ' hours ago';
    } else {
      return 'Updated ' . $days . ' days ago';
    }
  }

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

  // Get the selected filter option
  $filter = isset($_GET['filter']) ? $_GET['filter'] : 'battery';

?>

<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Dashboard</title>
    <link rel="icon" href="favicon.ico" type="image/x-icon">
    <link rel="stylesheet" href="css/style.css">
    <link rel="stylesheet" href="fonts/fa/css/font-awesome.min.css">
    <link rel="stylesheet" href="fonts/roboto/roboto.css">
  </head>
  <body>
    <?php ob_start();?>
      <li><div class="vl"></div></li>
      <button id="refreshButton" class="refresh-button" onclick="requestAllData()">Request ALL Data</button>
      <span style="width:20px">  </span>
      <li>
      <div class="filter-options">
          <label for="filter">Sort by:</label>
          <select id="filter" onchange="applyFilter()">
              <option value="select" <?php echo $filter === 'battery' ? 'selected' : ''; ?>>Node #</option>
              <option value="humidity" <?php echo $filter === 'humidity' ? 'selected' : ''; ?>>Humidity</option>
              <option value="temperature" <?php echo $filter === 'temperature' ? 'selected' : ''; ?>>Temperature</option>
              <option value="pressure" <?php echo $filter === 'pressure' ? 'selected' : ''; ?>>Pressure</option>
              <option value="last_updated" <?php echo $filter === 'last_updated' ? 'selected' : ''; ?>>Last Updated</option>
          </select>
      </div>
      </li>
    <?php $listContent = ob_get_clean();?>
    <?php include "navbar.php"?>
    <div class="sensor-grid">
    <?php
      // Fetch unique sensors from the database
      $result = $conn->query("SELECT DISTINCT pid FROM node_table");

      if ($result->num_rows > 0) {
          $sensor_data = array();

          while ($row = $result->fetch_assoc()) {
              $pid = $row['pid'];

              // Fetch the last record for the current sensor
              $sql_last = "SELECT * FROM sensor_data WHERE pid = $pid ORDER BY insertion_time DESC LIMIT 1";
              $result_last = $conn->query($sql_last);

              if ($result_last->num_rows > 0) {
                  $sensor_data[] = $result_last->fetch_assoc();
              }
          }

          // Sort the sensor data based on the selected filter
          usort($sensor_data, function($a, $b) use ($filter) {
              switch ($filter) {
                  case 'humidity':
                      return $b['humidity'] - $a['humidity'];
                  case 'temperature':
                      return $b['temp'] - $a['temp'];
                  case 'pressure':
                      return $b['pressure'] - $a['pressure'];
                  case 'last_updated':
                      return strtotime($b['insertion_time']) - strtotime($a['insertion_time']);
                  case 'battery':
                  default:
                      return $a['pid'] - $b['pid'];
              }
          });

          foreach ($sensor_data as $data) {
              $pid = $data['pid'];
              $temperature_c = $data['temp'];
              $temperature_f = ($temperature_c * 9/5) + 32;
              $humidity = $data['humidity'];
              $pressure = $data['pressure'] / 1013.25;
              $min_battery = min($data['battery1'], $data['battery2'], $data['battery3']);
              $battery_level_class = getBatteryLevelClass($min_battery);
              $last_update = strtotime($data['insertion_time']);
              $time_diff = time() - $last_update;

              // Fetch the node type from the node_table based on the pid
              $sql_node_type = "SELECT type FROM node_table WHERE pid = $pid";
              $result_node_type = $conn->query($sql_node_type);
              $node_type = '';

              if ($result_node_type->num_rows > 0) {
                  $row_node_type = $result_node_type->fetch_assoc();
                  $node_type = $row_node_type['type'];
              }

              echo '<div class="sensor-block">';
              echo '<div class="battery-indicator ' . $battery_level_class . '"></div>';
              echo '<div class="sensor-header">';
              echo '<h3>Node ' . $pid . '</h3>';
              echo '<span class="last-update">' . getRelativeTime($time_diff) . '</span>';
              echo '<button style="border:none; cursor:pointer; padding:5px 10px 5px 10px" onclick="window.location.href=\'node_details.php?node=' . $pid . '\'"><i class="fa fa-ellipsis-h fa-2x"></i></button>';
              echo '</div>';
              echo '<div class="sensor-body">';
              echo '<div class="sensor-data">';
              echo '<p><strong>Temp:</strong> ' . round($temperature_f, 1) . '°F</p>';
              echo '<p><strong>Humidity:</strong> ' . $humidity . '%</p>';
              //echo '<p><strong>Pressure:</strong> ' . round($pressure, 2) . ' atm</p>';
              echo '<p><strong>Battery:</strong> ' . $min_battery . 'V</p>';
              echo '</div>';
              echo '<div class="sensor-icon center"">';

              if ($node_type === 'special') {
                  echo '<i class="fa fa-star fa-2x"></i>';
              } elseif ($node_type === 'shared') {
                  echo '<i class="fa fa-share-alt fa-2x"></i>';
              } elseif ($node_type === 'sensor') {
                  echo '<i class="fa fa-thermometer-half fa-2x"></i>';
              }

              echo '</div>';
              echo '</div>';
              echo '<button class="collect-button" onclick="requestData(' . $pid . ')">Get data</button>';
              echo '</div>';
            }
        } else {
            echo 'No sensors found.';
        }
      ?>
    </div>
  </body>
  <script>
    
    const loadingBar = document.getElementById('loadingBar');
    const loadingText = document.getElementById('loadingText');
    const overlay = document.getElementById('overlay');

    function pollProgress(serviceId) {
        const url = `php/responses.php?service_id=${encodeURIComponent(serviceId)}`;
        const loadingText = document.getElementById('loadingText');

        fetch(url)
            .then(response => response.json())
            .then(data => {
                const progress = parseFloat(data.progress) * 100;
                console.log(progress)
                let progressText = ''; 

                if (progress < 0) {
                    progressText = 'Waiting for response';
                } else if (progress === 0) {
                    progressText = 'Acknowledged';
                } else if (progress > 0 && progress < 100) {
                    progressText = 'Working...';
                } else if (progress === 100) {
                    progressText = 'Completed';
                }

                loadingBar.style.width = `${progress}%`;
                loadingText.textContent = progressText;
                overlay.style.display = 'flex';
                // Show or hide overlay based on progress
                if (progress < 100) {
                    setTimeout(() => pollProgress(serviceId), 1000);
                } else {
                    overlay.style.display = 'none';
                    if (progress === 100) {
                        console.log('Operation completed successfully.');
                        setTimeout(() => {
                            window.location.reload();
                        }, 500);
                    } else if (progress === -100) {
                        console.error('Error or operation not found.');
                    }
                }
            })
            .catch(error => {
                console.error('Failed to fetch progress:', error);
                setTimeout(() => pollProgress(serviceId), 1000); // Retry polling after an error
            });
    }

    
    function requestData(pid) {
        const url = 'php/requests.php?node_pid='+pid+'&command=DATA';

        fetch(url)
            .then(response => response.text())
            .then(serviceId => {
                console.log("Service ID received:", serviceId);
                if (!isNaN(serviceId) && serviceId.trim() !== '') {
                    pollProgress(serviceId);
                } else {
                    console.error('Invalid service ID received:', serviceId);
                }
            })
            .catch(error => console.error('Error:', error));
    }

    // Function to handle the "Request ALL Data" button click
    function requestAllData() {
        const url = `php/requests.php?node_pid=NULL&command=ALL_DATA`;

        fetch(url)
            .then(response => response.text())
            .then(serviceId => {
                console.log("Service ID received:", serviceId);
                if (!isNaN(serviceId) && serviceId.trim() !== '') {
                    pollProgress(serviceId);  // Call pollProgress with the new serviceId
                } else {
                    console.error('Invalid service ID received:', serviceId);
                }
            })
            .catch(error => console.error('Error:', error));
    }

    function applyFilter() {
      const filterValue = document.getElementById('filter').value;
      const url = new URL(window.location.href);
      url.searchParams.set('filter', filterValue);
      window.location.href = url.toString();
    }
  </script>
</html>