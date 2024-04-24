<?php
    include 'dbh.php'; // Database connection file

    $service_id = $_GET['service_id'] ?? 0;

    // Fetch the latest progress for the given service_id
    $query = "SELECT status FROM responses WHERE service_id = ?";
    $stmt = $conn->prepare($query);
    $stmt->bind_param("i", $service_id);
    $stmt->execute();
    $result = $stmt->get_result();

    if ($result->num_rows > 0) {
        $row = $result->fetch_assoc();
        echo json_encode(['progress' => $row['status']]);
    } else {
        echo json_encode(['progress' => -1]);
    }

    $stmt->close();
    $conn->close();
?>