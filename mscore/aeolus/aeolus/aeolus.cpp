/*
    Copyright (C) 2003-2008 Fons Adriaensen <fons@kokkinizita.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "aeolus.h"
#include "model.h"

extern QString dataPath;
extern QString mscoreGlobalShare;

#include "event.h"

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void Aeolus::init(int samplerate)
      {
      setlocale(LC_ALL, "C"); // scanf of floats does not work otherwise

      QString stops = mscoreGlobalShare + "/sound/aeolus/stops";
      int n = strlen(qPrintable(stops));
      char* stopsPath = new char[n+1];
      strcpy(stopsPath, qPrintable(stops));

      QDir dir;
      QString waves = dataPath + "/aeolus/waves";
      dir.mkpath(waves);
      n = strlen(qPrintable(waves));
      char* wavesPath = new char[n+1];
      strcpy(wavesPath, qPrintable(waves));

      audio_init(samplerate);
      model = new Model (this, _midimap, stopsPath, "Aeolus", wavesPath);

      audio_start();
      model->init();
      printGui();
      }

//---------------------------------------------------------
//   Aeolus
//---------------------------------------------------------

Aeolus::Aeolus()
      {
      model = 0;
      MidiPatch* patch = new MidiPatch;
      patch->drum = false;
      patch->bank = 0;
      patch->prog = 0;
      patch->name = "Aeolus";
      patchList.append(patch);
      _sc_cmode = 0;
      _sc_group = 0;
      _audio = new M_audio_info;
      _running = false;
      _nplay = 0;
      _fsamp = 0;
      _nasect = 0;
      _ndivis = 0;
      nout = 0;
      _ifc_init = 0;
      for (int i = 0; i < NGROUP; i++)
            _ifelms [i] = 0;
      }

Aeolus::~Aeolus()
      {
      delete model;
      for (int i = 0; i < _nasect; i++)
            delete _asectp [i];
      for (int i = 0; i < _ndivis; i++)
            delete _divisp [i];
      _reverb.fini ();
      }

void Aeolus::setMasterTuning(double)
      {
      }

double Aeolus::masterTuning() const
      {
      return 440.0;
      }

bool Aeolus::loadSoundFont(const QString&)
      {
      return true;
      }

QString Aeolus::soundFont() const
      {
      return QString();
      }

//---------------------------------------------------------
//   play
//---------------------------------------------------------

void Aeolus::play(const Event& event)
      {
      int ch   = event.channel();
      int type = event.type();
      int m    = _midimap [ch] & 127;        // Keyboard and hold bits
      int f    = (_midimap [ch] >> 12) & 7;  // Control enabled if (f & 4)
      if (type == ME_NOTEON) {
            int n = event.dataA();
            int v = event.dataB();
            if (v == 0) {   // note off
                  if (n < 36)
                        ;
                  else if (n <= 96)
                        key_off(n - 36, m);
                  }
            else {            // note on
                  if (n < 36)
                        ;
                  else if (n <= 96)
                        key_on(n - 36, m);
                  }
            }
      else if (type == ME_CONTROLLER) {
            int p = event.dataA();
            int v = event.dataB();
            switch(p) {
                  case MIDICTL_HOLD:
                  case MIDICTL_ASOFF:
                  case MIDICTL_ANOFF:
                        break;
                  case MIDICTL_BANK:
                        break;
                  case MIDICTL_IFELM:
                        if (!(f & 4))
                              break;
                        if (v & 64) {
                              // Set mode or clear group.
                              _sc_cmode = (v >> 4) & 3;
                              _sc_group = v & 7;
                              if (_sc_cmode == 0)
                                    model->clr_group(_sc_group);
                              }
                        else if (_sc_cmode) {
                              // Set, reset or toggle stop.
                              model->set_ifelm (_sc_group, v & 31, _sc_cmode - 1);
                              }
                        break;
                  }
            }
      }

const QList<MidiPatch*>& Aeolus::getPatchInfo() const
      {
      return patchList;
      }

double Aeolus::masterGain() const
      {
      return 1.0;
      }

void Aeolus::setMasterGain(double)
      {
      }

double Aeolus::effectParameter(int /*effect*/, int /*parameter*/)
      {
      return 0.0;
      }

void Aeolus::setEffectParameter(int /*effect*/, int /*parameter*/, double /*value*/ )
      {
      }

void Aeolus::rewrite_label (const char *p)
      {
      strcpy (_tempstr, p);
      char* t = strstr (_tempstr, "-$");
      if (t)
            strcpy (t, t + 2);
      else {
            t = strchr (_tempstr, '$');
            if (t)
                  *t = ' ';
            }
      }

//---------------------------------------------------------
//   printGui
//---------------------------------------------------------

void Aeolus::printGui()
      {
      for (int i = 0; i < _ifc_init->_ndivis; ++i) {
            int group = i;
            rewrite_label (_ifc_init->_groupd [group]._label);
            printf ("Stops in group %s\n", _tempstr);
            uint32_t m = _ifelms [group];
            int n = _ifc_init->_groupd [group]._nifelm;
            for (int i = 0; i < n; i++) {
                  rewrite_label (_ifc_init->_groupd [group]._ifelmd [i]._label);
                  printf ("  %c %-7s %-1s\n", (m & 1) ? '+' : '-', _ifc_init->_groupd [group]._ifelmd [i]._mnemo, _tempstr);
                  m >>= 1;
                  }
            }
      }


