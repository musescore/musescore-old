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

#ifndef __MINSTRUMENT_H__
#define __MINSTRUMENT_H__

#include <stdio.h>
#include <QtXml/QDomDocument>
#include <QtCore/QList>

class MSynth;

//---------------------------------------------------------
//   Sample
//---------------------------------------------------------

class Sample {
      float* _data;
      int _frames;
      int _channels;

   public:
      Sample();
      ~Sample();
      bool read(const QString&);
      int frames() const   { return _frames;           }
      int channels() const { return _channels;         }
      float* data() const  { return _data + _channels; }
      };

//---------------------------------------------------------
//   Zone
//---------------------------------------------------------

class Zone {
      Sample* _sample;
      char _keyLo, _keyHi, _veloLo, _veloHi;
      char _keyBase;

   public:
      Zone() {}
      Zone(char keyLo, char keyHi, char veloLo, char veloHi);
      void init(char keyLo, char keyHi, char veloLo, char veloHi);
      void setSample(Sample* s) { _sample = s; }
      void setKeyBase(char val) { _keyBase = val; }
      int keyBase() const       { return _keyBase; }
      Sample* sample() const    { return _sample; }
      bool match(int key, int velo) const;
      };

//---------------------------------------------------------
//   Instrument
//---------------------------------------------------------

class Instrument {
      MSynth* _msynth;

      QString _name;
      int _program;
      QString instrumentPath;

      QList<Zone*> zones;

      bool read(QDomElement);

   public:
      Instrument(MSynth*);
      bool load(const char*);
      int program() const { return _program; }
      QString name() const { return _name;   }
      Zone* zone(int key, int velocity) const;
      };

#endif

