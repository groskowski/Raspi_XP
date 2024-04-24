<?php
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

// Function to log messages
function logMessage($message, $type) {
    global $conn;
    
    // Prepare and bind the SQL statement
    $stmt = $conn->prepare("INSERT INTO log (message, type) VALUES (?, ?)");
    $stmt->bind_param("ss", $message, $type);
    
    // Execute the SQL statement
    $stmt->execute();
    
    // Close the statement
    $stmt->close();
}

// Check if the request method is POST
if ($_SERVER["REQUEST_METHOD"] == "POST") {
    // Get the message and type from the POST data
    $message = $_POST["message"];
    $type = $_POST["type"];
    
    // Call the logMessage function
    logMessage($message, $type);
    
    // Return a success response
    echo "Message logged successfully";
}

// Close the database connection
$conn->close();
?>