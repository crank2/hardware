// Licensed under the Apache License, Version 2.0 (the "License");
 // you may not use this file except in compliance with the License.
 // You may obtain a copy of the License at
  
 //     http://www.apache.org/licenses/LICENSE-2.0
 //
 // Unless required by applicable law or agreed to in writing, software
 // distributed under the License is distributed on an "AS IS" BASIS,
 // WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 // See the License for the specific language governing permissions and
 // limitations under the License.
 //
 // Copyright 2020 Phil Schatzmann
  
 #pragma once
  
 #include <stdint.h>
 #include <stdbool.h>
 #include <algorithm>    // std::min
  
  
   struct __attribute__((packed)) Frame {
   int16_t channel1;
   int16_t channel2;
  
   Frame(int v=0){
     channel1 = channel2 = v;
   }
   
   Frame(int ch1, int ch2){
     channel1 = ch1;
     channel2 = ch2;
   }
  
 };
   
 // support for legacy name;
 using Channels = Frame;
  
 enum ChannelInfo {
     Both, 
     Left, 
     Right 
 };
  
 class SoundData {
   public:
      virtual int32_t get2ChannelData(int32_t pos, int32_t len, uint8_t *data) = 0;
      virtual int32_t getData(int32_t pos, Frame &channels) = 0;
      virtual void setDataRaw( uint8_t* data, int32_t len) = 0;
      bool doLoop();
      void setLoop(bool loop);
  
   private:
      bool automatic_loop;
 };
  
  
 class TwoChannelSoundData : public SoundData {
 public:
     TwoChannelSoundData(bool loop=false);
     TwoChannelSoundData(Frame *data, int32_t len, bool loop=false);
     void setData( Frame *data, int32_t len);
     void setDataRaw( uint8_t* data, int32_t len);
     int32_t getData(int32_t pos, int32_t len, Frame *data);
     int32_t getData(int32_t pos, Frame &channels);
     int32_t get2ChannelData(int32_t pos, int32_t len, uint8_t *data);
     // the number of frames
     int32_t count(){
       return len;
     }
 private:
     Frame* data;
     int32_t len; // length of all data in base unit of subclass
 };
  
  
 class OneChannelSoundData : public SoundData {
   public:
     OneChannelSoundData(bool loop=false, ChannelInfo channelInfo=Both);
     OneChannelSoundData(int16_t *data, int32_t len, bool loop=false, ChannelInfo channelInfo=Both);
     void setData( int16_t *data, int32_t len);
     void setDataRaw( uint8_t* data, int32_t len);
     int32_t getData(int32_t pos, int32_t len, int16_t *data);
     int32_t getData(int32_t pos, Frame &frame);
     int32_t get2ChannelData(int32_t pos, int32_t len, uint8_t *data);
   private:
     int16_t* data;
     int32_t len;
     ChannelInfo channelInfo;
 };
  
  
 class OneChannel8BitSoundData : public SoundData {
   public:
     OneChannel8BitSoundData(bool loop=false, ChannelInfo channelInfo=Both);
     OneChannel8BitSoundData(int8_t *data, int32_t len, bool loop=false, ChannelInfo channelInfo=Both);
     void setData( int8_t *data, int32_t len);
     void setDataRaw( uint8_t* data, int32_t len);
     int32_t getData(int32_t pos, int32_t len, int8_t *data);
     int32_t getData(int32_t pos, Frame &frame);
     int32_t get2ChannelData(int32_t pos, int32_t len, uint8_t *data);
   private:
     int8_t* data;
     int32_t len;
     ChannelInfo channelInfo;
 };