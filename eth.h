#include <SPI.h>
#include <Ethernet.h>

#define CS_PIN 5  // Chip Select Pin for W5500

// MAC address for your Ethernet shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// Server's IP address and port
const char* server = "209.38.236.253";  // Server IP address (for HTTP)
const int port = 80;                    // HTTP port

// Data to send
String myID = "16092100124";
String dataarray = "";

// Create an Ethernet client
EthernetClient client;

void setup() {
  Serial.begin(115200);

  // Initialize Ethernet
  Ethernet.init(CS_PIN);

  // Start Ethernet connection with DHCP
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    while (true)
      ;
  }

  // Print the assigned IP address
  Serial.print("IP Address: ");
  Serial.println(Ethernet.localIP());

  // Construct the request URL
  String request = "GET /api/savedata?id=" + myID + "&data=" + dataarray + " HTTP/1.1";

  // Connect to the server
  if (client.connect(server, port)) {
    Serial.println("Connected to server");

    // bool headersEnded = false;
    // Send HTTP GET request
    client.println(request);
    client.print("Host: ");
    client.println(server);
    client.println("Connection: close");
    client.println();

    // // Read the response
    // while (client.connected() || client.available()) {
    //   if (client.available()) {
    //     String line = client.readStringUntil('\n');
    //     // Serial.println(line);
    //     // Check for the end of headers
    //     if (line == "") {
    //       headersEnded = true;
    //       continue;
    //     }

    //     // If headers have ended, capture the payload
    //     if (headersEnded) {
    //       dataarray += line;
    //     }
    //   }
    // }


    // Read the response
    bool headersEnded = false;
    String chunkSizeStr = "";
    int chunkSize = 0;

    while (client.connected() || client.available()) {
      if (client.available()) {
        char c = client.read();
        if (headersEnded) {
          // If chunk size is not yet determined
          if (chunkSize == 0) {
            if (c == '\n') {
              // End of chunk size line
              chunkSize = strtol(chunkSizeStr.c_str(), NULL, 16);
              chunkSizeStr = "";
              if (chunkSize == 0) {
                // End of the chunked transfer
                break;
              }
            } else if (c != '\r') {
              // Collect chunk size digits
              chunkSizeStr += c;
            }
          } else {
            // Read chunk data
            dataarray += c;
            chunkSize--;
            if (chunkSize == 0) {
              // End of chunk data
              if (client.peek() == '\r') {
                client.read();  // Consume '\r'
                client.read();  // Consume '\n'
              }
              chunkSize = 0;
            }
          }
        } else {
          // Check for end of headers
          if (c == '\n') {
            if (client.peek() == '\r') {
              client.read();  // Consume '\r'
              client.read();  // Consume '\n'
              headersEnded = true;
            }
          }
        }
      }
    }

    // Close the connection
    client.stop();
    Serial.println("Connection closed");

    // Print the payload
    Serial.println("Payload:");
    Serial.println(dataarray);
  } else {
    Serial.println("Connection failed");
  }
}

void loop() {
  // Nothing to do here
}
