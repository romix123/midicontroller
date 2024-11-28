A Simple midicontroller based on beaatrmatrix mk2 hardware, 
see https://github.com/yobalint/beatrmatrix-mk2-arduino-based-ableton-controller

This version sends out serial midi CC codes (and receives them). On Mac or PC run A.V Perotta's serialToMidi app to
convert serialmidi to IAC driver midi that can be handled by any DAW.

On startup the module needs to be allocated to a midi channel. The controller will flash led 1 twice on startup. 
Press the desired channel to lock to that channel (to do make the choce persistent).

The pots operate CC 120, 12, 122 [0..127]
the mode, length, launch buttons send 127, followed by 0 on release on CC 116, 117, 118  – t u v 
The 16 switches produce 127 on first press, 0 on second (toggle). A green led designates the state of the toggle.

On PC or Mac run the app available on https://github.com/avperrotta/serialToMidi
![Scherm­afbeelding 2024-11-28 om 14 27 14](https://github.com/user-attachments/assets/33a88a1c-43b1-4c42-ac2d-0ad855f482d8)

Also (on Mac) create an IAC driver instance in 'Audio/midi configuratie'. Open the 'Midi-studio' window, open the IAC driver
Create one or more ports for the interface (calling them, for instance, serialmidi) and assigning 1 or more input/ouptut ports.
![Scherm­afbeelding 2024-11-28 om 14 27 28](https://github.com/user-attachments/assets/3dc09d2c-993c-43b8-a751-ce3a63ecf8ea)


Once you have the IAC driver configured, you can run the serialToMidi app. Select a serialport (on a Mac it will be called usbserial-xxx or wchusbserial-xxx) and baudrate 115200. 
Select IAC Driver serialmidi on the midid device and you will be good to go.
![Scherm­afbeelding 2024-11-28 om 14 27 54](https://github.com/user-attachments/assets/2ffa4e3d-4e0a-4737-bdfc-1dacb502689e)
