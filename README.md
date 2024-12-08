eccu - esp32-cam ota-ugradable http-client

- For OTA, use browser to visit :80/ota_upload, select "WŁĄCZ" ("on"), then visit :9000/ and use the interface to upload new firmware

- You need to setup server endpoint to handle frame uploads using multipart/form-data (The code is partially based on RandomNerdTutorials article (https://randomnerdtutorials.com/esp32-cam-http-post-php-arduino/), so the code should easily integrate with the upload.php example from that article or with a custom endpoint script written using any web framework.

- You need to replace values in `the confidential.sample/*` files' contents with your own values as well as rename the `confidential.sample` directory to `confidential`