#ifndef _IRemotingCommand_h
#define _IRemotingCommand_h

#include "RemotingUtillities.h"
#ifdef ARDUINO_ARCH_MEGAAVR
#include <ArduinoSTL.h>
#else
#include <algorithm>
#endif
#include <Wire.h>

typedef uint8_t* RawPacket_t;

class IRemotingCommand;
template <typename T>
class ValueRemotingCommand;
template<typename T>
class RemotingCommandData;

template<typename T>
union DataPacket
{
	struct Converted
	{
		uint8_t Size;
		uint8_t CommandID;
//#ifndef ARDUINO_SAM_DUE
		uint16_t Filler;
//#endif // 

		T Data;
	};
	Converted CommandInfo;
	uint8_t Raw[sizeof(Converted)];
};

uint8_t ReadSize(RawPacket_t packet);
uint8_t ReadCommandID(RawPacket_t packet);
template <typename T>
uint8_t* ReadData(RawPacket_t packet);
template <typename T>
T* ReadValue(RawPacket_t packet);

class ICmdSender
{
public:
	virtual void SendCommand(const RawPacket_t packet) = 0;
};
class CommandRegister;

class IRemotingCommand
{
protected:
	uint8_t m_commandID = 0;
	uint8_t m_typeID;
	TwoWire* m_stream;
	ICmdSender* m_sender;
	
	bool m_isMaster;
	virtual void Decode(RawPacket_t packet) {};
	virtual bool IsSameCommand(IRemotingCommand* other) const { return false; }
	void Send(RawPacket_t packet) { m_sender->SendCommand(packet); }
	IRemotingCommand() {}
public:
	uint8_t GetTypeID() const;
	friend CommandRegister;
};

uint8_t IDGen();
template <typename T>
uint8_t IDGenTemp();

extern uint8_t CurrentID;

template<typename T>
inline uint8_t* ReadData(RawPacket_t packet)
{
	auto address = ((ReadSize(packet) + 1) - sizeof(T));
	DebugValue(address);
	return packet + address;
}

template<typename T>
inline T* ReadValue(RawPacket_t packet)
{
	T* value = reinterpret_cast<T*>(ReadData<T>(packet));
	return value;
}

template<typename T>
inline uint8_t IDGenTemp()
{
	static uint8_t id = 0;
	if (id == 0) id = IDGen();
	return id;
}

template <typename T>
class ValueRemotingCommand : public IRemotingCommand
{
public:
	struct ValueChangedEventArgs
	{
		const T& OldValue;
		const T& NewValue;
		const Endpoint& Source;
	};

	using ValueChangedCallback_t = void (*)(ValueRemotingCommand<T>& sender, const ValueChangedEventArgs& args);
private:
	ValueChangedCallback_t m_changed = nullptr;
	T m_value;
protected:
	bool Equals(const T& v1, const T& v2) const;
	virtual void Decode(RawPacket_t packet) override;
	virtual bool IsSameCommand(IRemotingCommand* other) const override;
	void OnChanged(const T& newValue, const Endpoint& source);
public:
	ValueRemotingCommand();
	void AttachChangedCallback(ValueChangedCallback_t cb);
	const T& GetValue() const;
	void SetValue(T value);
};

template<typename T>
class RemotingCommandData : public IRemotingCommand
{
public:
	struct RecievedEventArgs
	{
		const T& Data;
	};

	using RecievedCallback_t = void(*)(RemotingCommandData<T> sender, RecievedEventArgs args);
private:
	RecievedCallback_t m_recievedCB = nullptr;
protected:
	virtual void Decode(RawPacket_t packet) override;
	virtual bool IsSameCommand(IRemotingCommand* other) const override;
public:
	RemotingCommandData();
	void AttachRecievedCallback(RecievedCallback_t cb);
};

template<typename T>
inline RemotingCommandData<T>::RemotingCommandData()
{
	m_typeID = IDGen();
}

template<typename T>
inline void RemotingCommandData<T>::Decode(RawPacket_t packet)
{
	DebugToolsFunctionBegin();
	m_recievedCB(*this, ReadValue<T>(packet));
}

template<typename T>
inline bool RemotingCommandData<T>::IsSameCommand(IRemotingCommand* other) const
{
	return GetTypeID() == other->GetTypeID();
}

template<typename T>
inline void RemotingCommandData<T>::AttachRecievedCallback(RecievedCallback_t cb)
{
	m_recievedCB = cb;
}

template<typename T>
inline bool ValueRemotingCommand<T>::Equals(const T& v1, const T& v2) const
{
	const uint8_t* b1 = reinterpret_cast<const uint8_t*>(&v1);
	const uint8_t* b2 = reinterpret_cast<const uint8_t*>(&v2);
	
	return std::equal(b1, b1 + sizeof(T), b2);
}

template<typename T>
inline void ValueRemotingCommand<T>::Decode(RawPacket_t packet)
{
	DebugToolsFunctionBegin();
	T* value = ReadValue<T>(packet);
	OnChanged(*value, Endpoint::Remote);
}

template<typename T>
inline bool ValueRemotingCommand<T>::IsSameCommand(IRemotingCommand* other) const
{
	return GetTypeID() == other->GetTypeID();
}

template<typename T>
inline void ValueRemotingCommand<T>::OnChanged(const T& newValue, const Endpoint& source)
{
	DebugToolsFunctionBegin();
	if (Equals(newValue, m_value)) return;
	if (!m_changed) return;
	const T old { m_value };
	m_changed(*this, { old, newValue, source });
}

template<typename T>
inline ValueRemotingCommand<T>::ValueRemotingCommand()
{
	m_typeID = IDGen();
}

template<typename T>
inline void ValueRemotingCommand<T>::AttachChangedCallback(ValueChangedCallback_t cb)
{
	m_changed = cb;
}

template<typename T>
inline const T& ValueRemotingCommand<T>::GetValue() const
{
	return m_value;
}

template<typename T>
inline void ValueRemotingCommand<T>::SetValue(T value)
{
	DebugToolsFunctionBegin();
	DataPacket<T> x = { sizeof(DataPacket<T>) - 1, m_commandID, 0, value };
	DebugMemAddress(&x);
	DebugMemAddress(&(x.CommandInfo.Size));
	DebugMemAddress(x.Raw);
	DebugValue(x.CommandInfo.Size);
	DebugValue(x.CommandInfo.CommandID);
	DebugValue(x.CommandInfo.Data);
	DebugValue(x.Raw[0]);
	DebugValue(x.Raw[1]);
	Send(x.Raw);
	OnChanged(value, Endpoint::Local);
}

#endif // !_IRemotingCommand_h