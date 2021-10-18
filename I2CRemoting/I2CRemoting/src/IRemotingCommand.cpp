    #include "IRemotingCommand.h"

uint8_t CurrentID = 1;

uint8_t ReadSize(RawPacket_t packet)
{
    return packet[0];
}

uint8_t ReadCommandID(RawPacket_t packet)
{
    return packet[1];
}

uint8_t IDGen()
{
    return CurrentID++;
}

uint8_t IRemotingCommand::GetTypeID() const
{
    return m_typeID;
}
