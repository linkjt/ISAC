#ISAC: Individual Sustainable Autonomous Cooker

An AI-driven, IoT-enabled micro-portioning cooking system designed to minimize household food waste by automating exact portion sizes and bridging the gap between human culinary intent and robotic precision.
  ##System Architecture & Data Flow

ISAC uses a distributed, edge-computed architecture. Instead of running heavy logic on the microcontroller, commands flow from user input to hardware actuation:

    User Input: The user speaks or types a meal request via a mobile-friendly web interface.

    NAS Webserver & LLM Engine: A local Network Attached Storage (NAS) hosts the webserver and a local instance of Ollama. A two-stage LLM pipeline (Sous Chef + Line Cook) processes the natural language into precise hardware parameters and JSON payloads.

    MQTT Protocol: The backend publishes the compiled JSON instruction over an MQTT broker running on the NAS.

    ESP32 Microcontroller: The ESP32 subscribes to the MQTT topic, executes the PID thermal control loop, and coordinates the mechanical dispensing and heating elements.

  ##Repository Structure

    ###cad/

        Contains the 3D computer-aided design (CAD) files for the modular dispenser and cooking chamber. 

    ###esp_src/

        Contains the production firmware written for the ESP32 microcontroller. This handles MQTT message parsing, hardware actuation, and the PID thermal control loop.

    ###server/

        Houses the backend infrastructure and webserver files hosted on the local NAS. The core orchestration script (console.js) manages the local Ollama API integration, translates natural language into structured cooking parameters, and publishes commands to the MQTT broker. (Note: Credentials and sensitive environment variables are excluded for security)

    ###img/

        Visual documentation and photos of physical prototypes.

    ###src/ (Legacy)

        Archived, early-stage source code written for a standard Arduino before migrating to the ESP32 IoT architecture.

    ###skem/ (Legacy)

        Archived KiCad schematic files from earlier electronics design iterations.