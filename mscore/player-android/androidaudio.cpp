//=============================================================================
//  MscorePlayer
//  $Id$
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "androidaudio.h"
#include "seq.h"

#define FRAME_SIZE 512

//---------------------------------------------------------
//   audioError
//---------------------------------------------------------

static const char* audioError(SLresult r)
      {
      switch(r) {
            case SL_RESULT_SUCCESS:                   return "success";
            case SL_RESULT_PRECONDITIONS_VIOLATED:    return "preconditions violated";
            case SL_RESULT_PARAMETER_INVALID:         return "parameter invalid";
            case SL_RESULT_MEMORY_FAILURE:            return "memory failure";
            case SL_RESULT_RESOURCE_ERROR:            return "resource error";
            case SL_RESULT_RESOURCE_LOST:             return "resource lost";
            case SL_RESULT_IO_ERROR:                  return "io error";
            case SL_RESULT_BUFFER_INSUFFICIENT:       return "buffer insufficient";
            case SL_RESULT_CONTENT_CORRUPTED:         return "content corrupted";
            case SL_RESULT_CONTENT_UNSUPPORTED:       return "content unsupported";
            case SL_RESULT_CONTENT_NOT_FOUND:         return "content not found";
            case SL_RESULT_PERMISSION_DENIED:         return "permission denied";
            case SL_RESULT_FEATURE_UNSUPPORTED:       return "feature usupported";
            case SL_RESULT_INTERNAL_ERROR:            return "internal error";
            case SL_RESULT_UNKNOWN_ERROR:             return "unknown error";
            case SL_RESULT_OPERATION_ABORTED:         return "operation aborted";
            case SL_RESULT_CONTROL_LOST:              return "control lost";
            default: return "unknown";
            }
      }

//---------------------------------------------------------
//   AndroidAudio
//---------------------------------------------------------

AndroidAudio::AndroidAudio(Seq* s)
   : Driver(s)
      {
      engineObject                 = 0;
      outputMixObject              = 0;
      outputMixEnvironmentalReverb = 0;
      bqPlayerObject               = 0;
      initialized                  = false;
      running                      = false;
      _sampleRate                  = 22050;    // 44100
      }

AndroidAudio::~AndroidAudio()
      {
      if (running)
            stop();
      }

//---------------------------------------------------------
//   playerCallback
//    this callback handler is called every time a
//    buffer finishes playing
//---------------------------------------------------------

void AndroidAudio::playerCallback(SLAndroidSimpleBufferQueueItf bq, void* context)
      {
      static short buffer[FRAME_SIZE * 2];
      memset(buffer, 0, sizeof(buffer));
      ((AndroidAudio*)context)->seq->process(FRAME_SIZE, buffer);
      (*bq)->Enqueue(bq, buffer, FRAME_SIZE * 2 * sizeof(short));
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

bool AndroidAudio::init()
      {
      // create engine
      if (slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL) != SL_RESULT_SUCCESS) {
            qDebug("AndroidAudio: create engine failed\n");
            return false;
            }

      // realize the engine
      if ((*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE) != SL_RESULT_SUCCESS) {
            qDebug("AndroidAudio: realize engine failed\n");
            return false;
            }

      // get the engine interface, which is needed in order to create other objects
      if ((*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine) != SL_RESULT_SUCCESS) {
            qDebug("AndroidAudio: get engine interface failed\n");
            return false;
            }

      // create output mix, with environmental reverb specified as a non-required interface
      const SLInterfaceID ids[1] = {
            SL_IID_ENVIRONMENTALREVERB
            };
      const SLboolean req[1] = {
            SL_BOOLEAN_FALSE
            };
      if ((*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, ids, req) != SL_RESULT_SUCCESS) {
            qDebug("AndroidAudio: create output mix failed\n");
            return false;
            }

      // realize the output mix
      if ((*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE) != SL_RESULT_SUCCESS) {
            qDebug("AndroidAudio: realize output mix failed\n");
            return false;
            }

      // configure audio source
      SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
            2
            };
      SLDataFormat_PCM format_pcm = {
            SL_DATAFORMAT_PCM,
            2,                                  // number of channels
//            SL_SAMPLINGRATE_44_1,
            SL_SAMPLINGRATE_22_05,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
            SL_BYTEORDER_LITTLEENDIAN
            };
      SLDataSource audioSrc = {
            &loc_bufq,
            &format_pcm
            };

      // configure audio sink
      SLDataLocator_OutputMix loc_outmix = {
            SL_DATALOCATOR_OUTPUTMIX,
            outputMixObject
            };
      SLDataSink audioSnk = {
            &loc_outmix,
            0     // &format_pcm
            };

      // create audio player
      const SLInterfaceID aids[1] = {
            SL_IID_BUFFERQUEUE
            };
      const SLboolean areq[1] = {
            SL_BOOLEAN_TRUE
            };
      if ((*engineEngine)->CreateAudioPlayer(engineEngine, &bqPlayerObject, &audioSrc,
         &audioSnk, 1, aids, areq) != SL_RESULT_SUCCESS) {
            qDebug("AndroidAudio: create audio player failed\n");
            return false;
            }

      // realize the player
      if ((*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE) != SL_RESULT_SUCCESS) {
            qDebug("AndroidAudio: realize audio player failed\n");
            return false;
            }

      // get the play interface
      if ((*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay) != SL_RESULT_SUCCESS) {
            qDebug("AndroidAudio: get player interface failed\n");
            return false;
            }

      // get the buffer queue interface
      if ((*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
         &bqPlayerBufferQueue) != SL_RESULT_SUCCESS) {
            qDebug("AndroidAudio: get buffer queue interface failed\n");
            return false;
            }

      // register callback on the buffer queue
      if ((*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue,
         playerCallback, this) != SL_RESULT_SUCCESS) {
            qDebug("AndroidAudio: register callback failed\n");
            return false;
            }

      initialized = true;
      return true;
      }

//---------------------------------------------------------
//   start
//    return true on success
//---------------------------------------------------------

bool AndroidAudio::start()
      {
      // set the player's state to playing
      SLresult r;
      r = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
      if (r) {
            qDebug("AndroidAudio: start failed, <%02x> %s\n", int(r), audioError(r));
            running = true;
            return false;
            }
      playerCallback(bqPlayerBufferQueue, this);
      playerCallback(bqPlayerBufferQueue, this);
      return true;
      }

//---------------------------------------------------------
//   stop
//    return true on success
//---------------------------------------------------------

bool AndroidAudio::stop()
      {
      SLresult r;
      r = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PAUSED);
      if (r) {
            qDebug("AndroidAudio: stop failed, <%02x> %s\n", int(r), audioError(r));
            running = false;
            return false;
            }
      return true;
      }

