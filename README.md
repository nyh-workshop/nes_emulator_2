# NES Emulator (2) for Chipkit (PIC32MZ-EF)
This is a basic NES emulator for Chipkit platform with PIC32MZ-EF microcontrollers. A very limited amount of games are supported - please test this using Mapper 0-based games.
The NES emulator core is written by the guys in OpenEDV forum: http://www.openedv.com/thread-266483-1-1.html and NES sound driver is by the author named "beaglescout007":  https://os.mbed.com/users/beaglescout007/code/Nucleo_Ex06_EMU/

The (2) is there because this is the 2nd try in porting an NES emulator - the 1st try was unsucessful (InfoNES), so the name got stuck there. 

## How to put a game inside?
As usual, the game are not included. Get your own game in *.NES file format, and then convert it to a C file using the HxD editor. Then copy it to "ROM.c".

## Required Hardware:
1.) ILI9341 SPI display.

2.) a breadboard

3.) NES Classic Controller (I2C)

4.) some jumper cables

5.) PIC32MZ-EF microcontroller with at least 64-pins (PIC32MZ2048EFH064).

6.) speaker and a small amplifier (PAM8403)

7.) 1st order low pass filter (24kHz cutoff - trial and error)

## Issues:
1.) The game runs at a relatively lower frame-rate than the actual NES. That is for sure. It would be a different story if the PIC32MZ runs more than 200MHz though. And yes - it is slower because the screen is using SPI instead of a parallel one.

2.) The screen sometimes get garbled between times but it recovers very quickly. I believe it is due to the long delays in setting up the drawing window in the ILI9341.

3.) Not all games are supported. Try games that are limited in only one screen first (no scrolling). And in certain games, the tiles in the screen are garbled. I do not have enough knowledge to check what is faulty inside the emulator - this is only a proof of concept testing for the PIC32MZ microcontrollers.
Other visual problems include:
 - No screen shakes (some games have that)
 - Some sprites are not showing properly
 - Pseudo-random colors at the left or right end of the screen
 
4.) I am trying to tackle the sound issue - a higher order low pass filter is mandatory if you want don't want to hear squealing from the speakers.

5.) The NES classic controller is being polled manually - if the connection isn't too good, the entire game (and the emulator) would stall and crash.

## Extra notes:
Version 1.0 - 2/6/2018