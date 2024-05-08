## <img src='images/gpstar_logo.png' width=50 align="left"/> GPStar Audio - Serial Library

This repository contains the serial communication control library for the [GPStar Audio](https://gpstartechnologies.com/products/gpstar-audio) board. It is compatible with Arduino and other microcontrollers.

[![](images/gpstar_audio_1.png)](https://gpstartechnologies.com/products/gpstar-audio)

GPStar Audio is a polyphonic audio player capable of playing up to 14 stereo wav files at the same time, layering them, mixing, cross fading, with independent track control of volume, pause, resume.

It's onboard stereo amplifier is capable of driving 2 speakers (2.5W @ 4Ω or 1.25W @ 8Ω per channel) from the onboard amplifier and also has a stereo out auxiliary jack. Powered with 5V to the 5V and GND pins, it can be controlled over a serial interface with a Arduino or any other microcontroller. It can be used for a variety of projects such as toys, musical instruments, props or any other embedded projects.

```
#include <AltSoftSerial.h>
#include <GPStarAudio.h>
```

By default, GPStar Audio is configured to use AltSoftSerial for communication. However if you want to use a hardware serial port on your microcontroller that you are using, then make the following changes in the `GPStarAuduio.h file`.

```
// ------------------------------------------------
// Uncomment only the serial class you wish to use.
#define __GPSTAR_AUDIO_USE_ALTSOFTSERIAL__
//#define __GPSTAR_AUDIO_USE_SERIAL1__
//#define __GPSTAR_AUDIO_USE_SERIAL2__
//#define __GPSTAR_AUDIO_USE_SERIAL3__
// ------------------------------------------------
```

Uncomment out the line with the serial port you wish to use and comment out the ones that are not used.

Example code can be found in the example folder in how to use this library.

## Serial commands ##

**GPStarAudio.start()** - This must be called first, to setup the serial communications.

**GPStarAudio.hello()** - This can be called to return if GPStar Audio can be detected.

**GPStarAudio.update()** - Calling this will process any incoming serial data from GPStar Audio. If you are using currentTrackStatus() calls, then you will want to call this often.

**GPStarAudio.getNumTracks()** - This returns the number of tracks on the Micro SD card.

**GPStarAudio.masterGain(int gain)** - This sets the master volume gain of the audio output. The range is -70 (quietest) to 0 (loudest).

**GPStarAudio.trackPlaySingle(int t)** - This will stop any tracks that are currently playing and play the selected track number provided.

**GPStarAudio.trackPlaySingle(int t, bool lock)** - This will stop any tracks that are currently playing and play the selected track number provided. If **lock** is set to **TRUE**, the track will not not ever be unloaded or stopped from the channel it has acquired if the maximum numbe of channels are in use, unless you manually tell the track to stop.
  
**GPStarAudio.trackPlay(int t)** - This will play the provided track number. Mixing and overlaying it with any other tracks that are currently playing.

**GPStarAudio.trackPlay(int t, bool lock)** - This will play the provided track number. Mixing and overlaying it with any other tracks that are currently playing. If **lock** is set to **TRUE**, the track will not not ever be unloaded or stopped from the channel it has acquired if the maximum numbe of channels are in use, unless you manually tell the track to stop.

**GPStarAudio.trackStop(int t)** - This stops the provided track number if it is currently playing. 
  
**GPStarAudio.trackPause(int t)** - This will pause the provided track number if it is currently playing. The track will still stay loaded and hold the current channel it is allocated.
  
**GPStarAudio.trackResume(int t)** - This will resume the provided track if it is currently paused.
  
**GPStarAudio.trackLoop(int t, bool enable)** - This will set a track to loop or not loop. When set to loop, the track will automatically replay itself until you call `GPStarAudio.trackStop(int t)` or set the the track looping off. Set to true to enable looping or false to disable looping.
  
**GPStarAudio.trackGain(int t, int gain)** - This will set the volume of the provided track number. The volume range is -70 (quietest) to 0 (loudest). 
  
**GPStarAudio.stopAllTracks()** - This will stop all tracks that are currently playing.
  
**GPStarAudio.trackFade(int t, int gain, int time, bool stopFlag)** - This will fade the currently playing provided track number. The track volume will smoothly change to the target volume gain you provide from whatever volume the track is already playing at. The **stopFlag** by default is false which will keep the tracking playing after the fade. However setting the **stopFlag** to true will make the track stop playing after the fade has finished.

**GPStarAudio.setLED(bool status)** - You can turn off the LED status indicator when setting **status** to false. Passing **true** enables the LED again. By default, the LED on GPStar Audio flashes and blinks to provide various status updates.

**GPStarAudio.gpstarShortTrackOverload(bool status)** - Enabled by default. GPStar Audio will detect mulitple versions of the same sound playing in succession and prevent it from overloading and taking too many audio channels, it will instead replay the file and save system resources.

**GPStarAudio.gpstarTrackForcebool status)** - Disabled by default. When enabled, GPStar Audio will forcibly take a audio channel and play any track you tell it to, even if all channels are used and locked.

**GPStarAudio.trackPlayingStatus(int trk)** - This will ask GPStar Audio if the provided track number is playing. After calling this method, call the `GPStarAudio.currentTrackStatus(int trk)` method to find out if the track is playing or not.
  
**GPStarAudio.currentTrackStatus(int trk)** - This will retrieve the status of a the provided track number if it is playing. You will want to use the `GPStarAudio.trackPlayinStatus(int trk)` method first to ask if the provided track is playing, then call this method soon after to retrieve the response.

## <img src='images/gpstar_logo.png' width=50 align="left"/>GPStar Audio - Connection Details
 
![](images/GPStarAudioPCB.png)

### Standard Connection UART

| Pins | Notes |
|------|-------|
| DTR/TX/RX/5V/GND/GND | This connection serves as the main power source and communication but it also serves as a UART programming header for software updates. Firmware can be flashed here with a standard 5V FTDI Basic. Connect a regulated 5V power source to the 5V and either GND pin to provide power to the device. |

---

### Special Connectors & Switches

| Label | Notes |
|-------|-------|
| BOOT/LOAD | A slide switch button. When set to BOOT, the system will operate, however when set to LOAD, then the system enters a programming mode and allows you to flash updated firmware over the UART connection. Make sure it is set to BOOT afterwards for it to operate. |
| SPKR-R | Right Speaker output from the on-board stereo amplifier. Capable of powering either a 4Ω 2.5W or 8Ω 1.25W speaker at 5V. |
| SPKR-L | Left Speaker output from the on-board stereo amplifier. Capable of powering either a 4Ω 2.5W or 8Ω 1.25W speaker at 5V. |
| AUX | ****Not labelled**** The on-board stereo auxiliary headphone jack outputs stereo sound and can be fed to any type of amplifier or audio device with a 3.5mm auxiliary input port. When a cable is inserted, the system will turn off the on-board stereo amplifier and output sound from this port instead. 

#### If you are using the on-board stereo amplifier with only one speaker for a mono output setup, then connect the single speaker to either SPKR-R or to SPKR-L only. 

#### The on-board stereo amplifier is capable of powering either a 4Ω 2.5W or 8Ω 1.25W speaker at 5V from each channel.

---

### Optional Connectors

| Label | Pins | Notes |
|-------|------|-------|
| ST-LINK | GND/3.3V/SWCLK/SWDIO | ****Not Labelled**** These 4 pins grouped together can be connected to a ST-LINK for debugging or as a alternative way to flash updated firmware. |
| TX2/RX2 | TX2/RX2 | Alternative serial communication port used for debugging. |
| TEST |  | A button located on the lower right corner of the board. Pressing this button will play the first track on the Micro SD Card. |

## Links

* [GPStar Audio product page](https://gpstartechnologies.com/products/gpstar-audio)
* [GPStar Audio firmware downloads](https://gpstartechnologies.com/pages/support-downloads)


## License

Released under the GNU GPLv3 license.

Copyright (c) 2024 GPStar Technologies.