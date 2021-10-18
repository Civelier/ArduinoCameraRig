#include "CommandRegister.h"
#include "RemotingUtillities.h"

CommandRegister CmdReg; 

class I2CReceiver
{
private:
	int m_available = -1;
	bool m_isWaitingForSend = false;
	RawPacket_t m_packet;
public:
	// Data available to send
	int AvailableToSend()
	{
		DebugToolsFunctionBeginNoWarn();
		return m_available;
	}
	void SetPacket(RawPacket_t packet)
	{
		DebugToolsFunctionBeginNoWarn();
		m_packet = packet;
		DebugMemAddress(packet);
		m_available = ReadSize(packet);
		DebugValue(m_available);
		PrintBuffer(m_packet, m_available + 1);
		CmdReg.SetInterrupt(HIGH);
	}
	void Send()
	{
		DebugToolsFunctionBeginAlloc(-(ReadSize(m_packet) + 1));
		CmdReg.m_wire->write(m_packet, m_available + 1);
		DeclareDeleteArr(uint8_t, ReadSize(m_packet) + 1) m_packet;
		m_available = -1;
		CmdReg.SetInterrupt(LOW);
	}
	void ClientReceiveCommand(int count)
	{
		DebugToolsFunctionBegin();
		RawPacket_t buff = DeclareNewArr(uint8_t, count + 1);
		buff[0] = count;
		CmdReg.m_wire->readBytes(buff + 1, count);
		PrintBuffer(buff, count);
		CmdReg.ManageCommand(buff);
		DeclareDeleteArr(uint8_t, ReadSize(m_packet) + 1) m_packet;
	}
	friend CommandRegister;
};

I2CReceiver Receiver;

void CommandRegister::SetInterrupt(bool state)
{
	digitalWrite(m_intPin, state);
}

bool CommandRegister::GetInterrupt()
{
	return digitalRead(m_intPin);
}

void CommandRegister::ManageCommand(RawPacket_t packet)
{
	DebugToolsFunctionBegin();
	if (ReadCommandID(packet) == 0 || ReadCommandID(packet) > m_cmdCount)
	{
		DebugError("Command ID out of range.");
		Serial.print("ID: ");
		Serial.println(ReadCommandID(packet));
		return;
	}
	m_commands[ReadCommandID(packet) - 1]->Decode(packet);
}

CommandRegister::CommandRegister()
{
}

void CommandRegister::SetupMaster(uint32_t interruptPin, TwoWire& wire, uint8_t clientAddress, uint32_t clock)
{
	DebugToolsFunctionBegin();
	m_wire = &wire;
	m_clientAddress = clientAddress;
	m_intPin = interruptPin;
	m_wire->begin();
	m_wire->setClock(clock);
	pinMode(m_intPin, INPUT);
}

void onReceiveClient(int howMany)
{
	Receiver.ClientReceiveCommand(howMany);
}

void onRequestClient()
{
	Receiver.Send();
}

void CommandRegister::SetupClient(uint32_t interruptPin, TwoWire& wire, uint8_t clientAddress, uint32_t clock)
{
	DebugToolsFunctionBegin();
	m_wire = &wire;
	m_clientAddress = 254;
	m_intPin = interruptPin;
	m_wire->begin(clientAddress);
	m_wire->setClock(clock);
	m_wire->onReceive(onReceiveClient);
	m_wire->onRequest(onRequestClient);
	pinMode(m_intPin, OUTPUT);
	SetInterrupt(LOW);
}

void CommandRegister::SendCommand(RawPacket_t packet)
{
	DebugToolsFunctionBegin();
	if (IsMaster())
	{
		DebugMemAddress(packet);

		DebugValue(ReadSize(packet));
		DebugValue(ReadCommandID(packet));

		Serial.println("Master sending");
		Serial.flush();
		m_wire->beginTransmission(m_clientAddress);
		Serial.println("Transmission begin");
		for (RawPacket_t iter = packet; iter < packet + ReadSize(packet) + 1; iter++)
		{
			Serial.print(static_cast<int>(*iter));
			Serial.print(' ');
		}
		Serial.println();
		Serial.flush();
		auto sent = m_wire->write(packet + 1, ReadSize(packet));
		DebugValue(sent);
		auto code = m_wire->endTransmission();
		DebugValue(code);
		Serial.println("Master sent");
		
	}
	else
	{
		m_packet = packet;
		Receiver.SetPacket(m_packet);
	}
}

void CommandRegister::AllocateCommands(uint8_t count)
{
	m_commands = new IRemotingCommand*[sizeof(IRemotingCommand**) * (size_t)count];
}

bool CommandRegister::IsMaster()
{
	return m_clientAddress != 255 && m_clientAddress != 254;
}

bool CommandRegister::IsClient()
{
	return m_clientAddress == 254;
}

uint8_t CommandRegister::TryReadData(RawPacket_t buff, uint8_t count, uint32_t timeout)
{
	DebugToolsFunctionBegin();
	uint8_t received = 0;
	while (m_wire->available())
	{
		if (received >= count) return received;
		(*buff) = m_wire->read();
		buff++;
		received++;
	}
	return received;
}

void CommandRegister::MasterRefresh()
{
	DebugToolsFunctionBegin();
	if (IsMaster()) // Should only be used by master
	{
		if (GetInterrupt()) // Is client waiting to send data
		{
			Serial.println("Client requesting to send");
			
			m_wire->requestFrom(m_clientAddress, 32_uc);
			
			int count = m_wire->read();

			if (!count)
			{
				DebugError("No data available");
				return;
			}
			DebugValue(count);

			// Allocate the buffer for the command plus the first byte for the size
			RawPacket_t buff = DeclareNew(uint8_t, count + 1);

			// Set the first byte to the size
			buff[0] = count;

			// Read the data (after the size byte)
			size_t bytesRead = m_wire->readBytes(buff + 1, count);
			
			Serial.print("Received ");
			Serial.print(bytesRead);
			Serial.println(":");
			PrintBuffer(buff, bytesRead + 1);

			if (bytesRead < count)
			{
				DebugError("Not enough bytes read!");
				goto end;
			}

			// Handle the incomming command 
			ManageCommand(buff);

			// After ManageCommand, references to buff should be released.
			// We can now free the buffer
			end:
			DeclareDeleteArr(uint8_t, count + 1) buff;
		}
	}
}
