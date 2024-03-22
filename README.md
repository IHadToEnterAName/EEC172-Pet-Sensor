# EEC172-Pet-Sensor 



## General Description

The project is basically a smart pet food sensor that analyzes the pets behavior
over time, so the user can understand their pet friend more. 


## Outline of the project

<u>***The code is divided into these parts:***</u>

    1. Senses whether the pet is there or not and sends a message through AWS
    whether it is eating or stopped eating based on that.

    2. Senses whenever the food bowl is empty or near empty so it can send a message
    through AWS for the user to refill its food.

    3. Detects whether the pet has not eaten in a long time either through change of
    code or AI.

    4. Checks whether the device has been flipped by the pet requiring user
    assistance ASAP.

    5. Display the Weight %'s into the OLED device.

<u>***Components Needed:***</u>

1. CC3200 Board
2. BreadBoard
3. Wires (MM,FM)
4. HR-S04 UltraSonic Sensor (normal GPIO connection)
5. OLED (SPI connection)
6. load cell (uses HX711 ADC connection)
7. HX711 ADC (uses GPIO connection)
8. Inbuilt accelerometer in the CC3200 (uses I<sup>2</sup>C)


## Project Creators

***Nasih Al-Barwani***\
***Mohammed Alburaidi***





    




    