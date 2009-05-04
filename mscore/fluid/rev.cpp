/*

  Freeverb

  Written by Jezar at Dreampoint, June 2000
  http://www.dreampoint.co.uk
  This code is public domain

  Translated to C by Peter Hanappe, Mai 2001
*/

#include "rev.h"

namespace FluidS {

/***************************************************************
 *
 *                           REVERB
 */

#define DC_OFFSET 1e-8

void fluid_allpass_setbuffer(fluid_allpass* allpass, fluid_real_t *buf, int size);
void fluid_allpass_init(fluid_allpass* allpass);
void fluid_allpass_setfeedback(fluid_allpass* allpass, fluid_real_t val);
fluid_real_t fluid_allpass_getfeedback(fluid_allpass* allpass);

void fluid_allpass_setbuffer(fluid_allpass* allpass, fluid_real_t *buf, int size)
      {
      allpass->bufidx = 0;
      allpass->buffer = buf;
      allpass->bufsize = size;
      }

void fluid_allpass_init(fluid_allpass* allpass)
      {
      int len = allpass->bufsize;
      fluid_real_t* buf = allpass->buffer;
      for (int i = 0; i < len; i++)
            buf[i] = DC_OFFSET; /* this is not 100 % correct. */
      }

void fluid_allpass_setfeedback(fluid_allpass* allpass, fluid_real_t val)
      {
      allpass->feedback = val;
      }

fluid_real_t fluid_allpass_getfeedback(fluid_allpass* allpass)
      {
      return allpass->feedback;
      }

#define fluid_allpass_process(_allpass, _input) \
{ \
  fluid_real_t output; \
  fluid_real_t bufout; \
  bufout = _allpass.buffer[_allpass.bufidx]; \
  output = bufout-_input; \
  _allpass.buffer[_allpass.bufidx] = _input + (bufout * _allpass.feedback); \
  if (++_allpass.bufidx >= _allpass.bufsize) { \
    _allpass.bufidx = 0; \
  } \
  _input = output; \
}

void fluid_comb_setbuffer(fluid_comb* comb, fluid_real_t *buf, int size);
void fluid_comb_init(fluid_comb* comb);
void fluid_comb_setdamp(fluid_comb* comb, fluid_real_t val);
fluid_real_t fluid_comb_getdamp(fluid_comb* comb);
void fluid_comb_setfeedback(fluid_comb* comb, fluid_real_t val);
fluid_real_t fluid_comb_getfeedback(fluid_comb* comb);

void fluid_comb_setbuffer(fluid_comb* comb, fluid_real_t *buf, int size)
      {
      comb->filterstore = 0;
      comb->bufidx = 0;
      comb->buffer = buf;
      comb->bufsize = size;
      }

void fluid_comb_init(fluid_comb* comb)
      {
      fluid_real_t* buf = comb->buffer;
      int len = comb->bufsize;
      for (int i = 0; i < len; i++)
            buf[i] = DC_OFFSET; /* This is not 100 % correct. */
      }

void fluid_comb_setdamp(fluid_comb* comb, fluid_real_t val)
      {
      comb->damp1 = val;
      comb->damp2 = 1 - val;
      }

fluid_real_t fluid_comb_getdamp(fluid_comb* comb)
      {
      return comb->damp1;
      }

void fluid_comb_setfeedback(fluid_comb* comb, fluid_real_t val)
      {
      comb->feedback = val;
      }

fluid_real_t fluid_comb_getfeedback(fluid_comb* comb)
      {
      return comb->feedback;
      }

#define fluid_comb_process(_comb, _input, _output) \
{ \
  fluid_real_t _tmp = _comb.buffer[_comb.bufidx]; \
  _comb.filterstore = (_tmp * _comb.damp2) + (_comb.filterstore * _comb.damp1); \
  _comb.buffer[_comb.bufidx] = _input + (_comb.filterstore * _comb.feedback); \
  if (++_comb.bufidx >= _comb.bufsize) { \
    _comb.bufidx = 0; \
  } \
  _output += _tmp; \
}

#define	fixedgain 0.015f
#define scalewet 3.0f
#define scaledamp 0.4f
#define scaleroom 0.28f
#define offsetroom 0.7f
#define initialroom 0.5f
#define initialdamp 0.5f
#define initialwet 1
#define initialdry 0
#define initialwidth 1

//---------------------------------------------------------
//   Reverb
//---------------------------------------------------------

Reverb::Reverb()
      {
      /* Tie the components to their buffers */
      fluid_comb_setbuffer(&combL[0], bufcombL1, combtuningL1);
      fluid_comb_setbuffer(&combR[0], bufcombR1, combtuningR1);
      fluid_comb_setbuffer(&combL[1], bufcombL2, combtuningL2);
      fluid_comb_setbuffer(&combR[1], bufcombR2, combtuningR2);
      fluid_comb_setbuffer(&combL[2], bufcombL3, combtuningL3);
      fluid_comb_setbuffer(&combR[2], bufcombR3, combtuningR3);
      fluid_comb_setbuffer(&combL[3], bufcombL4, combtuningL4);
      fluid_comb_setbuffer(&combR[3], bufcombR4, combtuningR4);
      fluid_comb_setbuffer(&combL[4], bufcombL5, combtuningL5);
      fluid_comb_setbuffer(&combR[4], bufcombR5, combtuningR5);
      fluid_comb_setbuffer(&combL[5], bufcombL6, combtuningL6);
      fluid_comb_setbuffer(&combR[5], bufcombR6, combtuningR6);
      fluid_comb_setbuffer(&combL[6], bufcombL7, combtuningL7);
      fluid_comb_setbuffer(&combR[6], bufcombR7, combtuningR7);
      fluid_comb_setbuffer(&combL[7], bufcombL8, combtuningL8);
      fluid_comb_setbuffer(&combR[7], bufcombR8, combtuningR8);
      fluid_allpass_setbuffer(&allpassL[0], bufallpassL1, allpasstuningL1);
      fluid_allpass_setbuffer(&allpassR[0], bufallpassR1, allpasstuningR1);
      fluid_allpass_setbuffer(&allpassL[1], bufallpassL2, allpasstuningL2);
      fluid_allpass_setbuffer(&allpassR[1], bufallpassR2, allpasstuningR2);
      fluid_allpass_setbuffer(&allpassL[2], bufallpassL3, allpasstuningL3);
      fluid_allpass_setbuffer(&allpassR[2], bufallpassR3, allpasstuningR3);
      fluid_allpass_setbuffer(&allpassL[3], bufallpassL4, allpasstuningL4);
      fluid_allpass_setbuffer(&allpassR[3], bufallpassR4, allpasstuningR4);
      /* Set default values */
      fluid_allpass_setfeedback(&allpassL[0], 0.5f);
      fluid_allpass_setfeedback(&allpassR[0], 0.5f);
      fluid_allpass_setfeedback(&allpassL[1], 0.5f);
      fluid_allpass_setfeedback(&allpassR[1], 0.5f);
      fluid_allpass_setfeedback(&allpassL[2], 0.5f);
      fluid_allpass_setfeedback(&allpassR[2], 0.5f);
      fluid_allpass_setfeedback(&allpassL[3], 0.5f);
      fluid_allpass_setfeedback(&allpassR[3], 0.5f);

      /* set values manually, since calling set functions causes update
         and all values should be initialized for an update
       */
      roomsize = initialroom * scaleroom + offsetroom;
      damp     = initialdamp * scaledamp;
      wet      = initialwet * scalewet;
      width    = initialwidth;
      gain     = fixedgain;

      update();   // now its okay to update reverb
      init();     // Clear all buffers
      }

void Reverb::init()
      {
      for (int i = 0; i < numcombs; i++) {
            fluid_comb_init(&combL[i]);
            fluid_comb_init(&combR[i]);
            }
      for (int i = 0; i < numallpasses; i++) {
            fluid_allpass_init(&allpassL[i]);
            fluid_allpass_init(&allpassR[i]);
            }
      }

void Reverb::processmix(fluid_real_t *in, fluid_real_t *left_out, fluid_real_t *right_out)
      {
      Reverb* rev = this;
      int i, k = 0;
      fluid_real_t outL, outR, input;

      for (k = 0; k < FLUID_BUFSIZE; k++) {
            outL = outR = 0;

            /* The original Freeverb code expects a stereo signal and 'input'
             * is set to the sum of the left and right input sample. Since
             * this code works on a mono signal, 'input' is set to twice the
             * input sample. */
            input = (2 * in[k] + DC_OFFSET) * rev->gain;

            /* Accumulate comb filters in parallel */
            for (i = 0; i < numcombs; i++) {
                  fluid_comb_process(rev->combL[i], input, outL);
                  fluid_comb_process(rev->combR[i], input, outR);
                  }
            /* Feed through allpasses in series */
            for (i = 0; i < numallpasses; i++) {
                  fluid_allpass_process(rev->allpassL[i], outL);
                  fluid_allpass_process(rev->allpassR[i], outR);
                  }

            /* Remove the DC offset */
            outL -= DC_OFFSET;
            outR -= DC_OFFSET;

            /* Calculate output MIXING with anything already there */
            left_out[k]  += outL * rev->wet1 + outR * rev->wet2;
            right_out[k] += outR * rev->wet1 + outL * rev->wet2;
            }
      }

void Reverb::update()
      {
      /* Recalculate internal values after parameter change */
      int i;

      wet1 = wet * (width / 2 + 0.5f);
      wet2 = wet * ((1 - width) / 2);

      for (i = 0; i < numcombs; i++) {
            fluid_comb_setfeedback(&combL[i], roomsize);
            fluid_comb_setfeedback(&combR[i], roomsize);
            }
      for (i = 0; i < numcombs; i++) {
            fluid_comb_setdamp(&combL[i], damp);
            fluid_comb_setdamp(&combR[i], damp);
            }
      }

/*
 The following get/set functions are not inlined, because
 speed is never an issue when calling them, and also
 because as you develop the reverb model, you may
 wish to take dynamic action when they are called.
*/
void Reverb::setroomsize(fluid_real_t value)
      {
      roomsize = (value * scaleroom) + offsetroom;
      update();
      }

fluid_real_t Reverb::getroomsize()
      {
      return (roomsize - offsetroom) / scaleroom;
      }

void Reverb::setdamp(fluid_real_t value)
      {
      damp = value * scaledamp;
      update();
      }

fluid_real_t Reverb::getdamp()
      {
      return damp / scaledamp;
      }

void Reverb::setlevel(fluid_real_t /*value*/)
      {
      //   wet = value * scalewet;
      //   update();
      }

fluid_real_t Reverb::getlevel()
      {
      return wet / scalewet;
      }

void Reverb::setwidth(fluid_real_t value)
      {
      width = value;
      update();
      }

}
