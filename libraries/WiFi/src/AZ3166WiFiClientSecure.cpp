/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "AZ3166WiFi.h"
#include "AZ3166WiFiClientSecure.h"
#include "SystemWiFi.h"
#include "TLSSocket.h"

WiFiClientSecure::WiFiClientSecure()
{
    _pTlsSocket = NULL;
    _useServerSocket = false;
    _caCert = NULL;
    _clientCert = NULL;
    _clientKey = NULL;
    _peekBufferLen = 0;
    _peekBufferPos = 0;
    _timeout = 2000;
}

WiFiClientSecure::WiFiClientSecure(TLSSocket* socket)
{
    _pTlsSocket = socket;
    _useServerSocket = true;
    _caCert = NULL;
    _clientCert = NULL;
    _clientKey = NULL;
    _peekBufferLen = 0;
    _peekBufferPos = 0;
    _timeout = 2000;
}

WiFiClientSecure::~WiFiClientSecure()
{
    stop();
}

void WiFiClientSecure::setCACert(const char* rootCA)
{
    _caCert = rootCA;
}

void WiFiClientSecure::setCertificate(const char* clientCert)
{
    _clientCert = clientCert;
}

void WiFiClientSecure::setPrivateKey(const char* privateKey)
{
    _clientKey = privateKey;
}

void WiFiClientSecure::setInsecure()
{
    _caCert = NULL;
}

int WiFiClientSecure::peek()
{
    if (available() > 0)
    {
        return _peekBuffer[_peekBufferPos];
    }
    return -1;
}

int WiFiClientSecure::connect(const char* host, unsigned short port)
{
    if (_pTlsSocket != NULL)
    {
        // Already connected
        return 0;
    }

    // Clear peek buffer for new connection
    _peekBufferLen = 0;
    _peekBufferPos = 0;

    NetworkInterface* netIface = WiFiInterface();
    if (netIface == NULL)
    {
        return 0;
    }

    if (_clientCert != NULL && _clientKey != NULL)
    {
        _pTlsSocket = new TLSSocket(_caCert, _clientCert, _clientKey, netIface);
    }
    else
    {
        _pTlsSocket = new TLSSocket(_caCert, netIface);
    }

    if (_pTlsSocket == NULL)
    {
        return 0;
    }

    if (_pTlsSocket->connect(host, (uint16_t)port) != NSAPI_ERROR_OK)
    {
        delete _pTlsSocket;
        _pTlsSocket = NULL;
        return 0;
    }

    return 1;
}

int WiFiClientSecure::connect(IPAddress ip, unsigned short port)
{
    return connect(ip.get_address(), (uint16_t)port);
}

size_t WiFiClientSecure::write(uint8_t b)
{
    return write(&b, 1);
}

size_t WiFiClientSecure::write(const uint8_t *buf, size_t size)
{
    if (_pTlsSocket != NULL)
    {
        int ret = _pTlsSocket->send((void*)buf, (int)size);
        return (ret > 0) ? (size_t)ret : 0;
    }
    return 0;
}

int WiFiClientSecure::available()
{
    // Return buffered data count if we have any
    if (_peekBufferLen > _peekBufferPos)
    {
        return _peekBufferLen - _peekBufferPos;
    }
    
    // No buffered data - try to read some
    if (_pTlsSocket != NULL)
    {
        _peekBufferPos = 0;
        int ret = _pTlsSocket->recv(_peekBuffer, sizeof(_peekBuffer));
        if (ret > 0)
        {
            _peekBufferLen = ret;
            return ret;
        }
        _peekBufferLen = 0;
    }
    
    return 0;
}

int WiFiClientSecure::read()
{
    // First check/fill the peek buffer
    if (available() > 0)
    {
        return _peekBuffer[_peekBufferPos++];
    }
    return -1;
}

int WiFiClientSecure::read(uint8_t* buf, size_t size)
{
    if (size == 0) return 0;
    
    size_t copied = 0;
    
    // First drain peek buffer
    while (copied < size && _peekBufferLen > _peekBufferPos)
    {
        buf[copied++] = _peekBuffer[_peekBufferPos++];
    }
    
    // If we got all requested from buffer, return
    if (copied == size)
    {
        return (int)copied;
    }
    
    // Need more data - read directly from socket
    if (_pTlsSocket != NULL && copied < size)
    {
        int ret = _pTlsSocket->recv((void*)(buf + copied), (int)(size - copied));
        if (ret > 0)
        {
            copied += ret;
        }
    }
    
    return (copied > 0) ? (int)copied : 0;
}

void WiFiClientSecure::flush()
{
}

void WiFiClientSecure::stop()
{
    // Clear peek buffer
    _peekBufferLen = 0;
    _peekBufferPos = 0;
    
    if (_pTlsSocket != NULL)
    {
        _pTlsSocket->close();
        if (!_useServerSocket)
        {
            delete _pTlsSocket;
            _pTlsSocket = NULL;
        }
    }
}

uint8_t WiFiClientSecure::connected()
{
    return (_pTlsSocket != NULL) ? 1 : 0;
}

WiFiClientSecure::operator bool()
{
    return (_pTlsSocket != NULL);
}
