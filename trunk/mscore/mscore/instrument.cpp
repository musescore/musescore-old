//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
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
#include "xml.h"
#include "drumset.h"
#include "articulation.h"
#include "utils.h"
#include "seq.h"
#include "tablature.h"
#include "instrtemplate.h"

Instrument InstrumentList::defaultInstrument;

//---------------------------------------------------------
//   parseInstrName
//---------------------------------------------------------

static QTextDocumentFragment parseInstrName(const QString& name)
      {
      if (name.isEmpty())
            return QTextDocumentFragment();
      QTextDocument doc;
      QTextCursor cursor(&doc);
      QTextCharFormat f = cursor.charFormat();
      QTextCharFormat sf(f);

      QFont font("MScore1");
      sf.setFont(font);

      QDomDocument dom;
      int line, column;
      QString err;
      if (!dom.setContent(name, false, &err, &line, &column)) {
            QString col, ln;
            col.setNum(column);
            ln.setNum(line);
            QString error = err + "\n at line " + ln + " column " + col;
            printf("parse instrument name: %s\n", qPrintable(error));
            printf("   data:<%s>\n", qPrintable(name));
            return QTextDocumentFragment();
            }

      for (QDomNode e = dom.documentElement(); !e.isNull(); e = e.nextSibling()) {
            for (QDomNode ee = e.firstChild(); !ee.isNull(); ee = ee.nextSibling()) {
                  QDomElement de1 = ee.toElement();
                  QString tag(de1.tagName());
                  if (tag == "symbol") {
                        QString name = de1.attribute(QString("name"));
                        if (name == "flat")
                              cursor.insertText(QString(0xe10d), sf);
                        else if (name == "sharp")
                              cursor.insertText(QString(0xe10c), sf);
                        }
                  QDomText t = ee.toText();
                  if (!t.isNull())
                        cursor.insertText(t.data(), f);
                  }
            }
      return QTextDocumentFragment(&doc);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void NamedEventList::write(Xml& xml, const QString& n) const
      {
      xml.stag(QString("%1 name=\"%2\"").arg(n).arg(name));
      if (!descr.isEmpty())
            xml.tag("descr", descr);
      foreach(const Event& e, events)
            e.write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void NamedEventList::read(QDomElement e)
      {
      name = e.attribute("name");
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "program") {
                  Event ev(ME_CONTROLLER);
                  ev.setController(CTRL_PROGRAM);
                  ev.setValue(e.attribute("value", "0").toInt());
                  events.append(ev);
                  }
            else if (tag == "controller") {
                  Event ev(ME_CONTROLLER);
                  ev.setController(e.attribute("ctrl", "0").toInt());
                  ev.setValue(e.attribute("value", "0").toInt());
                  events.append(ev);
                  }
            else if (tag == "descr")
                  descr = e.text();
            else
                  domError(e);
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
      _longName     = i._longName;
      _shortName    = i._shortName;
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
//   write
//---------------------------------------------------------

void InstrumentData::write(Xml& xml) const
      {
      xml.stag("Instrument");
      xml.stag("longName");
      xml.writeHtml(_longName.toHtml());
      xml.etag();
      xml.stag("shortName");
      xml.writeHtml(_shortName.toHtml());
      xml.etag();
      xml.tag("trackName", _trackName);
      if (_minPitchP > 0)
            xml.tag("minPitchP", _minPitchP);
      if (_maxPitchP < 127)
            xml.tag("maxPitchP", _maxPitchP);
      if (_minPitchA > 0)
            xml.tag("minPitchA", _minPitchA);
      if (_maxPitchA < 127)
            xml.tag("maxPitchA", _maxPitchA);
      if (_transpose.diatonic)
            xml.tag("transposeDiatonic", _transpose.diatonic);
      if (_transpose.chromatic)
            xml.tag("transposeChromatic", _transpose.chromatic);
      if (_useDrumset) {
            xml.tag("useDrumset", _useDrumset);
            _drumset->save(xml);
            }
      if (_tablature)
            _tablature->write(xml);
      foreach(const NamedEventList& a, _midiActions)
            a.write(xml, "MidiAction");
      foreach(const MidiArticulation& a, _articulation)
            a.write(xml);
      foreach(const Channel& a, _channel)
            a.write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   InstrumentData::read
//---------------------------------------------------------

void InstrumentData::read(QDomElement e)
      {
      int program = -1;
      int bank    = 0;
      int chorus  = 30;
      int reverb  = 30;
      int volume  = 100;
      int pan     = 60;
      bool customDrumset = false;

      _channel.clear();
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();

            if (tag == "longName")
                  _longName = QTextDocumentFragment::fromHtml(Xml::htmlToString(e));
            else if (tag == "shortName")
                  _shortName = QTextDocumentFragment::fromHtml(Xml::htmlToString(e));
            else if (tag == "trackName")
                  _trackName = val;
            else if (tag == "minPitch") {      // obsolete
                  _minPitchP = i;
                  _minPitchA = i;
                  }
            else if (tag == "maxPitch") {       // obsolete
                  _maxPitchP = i;
                  _maxPitchA = i;
                  }
            else if (tag == "minPitchA")
                  _minPitchA = i;
            else if (tag == "minPitchP")
                  _minPitchP = i;
            else if (tag == "maxPitchA")
                  _maxPitchA = i;
            else if (tag == "maxPitchP")
                  _maxPitchP = i;
            else if (tag == "transposition") {    // obsolete
                  _transpose.chromatic = i;
                  _transpose.diatonic = chromatic2diatonic(i);
                  }
            else if (tag == "transposeChromatic")
                  _transpose.chromatic = i;
            else if (tag == "transposeDiatonic")
                  _transpose.diatonic = i;
            else if (tag == "useDrumset") {
                  _useDrumset = i;
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
                  _drumset->load(e);
                  }
            else if (tag == "Tablature") {
                  _tablature = new Tablature();
                  _tablature->read(e);
                  }
            else if (tag == "MidiAction") {
                  NamedEventList a;
                  a.read(e);
                  _midiActions.append(a);
                  }
            else if (tag == "Articulation") {
                  MidiArticulation a;
                  a.read(e);
                  _articulation.append(a);
                  }
            else if (tag == "Channel" || tag == "channel") {
                  Channel a;
                  a.read(e);
                  _channel.append(a);
                  }
            else if (tag == "chorus")     // obsolete
                  chorus = i;
            else if (tag == "reverb")     // obsolete
                  reverb = i;
            else if (tag == "midiProgram")  // obsolete
                  program = i;
            else if (tag == "volume")     // obsolete
                  volume = i;
            else if (tag == "pan")        // obsolete
                  pan = i;
            else if (tag == "midiChannel")      // obsolete
                  ;
            else
                  domError(e);
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
      synti    = 0;     // -1;
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
//   write
//---------------------------------------------------------

void Channel::write(Xml& xml) const
      {
      if (name.isEmpty())
            xml.stag("Channel");
      else
            xml.stag(QString("Channel name=\"%1\"").arg(name));
      if (!descr.isEmpty())
            xml.tag("descr", descr);
      updateInitList();
      foreach(const Event& e, init) {
            if (e.type() != ME_INVALID)
                  e.write(xml);
            }
      if (synti)
            xml.tag("synti", seq->synthIndexToName(synti));
      if (mute)
            xml.tag("mute", mute);
      if (solo)
            xml.tag("solo", solo);
      foreach(const NamedEventList& a, midiActions)
            a.write(xml, "MidiAction");
      foreach(const MidiArticulation& a, articulation)
            a.write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Channel::read(QDomElement e)
      {
      synti = 0;
      name = e.attribute("name");
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            if (tag == "program") {
                  program = e.attribute("value", "-1").toInt();
                  if (program == -1)
                        program = val.toInt();
                  }
            else if (tag == "controller") {
                  int value = e.attribute("value", "0").toInt();
                  int ctrl  = e.attribute("ctrl", "0").toInt();
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
                              Event e(ME_CONTROLLER);
                              e.setController(ctrl);
                              e.setValue(value);
                              init.append(e);
                              }
                              break;
                        }
                  }
            else if (tag == "Articulation") {
                  MidiArticulation a;
                  a.read(e);
                  articulation.append(a);
                  }
            else if (tag == "MidiAction") {
                  NamedEventList a;
                  a.read(e);
                  midiActions.append(a);
                  }
            else if (tag == "synti") {
                  int idx = seq->synthNameToIndex(val);
                  synti = idx == -1 ? val.toInt() : idx;
                  }
            else if (tag == "descr")
                  descr = e.text();
            else
                  domError(e);
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
      Event e;
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
//   write
//---------------------------------------------------------

void MidiArticulation::write(Xml& xml) const
      {
      xml.stag(QString("Articulation name=\"%1\"").arg(name));
      if (!descr.isEmpty())
            xml.tag("descr", descr);
      xml.tag("velocity", velocity);
      xml.tag("gateTime", gateTime);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void MidiArticulation::read(QDomElement e)
      {
      name = e.attribute("name");
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString text(e.text());
            if (tag == "velocity") {
                  if (text.endsWith("%"))
                        text = text.left(text.size()-1);
                  velocity = text.toInt();
                  }
            else if (tag == "gateTime") {
                  if (text.endsWith("%"))
                        text = text.left(text.size()-1);
                  gateTime = text.toInt();
                  }
            else if (tag == "descr")
                  descr = e.text();
            else
                  domError(e);
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
         &&  i._longName.toHtml() == _longName.toHtml()
         &&  i._shortName.toHtml() == _shortName.toHtml()
         &&  i._trackName == _trackName
         &&  i.tablature() == tablature();
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

void Instrument::read(QDomElement e)
      {
      d->read(e);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Instrument::write(Xml& xml) const
      {
      d->write(xml);
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

//---------------------------------------------------------
//   fromTemplate
//---------------------------------------------------------

Instrument Instrument::fromTemplate(const InstrumentTemplate* t)
      {
      Instrument instr;
      instr.setAmateurPitchRange(t->minPitchA, t->maxPitchA);
      instr.setProfessionalPitchRange(t->minPitchP, t->maxPitchP);
      instr.longName() = parseInstrName(t->longName);
      instr.shortName() = parseInstrName(t->shortName);
      instr.setTrackName(t->trackName);
      instr.setTranspose(t->transpose);
      if (t->useDrumset) {
            instr.setUseDrumset(true);
            instr.setDrumset(new Drumset(*((t->drumset) ? t->drumset : smDrumset)));
            }
      instr.setMidiActions(t->midiActions);
      instr.setArticulation(t->articulation);
      instr.setChannel(t->channel);
      instr.setTablature(t->tablature ? new Tablature(*t->tablature) : 0);
      return instr;
      }

