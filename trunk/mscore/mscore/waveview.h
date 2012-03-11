//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __WAVEVIEW_H__
#define __WAVEVIEW_H__

class Audio;

//---------------------------------------------------------
//   WaveView
//---------------------------------------------------------

class WaveView : public QWidget
      {
      Q_OBJECT

   public:
      WaveView(QWidget* parent = 0);
      void setAudio(Audio*);
      };
#endif

