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

#ifndef __AUDIOQUEUE_P_H__
#define __AUDIOQUEUE_P_H__

#include "audioqueue.h"
#include <AudioToolBox/AudioToolBox.h>

class Seq;
class MidiDriver;

//---------------------------------------------------------
//   QueueAudioData
//---------------------------------------------------------

class QueueAudioData {
      Seq* seq;
      AudioQueueRef audioQueue;
      bool initialized;
      bool running;
      static void processAudio(void*, AudioQueueRef, AudioQueueBufferRef);

   public:
      QueueAudioData(Seq*);
      virtual ~QueueAudioData();
      virtual bool init();
      virtual bool start();
      virtual bool stop();
      };

#endif

