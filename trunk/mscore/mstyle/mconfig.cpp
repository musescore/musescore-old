//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2010 Werner Schweer and others
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

#include "mconfig.h"

bool MStyleConfigData::genericAnimationsEnabled = true;
bool MStyleConfigData::animationsEnabled = true;
bool MStyleConfigData::progressBarAnimationsEnabled = true;
bool MStyleConfigData::progressBarAnimated = true;

int  MStyleConfigData::menuBarAnimationType = MStyleConfigData::MB_FADE;
int  MStyleConfigData::menuAnimationType = MStyleConfigData::ME_FADE;

int  MStyleConfigData::genericAnimationsDuration = 300;
int  MStyleConfigData::toolBarAnimationsDuration = 500;
int  MStyleConfigData::menuAnimationsDuration;
int  MStyleConfigData::progressBarAnimationsDuration;
int  MStyleConfigData::progressBarBusyStepDuration;
int  MStyleConfigData::menuBarAnimationsDuration;
int  MStyleConfigData::menuFollowMouseAnimationsDuration;
int  MStyleConfigData::menuBarFollowMouseAnimationsDuration;
bool MStyleConfigData::menuBarAnimationsEnabled = true;
bool MStyleConfigData::menuAnimationsEnabled = true;
int  MStyleConfigData::toolBarAnimationType = MStyleConfigData::TB_FADE;

int  MStyleConfigData::menuHighlightMode = MStyleConfigData::MM_DARK;
bool MStyleConfigData::tabSubtleShadow = true;
bool MStyleConfigData::showMnemonics = true;
int  MStyleConfigData::scrollBarAddLineButtons = 2;
int  MStyleConfigData::scrollBarSubLineButtons = 2;
int  MStyleConfigData::tabStyle = MStyleConfigData::TS_SINGLE;
bool MStyleConfigData::viewDrawFocusIndicator = true;
bool MStyleConfigData::cacheEnabled = true;
int  MStyleConfigData::maxCacheSize = 1024*1024;
bool MStyleConfigData::widgetExplorerEnabled = true;
bool MStyleConfigData::drawWidgetRects = true;
int  MStyleConfigData::scrollBarWidth  = 13;

