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

int  MStyleConfigData::genericAnimationsDuration = 150;
int  MStyleConfigData::toolBarAnimationsDuration = 50;
int  MStyleConfigData::menuAnimationsDuration = 150;
int  MStyleConfigData::progressBarAnimationsDuration;
int  MStyleConfigData::progressBarBusyStepDuration = 50;
int  MStyleConfigData::menuBarAnimationsDuration = 150;
int  MStyleConfigData::menuFollowMouseAnimationsDuration = 40;
int  MStyleConfigData::menuBarFollowMouseAnimationsDuration = 80;
bool MStyleConfigData::menuBarAnimationsEnabled = true;
bool MStyleConfigData::menuAnimationsEnabled = true;
int  MStyleConfigData::toolBarAnimationType = MStyleConfigData::TB_FADE;

int  MStyleConfigData::menuHighlightMode = MStyleConfigData::MM_DARK;
bool MStyleConfigData::tabSubtleShadow = true;
bool MStyleConfigData::showMnemonics = true;
int  MStyleConfigData::scrollBarAddLineButtons = 2;
int  MStyleConfigData::scrollBarSubLineButtons = 1;
int  MStyleConfigData::tabStyle = MStyleConfigData::TS_SINGLE;
bool MStyleConfigData::viewDrawFocusIndicator = true;
bool MStyleConfigData::cacheEnabled = true;
int  MStyleConfigData::maxCacheSize = 512;
bool MStyleConfigData::widgetExplorerEnabled = false;
bool MStyleConfigData::drawWidgetRects = true;
int  MStyleConfigData::scrollBarWidth  = 12;    // 15;

bool MStyleConfigData::comboBoxTransitionsEnabled = true;
bool MStyleConfigData::labelTransitionsEnabled = true;
bool MStyleConfigData::lineEditTransitionsEnabled = true;
bool MStyleConfigData::stackedWidgetTransitionsEnabled = true;

int  MStyleConfigData::comboBoxTransitionsDuration = 75;
int  MStyleConfigData::labelTransitionsDuration = 75;
int  MStyleConfigData::lineEditTransitionsDuration = 150;
int  MStyleConfigData::stackedWidgetTransitionsDuration = 150;

bool MStyleConfigData::toolTipDrawStyledFrames = true;

