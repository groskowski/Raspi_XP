<?php
    include 'dbh.php';

    $node_pid = $_GET['node_pid'] ?? 'default_pid';
    $command = $_GET['command'] ?? 'default_command';
    
    $concatenatedString = $node_pid . $command . $time;
    $hash = hash('sha256', $concatenatedString);
    $numericHash = base_convert(substr($hash, 0, 8), 16, 10);
    $service_id = $numericHash % 10000000;

    $stmt = $conn->prepare("INSERT INTO requests (insertion_date, command, target, service_id) VALUES (NOW(), ?, ?, ?)");
    $stmt->bind_param("ssi", $command, $node_pid, $service_id);
    $stmt->execute();

    if ($stmt->error) {
        echo "Error: " . $stmt->error;
    } else {
        echo $service_id;
    }

    $stmt->close();
    $conn->close();
?>
