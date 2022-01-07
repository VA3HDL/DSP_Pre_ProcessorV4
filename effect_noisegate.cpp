#include <Arduino.h>
#include "effect_noisegate.h"
#include "utility/dspinst.h"

typedef struct  {
  float samplerate_;     // sample rate(Hz)     44100
  float attackTime_;     // attack time(seconds) 0.05
  float releaseTime_;    // release time(seconds)0.02
  float holdTime_;       // hold time(second)  0.0003
  float threshold_;      // threshold_(dB)        -24

  float holdTimeCount;   // hold time count (points)
  float linearThreshold; // linear threshold
  float at;              // attack time smoothing coefficient
  float rt;              // release time smoothing coefficient

  float attactCounter ;  // hold counter for attack time
  float releaseCounter ; // hold counter for release time
  float x_env ;
  float gca ;
  float gs[2] ;
}
NoiseGate_t;

int freeNoiseGate(NoiseGate_t* obj) {
  //Serial.println(__FUNCTION__);
  if (!obj)
  {
    return -1;
  }
  free(obj);
  return 0;
}

void AudioNoiseGate::update(void)
{
  audio_block_t *block = receiveWritable();
  if (!block) return;
  if (activemask == 1) {
    //Serial.println(__FUNCTION__);
    int16_t *p = block->data;
    int16_t *end = block->data + AUDIO_BLOCK_SAMPLES;
    while (p < end) {
      float b = *p;
      float x_abs = b > 0 ? b : -b;
      obj->x_env = obj->rt * obj->x_env + (1 - obj->at) * ((x_abs - obj->x_env > 0) ? x_abs - obj->x_env : 0);

      if (obj->x_env < obj->linearThreshold)
      {
        obj->gca = 0;
      }
      else
      {
        obj->gca = 1;
      }

      if (obj->gca < obj->gs[0])
      {
        // attack mode
        obj->releaseCounter = 0;
        if (++obj->attactCounter < obj->holdTimeCount)
        {
          // hold mode
          obj->gs[0] = obj->gs[1];
        }
        else
        {
          obj->gs[0] = obj->at * obj->gs[1] + (1 - obj->at) * obj->gca;
        }
        obj->gs[1] = obj->gs[0];

      }
      else {
        // release mode
        obj->attactCounter = 0;
        if (++obj->releaseCounter < obj->holdTimeCount)
        {
          // hold mode
          obj->gs[0] = obj->gs[1];
        }
        else
        {
          obj->gs[0] = obj->rt * obj->gs[1] + (1 - obj->rt) * obj->gca;
        }
        obj->gs[1] = obj->gs[0];
      }

      // apply gain
      *(int16_t *)p = obj->gs[0] * b;
      p += 1;
    }
    transmit(block);
  } else {
    // Effect disabled -> Passthrough -> Do nothing - Should be better to use a mixer
    transmit(block);
  }
  release(block);
}
