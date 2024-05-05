#include "TcpSlave.h"
#include "Enumerations.h"
#include "Log.h"

namespace ModbusAdapter
{
#ifndef __bswap_16
#define __bswap_16(num) (((uint16_t)num >> 8) | ((uint16_t)num << 8))
#endif

	TcpSlave::TcpSlave()
	{
	}
	void TcpSlave::init(long baudRate, uint32_t config, long tcpPort, uint8_t mosbusAddress)
	{
		_slavePort = tcpPort;
		_rtuMaster.Init(baudRate, config, mosbusAddress);
		_pServer = new WiFiServer(tcpPort);
		_pServer->begin();
		_rtuResultCode = EX_SLAVE_FAILURE;
	}

	void TcpSlave::close()
	{
		_pServer->close();
	}

	void TcpSlave::cbResponse(ResultCode res, uint8_t *requestFrame, uint8_t requestFrameLen, uint8_t *responseFrame, uint16_t responseFrameLen)
	{
		_rtuResultCode = res;
		if (res == EX_SUCCESS)
		{
			Transaction *trans = searchTransaction(requestFrame);
			if (trans)
			{
				uint8_t *mem = (uint8_t *)malloc(responseFrameLen);
				if (mem)
				{
					free(trans->_responseFrame);
					trans->_responseFrame = mem;
					memcpy(trans->_responseFrame, responseFrame, responseFrameLen);
					trans->_responseFrameLen = responseFrameLen;
					trans->timestamp = millis();
				}
			}
			else
			{
				Transaction newEntry;
				newEntry.timestamp = millis();
				uint8_t *memReq = (uint8_t *)malloc(requestFrameLen);
				if (memReq)
				{
					newEntry._requestFrame = memReq;
					memcpy(newEntry._requestFrame, requestFrame, requestFrameLen);
					newEntry._requestFrameLen = requestFrameLen;
					uint8_t *memResp = (uint8_t *)malloc(responseFrameLen);
					if (memResp)
					{
						newEntry._responseFrame = memResp;
						memcpy(newEntry._responseFrame, responseFrame, responseFrameLen);
						newEntry._responseFrameLen = responseFrameLen;
						_trans.push_back(newEntry);
					}
					else
					{
						free(memReq);
					}
				}
			}
		}
		return;
	}

	void TcpSlave::run()
	{
		MBAP_t _MBAP;
		cleanup();
		if (_pServer)
		{
			while (_pServer->hasClient())
			{
				WiFiClient *currentClient = new WiFiClient(_pServer->available());
				if (!currentClient || !currentClient->connected())
					continue;
				_clientIndex = getFreeClient();
				if (_clientIndex > -1)
				{
					_pClients[_clientIndex] = currentClient;
					continue;
				}
				// Close connection if MODBUSIP_MAX_CLIENTS reached
				delete currentClient;
			}
		}
		for (_clientIndex = 0; _clientIndex < MODBUSIP_MAX_CLIENTS; _clientIndex++)
		{
			if (!_pClients[_clientIndex])
				continue;
			if (!_pClients[_clientIndex]->connected())
				continue;
			uint32_t readStart = millis();
			while (millis() - readStart < MODBUSIP_MAX_READMS && _pClients[_clientIndex]->available() > sizeof(_MBAP))
			{
				_pClients[_clientIndex]->readBytes(_MBAP.raw, sizeof(_MBAP.raw)); // Get MBAP

				if (__bswap_16(_MBAP.protocolId) != 0)
				{												 // Check if MODBUSIP packet. __bswap is usless there.
					while (_pClients[_clientIndex]->available()) // Drop all incoming if wrong packet
						_pClients[_clientIndex]->read();
					continue;
				}
				_len = __bswap_16(_MBAP.length);
				_len--; // Do not count with last byte from MBAP
				if (_len > MODBUSIP_MAXFRAME)
				{																			   // Length is over MODBUSIP_MAXFRAME
					_len--;																	   // Subtract for read byte
					for (uint8_t i = 0; _pClients[_clientIndex]->available() && i < _len; i++) // Drop rest of packet
						_pClients[_clientIndex]->read();
				}
				else
				{
					free(_frame);
					_frame = (uint8_t *)malloc(_len);
					if (!_frame)
					{
						exceptionResponse((FunctionCode)_pClients[_clientIndex]->read(), EX_SLAVE_FAILURE);
						for (uint8_t i = 0; _pClients[_clientIndex]->available() && i < _len; i++) // Drop packet
							_pClients[_clientIndex]->read();
					}
					else
					{
						if (_pClients[_clientIndex]->readBytes(_frame, _len) < _len)
						{ // Try to read MODBUS frame
							exceptionResponse((FunctionCode)_frame[0], EX_ILLEGAL_VALUE);
						}
						else
						{
							if (_pClients[_clientIndex]->localPort() == _slavePort)
							{
								if (!SupportedFunction(_frame[0]))
								{
									exceptionResponse((FunctionCode)_frame[0], EX_ILLEGAL_FUNCTION);
								}
								else
								{
									// forward incoming frame to RTU master
									_rtuMaster.Transfer(_frame, _len, this);
									_rtuMaster.Run(); // read RTU responses
									Transaction *trans = searchTransaction(_frame);
									if (trans)
									{
										uint8_t *mem = (uint8_t *)malloc(trans->_responseFrameLen);
										if (mem)
										{
											_len = trans->_responseFrameLen;
											free(_frame);
											_frame = mem;
											memcpy(_frame, trans->_responseFrame, _len);
										}
									}
									else
									{
										exceptionResponse((FunctionCode)_frame[0], EX_TIMEOUT);
									}
								}
							}
						}
					}
				}
				if (_pClients[_clientIndex]->localPort() == _slavePort)
				{
					_MBAP.length = __bswap_16((_len + 1)); // _len+1 for last byte from MBAP
					size_t send_len = (uint16_t)_len + sizeof(_MBAP.raw);
					uint8_t sbuf[send_len];
					memcpy(sbuf, _MBAP.raw, sizeof(_MBAP.raw));
					memcpy(sbuf + sizeof(_MBAP.raw), _frame, _len);
					_pClients[_clientIndex]->write(sbuf, send_len);
					_pClients[_clientIndex]->flush();
				}
				if (_frame)
				{
					free(_frame);
					_frame = nullptr;
				}
				_len = 0;
			}
		}
		_clientIndex = -1;
	}

	void TcpSlave::exceptionResponse(FunctionCode fn, ResultCode excode)
	{
		free(_frame);
		_len = 2;
		_frame = (uint8_t *)malloc(_len);
		_frame[0] = fn + 0x80;
		_frame[1] = excode;
	}

	int8_t TcpSlave::getFreeClient()
	{
		for (uint8_t i = 0; i < MODBUSIP_MAX_CLIENTS; i++)
			if (!_pClients[i])
				return i;
		return -1;
	}

	void TcpSlave::cleanup()
	{
		// Free clients if not connected
		for (uint8_t i = 0; i < MODBUSIP_MAX_CLIENTS; i++)
		{
			if (_pClients[i] && !_pClients[i]->connected())
			{
				delete _pClients[i];
				_pClients[i] = nullptr;
			}
		}
		// remove old transactions
		for (auto it = _trans.begin(); it != _trans.end();)
		{
			if (millis() - it->timestamp > TRANSACTION_LIFESPAN)
			{
				free(it->_requestFrame);
				free(it->_responseFrame);
				it = _trans.erase(it);
			}
			else
				it++;
		}
	}
	Transaction *TcpSlave::searchTransaction(uint8_t *frame)
	{
		std::vector<Transaction>::iterator it = std::find_if(_trans.begin(), _trans.end(), [frame](Transaction &trans) { return memcmp(trans._requestFrame, frame, trans._requestFrameLen) == 0; });
		if (it != _trans.end())
			return &*it;
		return nullptr;
	}

	boolean TcpSlave::SupportedFunction(uint8_t fcode)
	{
		bool rVal = false;
		switch (fcode)
		{
		case FC_READ_REGS:
		case FC_READ_COILS:
		case FC_READ_INPUT_STAT:
		case FC_READ_INPUT_REGS:
		case FC_MEI:
			rVal = true;
			break;

		default:
			rVal = false;
		}
		return rVal;
	}
} // namespace ModbusAdapter