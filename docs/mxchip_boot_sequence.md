# MXChip IoT DevKit Boot Flow - Sequence Diagrams

This document describes the boot flow from power-on through the three possible operational modes. These diagrams render natively in GitHub, VS Code (with Markdown Preview), and most modern markdown viewers.

---

## 1. Main Boot Sequence (Power On)

```mermaid
sequenceDiagram
    autonumber
    participant HW as Hardware
    participant Main as main()
    participant Init as Initialization
    participant DC as DeviceConfig
    participant Btn as Button Check

    HW->>Main: Power On / Reset
    
    Main->>Init: Initialization()
    activate Init
    Init->>Init: mbed_lwip_init()
    Init->>Init: Serial.print("MXChip - Microsoft IoT Developer Kit")
    Init->>Init: SystemTickCounterInit()
    Init->>Init: Screen.init()
    Init->>Init: Turn off LEDs (WiFi/Azure/User/RGB)
    Init->>Init: Sensors.init()
    Init-->>Main: Hardware ready
    deactivate Init

    Main->>DC: DeviceConfig_Init(CONNECTION_PROFILE)
    activate DC
    DC->>DC: Load profile definition
    DC-->>Main: Profile set
    deactivate DC

    Main->>DC: DeviceConfig_LoadAll()
    activate DC
    DC->>DC: Read EEPROM zones
    DC->>DC: Parse URLs (extract host/port)
    DC->>DC: Extract DeviceId (from cert/connstr)
    DC-->>Main: Configuration loaded
    deactivate DC

    Main->>Main: __sys_setup()

    Main->>Btn: IsConfigurationMode() - Read Button A
    
    alt Button A pressed
        Btn-->>Main: true
        Main->>Main: EnterConfigurationMode()
    else Button B pressed
        Btn-->>Main: false
        Main->>Btn: IsAPMode() - Read Button B
        Btn-->>Main: true
        Main->>Main: EnterAPMode()
    else No button pressed
        Btn-->>Main: false
        Main->>Main: EnterUserMode()
    end
```

---

## 2. CLI Configuration Mode (Button A Pressed)

When the user holds **Button A** during reset, the device enters serial CLI configuration mode.

```mermaid
sequenceDiagram
    autonumber
    participant User as User (Serial Terminal)
    participant Main as EnterConfigurationMode()
    participant CLI as cli_main()
    participant Cfg as DeviceConfigCLI
    participant DC as DeviceConfig
    participant EE as EEPROM

    Main->>Main: Screen.print("Configuration")
    Main->>Main: InitSystemWiFi()
    Main->>Main: Display MAC ID on screen
    
    Main->>CLI: cli_main()
    activate CLI
    
    CLI->>User: print_help()<br/>"Configuration console:"<br/>"Active profile: MQTT..."<br/>"Commands: help, scan, set_wifi_ssid..."
    CLI->>User: "# " (prompt)
    
    loop Command Loop (infinite)
        User->>CLI: Enter command (e.g., "set_wifi_ssid MyNetwork")
        CLI->>CLI: get_input() - read serial
        CLI->>CLI: handle_input() - parse command
        
        alt System command (help, scan, version, exit)
            CLI->>CLI: Execute system command
            CLI->>User: Show result
        else Config command (set_*)
            CLI->>Cfg: config_dispatch_command()
            Cfg->>DC: Find setting metadata
            Cfg->>Cfg: Validate value
            Cfg->>DC: DeviceConfig_Save(setting, value)
            DC->>EE: Write to EEPROM zones
            EE-->>DC: OK
            DC-->>Cfg: Success
            Cfg-->>CLI: "INFO: Set WiFi SSID successfully"
            CLI->>User: Show result
        end
        
        CLI->>User: "# " (prompt)
    end
    
    Note over User,CLI: User types "exit"
    CLI->>CLI: reboot_and_exit_command()
    CLI->>CLI: mico_system_reboot()
    deactivate CLI
```

### CLI Commands Available

| Command | Description |
|---------|-------------|
| `help` | Show all available commands |
| `version` | Show SDK/firmware versions |
| `scan` | Scan available WiFi networks |
| `status` | Show current configuration status |
| `set_wifi_ssid <value>` | Set WiFi SSID |
| `set_wifi_pass <value>` | Set WiFi password |
| `set_broker_url <value>` | Set MQTT broker URL |
| `set_device_id <value>` | Set device ID |
| `set_ca_cert "<PEM>"` | Set CA certificate (use `\n` for newlines) |
| `exit` | Save and reboot |

---

## 3. Web AP Configuration Mode (Button B Pressed)

When the user holds **Button B** during reset, the device creates a WiFi Access Point and serves a web configuration UI.

```mermaid
sequenceDiagram
    autonumber
    participant User as User (Browser)
    participant Main as EnterAPMode()
    participant WiFi as SystemWiFi
    participant Web as SystemWeb
    participant HTTP as app_httpd
    participant DC as DeviceConfig
    participant EE as EEPROM

    Main->>Main: Screen.print("IoT DevKit - AP")
    Main->>WiFi: InitSystemWiFi()
    
    Main->>WiFi: SystemWiFiAPStart("AZ-xxxx", "")
    activate WiFi
    WiFi->>WiFi: EMW10xxInterface.set_interface(Soft_AP)
    WiFi->>WiFi: EMW10xxInterface.connect()
    WiFi-->>Main: AP started
    deactivate WiFi
    
    Main->>Main: Screen.print(AP_NAME, "192.168.0.1")
    
    Main->>Web: StartupSystemWeb()
    activate Web
    Web->>HTTP: httpd_server_start()
    HTTP->>HTTP: httpd_init()
    HTTP->>HTTP: httpd_start()
    HTTP->>HTTP: registerHttpHandlers()
    HTTP-->>Web: Server running
    
    Note over Web: while true: wait_ms 1000
    
    Note over User,EE: User connects to AZ-xxxx WiFi and opens browser
    
    User->>HTTP: GET http://192.168.0.1/
    activate HTTP
    HTTP->>HTTP: webSettingsPage()
    HTTP->>WiFi: WiFi scan up to 50 APs
    WiFi-->>HTTP: AP list
    HTTP->>DC: DeviceConfig_Read for each setting
    DC->>EE: Read EEPROM
    EE-->>DC: Current values
    DC-->>HTTP: Pre-fill data
    HTTP->>HTTP: Build HTML form
    HTTP-->>User: HTML Configuration Page
    deactivate HTTP
    
    User->>User: Fill form with WiFi and settings
    
    User->>HTTP: POST /result multipart form
    activate HTTP
    HTTP->>HTTP: webSettingsResultPage()
    HTTP->>HTTP: parseFormData()
        
        loop For each setting
            HTTP->>HTTP: Validate value
            HTTP->>DC: DeviceConfig_Save(setting, value)
            DC->>EE: Write EEPROM
            EE-->>DC: OK
        end
        
        HTTP-->>User: Result Page - Settings Saved - Rebooting
        
        HTTP->>HTTP: wait_ms 3000
        HTTP->>HTTP: mico_system_reboot
        deactivate HTTP
    deactivate Web
```

---

## 4. Normal User Mode (No Buttons Pressed)

When no buttons are pressed during boot, the device loads configuration and starts the user's Arduino sketch.

```mermaid
sequenceDiagram
    autonumber
    participant Main as EnterUserMode()
    participant Ardu as _main_arduino.cpp
    participant Thread as arduino_thread
    participant Sketch as User Sketch
    participant WiFi as SystemWiFi
    participant DC as DeviceConfig

    Main->>Main: Serial.print("Press Button A/B to enter config mode")
    
    Main->>Ardu: start_arduino()
    activate Ardu
    Ardu->>Thread: Create Thread(osPriorityNormal, 8KB stack)
    Ardu->>Thread: arduino_thread.start(arduino_main)
    deactivate Ardu
    
    activate Thread
    Thread->>Sketch: setup()
    activate Sketch
    
    Note over Sketch,DC: User typically connects WiFi in setup()
    
    Sketch->>WiFi: SystemWiFiConnect()
    activate WiFi
    WiFi->>DC: DeviceConfig_GetWifiSsid()
    DC-->>WiFi: "MyNetwork"
    WiFi->>DC: DeviceConfig_GetWifiPassword()
    DC-->>WiFi: "password123"
    WiFi->>WiFi: EMW10xxInterface.connect()
    WiFi->>WiFi: SyncTime() via NTP
    WiFi-->>Sketch: Connected
    deactivate WiFi
    
    Note over Sketch,DC: User may access other config
    Sketch->>DC: DeviceConfig_GetBrokerHost()
    DC-->>Sketch: "mqtt.example.com"
    Sketch->>DC: DeviceConfig_GetCACert()
    DC-->>Sketch: "-----BEGIN CERTIFICATE-----..."
    
    Sketch-->>Thread: setup() complete
    deactivate Sketch
    
    loop Main Loop (forever)
        Thread->>Sketch: loop()
        activate Sketch
        Sketch->>Sketch: Read sensors
        Sketch->>Sketch: Send MQTT messages
        Sketch->>Sketch: Update OLED display
        Sketch-->>Thread: loop() iteration done
        deactivate Sketch
    end
    deactivate Thread
    
    Note over Main: Main thread idles: wait_ms(60000) in loop
```

---

## 5. Configuration Profile System

```mermaid
flowchart TB
    subgraph Compile["Compile Time (platformio.ini)"]
        Profile["CONNECTION_PROFILE"]
    end
    
    Profile --> None["PROFILE_NONE<br/>No EEPROM usage<br/>Config in code"]
    Profile --> MQTT["PROFILE_MQTT_*"]
    Profile --> IoTHub["PROFILE_IOTHUB_*"]
    Profile --> DPS["PROFILE_DPS_*"]
    
    MQTT --> MQTT_UP["MQTT_USERPASS<br/>Username/Password"]
    MQTT --> MQTT_TLS["MQTT_USERPASS_TLS<br/>User/Pass + TLS"]
    MQTT --> MQTT_MTLS["MQTT_MTLS<br/>Mutual TLS"]
    
    IoTHub --> IOT_SAS["IOTHUB_SAS<br/>Connection String"]
    IoTHub --> IOT_CERT["IOTHUB_CERT<br/>X.509 Certificate"]
    
    DPS --> DPS_SAS["DPS_SAS<br/>Symmetric Key"]
    DPS --> DPS_CERT["DPS_CERT<br/>X.509 Certificate"]
    DPS --> DPS_GROUP["DPS_SAS_GROUP<br/>Group Enrollment"]
    
    subgraph EEPROM["EEPROM Zone Mapping"]
        Z3["Zone 3 (120B)<br/>WiFi SSID"]
        Z10["Zone 10 (88B)<br/>WiFi Password"]
        Z5["Zone 5 (224B)<br/>Broker URL /<br/>Connection String"]
        Z0["Zone 0 (880B)<br/>CA Certificate"]
        Z67["Zones 6+7 (1464B)<br/>Client Certificate"]
        Z8["Zone 8 (880B)<br/>Client Key"]
    end
    
    MQTT_UP --> Z3
    MQTT_UP --> Z10
    MQTT_UP --> Z5
    
    MQTT_MTLS --> Z3
    MQTT_MTLS --> Z10
    MQTT_MTLS --> Z0
    MQTT_MTLS --> Z67
    MQTT_MTLS --> Z8
```

---

## 6. Boot Decision Tree

```mermaid
flowchart TD
    Start([Power On / Reset]) --> Init[Initialization<br/>LWIP, Screen, LEDs, Sensors]
    Init --> LoadConfig[DeviceConfig_Init<br/>DeviceConfig_LoadAll]
    LoadConfig --> CheckA{Button A<br/>pressed?}
    
    CheckA -->|Yes| CLI[CLI Configuration Mode<br/>Serial console<br/>Text commands]
    CheckA -->|No| CheckB{Button B<br/>pressed?}
    
    CheckB -->|Yes| AP[AP Configuration Mode<br/>WiFi AP: AZ-xxxx<br/>Web UI at 192.168.0.1]
    CheckB -->|No| User[User Mode<br/>Run Arduino sketch<br/>setup + loop]
    
    CLI --> |exit command| Reboot([Reboot])
    AP --> |Form submitted| Reboot
    
    User --> Loop((Infinite Loop))
    
    style Start fill:#90EE90
    style CLI fill:#FFB6C1
    style AP fill:#87CEEB
    style User fill:#DDA0DD
    style Reboot fill:#F0E68C
```

---

## Summary

| Mode | Trigger | Interface | Purpose |
|------|---------|-----------|---------|
| **CLI Config** | Button A at boot | Serial terminal | Text-based configuration |
| **Web AP Config** | Button B at boot | WiFi AP + Browser | Form-based configuration |
| **User Mode** | No buttons | Arduino sketch | Normal operation |

All configuration is stored in **EEPROM** and persists across reboots. The active **profile** (set at compile time) determines which settings are available and how EEPROM zones are mapped.
