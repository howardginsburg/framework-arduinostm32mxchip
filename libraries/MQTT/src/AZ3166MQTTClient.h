// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

#ifndef _AZ3166_MQTT_CLIENT_H_
#define _AZ3166_MQTT_CLIENT_H_

#include "MQTTClient.h"
#include "MQTTNetwork.h"
#include "MQTTNetworkTLS.h"
#include "MQTTmbed.h"
#include "EEPROMInterface.h"

// Default packet sizes
#ifndef AZ3166_MQTT_PACKET_SIZE
#define AZ3166_MQTT_PACKET_SIZE 256
#endif

#ifndef AZ3166_MQTT_MAX_HANDLERS
#define AZ3166_MQTT_MAX_HANDLERS 5
#endif

/**
 * @class AZ3166MQTTClient
 * @brief High-level Arduino-style MQTT client wrapper with TLS support
 * 
 * This class provides a simplified interface for MQTT connections supporting:
 * - Unsecured connections (plain TCP)
 * - Server-only TLS with username/password authentication
 * - Mutual TLS with client certificate authentication
 * 
 * Certificates can be provided at compile-time or loaded from secure EEPROM.
 * 
 * Example usage:
 * @code
 *   AZ3166MQTTClient mqtt;
 *   
 *   // Unsecured connection
 *   mqtt.connect("broker.example.com", 1883, "user", "pass");
 *   
 *   // Server-only TLS
 *   mqtt.connectSecure("broker.example.com", 8883, caCert, "user", "pass");
 *   
 *   // Mutual TLS
 *   mqtt.connectMutualTLS("broker.example.com", 8883, caCert, clientCert, clientKey);
 *   
 *   mqtt.subscribe("topic/test", MQTT::QOS0, messageHandler);
 *   mqtt.publish("topic/test", "Hello World");
 *   
 *   while (true) {
 *       mqtt.loop();
 *   }
 * @endcode
 */
class AZ3166MQTTClient
{
public:
    typedef void (*MessageCallback)(MQTT::MessageData&);
    
    /**
     * @brief Connection mode enumeration
     */
    enum ConnectionMode {
        MODE_NONE = 0,
        MODE_UNSECURED,
        MODE_TLS_SERVER_ONLY,
        MODE_TLS_MUTUAL
    };

    /**
     * @brief Construct MQTT client with default settings
     * @param commandTimeout  Command timeout in milliseconds (default 30000)
     */
    AZ3166MQTTClient(unsigned int commandTimeout = 30000)
        : _commandTimeout(commandTimeout),
          _mqttNetwork(NULL),
          _mqttNetworkTLS(NULL),
          _mqttClient(NULL),
          _mqttClientTLS(NULL),
          _mode(MODE_NONE),
          _connected(false)
    {
    }

    ~AZ3166MQTTClient()
    {
        disconnect();
    }

    /**
     * @brief Connect to MQTT broker without TLS (insecure)
     * @param host      Broker hostname
     * @param port      Broker port (typically 1883)
     * @param clientId  MQTT client ID
     * @param username  Username for authentication (can be NULL)
     * @param password  Password for authentication (can be NULL)
     * @return 0 on success, negative on failure
     */
    int connect(const char* host, int port, const char* clientId,
                const char* username = NULL, const char* password = NULL)
    {
        cleanup();
        
        _mqttNetwork = new MQTTNetwork();
        if (_mqttNetwork == NULL)
        {
            return -1;
        }

        int ret = _mqttNetwork->connect(host, port);
        if (ret != 0)
        {
            cleanup();
            return ret;
        }

        _mqttClient = new MQTT::Client<MQTTNetwork, Countdown, 
                                        AZ3166_MQTT_PACKET_SIZE, 
                                        AZ3166_MQTT_MAX_HANDLERS>(*_mqttNetwork, _commandTimeout);
        if (_mqttClient == NULL)
        {
            cleanup();
            return -1;
        }

        ret = mqttConnect(clientId, username, password);
        if (ret != 0)
        {
            cleanup();
            return ret;
        }

        _mode = MODE_UNSECURED;
        _connected = true;
        return 0;
    }

    /**
     * @brief Connect to MQTT broker with server-only TLS
     * @param host      Broker hostname
     * @param port      Broker port (typically 8883)
     * @param caCert    CA certificate in PEM format
     * @param clientId  MQTT client ID
     * @param username  Username for authentication
     * @param password  Password for authentication
     * @return 0 on success, negative on failure
     */
    int connectSecure(const char* host, int port, const char* caCert,
                      const char* clientId, const char* username = NULL, 
                      const char* password = NULL)
    {
        cleanup();

        _mqttNetworkTLS = new MQTTNetworkTLS(caCert);
        if (_mqttNetworkTLS == NULL)
        {
            return -1;
        }

        int ret = _mqttNetworkTLS->connect(host, port);
        if (ret != 0)
        {
            cleanup();
            return ret;
        }

        _mqttClientTLS = new MQTT::Client<MQTTNetworkTLS, Countdown,
                                          AZ3166_MQTT_PACKET_SIZE,
                                          AZ3166_MQTT_MAX_HANDLERS>(*_mqttNetworkTLS, _commandTimeout);
        if (_mqttClientTLS == NULL)
        {
            cleanup();
            return -1;
        }

        ret = mqttConnectTLS(clientId, username, password);
        if (ret != 0)
        {
            cleanup();
            return ret;
        }

        _mode = MODE_TLS_SERVER_ONLY;
        _connected = true;
        return 0;
    }

    /**
     * @brief Connect to MQTT broker with mutual TLS (client certificate)
     * @param host          Broker hostname
     * @param port          Broker port (typically 8883)
     * @param caCert        CA certificate in PEM format
     * @param clientCert    Client certificate in PEM format
     * @param clientKey     Client private key in PEM format
     * @param clientId      MQTT client ID (can be derived from cert CN)
     * @param username      Optional username (some brokers require this even with X.509)
     * @return 0 on success, negative on failure
     */
    int connectMutualTLS(const char* host, int port, const char* caCert,
                         const char* clientCert, const char* clientKey,
                         const char* clientId, const char* username = NULL)
    {
        cleanup();

        _mqttNetworkTLS = new MQTTNetworkTLS(caCert, clientCert, clientKey);
        if (_mqttNetworkTLS == NULL)
        {
            return -1;
        }

        int ret = _mqttNetworkTLS->connect(host, port);
        if (ret != 0)
        {
            cleanup();
            return ret;
        }

        _mqttClientTLS = new MQTT::Client<MQTTNetworkTLS, Countdown,
                                          AZ3166_MQTT_PACKET_SIZE,
                                          AZ3166_MQTT_MAX_HANDLERS>(*_mqttNetworkTLS, _commandTimeout);
        if (_mqttClientTLS == NULL)
        {
            cleanup();
            return -1;
        }

        // For mutual TLS, username may be required by some brokers
        ret = mqttConnectTLS(clientId, username, NULL);
        if (ret != 0)
        {
            cleanup();
            return ret;
        }

        _mode = MODE_TLS_MUTUAL;
        _connected = true;
        return 0;
    }

    /**
     * @brief Connect using certificates stored in secure EEPROM
     * @param host      Broker hostname
     * @param port      Broker port
     * @param clientId  MQTT client ID
     * @param useMutualTLS  If true, also load client cert/key from EEPROM
     * @return 0 on success, negative on failure
     */
    int connectFromEEPROM(const char* host, int port, const char* clientId,
                          bool useMutualTLS = false)
    {
        EEPROMInterface eeprom;
        
        // Read CA certificate
        static char caCert[AZ_IOT_X509_MAX_LEN + 1];
        if (eeprom.readX509Cert(caCert, sizeof(caCert)) != 0)
        {
            return -1;
        }
        
        if (useMutualTLS)
        {
            // Read client certificate and key
            static char clientCert[CLIENT_CERT_MAX_LEN + 1];
            static char clientKey[CLIENT_KEY_MAX_LEN + 1];
            
            if (eeprom.readClientCert(clientCert, sizeof(clientCert)) != 0)
            {
                return -1;
            }
            if (eeprom.readClientKey(clientKey, sizeof(clientKey)) != 0)
            {
                return -1;
            }
            
            return connectMutualTLS(host, port, caCert, clientCert, clientKey, clientId);
        }
        else
        {
            // Read username/password for server-only TLS
            static char username[DEVICE_ID_MAX_LEN + 1];
            static char password[DEVICE_PASSWORD_MAX_LEN + 1];
            
            eeprom.readDeviceID(username, sizeof(username));
            eeprom.readDevicePassword(password, sizeof(password));
            
            return connectSecure(host, port, caCert, clientId, 
                                username[0] ? username : NULL,
                                password[0] ? password : NULL);
        }
    }

    /**
     * @brief Disconnect from the broker
     */
    void disconnect()
    {
        if (_mqttClient != NULL && _mqttClient->isConnected())
        {
            _mqttClient->disconnect();
        }
        if (_mqttClientTLS != NULL && _mqttClientTLS->isConnected())
        {
            _mqttClientTLS->disconnect();
        }
        cleanup();
        _connected = false;
        _mode = MODE_NONE;
    }

    /**
     * @brief Subscribe to a topic
     * @param topic     Topic filter
     * @param qos       Quality of service
     * @param callback  Message handler callback
     * @return 0 on success
     */
    int subscribe(const char* topic, MQTT::QoS qos, MessageCallback callback)
    {
        if (_mqttClient != NULL)
        {
            return _mqttClient->subscribe(topic, qos, callback);
        }
        else if (_mqttClientTLS != NULL)
        {
            return _mqttClientTLS->subscribe(topic, qos, callback);
        }
        return -1;
    }

    /**
     * @brief Unsubscribe from a topic
     * @param topic  Topic filter
     * @return 0 on success
     */
    int unsubscribe(const char* topic)
    {
        if (_mqttClient != NULL)
        {
            return _mqttClient->unsubscribe(topic);
        }
        else if (_mqttClientTLS != NULL)
        {
            return _mqttClientTLS->unsubscribe(topic);
        }
        return -1;
    }

    /**
     * @brief Publish a message
     * @param topic      Topic name
     * @param payload    Message payload
     * @param len        Payload length (0 to use strlen)
     * @param qos        Quality of service
     * @param retained   Retain flag
     * @return 0 on success
     */
    int publish(const char* topic, const char* payload, size_t len = 0,
                MQTT::QoS qos = MQTT::QOS0, bool retained = false)
    {
        if (len == 0)
        {
            len = strlen(payload);
        }
        
        if (_mqttClient != NULL)
        {
            return _mqttClient->publish(topic, (void*)payload, len, qos, retained);
        }
        else if (_mqttClientTLS != NULL)
        {
            return _mqttClientTLS->publish(topic, (void*)payload, len, qos, retained);
        }
        return -1;
    }

    /**
     * @brief Process incoming messages and keepalive
     * @param timeoutMs  Yield timeout in milliseconds
     * @return 0 on success
     */
    int loop(unsigned long timeoutMs = 100)
    {
        if (_mqttClient != NULL)
        {
            return _mqttClient->yield(timeoutMs);
        }
        else if (_mqttClientTLS != NULL)
        {
            return _mqttClientTLS->yield(timeoutMs);
        }
        return -1;
    }

    /**
     * @brief Check connection status
     * @return true if connected
     */
    bool isConnected() const
    {
        if (_mqttClient != NULL)
        {
            return _mqttClient->isConnected();
        }
        else if (_mqttClientTLS != NULL)
        {
            return _mqttClientTLS->isConnected();
        }
        return false;
    }

    /**
     * @brief Get current connection mode
     * @return Connection mode enum value
     */
    ConnectionMode getConnectionMode() const
    {
        return _mode;
    }

    /**
     * @brief Set default message handler for unhandled messages
     * @param callback  Message handler callback
     */
    void setDefaultMessageHandler(MessageCallback callback)
    {
        if (_mqttClient != NULL)
        {
            _mqttClient->setDefaultMessageHandler(callback);
        }
        else if (_mqttClientTLS != NULL)
        {
            _mqttClientTLS->setDefaultMessageHandler(callback);
        }
    }

private:
    void cleanup()
    {
        if (_mqttClient != NULL)
        {
            delete _mqttClient;
            _mqttClient = NULL;
        }
        if (_mqttClientTLS != NULL)
        {
            delete _mqttClientTLS;
            _mqttClientTLS = NULL;
        }
        if (_mqttNetwork != NULL)
        {
            _mqttNetwork->disconnect();
            delete _mqttNetwork;
            _mqttNetwork = NULL;
        }
        if (_mqttNetworkTLS != NULL)
        {
            _mqttNetworkTLS->disconnect();
            delete _mqttNetworkTLS;
            _mqttNetworkTLS = NULL;
        }
    }

    int mqttConnect(const char* clientId, const char* username, const char* password)
    {
        MQTTPacket_connectData options = MQTTPacket_connectData_initializer;
        options.MQTTVersion = 4;  // MQTT 3.1.1
        options.clientID.cstring = (char*)clientId;
        options.keepAliveInterval = 60;
        options.cleansession = 1;
        
        if (username != NULL)
        {
            options.username.cstring = (char*)username;
        }
        if (password != NULL)
        {
            options.password.cstring = (char*)password;
        }
        
        return _mqttClient->connect(options);
    }

    int mqttConnectTLS(const char* clientId, const char* username, const char* password)
    {
        MQTTPacket_connectData options = MQTTPacket_connectData_initializer;
        options.MQTTVersion = 4;  // MQTT 3.1.1
        options.clientID.cstring = (char*)clientId;
        options.keepAliveInterval = 60;
        options.cleansession = 1;
        
        if (username != NULL)
        {
            options.username.cstring = (char*)username;
        }
        if (password != NULL)
        {
            options.password.cstring = (char*)password;
        }
        
        return _mqttClientTLS->connect(options);
    }

    unsigned int _commandTimeout;
    MQTTNetwork* _mqttNetwork;
    MQTTNetworkTLS* _mqttNetworkTLS;
    MQTT::Client<MQTTNetwork, Countdown, AZ3166_MQTT_PACKET_SIZE, AZ3166_MQTT_MAX_HANDLERS>* _mqttClient;
    MQTT::Client<MQTTNetworkTLS, Countdown, AZ3166_MQTT_PACKET_SIZE, AZ3166_MQTT_MAX_HANDLERS>* _mqttClientTLS;
    ConnectionMode _mode;
    bool _connected;
};

#endif // _AZ3166_MQTT_CLIENT_H_
