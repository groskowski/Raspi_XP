<?php include "php/dbh.php"; ?>
<!DOCTYPE html>
<html>
<head>
    <title>Developer Log</title>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link rel="stylesheet" href="css/style.css">
    <link rel="stylesheet" href="fonts/fa/css/font-awesome.min.css">
    <link rel="stylesheet" href="fonts/roboto/roboto.css">
    <style>
        body {
            font-family: 'Roboto', sans-serif;
            font-size: 18px;
            line-height: 1.5;
        }
        table {
            width: 80%;
            border-collapse: collapse;
            margin: auto;
            margin-top: 20px;            
        }
        th, td {
            padding: 8px;
            text-align: left;
            border-bottom: 1px solid #ddd;
        }
        th {
            background-color: #f2f2f2;
        }
        .error {
            color: #ff0000;
            font-weight: bold;
        }
        .warning {
            color: #ff8c00;
            font-weight: bold;
        }
        .info {
            color: #008000;
            font-weight: bold;
        }
    </style>
</head>
<body style="text-align:center;">
    <?php include "navbar.php"; ?>
    <h2 style="margin-top:20px">Developer Log</h2>
    <table>
        <tr>
            <th>Time</th>
            <th>Message</th>
        </tr>
        <?php
        // Retrieve log entries from the database
        $sql = "SELECT * FROM log ORDER BY time DESC";
        $result = $conn->query($sql);
        if ($result->num_rows > 0) {
            while ($row = $result->fetch_assoc()) {
                $time = $row['time'];
                $message = $row['message'];
                $type = $row['type'];

                // Determine the CSS class and icon based on the log type
                $class = '';
                $icon = '';
                if ($type === 'error') {
                    $class = 'error';
                    $icon = '<i class="fa fa-exclamation"></i>';
                } elseif ($type === 'warning') {
                    $class = 'warning';
                    $icon = '<i class="fa fa-exclamation-triangle"></i>';
                } elseif ($type === 'info') {
                    $class = 'info';
                    $icon = '<i class="fa fa-info-circle"></i>';
                }

                echo "<tr>";
                echo "<td>$time</td>";
                echo "<td style='font-weight:600;'><span class='$class'>$icon [" . strtoupper($type) . "]</span> $message</td>";
                echo "</tr>";
            }
        } else {
            echo "<tr><td colspan='2'>No log entries found.</td></tr>";
        }
        $conn->close();
        ?>
    </table>
</body>
</html>