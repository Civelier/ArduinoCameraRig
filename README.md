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
6. Import the script in the computer interface (see next section).


### Computer interface ([CameraRigController](https://github.com/Civelier/ArduinoCameraRig/tree/main/ControllerProject/CameraRigController))
This section is about the computer interface. The goal of the interface is to interact with the exported blender animation and the main Arduino.
#### Configuration
There aren't any way as of now to save a custom configuration file. But changes to the configuration will be automatically saved to a config file so changes are saved. In the config tabs, you will see multiple parameters:

Motor channel name : Decorative name (has no impact on the actual information sent to Arduino).

Motor channel ID : This is the ID of the physical motor channel on the Arduino.

Animation Channel ID : This is the blender animation index. In a case where only a rotation was exported, IDs 0, 1 and 2 would respectively match X rotation, Y rotation and Z rotation.

Steps per revolution : This is the physical amount of steps per revolutions the stepper motor has (the vast majority of the time this will be 200, but it depends on the motor).

#### Sending the animation
First of all, you'll have to import the file exported from blender (File -> Open). Then connect the Arduino via USB. In Connection -> Ports, you should now see a list of ports (normally, there should be one, if there are more, unplug the Arduino and note the name of the port that disappears in the list). Select the Arduino's port and click on Play. The Arduino will then receive the animation data and start playing it back.

### Main Arduino program ([CamerArduinoRig](https://github.com/Civelier/ArduinoCameraRig/tree/main/ArduinoProject/CamerArduinoRig))
Coming soon.

### Secondary Arduino for Android communication and remote control (Future)
### Android development ([Open Camera Rig](https://github.com/Civelier/OpenCameraRig))
