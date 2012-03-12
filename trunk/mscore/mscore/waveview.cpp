//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "waveview.h"
#include "piano.h"
#include "libmscore/audio.h"
#include "libmscore/score.h"

#include <vorbis/vorbisfile.h>
//---------------------------------------------------------
//   VorbisData
//---------------------------------------------------------

struct VorbisData {
      int pos;          // current position in audio->data()
      QByteArray data;
      };

static VorbisData vorbisData;

static size_t ovRead(void* ptr, size_t size, size_t nmemb, void* datasource);
static int ovSeek(void* datasource, ogg_int64_t offset, int whence);
static long ovTell(void* datasource);

static ov_callbacks ovCallbacks = {
      ovRead, ovSeek, 0, ovTell
      };

//---------------------------------------------------------
//   ovRead
//---------------------------------------------------------

static size_t ovRead(void* ptr, size_t size, size_t nmemb, void* datasource)
      {
      VorbisData* vd = (VorbisData*)datasource;
      size_t n = size * nmemb;
      if (vd->data.size() < int(vd->pos + n))
            n = vd->data.size() - vd->pos;
      if (n) {
            const char* src = vd->data.data() + vd->pos;
            memcpy(ptr, src, n);
            vd->pos += n;
            }
      return n;
      }

//---------------------------------------------------------
//   ovSeek
//---------------------------------------------------------

static int ovSeek(void* datasource, ogg_int64_t offset, int whence)
      {
      VorbisData* vd = (VorbisData*)datasource;
      switch(whence) {
            case SEEK_SET:
                  vd->pos = offset;
                  break;
            case SEEK_CUR:
                  vd->pos += offset;
                  break;
            case SEEK_END:
                  vd->pos = vd->data.size() - offset;
                  break;
            }
      return 0;
      }

//---------------------------------------------------------
//   ovTell
//---------------------------------------------------------

static long ovTell(void* datasource)
      {
      VorbisData* vd = (VorbisData*)datasource;
      return vd->pos;
      }


//---------------------------------------------------------
//   WaveView
//---------------------------------------------------------

WaveView::WaveView(QWidget* parent)
   : QWidget(parent)
      {
      _xpos   = 0;
      _xmag   = 0.1;
      _timeType = TICKS;
      setMinimumHeight(50);
      }

//---------------------------------------------------------
//   setAudio
//---------------------------------------------------------

void WaveView::setAudio(Audio* audio)
      {
      OggVorbis_File vf;
      vorbisData.pos  = 0;
      vorbisData.data = audio->data();
      int n = ov_open_callbacks(&vorbisData, &vf, 0, 0, ovCallbacks);
      if (n < 0) {
            printf("ogg open failed: %d\n", n);
            return;
            }
      long samples = 0;
      for(;;) {
            float** pcm;
            int section;
            long rn = ov_read_float(&vf, &pcm, 10000, &section);
            if (rn == 0)
                  break;
            // memcpy(l, pcm[0], rn * sizeof(float));
            // memcpy(r, pcm[1], rn * sizeof(float));
            samples += rn;
            }
      ov_clear(&vf);
      }

//---------------------------------------------------------
//   setXpos
//---------------------------------------------------------

void WaveView::setXpos(int val)
      {
      _xpos = val;
      update();
      }

//---------------------------------------------------------
//   setMag
//---------------------------------------------------------

void WaveView::setMag(double x, double)
      {
      if (_xmag != x) {
            _xmag = x;
            update();
            }
      }

//---------------------------------------------------------
//   moveLocator
//---------------------------------------------------------

void WaveView::moveLocator(int i)
      {
      if (_locator[i].valid()) {
            update();
            // qreal x = qreal(pos2pix(_locator[i]));
            // locatorLines[i]->setPos(QPointF(x, 0.0));
            }
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void WaveView::setScore(Score* s, Pos* lc)
      {
      _score = s;
      _locator = lc;
      _cursor.setContext(s->tempomap(), s->sigmap());
      }

static const int MAP_OFFSET = 5;

//---------------------------------------------------------
//   pos2pix
//---------------------------------------------------------

int WaveView::pos2pix(const Pos& p) const
      {
      return lrint((p.time(_timeType)+480) * _xmag)
         + MAP_OFFSET - _xpos + pianoWidth;
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void WaveView::paintEvent(QPaintEvent* event)
      {
      QPainter p(this);
      int xoffset = pianoWidth + 5;
      QRect rt(0, 0, xoffset, height());
      QRect r(event->rect());

      p.fillRect(r, Qt::yellow);
      if (rt.intersects(r.translated(_xpos, 0)))
            p.fillRect(rt.translated(-_xpos, 0), Qt::lightGray);

      QPen pen(Qt::lightGray, 2);
      p.setPen(pen);
      int y = height() / 2;
      int x = xoffset - _xpos;
      int w = width() - x;
      if (w > 0)
            p.drawLine(0, y, width(), y);

      static const QColor lcColors[3] = { Qt::red, Qt::blue, Qt::blue };
      for (int i = 0; i < 3; ++i) {
            if (!_locator[i].valid())
                  continue;
            QPen pen(lcColors[i], 3);
            p.setPen(pen);
            int xp      = pos2pix(_locator[i]);
            p.drawLine(xp, 0, xp, height());
            }
      }

