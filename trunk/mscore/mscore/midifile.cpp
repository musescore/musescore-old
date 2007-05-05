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

//---------------------------------------------------------
//   quantizeLen
//---------------------------------------------------------

static int quantizeLen(int division, int len)
      {
      if (len < division/12)
            len = division/8;
      else if (len < division/6)
            len = division/4;
      else if (len < division/3)
            len = division/2;
      else if (len < division+division/2)
            len = division;
      else if (len < division * 3)
            len = division*2;
      else if (len < division * 6)
            len = division * 4;
      else if (len < division * 12)
            len = division * 8;
      else
            len = division * 16;
      return len;
      }

//---------------------------------------------------------
//   MidiEvent
//---------------------------------------------------------

MidiEvent::MidiEvent()
      {
      data    = 0;
      dataLen = 0;
      len     = 0;
      tick    = 0;
      channel = -1;
      port    = -1;
      }

MidiEvent::~MidiEvent()
      {
      if (data)
            delete data;
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
            int ntick = i.key();
            putvl(ntick - tick);    // write tick delta
            writeEvent(t->outChannel(), i.value());
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
//   writeEvent
//---------------------------------------------------------

void MidiFile::writeEvent(int c, const MidiEvent* event)
      {
      int nstat = event->type;

      // we dont save meta data into smf type 0 files:

      nstat |= c;
      //
      //  running status; except for Sysex- and Meta Events
      //
      if (((nstat & 0xf0) != 0xf0) && (nstat != status)) {
            status = nstat;
            put(nstat);
            }
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
                  putvl(event->len + 1);  // including 0xf7
                  write(event->data, event->len);
                  put(0xf7);
                  status = -1;      // invalidate running status
                  break;
            case ME_META:
                  put(0xff);
                  put(event->dataA);
                  putvl(event->len);
                  write(event->data, event->len);
                  status = -1;
                  break;
            }
      }

//---------------------------------------------------------
//   readMidi
//    return true on error
//---------------------------------------------------------

bool MidiFile::read(QIODevice* in)
     {
      fp = in;
      _tracks.clear();

      char tmp[4];

      read(tmp, 4);
      int len = readLong();
      if (memcmp(tmp, "MThd", 4) || len < 6) {
            printf("bad midifile: MThd expected\n");
            return true;
            }

      _format  = readShort();
      int ntracks = readShort();
      _division = readShort();

      if (_division < 0)
            _division = (-(_division/256)) * (_division & 0xff);
      if (len > 6)
            skip(len-6); // skip the excess

      switch (_format) {
            case 0:
                  if (readTrack(true))
                        return true;
                  break;
            case 1:
                  for (int i = 0; i < ntracks; i++) {
                        if (readTrack(false))
                              return true;
                        }
                  break;
            default:
            case 2:
                  printf("midi fileformat %d not implemented!\n", _format);
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   readTrack
//    return true on error
//---------------------------------------------------------

bool MidiFile::readTrack(bool)
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
            if ((event->type == ME_META)) {
                  int mt = event->dataA;
                  if (mt == 0x2f)         // end of track
                        break;
                  track->insert(event);
                  continue;
                  }
            if (event->channel == -1) {
                  printf("importMidi: event type 0x%02x not implemented\n", event->type);
                  // track->insert(event);
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

      MidiEvent* event = new MidiEvent;
      event->tick = click;    // (click * division + fileDivision/2) / fileDivision;

      if (me == 0xf0 || me == 0xf7) {
            status  = -1;                  // no running status
            int len = getvl();
            if (len == -1) {
                  printf("readEvent: error 3\n");
                  delete event;
                  return 0;
                  }
            event->data    = new unsigned char[len];
            event->dataLen = len;
            event->type    = ME_SYSEX;
            if (read(event->data, len)) {
                  printf("readEvent: error 4\n");
                  delete event;
                  return 0;
                  }
            if (event->data[len-1] != 0xf7) {
                  printf("SYSEX does not end with 0xf7!\n");
                  // more to come?
                  }
            else
                  event->dataLen--;      // don't count 0xf7
            }
      else if (me == ME_META) {
            status = -1;                  // no running status
            uchar type;
            if (read(&type, 1)) {
                  printf("readEvent: error 5\n");
                  delete event;
                  return 0;
                  }
            int len = getvl();                // read len
            if (len == -1) {
                  printf("readEvent: error 6\n");
                  delete event;
                  return 0;
                  }
            event->type    = ME_META;
            event->dataLen = len;
            event->data    = new unsigned char[len+1];
            event->dataA   = type;

            if (len) {
                  if (read(event->data, len)) {
                        printf("readEvent: error 7\n");
                        delete event;
                        return 0;
                        }
                  }
            event->data[len] = 0;
            }
      else {
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
            event->channel = status & 0x0f;
            event->type    = status & 0xf0;
            event->dataA   = a & 0x7f;
            b = 0;
            switch (status & 0xf0) {
                  case 0x80:        // note off
                  case 0x90:        // note on
                  case 0xa0:        // polyphone aftertouch
                  case 0xb0:        // controller
                  case 0xe0:        // pitch bend
                        if (read(&b, 1)) {
                              printf("readEvent: error 15\n");
                              delete event;
                              return 0;
                              }
                        if ((status & 0xf0) == 0xe0)
                              event->dataA = (((((b & 0x80) ? 0 : b) << 7) + a) - 8192);
                        else
                              event->dataB = (b & 0x80 ? 0 : b);
                        break;
                  case 0xc0:        // Program Change
                  case 0xd0:        // Channel Aftertouch
                        break;
                  default:          // f1 f2 f3 f4 f5 f6 f7 f8 f9
                        printf("BAD STATUS 0x%02x, me 0x%02x\n", status, me);
                        delete event;
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
            }
      return event;
      }

//---------------------------------------------------------
//   mergeNoteOnOff
//    find matching note on / note off events and merge
//    into a note event with tick len
//---------------------------------------------------------

void MidiTrack::mergeNoteOnOff()
      {
      foreach (MidiEvent* event, _events)      // invalidate all note len
            if (event->isNote())
                  event->len = -1;

      foreach (MidiEvent* ev, _events) {
            if (!ev->isNote())
                  continue;
            int tick = ev->tick;
            if (ev->isNoteOff()) {
                  // printf("NOTE OFF without Note ON at tick %d\n", tick);
                  continue;
                  }

            iEvent k;
            for (k = _events.lowerBound(tick); k != _events.end(); ++k) {
                  MidiEvent* event = k.value();
                  if (ev->isNoteOff(event)) {
                        int t = k.key() - tick;
                        if (t <= 0)
                              t = 1;
                        ev->len = t;
                        ev->dataC = event->dataC;
                        break;
                        }
                  }
            if (k == _events.end()) {
                  printf("-no note-off! %d pitch %d velo %d\n",
                     tick, ev->dataA, ev->dataB);
                  //
                  // note off at end of bar
                  //
                  int endTick = ev->tick + 1; // song->roundUpBar(ev->tick + 1);
                  ev->len = endTick - ev->tick;
                  }
            else {
                  _events.erase(k);      // memory leak
                  }
            }

      // DEBUG: any note off left?

      foreach (const MidiEvent* ev, _events) {
            if (ev->isNoteOff()) {
                  printf("+extra note-off! %d pitch %d velo %d\n",
                           ev->tick, ev->dataA, ev->dataB);
                  }
            }
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
//   process1
//---------------------------------------------------------

void MidiFile::process1()
      {
      foreach (MidiTrack* t, _tracks)
            t->mergeNoteOnOff();
      }

//---------------------------------------------------------
//   cleanup
//    - quantize
//    - remove overlaps
//---------------------------------------------------------

void MidiTrack::cleanup()
	{
//      const int tickRaster = mf->division()/2; 	// 1/8 quantize
      const int tickRaster = mf->division()/4; 	// 1/16 quantize
      EventList dl;

      //
      //	quantize
      //
      foreach(MidiEvent* e, _events) {
            if (e->isNote()) {
	            int len  = quantizeLen(mf->division(), e->len);
      	      int tick = (e->tick / tickRaster) * tickRaster;

	            e->tick = tick;
      	      e->len  = len;
                  }
		dl.insert(e->tick, e);
            }
      //
      //
      //
      _events.clear();

      for(iEvent i = dl.begin(); i != dl.end(); ++i) {
            MidiEvent* e = i.value();
            if (e->isNote()) {
                  iEvent ii = i;
                  ++ii;
                  for (; ii != dl.end(); ++ii) {
                        MidiEvent* ee = ii.value();
                        if (!ee->isNote() || ee->pitch() != e->pitch())
                              continue;
                        if (ee->tick >= e->tick + e->len)
                              break;
                        e->len = ee->tick - e->tick;
                        break;
                        }
                  if (e->len <= 0)
                        continue;
                  }
		_events.insert(e->tick, e);
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
            int tick = (e->tick * newDivision + division/2) / division;
            e->tick = tick;
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
                  if (e->isChannelEvent() && !channel.contains(e->channel))
                        channel.append(e->channel);
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
                        int ch  = e->channel;
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
            int tick = e->tick + ticks;
            if (tick < 0)
                  tick = 0;
            e->tick = tick;
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

