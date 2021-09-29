# Arduino Camera Rig
## Introduction
This is an ongoing project to make a motion controlled camera rig. There is an extension to this project: [Open Camera Rig](https://github.com/Civelier/OpenCameraRig).

## Components
- Arduino Due
- NEMA stepper motors (I'm using [these](https://www.amazon.ca/gp/product/B081R33M5N/ref=ppx_od_dt_b_asin_title_s00?ie=UTF8&psc=1))
- Stepper motor drivers (I'm using [TMC2208](https://www.amazon.ca/gp/product/B07TVNB861/ref=ppx_yo_dt_b_asin_title_o01_s00?ie=UTF8&psc=1))
- A 12V power supply
- Jumper cables
- Capacitor (Rated for at least 20V, I'm using 47uF) [IMPORTANT]
- Kill switch [Optional]
- 3V3 to 5V level converters (because the Due runs on 3,3V and the drivers work on 5V like [these](https://www.amazon.ca/gp/product/B07LG646VS/ref=ppx_yo_dt_b_asin_image_o06_s01?ie=UTF8&psc=1))
- Breadboard

## Assembling
Coming soon...

## Sections
This project is split in multiple sections:
### Blender script ([BlenderScript](https://github.com/Civelier/ArduinoCameraRig/tree/main/BlenderScript))
The blender script is made to be used with the [Blender](https://www.blender.org/) app. It takes a camera animation curve (or any animation curve) and exports the keyframes in a text file.
#### Usage
1. Open a new blender project.
2. Go in the Scripting tab.
3. Click on open and browse to the ExportCamMotion.py file.
4. After having made an animation, run the script. For first time run, you need to start it from the scripting tab. After, you will find it in the toolbar File -> Export -> Export Camera motion.
5. In the widow that opens, you'll find a dropdown box to select the animation curve you want to export. If you are unsure of what the name of the animation is, open a dope sheet editor (in one of the docked panels) and select the object that the animation is assigned to. You'll see the name. It should look something like <CubeAction> or <CameraAction.001>.
6. Import the script in the computer interface (see [Sending the animation](https://github.com/Civelier/ArduinoCameraRig/tree/main#sending-the-animation)).


### Computer interface ([CameraRigController](https://github.com/Civelier/ArduinoCameraRig/tree/main/ControllerProject/CameraRigController))
This section is about the computer interface. The goal of the interface is to interact with the exported blender animation and the main Arduino.
#### Configuration
There aren't any way as of now to save a custom configuration file. But changes to the configuration will be automatically saved to a config file so changes are saved. In the config tabs, you will see multiple parameters:

Motor channel name: Decorative name (has no impact on the actual information sent to Arduino).

Motor channel ID: This is the ID of the physical motor channel on the Arduino.

Animation Channel ID: This is the blender animation index. In a case where only a rotation was exported, IDs 0, 1 and 2 would respectively match X rotation, Y rotation and Z rotation.

Steps per revolution: This is the physical amount of steps per revolutions the stepper motor has (the vast majority of the time this will be 200, but it depends on the motor).

#### Sending the animation
First of all, you'll have to import the file exported from blender (File -> Open). Then connect the Arduino via USB. In Connection -> Ports, you should now see a list of ports (normally, there should be one, if there are more, unplug the Arduino and note the name of the port that disappears in the list). Select the Arduino's port and click on Play. The Arduino will then receive the animation data and start playing it back.

### Main Arduino program ([CamerArduinoRig](https://github.com/Civelier/ArduinoCameraRig/tree/main/ArduinoProject/CamerArduinoRig))
The Arduino I'm using is the Due because of its high frequency (80 MHz) and SRAM capacity (96 KB). The information getting transmitted to the Arduino is directly the keyframes from Blender but scaled for the amounts of steps per revolutions of the motor (see [Configuration](https://github.com/Civelier/ArduinoCameraRig/tree/main#configuration)). The Arduino computes an easing curve based on the specific moment the instruction is called and interpolates if a step is required.

#### Debugging
Connect the Arduino via USB and open a serial port.

Commands take the form of space separated values.
```
<Command ID> [<Parameter 1> <Parameter 2> ...]
```

Commands:
##### Get status:
Command ID: 1\
No parameters.\
Behavior :\
Requests the Arduino to send the current status. The Arduino will send one of the following values:
```cpp
enum class StatusCode : uint16_t
{
	Ready = 1,
	Running = 2,
	Done = 3,
	Debug = 4,
	ReadyForInstruction = 5,
	Value = 6,
	DebugBlockBegin = 7,
	DebugBlockEnd = 8,
	Error = 0b10000000,
	MotorChannelOutOfRangeError = Error | 1,
};
```
```Ready``` indicates the Arduino is in standby, waiting for instructions.\
```Running``` indicates the Arduino is currently playing back an animation.\
```Done``` will be sent once the playback is complete.\
```Debug``` indicates the next line sent through the stream will be meant as a debug line of text.\
```ReadyForInstruction``` is obsolete.\
```Value``` indicates the next line will be a value (coming from a request).\
```DebugBlockBegin``` indicates the start of a debug block.\
```DebugBlockEnd``` indicates the end of the debug block.\
```Error``` is a bit flag used to determine if the code is an error.\
```MotorChannelOutOfRangeError``` indicates the requested motor channel ID (see [Configuration](https://github.com/Civelier/ArduinoCameraRig/tree/main#configuration)) is out of range. This can happen as a result of a bug or as an invalid keyframe (see [Send Keyframe]).

##### Error clear:
Command ID: 2\
No parameters.\
Behavior:\
Unused.

##### Motor reset:
Usage:\
```3 <Motor 0 offset> <Motor 1 offset> ...```\
Command ID: 3\
Parameters:\
List of offset in steps of every motor.\
Behavior:\
Unused.

##### Start request:
Command ID: 4\
No parameters.\
Behavior:\
Instructs the Arduino to start playback.

##### Request sync micros:
Command ID: 5\
No parameters.\
Behavior:\
In order to ensure a consistent precision during playback, a "clock" is used. When started, it will give the time relative to its starting point in microseconds. The Arduino will reply using the ```Debug``` status code (see [Get status]()) followed by the time in us since the start of the sync clock on the next line.

##### Request start sync:
Command ID: 6\
No parameters.\
Behavior:\
Forces the start of the time sync clock.

##### Send keyframe:
Usage:\
```7 <Motor channel ID> <Millisecond> <Steps>```\
Command ID: 7\
Parameters:\
Motor channel ID: The channel this keyframe applies to.\
Millisecond: The position in milliseconds of the keyframe.\
Steps: The step at which the motor should be at <Millisecond>.\


### Secondary Arduino for Android communication and remote control (Future)
### Android development ([Open Camera Rig](https://github.com/Civelier/OpenCameraRig))
