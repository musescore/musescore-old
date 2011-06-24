//=============================================================================
//  MuseSynth
//  Music Software Synthesizer
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __MEVENT_H__
#define __MEVENT_H__

//---------------------------------------------------------
//   Event
//---------------------------------------------------------

class Event {
      int _type;
      int _channel;
      int _dataA;
      int _dataB;
      float _tuning;

   public:
      Event() {}
      Event(int t, int c, int a, int b) : _type(t), _channel(c), _dataA(a), _dataB(b) {};
      int type() const     { return _type; }
      int channel() const  { return _channel; }
      int dataA() const    { return _dataA; }
      int dataB() const    { return _dataB; }
      int controller() const { return _dataA; }
      int value() const    { return _dataB; }
      float tuning() const { return _tuning; }
      };

static const int ME_NOTEON     = 0x90;
static const int ME_CONTROLLER = 0xb0;

static const int CTRL_SUSTAIN = 0x40;
static const int CTRL_PROGRAM = 0x40001;
static const int CTRL_PITCH   = 0x40002;

#endif

