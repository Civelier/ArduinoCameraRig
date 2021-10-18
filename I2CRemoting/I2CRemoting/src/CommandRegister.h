#ifndef _CommandRegister_h
#define _CommandRegister_h

#if defined(ARDUINO) && ARDUINO >= 100

#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "IRemotingCommand.h"
#include "DebugTools.h"
#include <Wire.h>

#define I2C_SPEED_MAX 400000
#define I2C_SPEED_DEFAULT 100000

#ifdef ARDUINO_ARCH_MEGAAVR
#include "DebugTools.h"
#endif

struct DemoCommandData
{
	int X;
};

class DemoCommand : public RemotingCommandData<DemoCommandData>
{};

inline constexpr uint8_t operator "" _uc(unsigned long long arg) noexcept
{
	return static_cast<uint8_t>(arg);
}
class I2CReceiver;
class CommandRegister : public ICmdSender
{
private:
	IRemotingCommand** m_commands;
	uint8_t m_cmdCount = 0;
	RawPacket_t m_packet;
	uint8_t m_clientAddress = 255;
	TwoWire* m_wire;
	uint32_t m_intPin;
private:
	void SetInterrupt(bool state);
	bool GetInterrupt();
	void ManageCommand(RawPacket_t packet);
	template <typename T>
	void SendCommand(const DataPacket<T>& packet);
	uint8_t TryReadData(RawPacket_t buff, uint8_t count, uint32_t timeout = 200);
public:
	CommandRegister();
	virtual void SendCommand(RawPacket_t packet) override;
	void SetupMaster(uint32_t interruptPin = 2, TwoWire& wire = Wire, uint8_t clientAddress = 0x6F_uc, uint32_t clock = I2C_SPEED_MAX);
	void SetupClient(uint32_t interruptPin = 4, TwoWire& wire = Wire, uint8_t clientAddress = 0x6F_uc, uint32_t clock = I2C_SPEED_MAX);
	void AllocateCommands(uint8_t count);
	template <typename TCommand>
	const TCommand* const RegisterCommand();
	template <typename TValue>
	ValueRemotingCommand<TValue>* RegisterProperty();
	bool IsMaster();
	bool IsClient();
	/// <summary>
	/// Call this from master to initiate transmission to client if client needs to send data.
	/// </summary>
	void MasterRefresh();
	friend IRemotingCommand;
	friend I2CReceiver;
};



template<typename TCommand>
inline const TCommand* const CommandRegister::RegisterCommand() 
{
	TCommand* cmd = new TCommand();
	cmd->m_stream = m_wire;
	cmd->m_isMaster = IsMaster();
	cmd->m_sender = this;
	cmd->m_commandID = m_cmdCount + 1;
	m_commands[m_cmdCount] = cmd;
	m_cmdCount++;
	return cmd;
}

template<typename TValue>
inline ValueRemotingCommand<TValue>* CommandRegister::RegisterProperty()
{
	ValueRemotingCommand<TValue>* prop = new ValueRemotingCommand<TValue>();
	prop->m_stream = m_wire;
	prop->m_isMaster = IsMaster();
	prop->m_sender = this;
	prop->m_commandID = m_cmdCount + 1;
	m_commands[m_cmdCount] = prop;
	m_cmdCount++;
	return prop;
}

template<typename T>
inline void CommandRegister::SendCommand(const DataPacket<T>& packet)
{
	DebugToolsFunctionBeginNoWarn();
	if (IsMaster())
	{
		m_wire->beginTransmission(m_clientAddress);
		m_wire->write(packet.Raw + 1, packet.Size);
		m_wire->endTransmission();
	}
	else
	{
		m_packet = { packet.Raw };
		SetInterrupt(HIGH);
	}
}

extern CommandRegister CmdReg;

#endif // !_CommandRegister_h