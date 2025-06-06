/**
 *   GPStarAudio.cpp
 *   Copyright (C) 2024 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
 *
 *   Based on the serial protocol library by Jamie Robertson.
 *   Compatible with both the GPStar Audio and Robertsonics WAV Trigger audio devices.
 *   Copyright (c) 2014

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

#include "GPStarAudio.h"

void gpstarAudio::start(Stream& _port) {
  versionRcvd = false;
  sysInfoRcvd = false;
  gpsInfoRcvd = false;

  GPStarSerial = &_port;

  flush();
}

void gpstarAudio::flush(void) {
  rxCount = 0;
  rxLen = 0;
  rxMsgReady = false;

  for(uint8_t i = 0; i < MAX_NUM_VOICES; i++) {
    voiceTable[i] = 0xffff;
  }

  while(GPStarSerial->available()) {
    GPStarSerial->read();
  }
}

void gpstarAudio::serialFlush(void) {
  GPStarSerial->flush();
}

void gpstarAudio::update(void) {
  uint8_t dat;
  uint8_t voice;
  uint16_t track;

  rxMsgReady = false;

  while(GPStarSerial->available() > 0) {
    dat = GPStarSerial->read();

    if((rxCount == 0) && (dat == SOM1)) {
      rxCount++;
    }
    else if(rxCount == 1) {
      if(dat == SOM2) {
        rxCount++;
      }
      else {
        rxCount = 0; // Bad serial data.
      }
    }
    else if(rxCount == 2) {
      if(dat == SOM1 || dat == SOM2 || dat == EOM) {
        rxCount = 0; // Bad serial data.
      }
      else if(dat <= MAX_MESSAGE_LEN) {
        rxCount++;
        rxLen = dat - 1;
      }
      else {
        rxCount = 0; // Bad serial data.
      }
    }
    else if((rxCount > 2) && (rxCount < rxLen)) {
      rxMessage[rxCount - 3] = dat;
      rxCount++;
      
      if(rxMessage[0] == RSP_GPSTAR_HELLO) {
	      // Skip the extra check upon the GPStar hello check.
      }
      else if(dat == SOM1 || dat == SOM2 || dat == EOM) {
        rxCount = 0; // Bad serial data.
      }
    }
    else if(rxCount == rxLen) {
      if(dat == EOM) {
        rxMsgReady = true;
      }
      else {
        rxCount = 0; // Bad serial data.
      }
    }
    else {
      rxCount = 0; // Bad serial data.
    }

    if(rxMsgReady) {
      switch (rxMessage[0]) {
        case RSP_TRACK_REPORT_EX:
          track = rxMessage[2];
          track = (track << 8) + rxMessage[1];

          currentTrack = track;

          // 0 = not playing. 1 = playing.
          if(rxMessage[3] == 0) {
            bCurrentTrackStatus = false;
          }
          else {
            bCurrentTrackStatus = true;
          }

          // Set trackCounter to false to reset it.
          trackCounter = false;
        break;

        case RSP_TRACK_REPORT:
          // WT.
          track = rxMessage[2];
          track = (track << 8) + rxMessage[1] + 1;
          voice = rxMessage[3];
          if(voice < MAX_NUM_VOICES) {
            if(rxMessage[4] == 0) {
              if(track == voiceTable[voice])
                voiceTable[voice] = 0xffff;
            }
            else
              voiceTable[voice] = track;
          }
        break;

        case RSP_VERSION_STRING:
          // WT.
          for(uint8_t i = 0; i < (VERSION_STRING_LEN - 1); i++) {
            version[i] = rxMessage[i + 1];
          }
          version[VERSION_STRING_LEN - 1] = 0;
          versionRcvd = true;
        break;

        case RSP_SYSTEM_INFO:
          // WT.
          numVoices = rxMessage[1];
          numTracks = rxMessage[3];
          numTracks = (numTracks << 8) + rxMessage[2];
          sysInfoRcvd = true;
        break;

        case RSP_GPSTAR_HELLO:
          // GP.
          numVoices = rxMessage[1];
          numTracks = rxMessage[3];
          numTracks = (numTracks << 8) + rxMessage[2];

          if(rxLen >= GPSTAR_HELLO_LEN) {
            versionNumber = rxMessage[5];
            versionNumber = (versionNumber << 8) + rxMessage[4];
          }

          gpsInfoRcvd = true;
        break;
      }

      rxCount = 0;
      rxLen = 0;
      rxMsgReady = false;
    }
  }
}

bool gpstarAudio::currentTrackStatus(uint16_t trk) {
  if(trk == currentTrack) {
    if(bCurrentTrackStatus) {
      return true;
    }
  }

  return false;
}

__attribute__((deprecated("trackCounterReset() is deprecated. Please use isTrackCounterReset() instead.")))
bool gpstarAudio::trackCounterReset() {
  return isTrackCounterReset();
}

bool gpstarAudio::isTrackCounterReset() {
  // trackCounter is reset if it is true.
  return trackCounter;
}

__attribute__((deprecated("resetTrackCounter(bool) is deprecated and will always reset to true. Please use resetTrackCounter() without passing a parameter instead.")))
void gpstarAudio::resetTrackCounter(bool bReset) {
  // Ignore the passed parameter and reset trackCounter.
  (void)bReset;
  resetTrackCounter();
}

void gpstarAudio::resetTrackCounter() {
  // Resetting the variable means to set it to true.
  trackCounter = true;
}

void gpstarAudio::trackPlayingStatus(uint16_t trk) {
  uint8_t txbuf[7];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x07;
  txbuf[3] = CMD_GET_TRACK_STATUS;
  txbuf[4] = (uint8_t)trk;
  txbuf[5] = (uint8_t)(trk >> 8);
  txbuf[6] = EOM;
  GPStarSerial->write(txbuf, 7);
}

bool gpstarAudio::isTrackPlaying(uint16_t trk) {
  update();

  for(uint8_t i = 0; i < MAX_NUM_VOICES; i++) {
    if(voiceTable[i] == trk) {
      return true;
    }
  }

  return false;
}

void gpstarAudio::masterGain(int16_t gain) {
  uint8_t txbuf[7];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x07;
  txbuf[3] = CMD_MASTER_VOLUME;
  txbuf[4] = (uint8_t)gain;
  txbuf[5] = (uint8_t)(gain >> 8);
  txbuf[6] = EOM;
  GPStarSerial->write(txbuf, 7);
}

void gpstarAudio::setAmpPwr(bool enable) {
  uint8_t txbuf[6];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x06;
  txbuf[3] = CMD_AMP_POWER;
  txbuf[4] = enable;
  txbuf[5] = EOM;
  GPStarSerial->write(txbuf, 6);
}

void gpstarAudio::setReporting(bool enable) {
  uint8_t txbuf[6];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x06;
  txbuf[3] = CMD_SET_REPORTING;
  txbuf[4] = enable;
  txbuf[5] = EOM;
  GPStarSerial->write(txbuf, 6);
}

bool gpstarAudio::getVersion(char *pDst) {
  update();

  if(!versionRcvd) {
    return false;
  }

  for(uint8_t i = 0; i < (VERSION_STRING_LEN - 1); i++) {
    pDst[i] = version[i];
  }

  return true;
}

uint16_t gpstarAudio::getVersionNumber(void) {
  return versionNumber;
}

uint16_t gpstarAudio::getNumTracks(void) {
  update();

  return numTracks;
}

void gpstarAudio::trackPlaySolo(uint16_t trk) {
  trackControl(trk, TRK_PLAY_SOLO);
}

void gpstarAudio::trackPlaySolo(uint16_t trk, bool lock) {
  trackControl(trk, TRK_PLAY_SOLO, lock);
}

void gpstarAudio::trackPlaySolo(uint16_t trk, bool lock, uint16_t i_trk_start_delay) {
  trackControl(trk, TRK_PLAY_SOLO, lock, i_trk_start_delay);
}

void gpstarAudio::trackPlaySolo(uint16_t trk, bool lock, uint16_t i_trk_start_delay, uint16_t trk2, bool loop_trk2, uint16_t trk2_start_time) {
  trackControl(trk, TRK_PLAY_SOLO, lock, i_trk_start_delay, trk2, loop_trk2, trk2_start_time);
}

void gpstarAudio::trackPlayPoly(uint16_t trk) {
  trackControl(trk, TRK_PLAY_POLY);
}

void gpstarAudio::trackPlayPoly(uint16_t trk, bool lock) {
  trackControl(trk, TRK_PLAY_POLY, lock);
}

void gpstarAudio::trackPlayPoly(uint16_t trk, bool lock, uint16_t i_trk_start_delay) {
  trackControl(trk, TRK_PLAY_POLY, lock, i_trk_start_delay);
}

void gpstarAudio::trackPlayPoly(uint16_t trk, bool lock, uint16_t i_trk_start_delay, uint16_t trk2, bool loop_trk2, uint16_t trk2_start_time) {
  trackControl(trk, TRK_PLAY_POLY, lock, i_trk_start_delay, trk2, loop_trk2, trk2_start_time);
}

void gpstarAudio::trackLoad(uint16_t trk) {
  trackControl(trk, TRK_LOAD);
}

void gpstarAudio::trackLoad(uint16_t trk, bool lock) {
  trackControl(trk, TRK_LOAD, lock);
}

void gpstarAudio::trackStop(uint16_t trk) {
  trackControl(trk, TRK_STOP);
}

void gpstarAudio::trackPause(uint16_t trk) {
  trackControl(trk, TRK_PAUSE);
}

void gpstarAudio::trackResume(uint16_t trk) {
  trackControl(trk, TRK_RESUME);
}

void gpstarAudio::trackLoop(uint16_t trk, bool enable) {
  if(enable) {
    trackControl(trk, TRK_LOOP_ON);
  }
  else {
    trackControl(trk, TRK_LOOP_OFF);
  }
}

void gpstarAudio::trackControl(uint16_t trk, uint8_t code) {
  uint8_t txbuf[8];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x08;
  txbuf[3] = CMD_TRACK_CONTROL;
  txbuf[4] = code;
  txbuf[5] = (uint8_t)trk;
  txbuf[6] = (uint8_t)(trk >> 8);
  txbuf[7] = EOM;
  GPStarSerial->write(txbuf, 8);
}

void gpstarAudio::trackControl(uint16_t trk, uint8_t code, bool lock) {
  uint8_t txbuf[9];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x09;
  txbuf[3] = CMD_TRACK_CONTROL_EX;
  txbuf[4] = code;
  txbuf[5] = (uint8_t)trk;
  txbuf[6] = (uint8_t)(trk >> 8);
  txbuf[7] = lock;
  txbuf[8] = EOM;
  GPStarSerial->write(txbuf, 9);
}

void gpstarAudio::trackControl(uint16_t trk, uint8_t code, bool lock, uint16_t trk1_start_time) {
  uint8_t txbuf[11];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x0b;
  txbuf[3] = CMD_TRACK_CONTROL_CACHE;
  txbuf[4] = code;
  txbuf[5] = (uint8_t)trk;
  txbuf[6] = (uint8_t)(trk >> 8);
  txbuf[7] = lock;
  txbuf[8] = (uint8_t)trk1_start_time;
  txbuf[9] = (uint8_t)(trk1_start_time >> 8);    
  txbuf[10] = EOM;
  GPStarSerial->write(txbuf, 11);
}

void gpstarAudio::trackControl(uint16_t trk, uint8_t code, bool lock, uint16_t trk1_start_time, uint16_t trk2, bool loop_trk2, uint16_t trk2_start_time) {
  uint8_t txbuf[16];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x10;
  txbuf[3] = CMD_TRACK_CONTROL_QUEUE;
  txbuf[4] = code;
  txbuf[5] = (uint8_t)trk;
  txbuf[6] = (uint8_t)(trk >> 8);
  txbuf[7] = lock;
  txbuf[8] = (uint8_t)trk2;
  txbuf[9] = (uint8_t)(trk2 >> 8);  
  txbuf[10] = loop_trk2;
  txbuf[11] = (uint8_t)trk2_start_time;
  txbuf[12] = (uint8_t)(trk2_start_time >> 8);
  txbuf[13] = (uint8_t)trk1_start_time;
  txbuf[14] = (uint8_t)(trk1_start_time >> 8);  
  txbuf[15] = EOM;
  GPStarSerial->write(txbuf, 16);
}

void gpstarAudio::trackQueueClear() {
  uint8_t txbuf[5];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x05;
  txbuf[3] = CMD_TRACK_QUEUE_CLEAR;
  txbuf[4] = EOM;
  GPStarSerial->write(txbuf, 5);
}

void gpstarAudio::stopAllTracks(void) {
  uint8_t txbuf[5];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x05;
  txbuf[3] = CMD_STOP_ALL;
  txbuf[4] = EOM;
  GPStarSerial->write(txbuf, 5);
}

void gpstarAudio::resumeAllInSync(void) {
  uint8_t txbuf[5];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x05;
  txbuf[3] = CMD_RESUME_ALL_SYNC;
  txbuf[4] = EOM;
  GPStarSerial->write(txbuf, 5);
}

void gpstarAudio::trackGain(uint16_t trk, int16_t gain) {
  uint8_t txbuf[9];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x09;
  txbuf[3] = CMD_TRACK_VOLUME;
  txbuf[4] = (uint8_t)trk;
  txbuf[5] = (uint8_t)(trk >> 8);
  txbuf[6] = (uint8_t)gain;
  txbuf[7] = (uint8_t)(gain >> 8);
  txbuf[8] = EOM;
  GPStarSerial->write(txbuf, 9);
}

void gpstarAudio::trackFade(uint16_t trk, int16_t gain, uint16_t time, bool stopFlag) {
  uint8_t txbuf[12];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x0c;
  txbuf[3] = CMD_TRACK_FADE;
  txbuf[4] = (uint8_t)trk;
  txbuf[5] = (uint8_t)(trk >> 8);
  txbuf[6] = (uint8_t)gain;
  txbuf[7] = (uint8_t)(gain >> 8);
  txbuf[8] = (uint8_t)time;
  txbuf[9] = (uint8_t)(time >> 8);
  txbuf[10] = stopFlag;
  txbuf[11] = EOM;
  GPStarSerial->write(txbuf, 12);
}

void gpstarAudio::samplerateOffset(int16_t offset) {
  uint8_t txbuf[7];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x07;
  txbuf[3] = CMD_SAMPLERATE_OFFSET;
  txbuf[4] = (uint8_t)offset;
  txbuf[5] = (uint8_t)(offset >> 8);
  txbuf[6] = EOM;
  GPStarSerial->write(txbuf, 7);
}

void gpstarAudio::setTriggerBank(uint8_t bank) {
  uint8_t txbuf[6];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x06;
  txbuf[3] = CMD_SET_TRIGGER_BANK;
  txbuf[4] = bank;
  txbuf[5] = EOM;
  GPStarSerial->write(txbuf, 6);
}

// Turn on or off the LED on GPStar Audio. Default is on.
void gpstarAudio::gpstarLEDStatus(bool enable) {
  uint8_t txbuf[5];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x05;

  if(enable) {
    txbuf[3] = CMD_LED_ON;
  }
  else {
    txbuf[3] = CMD_LED_OFF;
  }

  txbuf[4] = EOM;
  GPStarSerial->write(txbuf, 5);
}

// Turn on track short overload or turn it off.
void gpstarAudio::gpstarShortTrackOverload(bool enable) {
  uint8_t txbuf[5];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x05;

  if(enable) {
    txbuf[3] = CMD_SHORT_OVERLOAD_ON;
  }
  else {
    txbuf[3] = CMD_SHORT_OVERLOAD_OFF;
  }

  txbuf[4] = EOM;
  GPStarSerial->write(txbuf, 5);
}

// Turn on track force or turn it off.
void gpstarAudio::gpstarTrackForce(bool enable) {
  uint8_t txbuf[5];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x05;

  if(enable) {
    txbuf[3] = CMD_TRACK_FORCE_ON;
  }
  else {
    txbuf[3] = CMD_TRACK_FORCE_OFF;
  }

  txbuf[4] = EOM;
  GPStarSerial->write(txbuf, 5);
}

void gpstarAudio::requestVersionString(void) {
  uint8_t txbuf[5];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x05;
  txbuf[3] = CMD_GET_VERSION;
  txbuf[4] = EOM;
  GPStarSerial->write(txbuf, 5);
}

void gpstarAudio::requestSystemInfo(void) {
  uint8_t txbuf[5];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x05;
  txbuf[3] = CMD_GET_SYS_INFO;
  txbuf[4] = EOM;
  GPStarSerial->write(txbuf, 5);
}

void gpstarAudio::hello(void) {
  uint8_t txbuf[5];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x05;
  txbuf[3] = CMD_GET_GPSTAR_HELLO;
  txbuf[4] = EOM;
  GPStarSerial->write(txbuf, 5);
}

bool gpstarAudio::wasSysInfoRcvd(void) {
  update();

  return sysInfoRcvd;
}

bool gpstarAudio::gpstarAudioHello(void) {
  update();

  return gpsInfoRcvd;
}