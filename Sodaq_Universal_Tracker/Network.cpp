/*
Copyright (c) 2018, SODAQ
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#include "Network.h"

#define DEBUG

#ifdef DEBUG
#define debugPrintLn(...) { if (this->_diagStream) this->_diagStream->println(__VA_ARGS__); }
#define debugPrint(...) { if (this->_diagStream) this->_diagStream->print(__VA_ARGS__); }
#warning "Debug mode is ON"
#else
#define debugPrintLn(...)
#define debugPrint(...)
#endif

LoraNetwork loraNetwork;
NbiotNetwork nbiotNetwork;
Network3G network3G;
LteNetwork lteNetwork;

static bool isImeiInitialized;
static char cachedImei[16];
static bool isModuleVersionInitialized;
static char cachedModuleVersion[50];

bool Network::init(Uart & modemStream, DataReceiveCallback callback, uint32_t(*getNow)(), InitConsoleMessages messages, InitJoin join)
{
    switch (_networkType) {
    case NETWORK_TYPE_NBIOT_R4:
    case NETWORK_TYPE_NBIOT_N2: {
        if (_diagStream) {
            nbiotNetwork.setDiag(_diagStream);
        }
        
        if (_consoleStream) {
            nbiotNetwork.setConsoleStream(_consoleStream);
        }

        bool shouldUseR4 = (_networkType == NETWORK_TYPE_NBIOT_R4);
        
        return nbiotNetwork.init(modemStream, callback, messages, join, shouldUseR4);
    }
    case NETWORK_TYPE_LORA: {
        if (_diagStream) {
            loraNetwork.setDiag(_diagStream);
        }
        
        if (_consoleStream) {
            loraNetwork.setConsoleStream(_consoleStream);
        }
        
        return loraNetwork.init(modemStream, callback, getNow, messages, join);
    }
    case NETWORK_TYPE_2G_3G: {
        if (_diagStream) {
            network3G.setDiag(_diagStream);
        }
        
        if (_consoleStream) {
            network3G.setConsoleStream(_consoleStream);
        }
        
        return network3G.init(modemStream, callback, messages, join);
    }
    case NETWORK_TYPE_LTEM: {
        if (_diagStream) {
            lteNetwork.setDiag(_diagStream);
        }

        if (_consoleStream) {
            lteNetwork.setConsoleStream(_consoleStream);
        }

        return lteNetwork.init(modemStream, callback, messages, join);
    }
    default: {
        debugPrintLn("Unsupported network type");
        return false;
    }
    }
}

uint8_t Network::transmit(uint8_t * buffer, uint8_t size)
{
    switch (_networkType) {
    case NETWORK_TYPE_NBIOT_R4:
    case NETWORK_TYPE_NBIOT_N2: {
        return nbiotNetwork.transmit(buffer, size);
    }
    case NETWORK_TYPE_LORA: {
        return LoRa.transmit(buffer, size);
    }
    case NETWORK_TYPE_2G_3G: {
        return network3G.transmit(buffer, size);
    }
    case NETWORK_TYPE_LTEM: {
        return lteNetwork.transmit(buffer, size);
    return 0;
    }
    default: {
        debugPrintLn("Unsupported network type");
        return 0;
    }
    }
}

void Network::loopHandler()
{
    switch (_networkType) {
    case NETWORK_TYPE_NBIOT_R4:
    case NETWORK_TYPE_NBIOT_N2: {
        nbiotNetwork.loopHandler();
        break;
    }
    case NETWORK_TYPE_LORA: {
        LoRa.loopHandler();
        break;
    }
    case NETWORK_TYPE_2G_3G: {
        network3G.loopHandler();
        break;
    }
    case NETWORK_TYPE_LTEM: {
        lteNetwork.loopHandler();
        break;
    }
    default: {
        debugPrintLn("Unsupported network type");
        break;
    }
    }
}

void Network::sleep()
{
    switch (_networkType) {
        case NETWORK_TYPE_NBIOT_R4:
        case NETWORK_TYPE_NBIOT_N2: {
            nbiotNetwork.sleep();
            break;
        }
        case NETWORK_TYPE_LORA: {
            loraNetwork.sleep();
            break;
        }
        case NETWORK_TYPE_2G_3G: {
            network3G.sleep();
            break;
        }
        case NETWORK_TYPE_LTEM: {
            lteNetwork.sleep();
            break;
        }
        default: {
            debugPrintLn("Unsupported network type");
            break;
        }
    }
}

void Network::setActive(bool on)
{
    switch (_networkType) {
    case NETWORK_TYPE_NBIOT_R4:
    case NETWORK_TYPE_NBIOT_N2: {
        nbiotNetwork.setActive(on);
        break;
    }
    case NETWORK_TYPE_LORA: {
        loraNetwork.setActive(on);
        break;
    }
    case NETWORK_TYPE_2G_3G: {
        network3G.setActive(on);
        break;
    }
    case NETWORK_TYPE_LTEM: {
        lteNetwork.setActive(on);
        break;
    }
    default: {
        debugPrintLn("Unsupported network type");
        break;
    }
    }
}

uint32_t Network::getBaudRate()
{
    switch (_networkType) {
        case NETWORK_TYPE_NBIOT_R4:
        case NETWORK_TYPE_NBIOT_N2: {
            return nbiotNetwork.getBaudRate();
        }
        case NETWORK_TYPE_LORA: {
            return loraNetwork.getBaudRate();
        }
        case NETWORK_TYPE_2G_3G: {
            return network3G.getBaudRate();
        }
        case NETWORK_TYPE_LTEM: {
            return lteNetwork.getBaudRate();
        }
        default: {
            debugPrintLn("Unsupported network type");
            break;
        }
    }

    return 0;
}

void Network::setNetworkType(NetworkType type)
{
    _networkType = type;
}

LoraNetwork & Network::getLoraNetwork()
{
    return loraNetwork;
}

const char* Network::getIMEI()
{
#if defined(ARDUINO_SODAQ_SFF) || defined(ARDUINO_SODAQ_SARA)
    if (!isImeiInitialized) {
        char tmpBuffer[16];
        bool success = false;

        switch (_networkType) {
            case NETWORK_TYPE_NBIOT_R4:
            case NETWORK_TYPE_NBIOT_N2: {
                success = nbiotNetwork.getIMEI(tmpBuffer, sizeof(tmpBuffer));
                break;
            }
            case NETWORK_TYPE_LORA: {
                break;
            }
            case NETWORK_TYPE_2G_3G: {
                success = network3G.getIMEI(tmpBuffer, sizeof(tmpBuffer));
                break;
            }
            case NETWORK_TYPE_LTEM: {
                success = lteNetwork.getIMEI(tmpBuffer, sizeof(tmpBuffer));
                break;
            }
            default: {
                debugPrintLn("Unsupported network type");
                break;
            }
        }

        if (success) {
            strncpy(cachedImei, tmpBuffer, sizeof(cachedImei));
            isImeiInitialized = true;
        }
    }

    return cachedImei;
#else
    return "NA";
#endif
}

const char* Network::getModuleVersion()
{
#if defined(ARDUINO_SODAQ_SFF) || defined(ARDUINO_SODAQ_SARA)
    if (!isModuleVersionInitialized) {
        char tmpBuffer[50];
        bool success = false;

        switch (_networkType) {
        case NETWORK_TYPE_NBIOT_R4:
        case NETWORK_TYPE_NBIOT_N2: {
            success = nbiotNetwork.getModuleVersion(tmpBuffer, sizeof(tmpBuffer));
            break;
        }
        case NETWORK_TYPE_LORA: {
            break;
        }
        case NETWORK_TYPE_2G_3G: {
            // TODO success = network3G.getVersion(tmpBuffer, sizeof(tmpBuffer));
            break;
        }
        case NETWORK_TYPE_LTEM: {
            success = lteNetwork.getModuleVersion(tmpBuffer, sizeof(tmpBuffer));
            break;
        }
        default: {
            debugPrintLn("Unsupported network type");
            break;
        }
        }

        if (success) {
            strncpy(cachedModuleVersion, tmpBuffer, sizeof(cachedModuleVersion));
            isModuleVersionInitialized = true;
        }
    }

    return cachedModuleVersion;
#else
    return "NA";
#endif
}