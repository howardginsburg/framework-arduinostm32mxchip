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

#ifndef wificlientsecure_h
#define wificlientsecure_h

#include "Arduino.h"
#include "Client.h"
#include "IPAddress.h"

class TLSSocket;

class WiFiClientSecure : public Client
{
public:
  WiFiClientSecure();
  WiFiClientSecure(TLSSocket* socket);
  ~WiFiClientSecure();

  void setCACert(const char* rootCA);
  void setCertificate(const char* clientCert);
  void setPrivateKey(const char* privateKey);
  void setInsecure();

  virtual int connect(IPAddress ip, unsigned short port);
  virtual int connect(const char *host, unsigned short port);
  virtual size_t write(uint8_t);
  virtual size_t write(const uint8_t *buf, size_t size);
  virtual int available();
  virtual int read();
  virtual int read(uint8_t *buf, size_t size);
  virtual void flush();
  virtual void stop();
  virtual uint8_t connected();
  virtual operator bool();
  virtual int peek();

  friend class WiFiServer;
  void setTimeout(unsigned int timeout) { _timeout = timeout; }

private:
  TLSSocket* _pTlsSocket;
  bool _useServerSocket;
  const char* _caCert;
  const char* _clientCert;
  const char* _clientKey;
  
  // Read buffer for proper available()/peek() support
  uint8_t _peekBuffer[64];
  int _peekBufferLen;
  int _peekBufferPos;
  unsigned int _timeout;  // Socket timeout in ms
};

// Backward/compatibility aliases
typedef WiFiClientSecure AZ3166WiFiClientSecure;
typedef WiFiClientSecure AZ3166WifiClientSecure;

#endif
