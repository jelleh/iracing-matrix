# iRacing Matrix

## Description

Arduino based low latency RPM and gear indicator for the iRacing motorsport simulator.  

## Overview

The goal is to build a plug and race, easy to setup, low latency gear and RPM indicator for iRacing.  

*iRacing Matrix pictures*  
![Housing front](./media/iracing_matrix_1.jpg)
![Housing back](./media/iracing_matrix_2.jpg)
![Housing inside](./media/iracing_matrix_inside.jpg)
>STL files included and available on thingiverse
  
**iRacing Matrix in action**  

*Running the executable*  
![Executable](./media/iracing_matrix-exe.gif)

*Calibrate the RPM indicator and drive*  
![Calibrate](./media/iracing_matrix_1.gif)

*Fast cycle through gears*  
![Race](./media/iracing_matrix_2.gif)



## Components

To build iRacing Matrix, you will need the following components:

1. **Microcontroller Board:**
   - Description: The brain is an Arduino board with usb, the Arduino Nano will fit the housing.
   - Example: Arduino Nano

1. **Housing:**
   - Description: The provided housing is build to fit an Arduino nano, feel free to build your own housing.
   - Example: Available in the housing directory or at Thingiverse

2. **Gear indicator:**
   - Description: Gear indicator is a MAX7219 Dot Led Matrix Module
   - Example: MAX7219 Dot Led Matrix Module

3. **RPM and brake indicator:**
   - Description: RPM and brake indicator are two 8 channel WS2812 505 RGB LED strips
   - Example: WS2812 5050 8 channel RGB LED

4. **Display:**
   - Description: MAX7219 based 8 digits 7 segments display
   - Example: MAX7219 8 digits 7 segments

6. **Screws, Wires and hot glue:**
   - Description: Used for assembling and connecting components.
   - Example:
     - Silicone 28AWG wires
     - M1.7x6 screws
     - Hot glue gun

## Schemnatic

![Wires](./media/iracing_matrix_wires.jpg)

## Housing

Download [iracing_matrix-fron.stl](./housing/iracing_matrix-front.stl)  
Download [iracing_matrix-base.stl](./housing/iracing_matrix-base.stl)  

*front*  
![Housing front](./media/iracing_matrix-front-stl.jpg)

*base*  
![Housing base](./media/iracing_matrix-base-stl.jpg)

## Executable

Download the prebuild [iracing_matrix.exe](program/x64/Release/iracing_matrix.exe), or buid it yourself with Visual Studio.  
The executable will interact with the iracing API, and send updates over the COM port to the Arduino.  
When started, it wil scan all the available COM ports for the iRacing Matrix,  
You can provide a fixed COM in the paramaters for faster startup.  
> iracing_matrix.exe [COM port]  
> e.g.: *iracing_matrix.exe 5*  

## Calibration
The RPM indicator needs to be calibrated once per session.  
When calibration is needed, iRacing Matrix will display *fll thr* on the 7-segment digits.  
Completely stop the car, enter neutral gear, and full throtle.  

## Getting Started

Follow these steps to build and use iRacing Matrix:

1. **Assemble iRacing Matrix:**
   - Connect the microcontroller to your computer and upload the provided Arduino code [iracing_matrix.ino](sketch/iracing_matrix/iracing_matrix.ino).
   - Remove the sides (mounting holes) from the 7-segment display with a hacksaw, otherwise it will not fit. (see pictures)
   - Follow the circuit diagram to connect the components to the microcontroller.
   - Download the [stl files](./housing), and print the housing.
   - Glue the components to the front part of the housing with some hot glue.
   - Secure the Arduino nano in the housing, first put the mini usb in the opening, and push the Arduino all the way down.
   - Attach the front and base toghther with seven M1.7x6 screws.

2. **Connect:**
   - Connect the Arduino via usb to you computer.

3. **Run:**
   - Run the [iracing_matrix.exe](program/x64/Release/iracing_matrix.exe) on your iRacing pc. It will automiticly scan the COM ports for iRacing Matrix.

4. **Calibrate:**
   - Enter an iRacing session, and full throtle in netrual gear to calibrate the RPM indicator.

5. **Race:**
   - Exit the pit, and race.

## License

This project is licensed under the [BSD-3-Clause](LICENSE).

---
