document.addEventListener('DOMContentLoaded', function () {
  fetchData(); // Initial data fetch when the page loads

  const refreshButton = document.getElementById('refreshButton');

  // Add click event listener to the refresh button
  refreshButton.addEventListener('click', function () {
    fetchData();
  });

  //------------------------------------------// Action on button click
  function refreshIndividual(buttonId) {
    console.log(buttonId);
  }

  // Function to navigate to a different page based on the sensor ID
  function navigateToPage(sensorId) {
    // Construct the URL for the destination page
    var destinationPage = sensorId + '.php';

    // Navigate to the destination page
    window.location.href = destinationPage;
  }

  // Add click event listeners to each sensor div
  for (let i = 1; i <= 16; i++) {
    const sensorId = 'sensor' + i;
    const sensorDiv = document.getElementById(sensorId);

    // Add click event listener to each sensor div
    sensorDiv.addEventListener('click', function () {
      navigateToPage(sensorId);
    });

    // Add click event listener to each refresh button inside the sensor div
    const buttonMessage = i; //------------------------------// Modify this to modify messages on button click
    const buttonId = 'button' + i;
    const buttonDiv = document.getElementById(buttonId);

    buttonDiv.addEventListener('click', function (event) {
      // Prevent propagation of the click event to the parent div
      event.stopPropagation();
      
      // Call the refresh function
      refreshIndividual(buttonMessage);
    });
  }

  // Function to fetch data from the server
  function fetchData() {
    fetch('fetch.php')
      .then(response => response.json())
      .then(data => {
        // Update HTML content with fetched data
        updateHtmlWithFetchedData(data);
      })
      .catch(error => console.error('Error fetching data:', error));
  }

  // Function to update HTML content with fetched data
  function updateHtmlWithFetchedData(data) {
    // Iterate through the data and update each sensor div
    data.forEach(sensorData => {
      const sensorId = sensorData.sensor_id;
      document.getElementById('temperature' + sensorId).innerText = sensorData.temperature + "\u2109";
      document.getElementById('battery' + sensorId).innerText = sensorData.battery + "%";
      document.getElementById('cameraStatus' + sensorId).innerText = sensorData.camera_status;
    });
  }
});
