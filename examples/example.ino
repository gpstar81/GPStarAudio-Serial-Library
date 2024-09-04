/**
 *   GPStar Audio example usage.
 *   Copyright (C) 2024 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 *
 *   Instructions for usage with a Arduino Uno or Arduino Nano.
 *   1. Download and install AltSoftSerial library.
 *   2. Copy the wav files from the example folder to a Micro SD Card.
 *   3. Connect 4 wires from the UNO or Nano to the GPStar UART connector / pins:
 
 *   UNO / Nano            GPStar Audio
 *   ----------            ------------
 *      GND      <----->       GND
 *      5V       <----->       5V
 *     Pin 8     <----->       TX
 *     Pin 9     <----->       RX
 */


#include <AltSoftSerial.h> // When using AltSoftSerial.
//#include <HardwareSerial.h> // When using hardware serial.

#include <GPStarAudio.h>
gpstarAudio gpstar;

bool b_gpstar_audio_found = false;
int i_number_of_tracks = 0;

void setup() {
  // A quick delay to allow GPStar Audio to boot up.
  delay(1000);

  // Please note: GPStar Audio uses 57600 baudrate by default. You can configure this by adding the setting to the Micro SD Card ini configuration file. Please see the README.MD file for more information.

  AltSoftSerial.begin(57600); // When using AltSoftSerial.
  gpstar.start(AltSoftSerial);

  //Serial3.begin(57000); // When using hardware serial. Pick the serial port you want to use.
  //gpstar.start(Serial3);

  delay(10);

  // Allow time for hello command and other data to respond.
  gpstar.hello();

  delay(350);

  if(gpstar.gpstarAudioHello()) {
    // Stop all tracks.
    gpstar.stopAllTracks();

    // Get the number of tracks on the Micro SD Card.
    i_number_of_tracks = gpstar.getNumTracks();

    b_gpstar_audio_found = true;
  }
}

void loop() {
  if(b_gpstar_audio_found == true) {
    // Play track 1 at 100% volume and set it to loop.
    gpstar.trackGain(1, 0);
    gpstar.trackPlay(1);
    gpstar.trackLoop(1, 1);

    delay(5000);

    // Play track 2 at 100% volume. The track will stop playing after it has ended.
    gpstar.trackGain(2, 0);
    gpstar.trackPlay(2);
    
    delay(5000);

    // Fade in track 3 over 4 seconds to 100% volume, and let it loop
    gpstar.trackGain(3, -70);
    gpstar.trackPlay(3);
    gpstar.trackFade(3, 0, 4000, 0);
    gpstar.trackLoop(3, 1);

    delay(10000);

    // Pause track 1
    gpstar.trackPause(1);
    delay(5000);

    // Stop track 2
    gpstar.trackStop(2);

    // Then resume track 1
    gpstar.trackResume(1);

    delay(5000);

    // Fade out track 3 over 4 seconds and have it stop when the fade has ended.
    gpstar.trackFade(3, -70, 4000, 1);

    delay(5000);

    // Stop all tracks.
    gpstar.stopAllTracks();

    delay(2000);
  }
}
