/* Audio Library for Teensy
   Based on objects by Paul Stoffregen

   Development of this audio library was funded by PJRC.COM, LLC by sales of
   Teensy and Audio Adaptor boards.  Please support PJRC's efforts to develop
   open source software by purchasing Teensy or other PJRC products.

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice, development funding notice, and this permission
   notice shall be included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/
#ifndef effect_noisegate_h_
#define effect_noisegate_h_

#include "Arduino.h"
#include "AudioStream.h"

class AudioNoiseGate : public AudioStream
{
  public:
    AudioNoiseGate(void) : AudioStream(1, inputQueueArray) {

    }

    void enable(float myAttackTime, float myReleaseTime, float myThreshold, float myHoldTime) {
      //Serial.println(__FUNCTION__);
      setNoiseGate(obj, myAttackTime, myReleaseTime, myThreshold, myHoldTime, AUDIO_SAMPLE_RATE);
      activemask = 1;
    }

    void disable() {
      //Serial.println(__FUNCTION__);
      activemask = 0;
    }

    // Default Noise Gate parameters
    float myAttackTime = 0.0f;
    float myReleaseTime = 0.0f;
    float myThreshold = 0.0f;
    float myHoldTime = 0.0f;

    typedef struct  {
      float samplerate_;      // sample rate(Hz)     44100
      float attackTime_;      // attack time(seconds) 0.01
      float releaseTime_;     // release time(seconds)0.02
      float holdTime_;        // hold time(second)  0.0003
      float threshold_;       // threshold_(dB)       35.0

      float holdTimeCount;    // hold time count (points)
      float linearThreshold;  // linear threshold
      float at;               // attack time smoothing coefficient
      float rt;               // release time smoothing coefficient

      float attactCounter ;   // hold counter for attack time
      float releaseCounter ;  // hold counter for release time
      float x_env ;
      float gca ;
      float gs[2] ;
    }
    NoiseGate_t;

    virtual void update(void);

  private:
    audio_block_t *inputQueueArray[1];
    uint8_t activemask;
    uint8_t len = 128;

    NoiseGate_t* newNoiseGate() {
      //Serial.println(__FUNCTION__);
      NoiseGate_t* obj = (NoiseGate_t*)malloc(sizeof(NoiseGate_t));
      memset(obj, 0, sizeof(NoiseGate_t));
      if (!obj)
      {
        return NULL;
      }
      return obj;
    }

    void setNoiseGate(NoiseGate_t* obj, float attackTime, float releaseTime, float threshold, float holdTime, float samplerate) {
      //Serial.println(__FUNCTION__);
      obj->attackTime_ = attackTime;
      obj->releaseTime_ = releaseTime;
      obj->threshold_ = threshold;
      obj->holdTime_ = holdTime;
      obj->samplerate_ = samplerate;

      obj->holdTimeCount = obj->holdTime_ * obj->samplerate_;
      obj->linearThreshold = powf(10.0f, obj->threshold_ / 20.0f);
      obj->at = exp(-logf(9) / (obj->samplerate_ * obj->attackTime_));
      obj->rt = exp(-logf(9) / (obj->samplerate_ * obj->releaseTime_));
    }

    NoiseGate_t* createNoiseGate(float attackTime, float releaseTime, float threshold, float samplerate) {
      //Serial.println(__FUNCTION__);
      NoiseGate_t* obj = newNoiseGate();
      if (!obj) {
        return NULL;
      }

      setNoiseGate(obj, attackTime, releaseTime, threshold, myHoldTime, samplerate);
      return obj;
    }

    NoiseGate_t* obj = createNoiseGate(myAttackTime, myReleaseTime, myThreshold, AUDIO_SAMPLE_RATE);

    // free the noise gate
    int freeNoiseGate(NoiseGate_t * obj);
};

#endif
