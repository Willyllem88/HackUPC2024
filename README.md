# HackUPC2024
This project, developed by Pau Belda, Raül Box, Guillem Cabré, and Marc Peñalver during the 2024 edition of HackUPC, was based on a challenge proposed by Grafana. The objective was to create a program that could collect and visualize temperature and humidity data from a room. We were supposed to use Grafana's application to visualize the data.

Hardware: we used an ESP32 microchip and a KY-015 temperature and humidity sensor for data collection. The KY-015 sensor was connected to the ESP32 using a serial connection to its 5th pin. The ESP32's built-in WiFi module enabled us to send the collected data to another computer through a TCP/IP connection.

Software: the code for the ESP32 was developed using ArduinoIDE. A separate C program in the receiving computer (the server) executed a socket which accepted the data. After that the data was processed and, using functions from the library "libpq-fe.h", the program established a connection with our database and introduced the data in it. We used PostgreSQL for the database and created a table with fields for time (in seconds), temperature (in degrees Celsius), and humidity (in percentage). Finally, we connected Grafana to the PostgreSQL database to visualize the data in graphical form.

By using this setup, we were able to collect and visualize room temperature and humidity data effectively, providing a useful tool for data analysis.
