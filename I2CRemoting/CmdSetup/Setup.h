#ifndef _Setup_h
#define _Setup_h

#include "CommandRegister.h"


extern ValueRemotingCommand<int32_t>* TestInt;
using IntProp_t = ValueRemotingCommand<int32_t>;
using IntEventArgs_t = IntProp_t::ValueChangedEventArgs;
void OnTestIntChanged(IntProp_t& sender, const IntEventArgs_t& args);

extern ValueRemotingCommand<float>* TestFloat;
using FloatProp_t = ValueRemotingCommand<float>;
using FloatEventArgs_t = FloatProp_t::ValueChangedEventArgs;
void OnTestFloatChanged(FloatProp_t& sender, const FloatEventArgs_t& args);


void CmdSetup();

#endif // !_Setup_h

