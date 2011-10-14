//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: pa.h 4147 2011-04-07 14:39:44Z wschweer $
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#ifndef __ANDROID_AUDIO_H__
#define __ANDROID_AUDIO_H__

#include "driver.h"
#include "config.h"

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <SLES/OpenSLES_AndroidConfiguration.h>

class Synth;
class Seq;

//---------------------------------------------------------
//   AndroidAudio
//---------------------------------------------------------

class AndroidAudio : public Driver {
      bool initialized;
      bool running;
      int _sampleRate;

      SLObjectItf engineObject;
      SLEngineItf engineEngine;

      SLObjectItf outputMixObject;
      SLEnvironmentalReverbItf outputMixEnvironmentalReverb;

      SLObjectItf bqPlayerObject;
      SLPlayItf bqPlayerPlay;
      SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
      SLEffectSendItf bqPlayerEffectSend;

      static void playerCallback(SLAndroidSimpleBufferQueueItf, void*);

   public:
      AndroidAudio(Seq*);
      virtual ~AndroidAudio();
      virtual bool init();
      virtual bool start();
      virtual bool stop();
      virtual int getState()         { return 0; }
      virtual int sampleRate() const { return _sampleRate; }
      };

#endif


