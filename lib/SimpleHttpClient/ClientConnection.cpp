////////////////////////////////////////////////////////////////////////////////
/// @brief client connection
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2010-2011 triagens GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is triAGENS GmbH, Cologne, Germany
///
/// @author Jan Steemann
/// @author Copyright 2012, triagens GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#include "ClientConnection.h"

#include <netinet/in.h>
#include <netinet/tcp.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

using namespace triagens::basics;
using namespace triagens::httpclient;
using namespace triagens::rest;
using namespace std;

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup httpclient
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief creates a new client connection
////////////////////////////////////////////////////////////////////////////////

ClientConnection::ClientConnection (Endpoint* endpoint,
                                    double requestTimeout,
                                    double connectTimeout,
                                    size_t connectRetries) :
  GeneralClientConnection(endpoint, requestTimeout, connectTimeout, connectRetries) {
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroys a client connection
////////////////////////////////////////////////////////////////////////////////

ClientConnection::~ClientConnection () {
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                   private methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup httpclient
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief check whether the socket is still alive
////////////////////////////////////////////////////////////////////////////////

bool ClientConnection::checkSocket () {
  int so_error = -1;
  socklen_t len = sizeof so_error;

  assert(_socket > 0);

  getsockopt(_socket, SOL_SOCKET, SO_ERROR, &so_error, &len);

  if (so_error == 0) {
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                         protected virtual methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup httpclient
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief connect
////////////////////////////////////////////////////////////////////////////////

bool ClientConnection::connectSocket () {
  if (_endpoint->isConnected()) {
    _endpoint->disconnect();
  }
  _socket = _endpoint->connect(_connectTimeout, _requestTimeout);

  if (_socket == 0) {
    return false;
  }

  if (checkSocket()) {
    return _endpoint->isConnected();
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief disconnect
////////////////////////////////////////////////////////////////////////////////

void ClientConnection::disconnectSocket () {
  _endpoint->disconnect();
  _socket = 0;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief prepare connection for read/write I/O
////////////////////////////////////////////////////////////////////////////////

bool ClientConnection::prepare (const double timeout, const bool isWrite) const {
  struct timeval tv;
  fd_set fdset;

  assert(_socket > 0);

  tv.tv_sec = (uint64_t) timeout;
  tv.tv_usec = ((uint64_t) (timeout * 1000000.0)) % 1000000;

  FD_ZERO(&fdset);
  FD_SET(_socket, &fdset);

  fd_set* readFds = NULL;
  fd_set* writeFds = NULL;

  if (isWrite) {
    writeFds = &fdset;
  }
  else {
    readFds = &fdset;
  }

  if (select(_socket + 1, readFds, writeFds, NULL, &tv) > 0) {
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief write data to the connection
////////////////////////////////////////////////////////////////////////////////

bool ClientConnection::write (void* buffer, size_t length, size_t* bytesWritten) {
  if (!checkSocket()) {
    return false;
  }

#ifdef __APPLE__
  int status = ::send(_socket, buffer, length, 0);
#else
  int status = ::send(_socket, buffer, length, MSG_NOSIGNAL);
#endif

  *bytesWritten = status;

  return (status >= 0);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief read data from the connection
////////////////////////////////////////////////////////////////////////////////
    
bool ClientConnection::read (StringBuffer& stringBuffer) {
  if (!checkSocket()) {
    return false;
  }
  
  assert(_socket > 0);

  do {
    char buffer[READBUFFER_SIZE];

    int lenRead = ::read(_socket, buffer, READBUFFER_SIZE - 1);

    if (lenRead <= 0) {
      // error: stop reading
      break;
    }

    stringBuffer.appendText(buffer, lenRead);
  }
  while (readable());

  return true;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief return whether the connection is readable
////////////////////////////////////////////////////////////////////////////////
    
bool ClientConnection::readable () {
  if (prepare(0.0, false)) {
    return checkSocket();
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////
