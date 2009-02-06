//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2007 Werner Schweer and others
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
#include "part.h"
#include "note.h"
#include "drumset.h"
#include "utils.h"

extern QTextStream cout;
extern QTextStream eout;

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

static unsigned const char gmOnMsg[] = {
      0x7e,       // Non-Real Time header
      0x7f,       // ID of target device (7f = all devices)
      0x09,
      0x01
      };
static unsigned const char gsOnMsg[] = {
      0x41,       // roland id
      0x10,       // Id of target device (default = 10h for roland)
      0x42,       // model id (42h = gs devices)
      0x12,       // command id (12h = data set)
      0x40,       // address & value
      0x00,
      0x7f,
      0x00,
      0x41        // checksum?
      };
static unsigned const char xgOnMsg[] = {
      0x43,       // yamaha id
      0x10,       // device number (0)
      0x4c,       // model id
      0x00,       // address (high, mid, low)
      0x00,
      0x7e,
      0x00        // data
      };
const int  gmOnMsgLen = sizeof(gmOnMsg);
const int  gsOnMsgLen = sizeof(gsOnMsg);
const int  xgOnMsgLen = sizeof(xgOnMsg);

//---------------------------------------------------------
//    midi_meta_name
//---------------------------------------------------------

QString midiMetaName(int meta)
      {
      const char* s = "";
      switch (meta) {
            case 0:     s = "Sequence Number"; break;
            case 1:     s = "Text Event"; break;
            case 2:     s = "Copyright"; break;
            case 3:     s = "Sequence/Track Name"; break;
            case 4:     s = "Instrument Name"; break;
            case 5:     s = "Lyric"; break;
            case 6:     s = "Marker"; break;
            case 7:     s = "Cue Point"; break;
            case 8:
            case 9:
            case 0x0a:
            case 0x0b:
            case 0x0c:
            case 0x0d:
            case 0x0e:
            case 0x0f:  s = "Text"; break;
            case 0x20:  s = "Channel Prefix"; break;
            case 0x21:  s = "Port Change"; break;
            case 0x2f:  s = "End of Track"; break;
            case 0x51:  s = "Tempo"; break;
            case 0x54:  s = "SMPTE Offset"; break;
            case 0x58:  s = "Time Signature"; break;
            case 0x59:  s = "Key Signature"; break;
            case 0x74:  s = "Sequencer-Specific1"; break;
            case 0x7f:  s = "Sequencer-Specific2"; break;
            default:
                  break;
            }
      return QString(s);
      }

//---------------------------------------------------------
//   MidiFile
//---------------------------------------------------------

MidiFile::MidiFile()
      {
      fp               = 0;
      _format          = 1;
      _midiType        = MT_UNKNOWN;
      _noRunningStatus = false;
      _shortestNote    = ::division /32;       // 1/128
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
      writeShort(_format);          // format
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
      foreach(const Event* ev, t->events()) {
            int ntick = ev->ontime();
            putvl(ntick - tick);    // write tick delta
            //
            // if track channel != -1, then use this
            //    channel for all events in this track
            //
            if (ev->isChannelEvent() && (t->outChannel() != -1))
                  ((ChannelEvent*)ev)->setChannel(t->outChannel());
            ev->write(this);
            tick = ntick;
            }

      //---------------------------------------------------
      //    write "End Of Track" Meta
      //    write Track Len
      //

      putvl(1);
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
//   writeStatus
//---------------------------------------------------------

void MidiFile::writeStatus(int nstat, int c)
      {
      nstat |= (c & 0xf);
      //
      //  running status; except for Sysex- and Meta Events
      //
      if (_noRunningStatus || (((nstat & 0xf0) != 0xf0) && (nstat != status))) {
            status = nstat;
            put(nstat);
            }
      }

//---------------------------------------------------------
//   readMidi
//    return false on error
//---------------------------------------------------------

bool MidiFile::read(QIODevice* in)
     {
      fp = in;
      _tracks.clear();
      _siglist.clear();
      _siglist.add(0, 4, 4);   // default time signature

      curPos    = 0;

      char tmp[4];

      read(tmp, 4);
      int len = readLong();
      if (memcmp(tmp, "MThd", 4) || len < 6)
            throw(QString("bad midifile: MThd expected"));

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
                        return false;
                  break;
            case 1:
                  for (int i = 0; i < ntracks; i++) {
                        if (readTrack())
                              return false;
                        }
                  break;
            default:
                  throw(QString("midi file format %1 not implemented").arg(_format));
                  return false;
            }
      return true;
      }

//---------------------------------------------------------
//   readTrack
//    return true on error
//---------------------------------------------------------

bool MidiFile::readTrack()
      {
      char tmp[4];
      read(tmp, 4);
      if (memcmp(tmp, "MTrk", 4))
            throw(QString("bad midifile: MTrk expected"));
      int len       = readLong();       // len
      qint64 endPos = curPos + len;
      status        = -1;
      sstatus       = -1;  // running status, will not be reset on meta or sysex
      click         =  0;
      MidiTrack* track  = new MidiTrack(this);
      _tracks.push_back(track);

      int port = 0;
      track->setOutPort(port);
      track->setOutChannel(-1);

      for (;;) {
            Event* event = readEvent();
            if (event == 0)
                  return true;

            // check for end of track:
            if ((event->type() == ME_META)) {
                  int mt = ((MetaEvent*)event)->metaType();
                  if (mt == 0x2f)         // end of track
                        break;
                  track->append(event);
                  continue;
                  }
            track->append(event);
            }
      if (curPos != endPos) {
            eout << "bad track len: " << endPos << " != " << curPos
               << ", " << (endPos - curPos) << " bytes too much\n";
            if (curPos < endPos) {
                  eout << "  skip " << (endPos-curPos) << "\n";
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
      mf          = f;
      _outChannel = -1;
      _outPort    = -1;
      _drumTrack  = false;
      _hasKey     = false;
      _staffIdx   = -1;
      }

MidiTrack::~MidiTrack()
      {
      }

//---------------------------------------------------------
//   readEvent
//---------------------------------------------------------

Event* MidiFile::readEvent()
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
            return new SysexEvent(ontime, dataLen, data);
            }

      if (me == ME_META) {
            status = -1;                  // no running status
            uchar type;
            if (read(&type, 1)) {
                  printf("readEvent: error 5\n");
                  return 0;
                  }
            dataLen = getvl();                // read len
            if (dataLen == -1) {
                  printf("readEvent: error 6\n");
                  return 0;
                  }
            data = new unsigned char[dataLen + 1];
            if (dataLen) {
                  if (read(data, dataLen)) {
                        printf("readEvent: error 7\n");
                        delete data;
                        return 0;
                        }
                  }
            data[dataLen] = 0;
            return new MetaEvent(ontime, type, dataLen, data);
            }

      if (me & 0x80) {                     // status byte
            status   = me;
            sstatus  = status;
            if (read(&a, 1)) {
                  printf("readEvent: error 9\n");
                  return 0;
                  }
            }
      else {
            if (status == -1) {
                  printf("readEvent: no running status, read 0x%02x\n", me);
                  printf("sstatus ist 0x%02x\n", sstatus);
                  if (sstatus == -1) {
                        return 0;
                        }
                  status = sstatus;
                  }
            a = me;
            }
      int channel = status & 0x0f;
      b           = 0;
      Event* event = 0;
      switch (status & 0xf0) {
            case ME_NOTEOFF:
            case ME_NOTEON:
            case ME_POLYAFTER:
            case ME_CONTROLLER:        // controller
            case ME_PITCHBEND:        // pitch bend
                  if (read(&b, 1)) {
                        printf("readEvent: error 15\n");
                        return 0;
                        }
                  break;
            }
      switch (status & 0xf0) {
            case ME_NOTEOFF:
                  event = new NoteOff(ontime, channel, a & 0x7f, b & 0x7f);
                  break;
            case ME_NOTEON:
                  event = new NoteOn(ontime, channel, a & 0x7f, b & 0x7f);
                  break;
            case ME_POLYAFTER:
                  event = new ControllerEvent(ontime, channel, CTRL_POLYAFTER,
                     ((a & 0x7f) << 8) + (b & 0x7f));
                  break;
            case ME_CONTROLLER:        // controller
                  event = new ControllerEvent(ontime, channel, a & 0x7f, b & 0x7f);
                  break;
            case ME_PITCHBEND:        // pitch bend
                  event = new ControllerEvent(ontime, channel, CTRL_PITCH,
                     ((((b & 0x80) ? 0 : b) << 7) + a) - 8192);
                  break;
            case ME_PROGRAM:
                  event = new ControllerEvent(ontime, channel, CTRL_PROGRAM, a & 0x7f);
                  break;
            case ME_AFTERTOUCH:
                  event = new ControllerEvent(ontime, channel, CTRL_PRESS, a & 0x7f);
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

      int hbank = 0xff;
      int lbank = 0xff;
      int rpnh  = -1;
      int rpnl  = -1;
      int datah = 0;
      int datal = 0;
      int dataType = 0; // 0 : disabled, 0x20000 : rpn, 0x30000 : nrpn;

      int n = _events.size();
      for (int i = 0; i < n; ++i) {
            Event* ev = _events[i];
            if (ev == 0)
                  continue;
            if ((ev->type() != ME_NOTEON) && (ev->type() != ME_NOTEOFF)) {
                  if (ev->type() == ME_CONTROLLER) {
                        ControllerEvent* ctrl = (ControllerEvent*)ev;
                        int val  = ctrl->value();
                        int cn   = ctrl->controller();
                        if (cn == CTRL_HBANK) {
                              hbank = val;
                              delete ev;
                              _events[i] = 0;
                              continue;
                              }
                        else if (cn == CTRL_LBANK) {
                              lbank = val;
                              delete ev;
                              _events[i] = 0;
                              continue;
                              }
                        else if (cn == CTRL_HDATA) {
                              datah = val;

                              // check if a CTRL_LDATA follows
                              // e.g. wie have a 14 bit controller:
                              int ii = i;
                              ++ii;
                              bool found = false;
                              for (; ii < n; ++ii) {
                                    Event* ev = _events[ii];
                                    if (ev && (ev->type() == ME_CONTROLLER)) {
                                          if (((ControllerEvent*)ev)->controller() == CTRL_LDATA) {
                                                // handle later
                                                found = true;
                                                }
                                          break;
                                          }
                                    }
                              if (!found) {
                                    if (rpnh == -1 || rpnl == -1) {
                                          printf("parameter number not defined, data 0x%x\n", datah);
                                          delete ev;
                                          _events[i] = 0;
                                          continue;
                                          }
                                    else {
                                          ctrl->setController(dataType | (rpnh << 8) | rpnl);
                                          ctrl->setValue(datah);
                                          }
                                    }
                              else {
                                    delete ev;
                                    _events[i] = 0;
                                    continue;
                                    }
                              }
                        else if (cn == CTRL_LDATA) {
                              datal = val;

                              if (rpnh == -1 || rpnl == -1) {
                                    printf("parameter number not defined, data 0x%x 0x%x, tick %d, channel %d\n",
                                       datah, datal, ctrl->ontime(), ctrl->channel());
                                    break;
                                    }
                              // assume that the sequence is always
                              //    CTRL_HDATA - CTRL_LDATA
                              // eg. that LDATA is always send last

                              // 14 Bit RPN/NRPN
                              ctrl->setController((dataType+0x30000) | (rpnh << 8) | rpnl);
                              ctrl->setValue((datah << 7) | datal);
                              }
                        else if (cn == CTRL_HRPN) {
                              rpnh = val;
                              dataType = 0x20000;
                              delete ev;
                              _events[i] = 0;
                              continue;
                              }
                        else if (cn == CTRL_LRPN) {
                              rpnl = val;
                              dataType = 0x20000;
                              delete ev;
                              _events[i] = 0;
                              continue;
                              }
                        else if (cn == CTRL_HNRPN) {
                              rpnh = val;
                              dataType = 0x30000;
                              delete ev;
                              _events[i] = 0;
                              continue;
                              }
                        else if (cn == CTRL_LNRPN) {
                              rpnl = val;
                              dataType = 0x30000;
                              delete ev;
                              _events[i] = 0;
                              continue;
                              }
                        else if (cn == CTRL_PROGRAM) {
                              ctrl->setValue((hbank << 16) | (lbank << 8) | ctrl->value());
                              el.insert(ctrl);
                              _events[i] = 0;
                              continue;
                              }
                        }
                  else if (ev->type() == ME_SYSEX) {
                        int len = ((SysexEvent*)ev)->len();
                        unsigned char* buffer = ((SysexEvent*)ev)->data();
                        if ((len == gmOnMsgLen) && memcmp(buffer, gmOnMsg, gmOnMsgLen) == 0) {
                              mf->_midiType = MT_GM;
                              delete ev;
                              _events[i] = 0;
                              continue;
                              }
                        if ((len == gsOnMsgLen) && memcmp(buffer, gsOnMsg, gsOnMsgLen) == 0) {
                              mf->_midiType = MT_GS;
                              delete ev;
                              _events[i] = 0;
                              continue;
                              }
                        if ((len == xgOnMsgLen) && memcmp(buffer, xgOnMsg, xgOnMsgLen) == 0) {
                              mf->_midiType = MT_XG;
                              delete ev;
                              _events[i] = 0;
                              continue;
                              }
                        if (buffer[0] == 0x43) {    // Yamaha
                              mf->_midiType = MT_XG;
                              int type   = buffer[1] & 0xf0;
                              if (type == 0x10) {
                                    if (buffer[1] != 0x10) {
                                          buffer[1] = 0x10;    // fix to Device 1
                                          }
                                    if ((len == xgOnMsgLen) && memcmp(buffer, xgOnMsg, xgOnMsgLen) == 0) {
                                          mf->_midiType = MT_XG;
                                          delete ev;
                                          _events[i] = 0;
                                          continue;
                                          }
                                    if (len == 7 && buffer[2] == 0x4c && buffer[3] == 0x08 && buffer[5] == 7) {
                                          // part mode
                                          // 0 - normal
                                          // 1 - DRUM
                                          // 2 - DRUM 1
                                          // 3 - DRUM 2
                                          // 4 - DRUM 3
                                          // 5 - DRUM 4
                                          if (buffer[6] != 0) {
                                                _drumTrack = true;
                                                }
                                          delete ev;
                                          _events[i] = 0;
                                          continue;
                                          }
                                    }
                              }
                        }
                  el.insert(ev);
                  _events[i] = 0;
                  continue;
                  }
            int tick = ev->ontime();
            if (ev->type() == ME_NOTEOFF || ((NoteOn*)ev)->velo() == 0) {
                  printf("-extra note off at %d\n", tick);
                  delete ev;
                  _events[i] = 0;
                  continue;
                  }
            NoteOn* no = (NoteOn*)ev;
            NoteEvent* note = new NoteEvent;
            note->setOntime(tick);
            note->setPitch(no->pitch());
            note->setVelo(no->velo());

            int k = i + 1;
            for (; k < n; ++k) {
                  Event* e = _events[k];
                  if (e == 0 || (e->type() != ME_NOTEON && e->type() != ME_NOTEOFF))
                        continue;
                  NoteOnOff* oo = (NoteOnOff*) e;
                  if ((oo->type() == ME_NOTEOFF || (oo->type() == ME_NOTEON && oo->velo() == 0))
                     && (oo->pitch() == no->pitch())) {
                        int t = e->ontime() - tick;
                        if (t <= 0)
                              t = 1;
                        note->setDuration(t);
                        note->setVelo(oo->velo());
                        delete e;
                        _events[k] = 0;
                        break;
                        }
                  }
            if (k == n) {
                  printf("-no note-off for note at %d\n", tick);
                  //
                  // note off at end of bar
                  //
                  int endTick = note->ontime() + 1; // song->roundUpBar(ev->ontime + 1);
                  note->setDuration(endTick - note->ontime());
                  }
            el.insert(note);
            delete ev;
            _events[i] = 0;
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
//   setOutChannel
//---------------------------------------------------------

void MidiTrack::setOutChannel(int n)
      {
      _outChannel = n;
      if (_outChannel == 9)
            _drumTrack = true;
      }

//---------------------------------------------------------
//   extractTimeSig
//---------------------------------------------------------

void MidiTrack::extractTimeSig(SigList* sigmap)
      {
      EventList el;

      foreach (Event* e, _events) {
            if (e->type() == ME_META && ((MetaEvent*)e)->metaType() == META_TIME_SIGNATURE) {
                  const unsigned char* data = ((MetaEvent*)e)->data();
                  int z  = data[0];
                  int nn = data[1];
                  int n  = 1;
                  for (int i = 0; i < nn; ++i)
                        n *= 2;
                  sigmap->add(e->ontime(), z, n);
                  }
            else
                  el.insert(e);
            }
      _events = el;
      if (sigmap->empty())                // set default
            sigmap->add(0, 4, 4);
      }

//---------------------------------------------------------
//   process1
//    - merge note on/off events into NoteEvent events
//    - extract time signatures and build siglist
//    - extract initialisation sysex and set MidiType
//    - find out if track is a drum track
//---------------------------------------------------------

void MidiFile::process1()
      {
      foreach (MidiTrack* t, _tracks) {
            t->mergeNoteOnOff();
            t->extractTimeSig(&_siglist);
            }
      }

//---------------------------------------------------------
//   quantize
//    process one segment (measure)
//---------------------------------------------------------

void MidiTrack::quantize(int startTick, int endTick, EventList* dst)
      {
      int division = mf->division();

      int mintick = division * 64;
      iEvent i = _events.begin();
      for (; i != _events.end(); ++i) {
            if ((*i)->ontime() >= startTick)
                  break;
            }
      iEvent si = i;
      for (; i != _events.end(); ++i) {
            Event* e = *i;
            if (e->ontime() >= endTick)
                  break;
            if (e->type() == ME_NOTE && (((NoteEvent*)e)->duration() < mintick))
                  mintick = ((NoteEvent*)e)->duration();
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

      if (mintick < mf->shortestNote())
            mintick = mf->shortestNote();

      int raster;
      if (mintick > division)
            raster = division;
      else
            raster = mintick;

      for (iEvent i = si; i != _events.end(); ++i) {
            Event* e = *i;
            if (e->ontime() >= endTick)
                  break;
            if (e->type() == ME_NOTE) {
                  NoteEvent* note = (NoteEvent*)e;
	            int len  = quantizeLen(division, note->duration(), raster);
      	      int tick = (note->ontime() / raster) * raster;
	            note->setOntime(tick);
      	      note->setDuration(len);
                  }
            dst->insert(e);
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
      foreach (Event* e, _events) {
            if (e->type() != ME_NOTE)
                  continue;
            int offtime  = ((NoteEvent*)e)->offtime();
            if (offtime > lastTick)
                  lastTick = offtime;
            }
      int startTick = 0;
      for (int i = 1;; ++i) {
            int endTick = mf->siglist().bar2tick(i, 0, 0);
            quantize(startTick, endTick, &dl);
            if (endTick > lastTick)
                  break;
            startTick = endTick;
            }

      //
      //
      //
      _events.clear();

      for(iEvent i = dl.begin(); i != dl.end(); ++i) {
            Event* e = *i;
            if (e->type() == ME_NOTE) {
                  iEvent ii = i;
                  ++ii;
                  for (; ii != dl.end(); ++ii) {
                        Event* ee = *ii;
                        if (ee->type() != ME_NOTE || ((NoteEvent*)ee)->pitch() != ((NoteEvent*)e)->pitch())
                              continue;
                        if (ee->ontime() >= e->ontime() + ((NoteEvent*)e)->duration())
                              break;
                        ((NoteEvent*)e)->setDuration(ee->ontime() - e->ontime());
                        break;
                        }
                  if (((NoteEvent*)e)->duration() <= 0)
                        continue;
                  }
		_events.insert(e);
            }
      }

//---------------------------------------------------------
//   changeDivision
//---------------------------------------------------------

void MidiTrack::changeDivision(int newDivision)
      {
      EventList dl;

      int division = mf->division();
      foreach(Event* e, _events) {
            int tick = (e->ontime() * newDivision + division/2) / division;
            e->setOntime(tick);
            if (e->type() == ME_NOTE) {
                  NoteEvent* n = (NoteEvent*)e;
                  n->setDuration((n->duration() * newDivision + division/2) / division);
                  }
		dl.insert(e);
            }
      _events = dl;     // ?
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

      SigList sl;
      for (iSigEvent is = _siglist.begin(); is != _siglist.end(); ++is) {
            int tick    = (is->first * newDivision + _division/2) / _division;
            SigEvent se = is->second;
            sl.add(tick, se);
            }
      _siglist = sl;

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
      //int n = _tracks.size();
      for (int i = 0; i < _tracks.size(); ++i) {
            QList<int> channel;
            MidiTrack* mt = _tracks.at(i);
            foreach(Event* e, mt->events()) {
                  if (e->isChannelEvent() && !channel.contains(((ChannelEvent*)e)->channel()))
                        channel.append(((ChannelEvent*)e)->channel());
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
                  Event* e = *ie;
                  if (e->isChannelEvent()) {
                        int ch  = ((ChannelEvent*)e)->channel();
                        int idx = channel.indexOf(ch);
                        MidiTrack* t = _tracks.at(i + idx);
                        if (t != mt) {
                              t->insert(e);
                              ie = el.erase(ie);
                              continue;
                              }
                        }
                  ++ie;
                  }
            i += nn - 1;
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

      foreach(Event* e, _events) {
            int tick = e->ontime() + ticks;
            if (tick < 0)
                  tick = 0;
            e->setOntime(tick);
		dl.insert(e);
            }
      _events = dl;
      }

//---------------------------------------------------------
//   isDrumTrack
//---------------------------------------------------------

bool MidiTrack::isDrumTrack() const
      {
      return _drumTrack;
      }

//---------------------------------------------------------
//   insert
//---------------------------------------------------------

void EventList::insert(Event* e)
      {
      int ontime = e->ontime();
      if (!isEmpty() && last()->ontime() > ontime) {
            for (iEvent i = begin(); i != end(); ++i) {
                  if ((*i)->ontime() > ontime) {
                        QList<Event*>::insert(i, e);
                        return;
                        }
                  }
            }
      append(e);
      }

//---------------------------------------------------------
//   readData
//---------------------------------------------------------

static void readData(unsigned char* d, int dataLen, QString s)
      {
      QStringList l = s.simplified().split(" ", QString::SkipEmptyParts);
      if (dataLen != l.size()) {
            printf("error converting data string <%s>\n", s.toLatin1().data());
            }
      int numberBase = 16;
      for (int i = 0; i < l.size(); ++i) {
            bool ok;
            *d++ = l.at(i).toInt(&ok, numberBase);
            if (!ok)
                  printf("error converting data val <%s>\n", l.at(i).toLatin1().data());
            }
      }

//---------------------------------------------------------
//   readXml
//---------------------------------------------------------

void MidiTrack::readXml(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull();  e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            if (tag == "NoteOn") {
                  int tick    = e.attribute("tick").toInt();
                  int channel = e.attribute("channel").toInt();
                  int pitch   = e.attribute("pitch").toInt();
                  int velo    = e.attribute("velo").toInt();
                  NoteOn* e   = new NoteOn(tick, channel, pitch, velo);
                  _events.append(e);
                  }
            else if (tag == "NoteOff") {
                  int tick    = e.attribute("tick").toInt();
                  int channel = e.attribute("channel").toInt();
                  int pitch   = e.attribute("pitch").toInt();
                  int velo    = e.attribute("velo").toInt();
                  NoteOff* e  = new NoteOff(tick, channel, pitch, velo);
                  _events.append(e);
                  }
            else if (tag == "Lyric") {
                  int tick    = e.attribute("tick").toInt();
                  QString s   = e.text();
                  char* data  = new char[s.length() + 1];
                  strcpy(data, s.toLatin1().data());
                  MetaEvent* e = new MetaEvent(tick, META_LYRIC, s.length(), (unsigned char*)data);
                  _events.append(e);
                  }
            else if (tag == "TrackName") {
                  int tick    = e.attribute("tick").toInt();
                  QString s   = e.text();
                  char* data  = new char[s.length() + 1];
                  strcpy(data, s.toLatin1().data());
                  MetaEvent* e = new MetaEvent(tick, META_TRACK_NAME, s.length(), (unsigned char*)data);
                  _events.append(e);
                  }
            else if (tag == "Controller") {
                  int tick    = e.attribute("tick").toInt();
                  int channel = e.attribute("channel").toInt();
                  int ctrl    = e.attribute("ctrl").toInt();
                  int value   = e.attribute("value").toInt();
                  ControllerEvent* e = new ControllerEvent(tick, channel, ctrl, value);
                  _events.append(e);
                  }
            else if (tag == "Key") {
                  int tick = e.attribute("tick").toInt();
                  int key  = e.attribute("key").toInt();
                  int sex  = e.attribute("sex").toInt();
                  unsigned char* data = new unsigned char[2];
                  data[0]  = key;
                  data[1]  = sex;
                  MetaEvent* e = new MetaEvent(tick, META_KEY_SIGNATURE, 2, data);
                  _events.append(e);
                  }
            else if (tag == "Tempo") {
                  int tick = e.attribute("tick").toInt();
                  int val  = e.attribute("value").toInt();
                  unsigned char* data = new unsigned char[3];
                  data[0] = val >> 16;
                  data[1] = val >> 8;
                  data[2] = val;
                  MetaEvent* e = new MetaEvent(tick, META_TEMPO, 3, data);
                  _events.append(e);
                  }
            else if (tag == "TimeSig") {
                  int tick    = e.attribute("tick").toInt();
                  int num     = e.attribute("num").toInt();
                  int denom   = e.attribute("denom").toInt();
                  int metro   = e.attribute("metro").toInt();
                  int quarter = e.attribute("quarter").toInt();

                  unsigned char* data = new unsigned char[4];
                  data[0] = num;
                  data[1] = denom;
                  data[2] = metro;
                  data[3] = quarter;
                  MetaEvent* e = new MetaEvent(tick, META_TIME_SIGNATURE, 4, data);
                  _events.append(e);
                  }
            else if (tag == "Meta") {
                  int type = e.attribute("type").toInt();
                  int len  = e.attribute("len").toInt();
                  int tick    = e.attribute("tick").toInt();
                  unsigned char* data = new unsigned char[len];
                  readData(data, len, e.text());
                  MetaEvent* e = new MetaEvent(tick, type, len, data);
                  _events.append(e);
                  }
            else if (tag == "Sysex") {
                  int tick = e.attribute("tick").toInt();
                  int len  = e.attribute("len").toInt();
                  unsigned char* data = new unsigned char[len];
                  readData(data, len, e.text());
                  SysexEvent* e = new SysexEvent(tick, len, data);
                  _events.append(e);
                  }
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   getInitProgram
//---------------------------------------------------------

int MidiTrack::getInitProgram()
      {
      foreach(const Event* e, _events) {
            if (e->type() != ME_CONTROLLER)
                  continue;
            ControllerEvent* mc = (ControllerEvent*)e;
            if (mc->controller() == CTRL_PROGRAM)
                  return mc->value();
            }
      return -1;
      }

//---------------------------------------------------------
//   readXml
//---------------------------------------------------------

void MidiFile::readXml(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull();  e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            if (tag == "format")
                  _format = val.toInt();
            else if (tag == "division")
                  _division = val.toInt();
            else if (tag == "Track") {
                  MidiTrack* track = new MidiTrack(this);
                  track->readXml(e);
                  _tracks.append(track);
                  }
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   findChords
//---------------------------------------------------------

void MidiTrack::findChords()
      {
      EventList dl;
      int n = _events.size();

      Drumset* drumset;
      if (_drumTrack)
            drumset = smDrumset;
      else
            drumset = 0;
      int jitter = 3;   // tick tolerance for note on/off

      for (int i = 0; i < n; ++i) {
            Event* e = _events[i];
            if (e == 0)
                  continue;
            if (e->type() != ME_NOTE) {
                  dl.append(e);
                  continue;
                  }

            NoteEvent* note  = (NoteEvent*)e;
            int ontime       = note->ontime();
            int offtime      = note->offtime();
            ChordEvent* chord = new ChordEvent();
            chord->setOntime(ontime);
            chord->setDuration(note->duration());
            chord->notes().append(note);
            int voice = 0;
            chord->setVoice(voice);
            dl.append(chord);
            _events[i] = 0;

            bool useDrumset = false;
            if (drumset) {
                  int pitch = note->pitch();
                  if (drumset->isValid(pitch)) {
                        useDrumset = true;
                        voice = drumset->voice(pitch);
                        chord->setVoice(voice);
                        }
                  }
            for (int k = i + 1; k < n; ++k) {
                  if (_events[k] == 0 || _events[k]->type() != ME_NOTE)
                        continue;
                  NoteEvent* nn = (NoteEvent*)_events[k];
                  if (nn->ontime() - jitter > ontime)
                        break;
                  if (qAbs(nn->ontime() - ontime) > jitter || qAbs(nn->offtime() - offtime) > jitter)
                        continue;
                  int pitch = nn->pitch();
                  if (useDrumset) {
                        if (drumset->isValid(pitch) && drumset->voice(pitch) == voice) {
                              chord->notes().append(nn);
                              _events[k] = 0;
                              }
                        }
                  else {
                        chord->notes().append(nn);
                        _events[k] = 0;
                        }
                  }
            }
      _events = dl;
      }


//---------------------------------------------------------
//   separateVoices
//---------------------------------------------------------

int MidiTrack::separateVoices(int /*maxVoices*/)
      {
      if (_drumTrack)
            return 2;
      return 1;
#if 0
      int n = _events.size();
      for (int i = 0; i < n; ++i) {
            Event* e = _events[i];
            if (e->type() != ME_CHORD)
                  continue;
            MidiChord* mc = (MidiChord*) e;
            mc->setVoice(0);
            }
      return 1;
#endif
      }
