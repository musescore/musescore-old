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

#ifndef __MSTYLE_H__
#define __MSTYLE_H__

#include "tileset.h"
#include "stylehelper.h"

class StyleHelper;
class FrameShadowFactory;

//---------------------------------------------------------
//   MenuBarBaseEngine
//---------------------------------------------------------

class MenuBarBaseEngine {

   public:
      bool isAnimated(const QWidget*, const QPoint&) const   { return false;   }
      qreal opacity(const QWidget*, const QPoint&) const     { return 1.0;     }
      QRect currentRect(const QWidget*, const QPoint&) const { return QRect(); }
      QRect animatedRect(const QWidget*) const               { return QRect(); }
      bool isTimerActive(const QWidget*) const               { return false;   }
      };

//---------------------------------------------------------
//   Animations
//---------------------------------------------------------

class Animations : public QObject {
      Q_OBJECT
      MenuBarBaseEngine* _menuBarEngine;

   public:
      Animations(QStyle*);
      MenuBarBaseEngine& menuBarEngine() const { return *_menuBarEngine; }
      void registerWidget(QWidget*) {}
      void unregisterWidget(QWidget*) {}
      };

//---------------------------------------------------------
//   Transitions
//---------------------------------------------------------

class Transitions {
   public:
      Transitions(QStyle*) {}
      void registerWidget(QWidget*) {}
      void unregisterWidget(QWidget*) {}
      };

//---------------------------------------------------------
//   WindowManager
//---------------------------------------------------------

class WindowManager {
   public:
      WindowManager(QStyle*) {}
      void registerWidget(QWidget*) {}
      void unregisterWidget(QWidget*) {}
      };

//---------------------------------------------------------
//   MStyle
//---------------------------------------------------------

class MStyle : public QCommonStyle {
      Q_OBJECT

      bool emptyControl(const QStyleOption*, QPainter*, const QWidget*) const { return true; }
      bool drawMenuBarItem(const QStyleOption*, QPainter*, const QWidget*) const;

      Animations* _animations;
      Transitions* _transitions;
      WindowManager* _windowManager;
      FrameShadowFactory* _frameShadowFactory;

      StyleHelper _helper;

      //! metrics
      /*! these are copied from the old KStyle WidgetProperties */
      enum InternalMetrics {
            GlowWidth = 1,

            // checkbox. Do not change, unless
            // changing the actual cached pixmap size
            CheckBox_Size = 21,
            CheckBox_BoxTextSpace = 4,

            // combobox
            ComboBox_FrameWidth = 3,
            ComboBox_ButtonWidth = 19,
            ComboBox_ButtonMargin = 2,
            ComboBox_ButtonMargin_Left = 0,
            ComboBox_ButtonMargin_Right = 4,
            ComboBox_ButtonMargin_Top = 2,
            ComboBox_ButtonMargin_Bottom = 1,

            ComboBox_ContentsMargin = 0,
            ComboBox_ContentsMargin_Left = 2,
            ComboBox_ContentsMargin_Right = 0,
            ComboBox_ContentsMargin_Top = 0,
            ComboBox_ContentsMargin_Bottom = 0,

            // dockwidgets
            DockWidget_FrameWidth = 0,
            DockWidget_SeparatorExtend = 3,
            DockWidget_TitleMargin = 3,

            // generic frames
            Frame_FrameWidth = 3,

            // group boxes
            GroupBox_FrameWidth = 3,

            // header
            Header_TextToIconSpace = 3,
            Header_ContentsMargin = 3,

            // line edit
            LineEdit_FrameWidth = 3,

            // menu item
            MenuItem_AccelSpace = 16,
            MenuItem_ArrowWidth = 11,
            MenuItem_ArrowSpace = 3,
            MenuItem_CheckWidth = 16,
            MenuItem_CheckSpace = 3,
            MenuItem_IconWidth = 12,
            MenuItem_IconSpace = 3,
            MenuItem_Margin = 2,
            MenuItem_MinHeight = 20,

            // menu bar item
            MenuBarItem_Margin = 3,
            MenuBarItem_Margin_Left = 5,
            MenuBarItem_Margin_Right = 5,

            // pushbuttons
            PushButton_ContentsMargin = 5,
            PushButton_ContentsMargin_Left = 8,
            PushButton_ContentsMargin_Top = -1,
            PushButton_ContentsMargin_Right = 8,
            PushButton_ContentsMargin_Bottom = 0,
            PushButton_MenuIndicatorSize = 8,
            PushButton_TextToIconSpace = 6,

            // progress bar
            ProgressBar_BusyIndicatorSize = 10,
            ProgressBar_GrooveMargin = 2,

            // scrollbar
            ScrollBar_MinimumSliderHeight = 21,

            // spin boxes
            SpinBox_FrameWidth = 3,
            SpinBox_ButtonWidth = 19,
            SpinBox_ButtonMargin = 0,
            SpinBox_ButtonMargin_Left = 2,
            SpinBox_ButtonMargin_Right = 6,
            SpinBox_ButtonMargin_Top = 4,
            SpinBox_ButtonMargin_Bottom = 2,

            // splitter
            Splitter_Width = 3,

            // tabs
            TabBar_BaseOverlap = 7,
            TabBar_BaseHeight = 2,
            TabBar_ScrollButtonWidth = 18,
            TabBar_TabContentsMargin = 4,
            TabBar_TabContentsMargin_Left = 5,
            TabBar_TabContentsMargin_Right = 5,
            TabBar_TabContentsMargin_Top = 2,
            TabBar_TabContentsMargin_Bottom = 4,
            TabBar_TabOverlap =0,

            TabWidget_ContentsMargin = 4,

            // toolbuttons
            ToolButton_ContentsMargin = 4,
            ToolButton_InlineMenuIndicatorSize = 8,
            ToolButton_InlineMenuIndicatorXOff = -11,
            ToolButton_InlineMenuIndicatorYOff = -10,
            ToolButton_MenuIndicatorSize = 11,

            Tree_MaxExpanderSize = 9
            };

      QSize expandSize(const QSize& size, int main, int left = 0, int top = 0, int right = 0, int bottom = 0) const;
      QSize checkBoxSizeFromContents(const QStyleOption*, const QSize&, const QWidget*) const;
      QSize comboBoxSizeFromContents(const QStyleOption*, const QSize&, const QWidget*) const;
      QSize headerSectionSizeFromContents(const QStyleOption*, const QSize&, const QWidget*) const;
      QSize menuItemSizeFromContents(const QStyleOption*, const QSize&, const QWidget*) const;
      QSize pushButtonSizeFromContents(const QStyleOption*, const QSize&, const QWidget*) const;
      QSize tabBarTabSizeFromContents(const QStyleOption*, const QSize& size, const QWidget*) const;
      QSize toolButtonSizeFromContents(const QStyleOption*, const QSize&, const QWidget*) const;
      //! returns true for vertical tabs
      bool isVerticalTab(const QStyleOptionTab* option) const { return isVerticalTab(option->shape); }
      bool isVerticalTab(const QTabBar::Shape& shape) const;
      void polishScrollArea(QAbstractScrollArea* scrollArea) const;
      bool drawPanelTipLabelPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget) const;

   public:
      MStyle();
      virtual void drawControl(ControlElement, const QStyleOption*,
         QPainter*, const QWidget* w = 0) const;
      virtual int pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget ) const;
      QSize sizeFromContents(ContentsType, const QStyleOption*, const QSize&, const QWidget*) const;
      virtual void polish(QWidget* widget);
      void unpolish(QWidget* widget);
      void drawPrimitive(PrimitiveElement, const QStyleOption*, QPainter*, const QWidget*) const;

      Animations& animations() const                 { return *_animations; }
      Transitions& transitions() const               { return *_transitions; }
      WindowManager& windowManager() const           { return *_windowManager; }
      FrameShadowFactory& frameShadowFactory() const { return *_frameShadowFactory; }
      };

typedef bool (MStyle::*StyleControl)(const QStyleOption*, QPainter*, const QWidget*) const;
typedef bool (MStyle::*StylePrimitive)(const QStyleOption*, QPainter*, const QWidget*) const;

#endif

