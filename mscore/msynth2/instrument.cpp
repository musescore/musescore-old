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

#include "instrument.h"
#include <stdio.h>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QStringList>
#include <sndfile.h>

//---------------------------------------------------------
//   domError
//---------------------------------------------------------

static void domError(QDomElement e)
      {
      QString tag(e.tagName());
      fprintf(stderr, "Instrument::load: unknown tag <%s>\n", qPrintable(e.tagName()));
      }

//---------------------------------------------------------
//   parsePitch
//    [a-g][#][0-9]
//---------------------------------------------------------

static char parsePitch(const QString& s)
      {
      char pitch;
      switch(s[0].unicode()) {
            case 'c': pitch = 0; break;
            case 'd': pitch = 2; break;
            case 'e': pitch = 4; break;
            case 'f': pitch = 5; break;
            case 'g': pitch = 7; break;
            case 'a': pitch = 9; break;
            case 'b': pitch = 11; break;
            default:  return -1;
            }
      if (s[1] == '#')
            pitch += 1;

      int octave = s.right(1).toInt() + 1;
      pitch += 12 * octave;
      return pitch;
      }

//---------------------------------------------------------
//   Sample
//---------------------------------------------------------

Sample::Sample()
      {
      _data     = 0;
      _frames   = 0;
      _channels = 0;
      }

Sample::~Sample()
      {
      delete _data;
      }

//---------------------------------------------------------
//   read
//    return true on success
//---------------------------------------------------------

bool Sample::read(const QString& s)
      {
//      printf("Sample::read: %s\n", qPrintable(s));
      SF_INFO info;
      memset(&info, 0, sizeof(info));
      SNDFILE* sf = sf_open(s.toLocal8Bit().data(), SFM_READ, &info);
      if (sf == 0) {
            printf("open <%s> failed\n", s.toLocal8Bit().data());
            return false;
            }
      _channels = info.channels;
      _frames   = info.frames;
      _data     = new float[(_frames + 3) * _channels];
      if (_frames != sf_readf_float(sf, _data + _channels, _frames)) {
            printf("Sample read failed: %s\n", sf_strerror(sf));
            sf_close(sf);
            return false;
            }
      for (int i = 0; i < _channels; ++i) {
            _data[i] = _data[i + _channels];
            _data[(_frames-2) * _channels + i] = _data[(_frames-3) * _channels + i];
            _data[(_frames-1) * _channels + i] = _data[(_frames-3) * _channels + i];
            }
      sf_close(sf);
      return true;
      }

//---------------------------------------------------------
//   Zone
//---------------------------------------------------------

Zone::Zone(char a, char b, char c, char d)
   : _keyLo(a), _keyHi(b), _veloLo(c), _veloHi(d)
      {
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void Zone::init(char a, char b, char c, char d)
      {
      _keyLo  = a;
      _keyHi  = b;
      _veloLo = c;
      _veloHi = d;
      }

//---------------------------------------------------------
//   Instrument
//---------------------------------------------------------

Instrument::Instrument(MSynth* s)
      {
      _msynth = s;
      _program = -1;
      }

//---------------------------------------------------------
//   load
//    return true on success
//---------------------------------------------------------

bool Instrument::load(const char* path)
      {
      printf("Instrument::load: %s\n", path);

      QFileInfo fi(path);
      QFile f(path);

      instrumentPath = fi.path();

      if (!f.open(QIODevice::ReadOnly)) {
            printf("Instrument::load: open %s failed\n", path);
            return false;
            }

      QDomDocument doc;
      int line, column;
      QString err;
      if (!doc.setContent(&f, false, &err, &line, &column)) {
            printf("Instrument::load: line %d col %d: %s\n",
               line, column, qPrintable(err));
            return false;
            }
      f.close();
      for (QDomElement e = doc.documentElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "MuseSynth") {
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        QString tag(ee.tagName());
                        if (tag == "Instrument") {
                              if (!read(ee)) {
                                    return false;
                                    }
                              }
                        else
                              domError(ee);
                        }
                  }
            else
                  domError(e);
            }
      return true;
      }

//---------------------------------------------------------
//   read
//    return true on success
//---------------------------------------------------------

bool Instrument::read(QDomElement e)
      {
      QString path;
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "name")
                  ;
            else if (tag == "patch")
                  _program = e.text().toInt();
            else if (tag == "path")
                  path = e.text();
            else if (tag == "Group") {
                  char veloLo = -1;
                  char veloHi = -1;
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        QString tag(ee.tagName());
                        if (tag == "ampeg_release")
                              ;
                        else if (tag == "velo") {
                              QStringList s = ee.text().split(',');
                              if (s.size() != 2) {
                                    fprintf(stderr, "syntax error in velo: <%s>\n", qPrintable(ee.text()));
                                    return false;
                                    }
                              veloLo = s[0].toInt();
                              veloHi = s[1].toInt();
                              }
                        else if (tag == "Region") {
                              char keyLo   = -1;
                              char keyHi   = -1;
                              char keyBase = -1;
                              Sample* sample = 0;
                              for (QDomElement eee = ee.firstChildElement(); !eee.isNull(); eee = eee.nextSiblingElement()) {
                                    QString tag(eee.tagName());
                                    if (tag == "sample") {
                                          sample = new Sample;
                                          if (!sample->read(path + "/" + eee.text())) {
                                                delete sample;
                                                sample = 0;
                                                }
                                          }
                                    else if (tag == "pitch") {
                                          QStringList s = eee.text().split(',');
                                          if (s.size() != 3) {
                                                fprintf(stderr, "syntax error in pitch: <%s>\n", qPrintable(eee.text()));
                                                return false;
                                                }
                                          keyBase = parsePitch(s[0]);
                                          keyLo   = parsePitch(s[1]);
                                          keyHi   = parsePitch(s[2]);
                                          }
                                    else
                                          domError(eee);
                                    }
                              if (  keyLo   >= 0 && keyLo   < 128
                                 && keyHi   >= 0 && keyHi   < 128
                                 && keyBase >= 0 && keyBase < 128
                                 && veloLo  >= 0 && veloLo  < 128
                                 && veloHi  >= 0 && veloHi  < 128
                                 && sample) {
                                    Zone* z = new Zone(keyLo, keyHi, veloLo, veloHi);
                                    z->setSample(sample);
                                    z->setKeyBase(keyBase);
                                    zones.append(z);
                                    }
                              else {
                                    fprintf(stderr, "bad zone values %d-%d %d-%d %d 0x%p\n",
                                       keyLo, keyHi, veloLo, veloHi, keyBase, sample);
                                    delete sample;
                                    return false;
                                    }
                              }
                        else
                              domError(ee);
                        }
                  }
            else
                  domError(e);
            }
      return true;
      }

//---------------------------------------------------------
//   match
//---------------------------------------------------------

bool Zone::match(int k, int v) const
      {
      return (k >= _keyLo) && (k <= _keyHi) && (v >= _veloLo) && (v <= _veloHi);
      }

//---------------------------------------------------------
//   zone
//---------------------------------------------------------

Zone* Instrument::zone(int key, int velocity) const
      {
      foreach(Zone* z, zones) {
            if (z->match(key, velocity))
                  return z;
            }
      printf("no zone for %d %d\n", key, velocity);
      return 0;
      }


