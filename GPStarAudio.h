/**
 *   GPStarAudio.h
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
 */

#pragma once

// ------------------------------------------------
// Uncomment only the serial class you wish to use.
#define __GPSTAR_AUDIO_USE_ALTSOFTSERIAL__
//#define __GPSTAR_AUDIO_USE_SERIAL1__
//#define __GPSTAR_AUDIO_USE_SERIAL2__
//#define __GPSTAR_AUDIO_USE_SERIAL3__
// ------------------------------------------------

#define CMD_TRACK_CONTROL        3
#define CMD_STOP_ALL             4
#define CMD_MASTER_VOLUME        5
#define CMD_TRACK_VOLUME         8
#define CMD_TRACK_FADE          10
#define CMD_TRACK_CONTROL_EX    13
#define CMD_GET_TRACK_STATUS    16
#define CMD_GET_GPSTAR_HELLO    17
#define CMD_LED_ON              18
#define CMD_LED_OFF             19
#define CMD_SHORT_OVERLOAD_ON   20
#define CMD_SHORT_OVERLOAD_OFF  21
#define CMD_TRACK_FORCE_ON      22
#define CMD_TRACK_FORCE_OFF     23

#define TRK_PLAY_SOLO            0
#define TRK_PLAY_POLY            1
#define TRK_PAUSE                2
#define TRK_RESUME               3
#define TRK_STOP                 4
#define TRK_LOOP_ON              5
#define TRK_LOOP_OFF             6

#define RSP_TRACK_REPORT_EX    133
#define RSP_GPSTAR_HELLO       134
#define MAX_MESSAGE_LEN         32

#define SOM1   0xf0
#define SOM2   0xaa
#define EOM    0x55

#ifdef __GPSTAR_AUDIO_USE_ALTSOFTSERIAL__
#include <AltSoftSerial.h>
#else
#include <HardwareSerial.h>
#ifdef __GPSTAR_AUDIO_USE_SERIAL1__
#define GPStarSerial Serial1
#define __GPSTAR_AUDIO_SERIAL_ASSIGNED__
#endif
#ifdef __GPSTAR_AUDIO_USE_SERIAL2__
#define GPStarSerial Serial2
#define __GPSTAR_AUDIO_SERIAL_ASSIGNED__
#endif
#ifdef __GPSTAR_AUDIO_USE_SERIAL3__
#define GPStarSerial Serial3
#define __GPSTAR_AUDIO_SERIAL_ASSIGNED__
#endif
#ifndef __GPSTAR_AUDIO_SERIAL_ASSIGNED__
#define GPStarSerial Serial
#endif
#endif

class gpstarAudio
{
public:
  gpstarAudio() {;}
  ~gpstarAudio() {;}
  void start(void);
  void update(void);
  void flush(void);
  uint16_t getNumTracks(void);
  void masterGain(int16_t gain);
  void stopAllTracks(void);
  void trackPlaySolo(uint16_t trk);
  void trackPlaySolo(uint16_t trk, bool lock);
  void trackPlay(uint16_t trk);
  void trackPlay(uint16_t trk, bool lock);
  void trackStop(uint16_t trk);
  void trackPause(uint16_t trk);
  void trackResume(uint16_t trk);
  void trackLoop(uint16_t trk, bool enable);
  void trackGain(uint16_t trk, int16_t gain);
  void trackFade(uint16_t trk, int16_t gain, uint16_t time, bool stopFlag);
  void trackPlayingStatus(uint16_t trk);
  bool currentTrackStatus(uint16_t trk);
  void serialFlush(void);
  void hello(void);
  void gpstarLEDStatus(bool status);
  void gpstarShortTrackOverload(bool status);
  void gpstarTrackForce(bool status);
  bool gpstarAudioHello(void);

private:
  void trackControl(uint16_t trk, uint8_t code);
  void trackControl(uint16_t trk, uint8_t code, bool lock);

  #ifdef __GPSTAR_AUDIO_USE_ALTSOFTSERIAL__
    AltSoftSerial GPStarSerial;
  #endif

  uint8_t rxMessage[MAX_MESSAGE_LEN];
  uint16_t numTracks;
  uint8_t numVoices;
  uint8_t rxCount;
  uint8_t rxLen;
  bool rxMsgReady;
  uint16_t currentTrack;
  bool b_currentTrackStatus;
  bool trackCounter;
  bool gpstarHello;
};