//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: audio.h,v 1.4 2006/03/02 17:08:30 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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

#ifndef __AUDIO_H__
#define __AUDIO_H__

//---------------------------------------------------------
//   Audio
//---------------------------------------------------------

class Audio {
   public:
      Audio() {}
      virtual ~Audio() {}
      virtual bool init() = 0;
      virtual bool start() = 0;
      virtual bool stop() = 0;
      virtual std::list<QString> inputPorts() = 0;
      virtual void stopTransport() = 0;
      virtual void startTransport() = 0;
      virtual int getState() = 0;
      virtual int sampleRate() const = 0;
      virtual bool isRealtime() const { return false; }
      };

#endif

