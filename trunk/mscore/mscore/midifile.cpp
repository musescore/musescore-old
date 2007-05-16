//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2007 Werner Schweer (ws@seh.de)
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

#include "midifile.h"
#include "xml.h"

#define BE_SHORT(x) ((((x)&0xFF)<<8) | (((x)>>8)&0xFF))
#ifdef __i486__
#define BE_LONG(x) \
     ({ int __value; \
        asm ("bswap %1; movl %1,%0" : "=g" (__value) : "r" (x)); \
       __value; })
#else
#define BE_LONG(x) ((((x)&0xFF)<<24) | \
		      (((x)&0xFF00)<<8) | \
		      (((x)&0xFF0000)>>8) | \
		      (((x)>>24)&0xFF))
#endif

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void MidiNote::dump(Xml& xml) const
      {
      xml.tagE(QString("Note  tick=\"%1\" channel=\"%2\" len=\"%3\" pitch=\"%4\" velo=\"%5\"")
         .arg(ontime()).arg(channel()).arg(duration()).arg(pitch()).arg(velo()));
      }

void MidiNoteOn::dump(Xml& xml) const
      {
      xml.tagE(QString("NoteOn  tick=\"%1\" channel=\"%2\" pitch=\"%3\" velo=\"%4\"")
            .arg(ontime()).arg(channel()).arg(pitch()).arg(velo()));
      }

void MidiNoteOff::dump(Xml& xml) const
      {
      xml.tagE(QString("NoteOff  tick=\"%1\" channel=\"%2\" pitch=\"%3\" velo=\"%4\"")
            .arg(ontime()).arg(channel()).arg(pitch()).arg(velo()));
      }

void MidiController::dump(Xml& xml) const
      {
      xml.tagE(QString("Controller tick=\"%1\" channel=\"%2\"").arg(ontime()).arg(channel()));
      }

void MidiSysex::dump(Xml& xml) const
      {
      xml.tagE(QString("Sysex tick=\"%1\"").arg(ontime()));
      }

void MidiMeta::dump(Xml& xml) const
      {
      switch(metaType()) {
            case META_TRACK_NAME:
                  xml.tag("TrackName",  QString("tick=\"%1\"").arg(ontime()), QString((char*)(data())));
                  break;
            case META_LYRIC:
                  xml.tag("Lyric",  QString("tick=\"%1\"").arg(ontime()), QString((char*)(data())));
                  break;
            case META_KEY_SIGNATURE:
                  {
                  const char* keyTable[] = {
                        "Ces", "Ges", "Des", "As", "Es", "Bes", "F",
                        "C",
                        "G", "D", "A", "E", "B", "Fis", "Cis"
                        };
                  int key = (char)(_data[0]) + 7;
                  if (key < 0 || key > 14) {
                        printf("bad key signature %d\n", key);
                        key = 0;
                        }
                  QString sex(_data[1] ? "Minor" : "Major");
                  QString keyName(keyTable[key]);
                  xml.tag("Key",  QString("tick=\"%1\"").arg(ontime()), QString("%1 %2").arg(keyName).arg(sex));
                  }
                  break;
            case META_TIME_SIGNATURE:
                  xml.tagE(QString("TimeSig tick=\"%1\" num=\"%2\" denom=\"%3\" metro=\"%4\" quarter=\"%5\"")
                     .arg(ontime())
                     .arg(int(_data[0]))
                     .arg(int(_data[1]))
                     .arg(int(_data[2]))
                     .arg(int(_data[3])));
                  break;

            case META_TEMPO:
            default:
                  xml.tagE(QString("Meta tick=\"%1\" type=\"%2\"").arg(ontime()).arg(int(metaType())));
                  break;
            }
      }

//---------------------------------------------------------
//   MidiFile
//---------------------------------------------------------

MidiFile::MidiFile()
      {
      fp        = 0;
      timesig_z = 4;
      timesig_n = 4;
      curPos    = 0;
      _midiType = MT_UNKNOWN;
      }

//---------------------------------------------------------
//   write
//    returns true on error
//---------------------------------------------------------

bool MidiFile::write(QIODevice* out)
      {
      fp = out;
      write("MThd", 4);
      writeLong(6);                 // header len
      writeShort(1);                // format
      writeShort(_tracks.size());
      writeShort(_division);
      foreach (const MidiTrack* t, _tracks) {
            if (writeTrack(t))
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   writeTrack
//---------------------------------------------------------

bool MidiFile::writeTrack(const MidiTrack* t)
      {
      write("MTrk", 4);
      qint64 lenpos = fp->pos();
      writeLong(0);                 // dummy len

      status   = -1;
      int tick = 0;
      const EventList events = t->events();
      for (ciEvent i = events.begin(); i != events.end(); ++i) {
            int ntick     = i.key();
            MidiEvent* ev = i.value();
            putvl(ntick - tick);    // write tick delta
            if (ev->isChannelEvent())
                  ((MidiChannelEvent*)ev)->setChannel(t->outChannel());
            ev->write(this);
            tick = ntick;
            }

      //---------------------------------------------------
      //    write "End Of Track" Meta
      //    write Track Len
      //

      putvl(0);
      put(0xff);        // Meta
      put(0x2f);        // EOT
      putvl(0);         // len 0
      qint64 endpos = fp->pos();
      fp->seek(lenpos);
      writeLong(endpos-lenpos-4);   // tracklen
      fp->seek(endpos);
      return false;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void MidiNoteOn::write(MidiFile* mf) const
      {
      mf->writeStatus(ME_NOTEON, channel());
      mf->put(_pitch);
      mf->put(_velo);
      }

void MidiNoteOff::write(MidiFile* mf) const
      {
      mf->writeStatus(ME_NOTEOFF, channel());
      mf->put(_pitch);
      mf->put(_velo);
      }

void MidiController::write(MidiFile* mf) const
      {
      }

void MidiMeta::write(MidiFile* mf) const
      {
      }

void MidiSysex::write(MidiFile* mf) const
      {
      mf->put(0xf0);
      mf->putvl(len() + 1);  // including 0xf7
      mf->write(data(), len());
      mf->put(0xf7);
      mf->resetRunningStatus();
      }

//---------------------------------------------------------
//   writeStatus
//---------------------------------------------------------

void MidiFile::writeStatus(int nstat, int c)
      {
      nstat |= c;
      //
      //  running status; except for Sysex- and Meta Events
      //
      if (((nstat & 0xf0) != 0xf0) && (nstat != status)) {
            status = nstat;
            put(nstat);
            }
      }
#if 0
      switch (event->type) {
            case ME_NOTEOFF:
            case ME_NOTEON:
            case ME_POLYAFTER:
            case ME_CONTROLLER:
            case ME_PITCHBEND:
                  put(event->dataA);
                  put(event->dataB);
                  break;
            case ME_PROGRAM:        // Program Change
            case ME_AFTERTOUCH:     // Channel Aftertouch
                  put(event->dataA);
                  break;
            case ME_SYSEX:
                  put(0xf0);
                  putvl(event->duration + 1);  // including 0xf7
                  write(event->data, event->dataLen);
                  put(0xf7);
                  status = -1;      // invalidate running status
                  break;
            }
      }
#endif

//---------------------------------------------------------
//   readMidi
//    return true on error
//---------------------------------------------------------

bool MidiFile::read(QIODevice* in)
     {
      fp = in;
      _tracks.clear();
      _siglist.clear();

      char tmp[4];

      read(tmp, 4);
      int len = readLong();
      if (memcmp(tmp, "MThd", 4) || len < 6) {
            printf("bad midifile: MThd expected\n");
            return true;
            }

      _format     = readShort();
      int ntracks = readShort();
      _division   = readShort();

      if (_division < 0)
            _division = (-(_division/256)) * (_division & 0xff);
      if (len > 6)
            skip(len-6); // skip the excess

      switch (_format) {
            case 0:
                  if (readTrack())
                        return true;
                  break;
            case 1:
                  for (int i = 0; i < ntracks; i++) {
                        if (readTrack())
                              return true;
                        }
                  break;
            default:
                  printf("midi fileformat %d not implemented!\n", _format);
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   readTrack
//    return true on error
//---------------------------------------------------------

bool MidiFile::readTrack()
      {
      char tmp[4];
      read(tmp, 4);
      if (memcmp(tmp, "MTrk", 4)) {
            printf("bad midifile: MTrk expected\n");
            return true;
            }
      int len       = readLong();       // len
      int endPos    = curPos + len;
      status        = -1;
      sstatus       = -1;  // running status, will not be reset on meta or sysex
      click         =  0;
      MidiTrack* track  = new MidiTrack(this);
      _tracks.push_back(track);

      int port = 0;
      track->setOutPort(port);
      track->setOutChannel(-1);

      for (;;) {
            MidiEvent* event = readEvent();
            if (event == 0)
                  return true;

            // check for end of track:
            if ((event->type() == ME_META)) {
                  int mt = ((MidiMeta*)event)->metaType();
                  if (mt == 0x2f)         // end of track
                        break;
                  track->insert(event);
                  continue;
                  }
            if (!event->isChannelEvent()) {
                  printf("importMidi: event type 0x%02x not implemented\n", event->type());
                  delete event;
                  continue;
                  }
            track->insert(event);
            }
      if (curPos != endPos) {
            printf("bad track len: %d != %d, %d bytes too much\n",
               endPos, curPos, endPos - curPos);
            if (curPos < endPos) {
                  printf("skip %d\n", endPos - curPos);
                  skip(endPos - curPos);
                  }
            }
      return false;
      }

/*---------------------------------------------------------
 *    read
 *    return false on error
 *---------------------------------------------------------*/

bool MidiFile::read(void* p, qint64 len)
      {
      curPos += len;
      qint64 rv = fp->read((char*)p, len);
      return rv != len;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

bool MidiFile::write(const void* p, qint64 len)
      {
      qint64 rv = fp->write((char*)p, len);
      if (rv == len)
            return false;
      printf("write midifile failed: %s\n", fp->errorString().toLatin1().data());
      return true;
      }

//---------------------------------------------------------
//   readShort
//---------------------------------------------------------

int MidiFile::readShort()
      {
      short format;
      read(&format, 2);
      return BE_SHORT(format);
      }

//---------------------------------------------------------
//   writeShort
//---------------------------------------------------------

void MidiFile::writeShort(int i)
      {
      int format = BE_SHORT(i);
      write(&format, 2);
      }

//---------------------------------------------------------
//   readLong
//   writeLong
//---------------------------------------------------------

int MidiFile::readLong()
      {
      int format;
      read(&format, 4);
      return BE_LONG(format);
      }

//---------------------------------------------------------
//   writeLong
//---------------------------------------------------------

void MidiFile::writeLong(int i)
      {
      int format = BE_LONG(i);
      write(&format, 4);
      }

/*---------------------------------------------------------
 *    skip
 *    This is meant for skipping a few bytes in a
 *    file or fifo.
 *---------------------------------------------------------*/

bool MidiFile::skip(qint64 len)
      {
      if (len <= 0)
            return false;
      char tmp[len];
      return read(tmp, len);
      }

/*---------------------------------------------------------
 *    getvl
 *    Read variable-length number (7 bits per byte, MSB first)
 *---------------------------------------------------------*/

int MidiFile::getvl()
      {
      int l = 0;
      for (int i = 0;i < 16; i++) {
            uchar c;
            if (read(&c, 1))
                  return -1;
            l += (c & 0x7f);
            if (!(c & 0x80)) {
                  return l;
                  }
            l <<= 7;
            }
      return -1;
      }

/*---------------------------------------------------------
 *    putvl
 *    Write variable-length number (7 bits per byte, MSB first)
 *---------------------------------------------------------*/

void MidiFile::putvl(unsigned val)
      {
      unsigned long buf = val & 0x7f;
      while ((val >>= 7) > 0) {
            buf <<= 8;
            buf |= 0x80;
            buf += (val & 0x7f);
            }
      for (;;) {
            put(buf);
            if (buf & 0x80)
                  buf >>= 8;
            else
                  break;
            }
      }

//---------------------------------------------------------
//   MidiTrack
//---------------------------------------------------------

MidiTrack::MidiTrack(MidiFile* f)
      {
      mf = f;
      _outChannel = -1;
      _outPort    = -1;
      }

MidiTrack::~MidiTrack()
      {
      }

//---------------------------------------------------------
//   readEvent
//---------------------------------------------------------

MidiEvent* MidiFile::readEvent()
      {
      uchar me, a, b;

      int nclick = getvl();
      if (nclick == -1) {
            printf("readEvent: error 1\n");
            return 0;
            }
      click += nclick;
      for (;;) {
            if (read(&me, 1)) {
                  printf("readEvent: error 2\n");
                  return 0;
                  }
            if (me >= 0xf1 && me <= 0xfe && me != 0xf7) {
                  printf("Midi: Unknown Message 0x%02x\n", me & 0xff);
                  }
            else
                  break;
            }

      MidiEvent* event;
      int ontime = click;
      unsigned char* data;
      int dataLen;

      if (me == 0xf0 || me == 0xf7) {
            status  = -1;                  // no running status
            int len = getvl();
            if (len == -1) {
                  printf("readEvent: error 3\n");
                  return 0;
                  }
            data    = new unsigned char[len];
            dataLen = len;
            if (read(data, len)) {
                  printf("readEvent: error 4\n");
                  delete data;
                  return 0;
                  }
            if (data[len-1] != 0xf7) {
                  printf("SYSEX does not end with 0xf7!\n");
                  // more to come?
                  }
            else
                  dataLen--;      // don't count 0xf7
            return new MidiSysex(ontime, dataLen, data);
            }

      if (me == ME_META) {
            status = -1;                  // no running status
            uchar type;
            if (read(&type, 1)) {
                  printf("readEvent: error 5\n");
                  return 0;
                  }
            int len = getvl();                // read len
            if (len == -1) {
                  printf("readEvent: error 6\n");
                  return 0;
                  }
            dataLen = len;
            data    = new unsigned char[len+1];
            if (len) {
                  if (read(data, len)) {
                        printf("readEvent: error 7\n");
                        delete data;
                        return 0;
                        }
                  }
            data[len] = 0;
            return new MidiMeta(ontime, type, dataLen, data);
            }

      if (me & 0x80) {                     // status byte
            status   = me;
            sstatus  = status;
            if (read(&a, 1)) {
                  printf("readEvent: error 9\n");
                  delete event;
                  return 0;
                  }
            }
      else {
            if (status == -1) {
                  printf("readEvent: no running status, read 0x%02x\n", me);
                  printf("sstatus ist 0x%02x\n", sstatus);
                  if (sstatus == -1) {
                        delete event;
                        return 0;
                        }
                  status = sstatus;
                  }
            a = me;
            }
      int channel = status & 0x0f;
      b           = 0;
      switch (status & 0xf0) {
            case 0x80:        // note off
                  if (read(&b, 1)) {
                        printf("readEvent: error 15\n");
                        return 0;
                        }
                  event = new MidiNoteOff(ontime, channel, a & 0x7f, b & 0x7f);
                  break;
            case 0x90:        // note on
                  if (read(&b, 1)) {
                        printf("readEvent: error 15\n");
                        return 0;
                        }
                  event = new MidiNoteOn(ontime, channel, a & 0x7f, b & 0x7f);
                  break;
            case 0xa0:        // polyphone aftertouch
                  if (read(&b, 1)) {
                        printf("readEvent: error 15\n");
                        return 0;
                        }
                  // event = new MidiNoteOff(ontime, channel, a, b);
                  break;
            case 0xb0:        // controller
                  if (read(&b, 1)) {
                        printf("readEvent: error 15\n");
                        return 0;
                        }
                  event = new MidiController(ontime, channel, a & 0x7f, b & 0x7f);
                  break;
            case 0xe0:        // pitch bend
                  if (read(&b, 1)) {
                        printf("readEvent: error 15\n");
                        return 0;
                        }
                  event = new MidiController(ontime, channel, CTRL_PITCH,
                     ((((b & 0x80) ? 0 : b) << 7) + a) - 8192);
                  break;
            case 0xc0:        // Program Change
                  event = new MidiController(ontime, channel, CTRL_PROGRAM, a & 0x7f);
                  break;
            case 0xd0:        // Channel Aftertouch
                  event = new MidiController(ontime, channel, CTRL_PRESS, a & 0x7f);
                  break;
            default:          // f1 f2 f3 f4 f5 f6 f7 f8 f9
                  printf("BAD STATUS 0x%02x, me 0x%02x\n", status, me);
                  return 0;
            }

      if ((a & 0x80) || (b & 0x80)) {
            printf("8't bit in data set(%02x %02x): tick %d read 0x%02x  status:0x%02x\n",
              a & 0xff, b & 0xff, click, me, status);
            printf("readEvent: error 16\n");
            if (b & 0x80) {
                  // Try to fix: interpret as channel byte
                  status   = b;
                  sstatus  = status;
                  return event;
                  }
            delete event;
            return 0;
            }
      return event;
      }

//---------------------------------------------------------
//   mergeNoteOnOff
//    find matching note on / note off events and merge
//    into a note event with tick duration
//---------------------------------------------------------

void MidiTrack::mergeNoteOnOff()
      {
      EventList el;

      for (iEvent i = _events.begin(); i != _events.end();) {
            MidiEvent* ev = i.value();
            if (ev->type() != ME_NOTEON && ev->type() != ME_NOTEOFF) {
                  el.insert(ev->ontime(), ev);
                  i = _events.erase(i);
                  continue;
                  }
            int tick = ev->ontime();
            if (ev->type() == ME_NOTEOFF || ((MidiNoteOn*)ev)->velo() == 0) {
                  printf("-extra note off at %d\n", tick);
                  i = _events.erase(i);
                  delete ev;
                  continue;
                  }
            MidiNoteOn* no = (MidiNoteOn*)ev;
            MidiNote* note = new MidiNote;
            note->setOntime(tick);
            note->setPitch(no->pitch());
            note->setVelo(no->velo());

            iEvent k;
            for (k = _events.lowerBound(tick); k != _events.end(); ++k) {
                  MidiEvent* e = k.value();
                  if (e->type() != ME_NOTEON && e->type() != ME_NOTEOFF)
                        continue;
                  MidiNoteOnOff* oo = (MidiNoteOnOff*) e;
                  if ((oo->type() == ME_NOTEOFF || (oo->type() == ME_NOTEON && oo->velo() == 0))
                     && (oo->pitch() == no->pitch())) {
                        int t = k.key() - tick;
                        if (t <= 0)
                              t = 1;
                        note->setDuration(t);
                        note->setVelo(oo->velo());
                        _events.erase(k);
                        delete e;
                        break;
                        }
                  }
            if (k == _events.end()) {
                  printf("-no note-off for note at %d\n", tick);
                  //
                  // note off at end of bar
                  //
                  int endTick = note->ontime() + 1; // song->roundUpBar(ev->ontime + 1);
                  note->setDuration(endTick - note->ontime());
                  }
            el.insert(note->ontime(), note);
            delete ev;
            i = _events.erase(i);
            }
      _events = el;
      }

//---------------------------------------------------------
//   empty
//    check for empty track
//---------------------------------------------------------

bool MidiTrack::empty() const
      {
      return _events.empty();
      }

//---------------------------------------------------------
//   extractTimeSig
//---------------------------------------------------------

void MidiTrack::extractTimeSig(SigList* sigmap)
      {
      EventList el;

      foreach (MidiEvent* e, _events) {
            if (e->type() == ME_META && ((MidiMeta*)e)->metaType() == META_TIME_SIGNATURE) {
                  const unsigned char* data = ((MidiMeta*)e)->data();
                  int z  = data[0];
                  int nn = data[1];
                  int n  = 1;
                  for (int i = 0; i < nn; ++i)
                        n *= 2;
                  sigmap->add(e->ontime(), z, n);
                  }
            else
                  el.insert(e->ontime(), e);
            }
      _events = el;
      }

//---------------------------------------------------------
//   process1
//    merge note on/off events
//    extract time signatures and build siglist
//---------------------------------------------------------

void MidiFile::process1()
      {
      foreach (MidiTrack* t, _tracks) {
            t->mergeNoteOnOff();
            t->extractTimeSig(&_siglist);
            }
      }

//---------------------------------------------------------
//   quantizeLen
//---------------------------------------------------------

static int quantizeLen(int, int len, int raster)
      {
      int rl = (len / raster) * raster;
      rl /= 2;
      if (rl == 0)
            rl = 1;
      rl = ((len + rl - 1) / rl) * rl;
      return rl;
      }

//---------------------------------------------------------
//   quantize
//    process one segment (measure)
//---------------------------------------------------------

void MidiTrack::quantize(int startTick, int endTick, EventList* dst)
      {
      int division = mf->division();

      int mintick = division * 64;
      for (iEvent i = _events.lowerBound(startTick); i != _events.lowerBound(endTick); ++i) {
            MidiEvent* e = *i;
            if (e->type() == ME_NOTE && (((MidiNote*)e)->duration() < mintick))
                  mintick = ((MidiNote*)e)->duration();
            }
      if (mintick <= division / 16)        // minimum duration is 1/64
            mintick = division / 16;
      else if (mintick <= division / 8)
            mintick = division / 8;
      else if (mintick <= division / 4)
            mintick = division / 4;
      else if (mintick <= division / 2)
            mintick = division / 2;
      else if (mintick <= division)
            mintick = division;
      else if (mintick <= division * 2)
            mintick = division * 2;
      else if (mintick <= division * 4)
            mintick = division * 4;
      else if (mintick <= division * 8)
            mintick = division * 8;
      int raster;
      if (mintick > division)
            raster = division;
      else
            raster = mintick;

      for (iEvent i = _events.lowerBound(startTick); i != _events.lowerBound(endTick); ++i) {
            MidiEvent* e = *i;
            if (e->type() == ME_NOTE) {
                  MidiNote* note = (MidiNote*)e;
	            int len  = quantizeLen(division, note->duration(), raster);
      	      int tick = (note->ontime() / raster) * raster;
	            note->setOntime(tick);
      	      note->setDuration(len);
                  }
            dst->insert(e->ontime(), e);
            }
      }

//---------------------------------------------------------
//   cleanup
//    - quantize
//    - remove overlaps
//---------------------------------------------------------

void MidiTrack::cleanup()
	{
      EventList dl;

      //
      //	quantize
      //
      int lastTick = 0;
      foreach (MidiEvent* e, _events) {
            if (!(e->type() == ME_NOTE))
                  continue;
            int offtime  = e->ontime() + ((MidiNote*)e)->duration();
            if (offtime > lastTick)
                  lastTick = offtime;
            }
      int startTick = 0;
      for (int i = 1;; ++i) {
            int endTick = mf->siglist().bar2tick(i, 0, 0);
            if (endTick > lastTick)
                  break;
            quantize(startTick, endTick, &dl);
            startTick = endTick;
            }

      //
      //
      //
      _events.clear();

      for(iEvent i = dl.begin(); i != dl.end(); ++i) {
            MidiEvent* e = i.value();
            if (e->type() == ME_NOTE) {
                  iEvent ii = i;
                  ++ii;
                  for (; ii != dl.end(); ++ii) {
                        MidiEvent* ee = ii.value();
                        if (ee->type() != ME_NOTE || ((MidiNote*)ee)->pitch() != ((MidiNote*)e)->pitch())
                              continue;
                        if (ee->ontime() >= e->ontime() + ((MidiNote*)e)->duration())
                              break;
                        ((MidiNote*)e)->setDuration(ee->ontime() - e->ontime());
                        break;
                        }
                  if (((MidiNote*)e)->duration() <= 0)
                        continue;
                  }
		_events.insert(e->ontime(), e);
            }
      }

//---------------------------------------------------------
//   changeDivision
//---------------------------------------------------------

void MidiTrack::changeDivision(int newDivision)
      {
      EventList dl;

      int division = mf->division();
      foreach(MidiEvent* e, _events) {
            int tick = (e->ontime() * newDivision + division/2) / division;
            e->setOntime(tick);
            if (e->type() == ME_NOTE) {
                  MidiNote* n = (MidiNote*)e;
                  n->setDuration((n->duration() * newDivision + division/2) / division);
                  }
		dl.insert(tick, e);
            }
      _events = dl;
      }

//---------------------------------------------------------
//   changeDivision
//---------------------------------------------------------

void MidiFile::changeDivision(int newDivision)
      {
      if (_division == newDivision)
            return;
      foreach (MidiTrack* t, _tracks)
            t->changeDivision(newDivision);
      _division = newDivision;
      }

//---------------------------------------------------------
//   sortTracks
//    sort tracks in instrument order
//---------------------------------------------------------

bool instrumentLessThan(const MidiTrack* t1, const MidiTrack* t2)
      {
      return t1->program < t2->program;
      }

void MidiFile::sortTracks()
      {
      qSort(_tracks.begin(), _tracks.end(), instrumentLessThan);
      }

//---------------------------------------------------------
//   separateChannel
//    if a track contains events for different midi channels,
//    then split events into separate tracks
//---------------------------------------------------------

void MidiFile::separateChannel()
      {
      int n = _tracks.size();
      for (int i = 0; i < n; ++i) {
            QList<int> channel;
            MidiTrack* mt = _tracks.at(i);
            foreach(MidiEvent* e, mt->events()) {
                  if (e->isChannelEvent() && !channel.contains(((MidiChannelEvent*)e)->channel()))
                        channel.append(((MidiChannelEvent*)e)->channel());
                  }
            mt->setOutChannel(channel.empty() ? 0 : channel[0]);
            int nn = channel.size();
            if (nn <= 1)
                  continue;
            qSort(channel);

            // split
            for (int ii = 1; ii < nn; ++ii) {
                  MidiTrack* t = new MidiTrack(this);
                  _tracks.insert(i + 1, t);
                  t->setOutChannel(channel[ii]);
                  }
            EventList& el = mt->events();
            for (iEvent ie = el.begin(); ie != el.end();) {
                  MidiEvent* e = *ie;
                  if (e->isChannelEvent()) {
                        int ch  = ((MidiChannelEvent*)e)->channel();
                        int idx = channel.indexOf(ch);
                        MidiTrack* t = _tracks.at(i * idx);
                        if (t != mt) {
                              t->insert(e);
                              ie = el.erase(ie);
                              continue;
                              }
                        }
                  ++ie;
                  }
            }
      }

//---------------------------------------------------------
//   move
//    Move all events in midifile.
//---------------------------------------------------------

void MidiFile::move(int ticks)
      {
      foreach(MidiTrack* track, _tracks)
            track->move(ticks);
      }

//---------------------------------------------------------
//   move
//---------------------------------------------------------

void MidiTrack::move(int ticks)
      {
      EventList dl;

      foreach(MidiEvent* e, _events) {
            int tick = e->ontime() + ticks;
            if (tick < 0)
                  tick = 0;
            e->setOntime(tick);
		dl.insert(tick, e);
            }
      _events = dl;
      }

//---------------------------------------------------------
//   isDrumTrack
//---------------------------------------------------------

bool MidiTrack::isDrumTrack() const
      {
      return outChannel() == 9;
      }

