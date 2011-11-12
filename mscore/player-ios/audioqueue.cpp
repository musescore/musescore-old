//=============================================================================
//  MscorePlayer
//  $Id$
//
//  Copyright (C) 2010 Werner Schweer
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

#include "audioqueue_p.h"
#include "seq.h"

#define FRAME_SIZE            1024
#define NUMBER_OF_BUFFERS     6

//---------------------------------------------------------
//   QueueAudioData
//---------------------------------------------------------

QueueAudioData::QueueAudioData(Seq* s)
      {
      seq = s;
      initialized = false;
      running = false;
      }

QueueAudioData::~QueueAudioData()
      {
      if (running)
            stop();
      if (initialized)
            AudioQueueDispose(audioQueue, false);
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

bool QueueAudioData::init()
      {
      OSStatus status = 0;

      //
      // Setup the audio device.
      //
      AudioStreamBasicDescription deviceFormat;
      deviceFormat.mSampleRate       = 44100;
      deviceFormat.mFormatID         = kAudioFormatLinearPCM;
      deviceFormat.mFormatFlags      = kLinearPCMFormatFlagIsFloat;
      deviceFormat.mBytesPerPacket   = 8;
      deviceFormat.mFramesPerPacket  = 1;
      deviceFormat.mBytesPerFrame    = 8;
      deviceFormat.mChannelsPerFrame = 2;
      deviceFormat.mBitsPerChannel   = 32;
      deviceFormat.mReserved         = 0;

      //
      // Create a new output AudioQueue
      // for the device.
      //
      status = AudioQueueNewOutput(&deviceFormat,  // Format
         processAudio,                             // Callback
         this,                                     // User data, passed to the callback
         CFRunLoopGetMain(),                       // RunLoop
         0, // kCFRunLoopDefaultMode,              // RunLoop mode
         0,                                        // Flags ; must be zero (per documentation)...
         &audioQueue);                             // Output

      Float32 volume = 1.0;
      AudioQueueSetParameter(audioQueue, kAudioQueueParam_Volume,  volume);
      //
      // Allocate buffers for the AudioQueue,
      // and pre-fill them.
      //
      static const int size = FRAME_SIZE * sizeof(float) * 2;
      for (int i = 0; i < NUMBER_OF_BUFFERS; ++i) {
            AudioQueueBufferRef buffer = 0;
            AudioQueueAllocateBuffer(audioQueue, size, &buffer);
            int n = buffer->mAudioDataBytesCapacity;
            memset(buffer->mAudioData, 0, n);
            buffer->mAudioDataByteSize = n;
            AudioQueueEnqueueBuffer(audioQueue, buffer, 0, 0);
            }
      initialized = true;
      return true;
      }

//---------------------------------------------------------
//   start
//---------------------------------------------------------

bool QueueAudioData::start()
      {
      printf("QueueAudioData::start()\n");
      AudioQueueStart(audioQueue, 0);
      running = true;
      return true;
      }

//---------------------------------------------------------
//   stop
//---------------------------------------------------------

bool QueueAudioData::stop()
      {
      printf("QueueAudioData::stop()\n");
      AudioQueueStop(audioQueue, true); // stop immediately, synchronously
      running = false;
      return true;
      }

//---------------------------------------------------------
//   AudioQueueCallback
//---------------------------------------------------------

void QueueAudioData::processAudio(void* p, AudioQueueRef inAQ, AudioQueueBufferRef inBuffer)
      {
      QueueAudioData* audio = static_cast<QueueAudioData*>(p);

      if (audio) {
            short* fp = (short*)(inBuffer->mAudioData);
            audio->seq->process(FRAME_SIZE, fp);
            inBuffer->mAudioDataByteSize = FRAME_SIZE * sizeof(float) * 2;
            }
      else {
            memset(inBuffer->mAudioData, 0, inBuffer->mAudioDataBytesCapacity);
            inBuffer->mAudioDataByteSize = inBuffer->mAudioDataBytesCapacity;
            }
      int r = AudioQueueEnqueueBuffer(inAQ, inBuffer, 0, 0);
      if (r)
            printf("audio error %d\n", r);
      }

//---------------------------------------------------------
//   QueueAudio
//---------------------------------------------------------

QueueAudio::QueueAudio(Seq* s)
   : Driver(s)
      {
      d = new QueueAudioData(s);
      state = TRANSPORT_STOP;
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

bool QueueAudio::init()
      {
      return d->init();
      }

//---------------------------------------------------------
//   start
//---------------------------------------------------------

bool QueueAudio::start()
      {
      return d->start();
      }

//---------------------------------------------------------
//   stop
//---------------------------------------------------------

bool QueueAudio::stop()
      {
      return d->stop();
      }

//---------------------------------------------------------
//   startTransport
//---------------------------------------------------------

void QueueAudio::startTransport()
      {
      state = TRANSPORT_PLAY;
      }

//---------------------------------------------------------
//   stopTransport
//---------------------------------------------------------

void QueueAudio::stopTransport()
      {
      state = TRANSPORT_STOP;
      }
