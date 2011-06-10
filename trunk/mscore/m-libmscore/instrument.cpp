//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: instrument.cpp 3693 2010-11-09 17:23:35Z wschweer $
//
//  Copyright (C) 2008-2010 Werner Schweer and others
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

#include "instrument_p.h"
#include "m-al/xml.h"
#include "drumset.h"
#include "articulation.h"
#include "utils.h"
#include "seq.h"
#include "tablature.h"

Instrument InstrumentList::defaultInstrument;

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void NamedEventList::read(XmlReader* r)
      {
      while (r->readAttribute()) {
            if (r->tag() == "name")
                  name = r->stringValue();
            }
      while (r->readElement()) {
            MString8 tag = r->tag();
            if (tag == "program") {
                  SeqEvent ev(ME_CONTROLLER);
                  ev.setController(CTRL_PROGRAM);
                  int val = 0;
                  while (r->readAttribute()) {
                        if (r->tag() == "value")
                              val = r->intValue();
                        }
                  ev.setValue(val);
                  events.append(ev);
                  r->read();
                  }
            else if (tag == "controller") {
                  SeqEvent ev(ME_CONTROLLER);
                  int ctrl  = 0;
                  int value = 0;
                  while (r->readAttribute()) {
                        if (r->tag() == "ctrl")
                              ctrl = r->intValue();
                        else if (r->tag() == "value")
                              value = r->intValue();
                        }
                  ev.setController(ctrl);
                  ev.setValue(value);
                  events.append(ev);
                  r->read();
                  }
            else if (r->readString("descr", &descr))
                  ;
            else
                  r->unknown();
            }
      }

//---------------------------------------------------------
//   operator
//---------------------------------------------------------

bool MidiArticulation::operator==(const MidiArticulation& i) const
      {
      return (i.name == name) && (i.velocity == velocity) && (i.gateTime == gateTime);
      }

//---------------------------------------------------------
//   Instrument
//---------------------------------------------------------

InstrumentData::InstrumentData()
      {
      Channel a;
      a.name  = "normal";
      _channel.append(a);

      _minPitchA   = 0;
      _maxPitchA   = 127;
      _minPitchP   = 0;
      _maxPitchP   = 127;
      _useDrumset  = false;
      _drumset     = 0;
      _tablature   = 0;
      }

InstrumentData::InstrumentData(const InstrumentData& i)
   : QSharedData(i)
      {
//      _longName     = i._longName;
//      _shortName    = i._shortName;
      _trackName    = i._trackName;
      _minPitchA    = i._minPitchA;
      _maxPitchA    = i._maxPitchA;
      _minPitchP    = i._minPitchP;
      _maxPitchP    = i._maxPitchP;
      _transpose    = i._transpose;
      _useDrumset   = i._useDrumset;
      _drumset      = 0;
      _tablature    = 0;
      setDrumset(i._drumset);
      setTablature(i._tablature);
      _midiActions  = i._midiActions;
      _articulation = i._articulation;
      _channel      = i._channel;
      }

//---------------------------------------------------------
//   ~InstrumentData
//---------------------------------------------------------

InstrumentData::~InstrumentData()
      {
      delete _tablature;
      delete _drumset;
      }

//---------------------------------------------------------
//   tablature
//    If instrument has no tablature, return default
//    (guitar) tablature
//---------------------------------------------------------

Tablature* InstrumentData::tablature() const
      {
      return _tablature ? _tablature : &guitarTablature;
      }

//---------------------------------------------------------
//   InstrumentData::read
//---------------------------------------------------------

void InstrumentData::read(XmlReader* r)
      {
      int program = -1;
      int bank    = 0;
      int chorus  = 30;
      int reverb  = 30;
      int volume  = 100;
      int pan     = 60;
      bool customDrumset = false;

      _channel.clear();
      while (r->readElement()) {
            MString8 tag = r->tag();
            QString val;

            if (r->readString("longName", &val))
                  ; // _longName = QTextDocumentFragment::fromHtml(Xml::htmlToString(e));
            else if (r->readString("shortName", &val))
                  ; // _shortName = QTextDocumentFragment::fromHtml(Xml::htmlToString(e));
            else if (r->readString("trackName", &_trackName))
                  ;
            else if (r->readChar("minPitchA", &_minPitchA))
                  ;
            else if (r->readChar("minPitchP", &_minPitchP))
                  ;
            else if (r->readChar("maxPitchA", &_maxPitchA))
                  ;
            else if (r->readChar("maxPitchP", &_maxPitchP))
                  ;
            else if (r->readChar("transposeChromatic", &_transpose.chromatic))
                  ;
            else if (r->readChar("transposeDiatonic", &_transpose.diatonic))
                  ;
            else if (r->readBool("useDrumset", &_useDrumset)) {
                  if (_useDrumset)
                        _drumset = new Drumset(*smDrumset);
                  }
            else if (tag == "Drum") {
                  // if we see on of this tags, a custom drumset will
                  // be created
                  if (_drumset == 0)
                        _drumset = new Drumset(*smDrumset);
                  if (!customDrumset) {
                        _drumset->clear();
                        customDrumset = true;
                        }
                  _drumset->load(r);
                  }
            else if (tag == "Tablature") {
                  _tablature = new Tablature();
                  _tablature->read(r);
                  }
            else if (tag == "MidiAction") {
                  NamedEventList a;
                  a.read(r);
                  _midiActions.append(a);
                  }
            else if (tag == "Articulation") {
                  MidiArticulation a;
                  a.read(r);
                  _articulation.append(a);
                  }
            else if (tag == "Channel" || tag == "channel") {
                  Channel a;
                  a.read(r);
                  _channel.append(a);
                  }
            else
                  r->unknown();
            }
      if (_channel.isEmpty()) {      // for backward compatibility
            Channel a;
            a.chorus  = chorus;
            a.reverb  = reverb;
            a.name    = "normal";
            a.program = program;
            a.bank    = bank;
            a.volume  = volume;
            a.pan     = pan;
            _channel.append(a);
            }
      if (_useDrumset) {
            if (_channel[0].bank == 0)
                  _channel[0].bank = 128;
            _channel[0].updateInitList();
            }
      }

//---------------------------------------------------------
//   action
//---------------------------------------------------------

NamedEventList* InstrumentData::midiAction(const QString& s, int channelIdx) const
      {
      // first look in channel list

      foreach(const NamedEventList& a, _channel[channelIdx].midiActions) {
            if (s == a.name)
                  return const_cast<NamedEventList*>(&a);
            }

      foreach(const NamedEventList& a, _midiActions) {
            if (s == a.name)
                  return const_cast<NamedEventList*>(&a);
            }
      return 0;
      }

//---------------------------------------------------------
//   Channel
//---------------------------------------------------------

Channel::Channel()
      {
      for(int i = 0; i < A_INIT_COUNT; ++i)
            init.append(0);
      channel  = -1;
      program  = -1;
      bank     = 0;
      volume   = 100;
      pan      = 64;
      chorus   = 30;
      reverb   = 30;

      mute     = false;
      solo     = false;
      soloMute = false;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Channel::read(XmlReader* r)
      {
      while (r->readAttribute()) {
            if (r->tag() == "name")
                  name = r->stringValue();
            }
      while (r->readElement()) {
            MString8 tag = r->tag();
            QString val;
            if (tag == "program") {
                  while (r->readAttribute()) {
                        if (r->tag() == "value")
                              program = r->intValue();
                        }
                  r->read();
                  }
            else if (tag == "controller") {
                  int value = 0;
                  int ctrl  = 0;
                  while (r->readAttribute()) {
                        if (r->tag() == "value")
                              value = r->intValue();
                        else if (r->tag() == "ctrl")
                              ctrl = r->intValue();
                        }
                  switch(ctrl) {
                        case CTRL_HBANK:
                              bank = (value << 7) + (bank & 0x7f);
                              break;
                        case CTRL_LBANK:
                              bank = (bank & ~0x7f) + (value & 0x7f);
                              break;
                        case CTRL_VOLUME:
                              volume = value;
                              break;
                        case CTRL_PANPOT:
                              pan = value;
                              break;
                        case CTRL_CHORUS_SEND:
                              chorus = value;
                              break;
                        case CTRL_REVERB_SEND:
                              reverb = value;
                              break;
                        default:
                              {
                              SeqEvent e(ME_CONTROLLER);
                              e.setController(ctrl);
                              e.setValue(value);
                              init.append(e);
                              }
                              break;
                        }
                  r->read();
                  }
            else if (tag == "Articulation") {
                  MidiArticulation a;
                  a.read(r);
                  articulation.append(a);
                  }
            else if (tag == "MidiAction") {
                  NamedEventList a;
                  a.read(r);
                  midiActions.append(a);
                  }
            else if (tag == "synti") {
                  r->skipElement((xmlChar*)"synti");
                  }
            else if (r->readString("descr", &descr))
                  ;
            else
                  r->unknown();
            }
      updateInitList();
      }

//---------------------------------------------------------
//   updateInitList
//---------------------------------------------------------

void Channel::updateInitList() const
      {
      for (int i = 0; i < A_INIT_COUNT; ++i) {
            // delete init[i];      memory leak
            init[i] = 0;
            }
      SeqEvent e;
      if (program != -1) {
            e.setType(ME_CONTROLLER);
            e.setController(CTRL_PROGRAM);
            e.setValue(program);
            init[A_PROGRAM] = e;
            }

      e.setType(ME_CONTROLLER);
      e.setController(CTRL_HBANK);
      e.setValue((bank >> 7) & 0x7f);
      init[A_HBANK] = e;

      e.setType(ME_CONTROLLER);
      e.setController(CTRL_LBANK);
      e.setValue(bank & 0x7f);
      init[A_LBANK] = e;

      e.setType(ME_CONTROLLER);
      e.setController(CTRL_VOLUME);
      e.setValue(volume);
      init[A_VOLUME] = e;

      e.setType(ME_CONTROLLER);
      e.setController(CTRL_PANPOT);
      e.setValue(pan);
      init[A_PAN] = e;

      e.setType(ME_CONTROLLER);
      e.setController(CTRL_CHORUS_SEND);
      e.setValue(chorus);
      init[A_CHORUS] = e;

      e.setType(ME_CONTROLLER);
      e.setController(CTRL_REVERB_SEND);
      e.setValue(reverb);
      init[A_REVERB] = e;
      }

//---------------------------------------------------------
//   channelIdx
//---------------------------------------------------------

int InstrumentData::channelIdx(const QString& s) const
      {
      int idx = 0;
      foreach(const Channel& a, _channel) {
            if (a.name.isEmpty() && s == "normal")
                  return idx;
            if (s == a.name)
                  return idx;
            ++idx;
            }
      return -1;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void MidiArticulation::read(XmlReader* r)
      {
      while (r->readAttribute()) {
            if (r->tag() == "name")
                  name = r->stringValue();
            }
      while (r->readElement()) {
            MString8 tag = r->tag();
            QString text;
            if (r->readString("velocity", &text)) {
                  if (text.endsWith("%"))
                        text = text.left(text.size()-1);
                  velocity = text.toInt();
                  }
            else if (r->readString("gateTime", &text)) {
                  if (text.endsWith("%"))
                        text = text.left(text.size()-1);
                  gateTime = text.toInt();
                  }
            else if (r->readString("descr", &descr))
                  ;
            else
                  r->unknown();
            }
      }

//---------------------------------------------------------
//   updateVelocity
//---------------------------------------------------------

void InstrumentData::updateVelocity(int* velocity, int channelIdx, const QString& name)
      {
      const Channel& c = _channel[channelIdx];
      foreach(const MidiArticulation& a, c.articulation) {
            if (a.name == name) {
                  *velocity = *velocity * a.velocity / 100;
                  return;
                  }
            }
      foreach(const MidiArticulation& a, _articulation) {
            if (a.name == name) {
                  *velocity = *velocity * a.velocity / 100;
                  return;
                  }
            }
      }

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool InstrumentData::operator==(const InstrumentData& i) const
      {
      return i._minPitchA == _minPitchA
         &&  i._maxPitchA == _maxPitchA
         &&  i._minPitchP == _minPitchP
         &&  i._maxPitchP == _maxPitchP
         &&  i._useDrumset == _useDrumset
         &&  i._midiActions == _midiActions
         &&  i._channel == _channel
         &&  i._articulation == _articulation
         &&  i._transpose.diatonic == _transpose.diatonic
         &&  i._transpose.chromatic == _transpose.chromatic
//         &&  i._longName.toHtml() == _longName.toHtml()
//         &&  i._shortName.toHtml() == _shortName.toHtml()
         &&  i._trackName == _trackName
         ;
      }

//---------------------------------------------------------
//   setUseDrumset
//---------------------------------------------------------

void InstrumentData::setUseDrumset(bool val)
      {
      _useDrumset = val;
      if (val && _drumset == 0) {
            _drumset = new Drumset(*smDrumset);
            }
      }

//---------------------------------------------------------
//   setDrumset
//---------------------------------------------------------

void InstrumentData::setDrumset(Drumset* ds)
      {
      delete _drumset;
      if (ds)
            _drumset = new Drumset(*ds);
      else
            _drumset = 0;
      }

//---------------------------------------------------------
//   setTablature
//---------------------------------------------------------

void InstrumentData::setTablature(Tablature* t)
      {
      delete _tablature;
      if (t)
            _tablature = new Tablature(*t);
      else
            _tablature = 0;
      }

//---------------------------------------------------------
//   Instrument
//---------------------------------------------------------

Instrument::Instrument()
      {
      d = new InstrumentData;
      }

Instrument::Instrument(const Instrument& s)
   : d(s.d)
      {
      }

Instrument::~Instrument()
      {
      }

//---------------------------------------------------------
//   operator=
//---------------------------------------------------------

Instrument& Instrument::operator=(const Instrument& s)
      {
      d = s.d;
      return *this;
      }

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool Instrument::operator==(const Instrument& s) const
      {
      return d->operator==(*s.d);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Instrument::read(XmlReader* e)
      {
      d->read(e);
      }

//---------------------------------------------------------
//   midiAction
//---------------------------------------------------------

NamedEventList* Instrument::midiAction(const QString& s, int channel) const
      {
      return d->midiAction(s, channel);
      }

//---------------------------------------------------------
//   channelIdx
//---------------------------------------------------------

int Instrument::channelIdx(const QString& s) const
      {
      return d->channelIdx(s);
      }

//---------------------------------------------------------
//   updateVelocity
//---------------------------------------------------------

void Instrument::updateVelocity(int* velocity, int channel, const QString& name)
      {
      d->updateVelocity(velocity, channel, name);
      }

//---------------------------------------------------------
//   minPitchP
//---------------------------------------------------------

int Instrument::minPitchP() const
      {
      return d->_minPitchP;
      }

//---------------------------------------------------------
//   maxPitchP
//---------------------------------------------------------

int Instrument::maxPitchP() const
      {
      return d->_maxPitchP;
      }

//---------------------------------------------------------
//   minPitchA
//---------------------------------------------------------

int Instrument::minPitchA() const
      {
      return d->_minPitchA;
      }

//---------------------------------------------------------
//   maxPitchA
//---------------------------------------------------------

int Instrument::maxPitchA() const
      {
      return d->_maxPitchA;
      }

//---------------------------------------------------------
//   setMinPitchP
//---------------------------------------------------------

void Instrument::setMinPitchP(int v)
      {
      d->setMinPitchP(v);
      }

//---------------------------------------------------------
//   setMaxPitchP
//---------------------------------------------------------

void Instrument::setMaxPitchP(int v)
      {
      d->setMaxPitchP(v);
      }

//---------------------------------------------------------
//   setMinPitchA
//---------------------------------------------------------

void Instrument::setMinPitchA(int v)
      {
      d->setMinPitchA(v);
      }

//---------------------------------------------------------
//   setMaxPitchA
//---------------------------------------------------------

void Instrument::setMaxPitchA(int v)
      {
      d->setMaxPitchA(v);
      }

//---------------------------------------------------------
//   transpose
//---------------------------------------------------------

Interval Instrument::transpose() const
      {
      return d->transpose();
      }

//---------------------------------------------------------
//   setTranspose
//---------------------------------------------------------

void Instrument::setTranspose(const Interval& v)
      {
      d->setTranspose(v);
      }

//---------------------------------------------------------
//   setDrumset
//---------------------------------------------------------

void Instrument::setDrumset(Drumset* ds)
      {
      d->setDrumset(ds);
      }

//---------------------------------------------------------
//   drumset
//---------------------------------------------------------

Drumset* Instrument::drumset() const
      {
      return d->drumset();
      }

//---------------------------------------------------------
//   useDrumset
//---------------------------------------------------------

bool Instrument::useDrumset() const
      {
      return d->useDrumset();
      }

//---------------------------------------------------------
//   setUseDrumset
//---------------------------------------------------------

void Instrument::setUseDrumset(bool val)
      {
      d->setUseDrumset(val);
      }

//---------------------------------------------------------
//   setAmateurPitchRange
//---------------------------------------------------------

void Instrument::setAmateurPitchRange(int a, int b)
      {
      d->setAmateurPitchRange(a, b);
      }

//---------------------------------------------------------
//   setProfessionalPitchRange
//---------------------------------------------------------

void Instrument::setProfessionalPitchRange(int a, int b)
      {
      d->setProfessionalPitchRange(a, b);
      }

//---------------------------------------------------------
//   channel
//---------------------------------------------------------

Channel& Instrument::channel(int idx)
      {
      return d->channel(idx);
      }

//---------------------------------------------------------
//   channel
//---------------------------------------------------------

const Channel& Instrument::channel(int idx) const
      {
      return d->channel(idx);
      }

//---------------------------------------------------------
//   midiActions
//---------------------------------------------------------

const QList<NamedEventList>& Instrument::midiActions() const
      {
      return d->midiActions();
      }

//---------------------------------------------------------
//   articulation
//---------------------------------------------------------

const QList<MidiArticulation>& Instrument::articulation() const
      {
      return d->articulation();
      }

//---------------------------------------------------------
//   channel
//---------------------------------------------------------

const QList<Channel>& Instrument::channel() const
      {
      return d->channel();
      }

//---------------------------------------------------------
//   setMidiActions
//---------------------------------------------------------

void Instrument::setMidiActions(const QList<NamedEventList>& l)
      {
      d->setMidiActions(l);
      }

//---------------------------------------------------------
//   setArticulation
//---------------------------------------------------------

void Instrument::setArticulation(const QList<MidiArticulation>& l)
      {
      d->setArticulation(l);
      }

//---------------------------------------------------------
//   setChannel
//---------------------------------------------------------

void Instrument::setChannel(const QList<Channel>& l)
      {
      d->setChannel(l);
      }

//---------------------------------------------------------
//   setChannel
//---------------------------------------------------------

void Instrument::setChannel(int i, const Channel& c)
      {
      d->setChannel(i, c);
      }

//---------------------------------------------------------
//   tablature
//---------------------------------------------------------

Tablature* Instrument::tablature() const
      {
      return d->tablature();
      }

//---------------------------------------------------------
//   setTablature
//---------------------------------------------------------

void Instrument::setTablature(Tablature* t)
      {
      d->setTablature(t);
      }

//---------------------------------------------------------
//   instrument
//---------------------------------------------------------

const Instrument& InstrumentList::instrument(int tick) const
      {
      if (empty())
            return defaultInstrument;
      ciInstrument i = upper_bound(tick);
      if (i == begin())
            return defaultInstrument;
      --i;
      return i->second;
      }

//---------------------------------------------------------
//   instrument
//---------------------------------------------------------

Instrument& InstrumentList::instrument(int tick)
      {
      if (empty())
            return defaultInstrument;
      iInstrument i = upper_bound(tick);
      if (i == begin())
            return defaultInstrument;
      --i;
      return i->second;
      }

//---------------------------------------------------------
//   setInstrument
//---------------------------------------------------------

void InstrumentList::setInstrument(const Instrument& instr, int tick)
      {
      std::pair<int, Instrument> instrument(tick, instr);
      std::pair<iInstrument,bool> p = insert(instrument);
      if (!p.second)
            (*this)[tick] = instr;
      }

#if 0
//---------------------------------------------------------
//   longName
//---------------------------------------------------------

const QTextDocumentFragment& Instrument::longName() const
      {
      return d->_longName;
      }

//---------------------------------------------------------
//   shortName
//---------------------------------------------------------

const QTextDocumentFragment& Instrument::shortName() const
      {
      return d->_shortName;
      }

//---------------------------------------------------------
//   longName
//---------------------------------------------------------

QTextDocumentFragment& Instrument::longName()
      {
      return d->_longName;
      }

//---------------------------------------------------------
//   setLongName
//---------------------------------------------------------

void Instrument::setLongName(const QTextDocumentFragment& f)
      {
      d->_longName = f;
      }

//---------------------------------------------------------
//   setShortName
//---------------------------------------------------------

void Instrument::setShortName(const QTextDocumentFragment& f)
      {
      d->_shortName = f;
      }

//---------------------------------------------------------
//   shortName
//---------------------------------------------------------

QTextDocumentFragment& Instrument::shortName()
      {
      return d->_shortName;
      }
#endif

//---------------------------------------------------------
//   trackName
//---------------------------------------------------------

QString Instrument::trackName() const
      {
      return d->_trackName;
      }

void Instrument::setTrackName(const QString& s)
      {
      d->_trackName = s;
      }

