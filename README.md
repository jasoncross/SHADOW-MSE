Introducing S.H.A.D.O.W. (Small Handheld Arduino Droid Operating Wand) MSE Edition
A MSE droid control system based on DanF's Padawan, and Darren Blum's S.H.A.D.O.W. that uses a PS3 Move Navigation Controller

This is a fork of Christopher Edwards' S.H.A.D.O.W. MSE Droid variant which is based on Darren Blum's S.H.A.D.O.W. system. It is specifically modified to support MSE (RC Car) Based Droids.

It is a slimmed down version of the original code base to only support what is used on these types of droids.
If you are already familiar with S.H.A.D.O.W. then this fork will feel very familiar.

This sketch works on an Arduino MEGA ADK, or Arduino MEGA or Arduino UNO with a USB Shield

The sketch supports the following MP3 players:

* Sparkfun MP3Trigger: https://www.sparkfun.com/products/13720
* MDFly MP3 Player: http://www.mdfly.com/products/sd-card-mp3-player-module-rs232-ttl.html

(I have removed the RogueMP3 support for now)

Pin Assignment:
* Steering Servo: 4
* ESC: 3
* MP3 Trigger: 2

Sound Mapping:
Note: when using L2 Throttle the L2 + and L1 + keystrokes still work, though they may not be very useful
* D-pad Up - Random DoDo Sound
* D-pad Right - Random Scramble Sound
* D-pad Down - Random Horn Sound
* D-pad Left - Random Droid Sound
* Cross (X) - Random Zap Sound
* Circle (O) - Scream
* L1 + Up - Imperial March
* L1 + Right - Cantina Song
* L1 + Down - Star Wars Theme
* L1 + Left - MSE Droid Song
* L1 + Circle (O) - Enable Auto Sounds
* L1 + Cross (X) - Disable Auto Sounds
* PS + Up - Volume Up
* PS + Right - Volume Max
* PS + Down - Volume Down
* PS + Left - Volume Mid
* PS + Circle (O) - Enable Drive
* PS + Cross (X) - Disable Drive

Additional information for the original S.H.A.D.O.W. Sketch can be found in this thread on Asromech.net:
http://astromech.net/forums/showthread.php?19298-S-H-A-D-O-W-Padawan-based-mini-PS3-Controller

All libraries distributed with original licenses as specified by the library.
serMP3 library comes from: https://forum.arduino.cc/index.php?topic=148983.0

Thank you to:
* Darren Blum aka KnightShade - S.H.A.D.O.W Sketch and rMP3 support
* MSE Droid Builders Facebook page
* Ben Black - MDFly Support
* Joe McFly - Testing support
* Christopher Edwards - MSE Variant this is modified from

---

A note on sounds and MDFly:  MDFly uses sounds based on order on the card, not file name.  I use the app FatSort - https://fatsort.sourceforge.io - for this.  


1. Become root 
```
sudo su -
```
2. Go to FatSort directory - in my case it's the user home directory
3. Unmount your SD card - in my case the volume is named MSE
```
diskutil unmount /Volumes/MSE
```
4. Run fatsort to order the files on the SD card by filename
```
./fatsort /dev/disk2s1
```
5. Re-mount SD - for some reason on my Mac I have to go into disk utility to do so.
6. Clean eject SD - I use "CleanMyDrive 2" for this.  It allows you to eject and removes all of the extra files the Mac puts on the SD card.
