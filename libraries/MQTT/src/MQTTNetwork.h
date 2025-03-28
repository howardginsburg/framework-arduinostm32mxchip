// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.

#ifndef _MQTTNETWORK_H_
#define _MQTTNETWORK_H_

#include "NetworkInterface.h"
#include "MQTTmbed.h"
#include "SystemWiFi.h"

class MQTTNetwork
{
  public:
    MQTTNetwork()
    {
        _tcpSocket = NULL;
    }

    ~MQTTNetwork()
    {
        if (_tcpSocket != NULL)
        {
            _tcpSocket->close();
            delete _tcpSocket;
            _tcpSocket = NULL;
        }
    }

    int read(unsigned char *buffer, int len, int timeout)
    {
        if (_tcpSocket == NULL)
        {
            return NSAPI_ERROR_NO_SOCKET;
        }

        _tcpSocket->set_timeout(timeout);
        return _tcpSocket->recv(buffer, len);
    }

    int write(unsigned char *buffer, int len, int timeout)
    {
        if (_tcpSocket == NULL)
        {
            return NSAPI_ERROR_NO_SOCKET;
        }

        _tcpSocket->set_timeout(timeout);
        return _tcpSocket->send(buffer, len);
    }

    int connect(const char *hostname, int port)
    {
        if (_tcpSocket != NULL)
        {
            return NSAPI_ERROR_OK;
        }
        else
        {
            _tcpSocket = new TCPSocket();
            if (_tcpSocket == NULL)
            {
                return NSAPI_ERROR_NO_SOCKET;
            }

            int ret;

            if (!((ret = _tcpSocket->open(WiFiInterface())) == 0 && (ret = _tcpSocket->connect(hostname, port)) == 0))
            {
                // open socket failed or connect host failed
                delete _tcpSocket;
                _tcpSocket = NULL;
            }

            return ret;
        }
    }

    int disconnect()
    {
        if (_tcpSocket == NULL)
        {
            return NSAPI_ERROR_OK;
        }
        else
        {
            int ret;
            if ((ret = _tcpSocket->close()) != NSAPI_ERROR_OK)
            {
                return ret;
            }

            delete _tcpSocket;
            _tcpSocket = NULL;
            return NSAPI_ERROR_OK;
        }
    }

  private:
    TCPSocket *_tcpSocket;
};

#endif // _MQTTNETWORK_H_
