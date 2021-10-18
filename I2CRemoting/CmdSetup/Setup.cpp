#include "Setup.h"

ValueRemotingCommand<int32_t>* TestInt = nullptr;
ValueRemotingCommand<float>* TestFloat = nullptr;

void CmdSetup()
{
	CmdReg.AllocateCommands(2);
	TestInt = CmdReg.RegisterProperty<int32_t>();
	TestInt->AttachChangedCallback(OnTestIntChanged);
	TestFloat = CmdReg.RegisterProperty<float>();
	TestFloat->AttachChangedCallback(OnTestFloatChanged);
}
