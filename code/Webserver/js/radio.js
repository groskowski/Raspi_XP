document.addEventListener('DOMContentLoaded', function () {
  // Placeholder function for sensor variable
  function sensorVariable(sensorId) {
      // Print variable
      console.log(sensorId);
  }

  // Add click event listeners to each sensor div
  for (let i = 1; i <= 16; i++) {
      const sensorId = 'sensor' + i;
      const sensorDiv = document.getElementById(sensorId);

      // Add click event listener to each sensor div
      sensorDiv.addEventListener('click', function () {
          sensorVariable(i);
      });
  }

});
