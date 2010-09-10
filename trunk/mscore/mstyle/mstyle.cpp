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

#include "mstyle.h"
#include <math.h>
#include "colorutils.h"
#include "frameshadow.h"

enum MenuHighlightMode {
      MM_STRONG, MM_DARK
      };

MenuHighlightMode menuHighlightMode = MM_DARK;

#define OxygenStyleConfigData_scrollBarWidth          8
#define OxygenStyleConfigData_toolTipDrawStyledFrames true
#define OxygenStyleConfigData_toolTipTransparent     true

//---------------------------------------------------------
//   Animations
//---------------------------------------------------------

Animations::Animations(QStyle*)
      {
      _menuBarEngine = 0;
      }

//---------------------------------------------------------
//   MStyle
//---------------------------------------------------------

MStyle::MStyle()
   : QCommonStyle()
      {
      _animations = new Animations(this);
      _transitions = new Transitions(this);
      _windowManager = new WindowManager(this);
      _frameShadowFactory = new FrameShadowFactory(this);
      }

//---------------------------------------------------------
//   drawControl
//---------------------------------------------------------

void MStyle::drawControl(ControlElement e, const QStyleOption* o,
   QPainter* p, const QWidget* w) const
      {
      p->save();
      StyleControl fcn(0);
      switch(e) {
            case CE_MenuBarEmptyArea: fcn = &MStyle::emptyControl; break;
            case CE_MenuBarItem:      fcn = &MStyle::drawMenuBarItem; break;
            case CE_MenuItem:
            default:
                  break;
            }
      p->restore();
      if (!(fcn && (this->*fcn)(o, p, w)))
            QCommonStyle::drawControl(e, o, p, w);
      }

//---------------------------------------------------------
//   drawMenuBarItem
//---------------------------------------------------------

bool MStyle::drawMenuBarItem(const QStyleOption* option, QPainter* painter,
   const QWidget* widget) const
      {
      const QStyleOptionMenuItem* menuOpt = ::qstyleoption_cast<const QStyleOptionMenuItem*>(option);
      if (!menuOpt)
            return true;

      const State& flags(option->state);
      const bool enabled(flags & State_Enabled);
      const QRect& r(option->rect);
      const QPalette& palette(option->palette);

      if (enabled) {
            const bool active(flags & State_Selected);
            const bool animated(animations().menuBarEngine().isAnimated(widget, r.topLeft()));
            const qreal opacity(animations().menuBarEngine().opacity(widget, r.topLeft()));
            const QRect currentRect(animations().menuBarEngine().currentRect(widget, r.topLeft()));
            const QRect animatedRect(animations().menuBarEngine().animatedRect(widget));

            const bool intersected(animatedRect.intersects(r));
            const bool current(currentRect.contains(r.topLeft()));
            const bool timerIsActive(animations().menuBarEngine().isTimerActive(widget));

            // do nothing in case of empty intersection between animated rect and current
            if ((intersected || !animated || animatedRect.isNull()) && (active || animated || timerIsActive)) {
                  QColor color(palette.color(QPalette::Window));
                  if (menuHighlightMode != MM_DARK) {
                        if (flags & State_Sunken) {
                              if (menuHighlightMode == MM_STRONG)
                                    color = palette.color(QPalette::Highlight);
                              else
                                    color = ColorUtils::mix(color, ColorUtils::tint(color, palette.color(QPalette::Highlight), 0.6));
                              }
                        else {
                              if (menuHighlightMode == MM_STRONG)
                                    color = ColorUtils::tint(color, _helper.viewHoverBrush().brush(palette).color());
                              else
                                    color = ColorUtils::mix(color, ColorUtils::tint(color, _helper.viewHoverBrush().brush(palette).color()));
                              }
                        }
                  else
                        color = _helper.calcMidColor(_helper.backgroundColor(color, widget, r.center()));

                  // drawing
                  if (animated && intersected) {
                        _helper.holeFlat(color, 0.0)->render(animatedRect.adjusted(1,1,-1,-1), painter, TileSet::Full);
                        }
                  else if (timerIsActive && current) {
                        _helper.holeFlat(color, 0.0)->render(r.adjusted(1,1,-1,-1), painter, TileSet::Full);
                        }
                  else if (animated && current) {
                        color.setAlphaF(opacity);
                        _helper.holeFlat(color, 0.0)->render(r.adjusted(1,1,-1,-1), painter, TileSet::Full);
                        }
                  else if (active ) {
                        _helper.holeFlat(color, 0.0)->render(r.adjusted(1,1,-1,-1), painter, TileSet::Full);
                        }
                  }
            }

      // text
      QPalette::ColorRole role(QPalette::WindowText);
      if (menuHighlightMode == MM_STRONG && (flags & State_Sunken) && enabled)
            role = QPalette::HighlightedText;
      drawItemText(painter, r, Qt::AlignCenter | Qt::TextShowMnemonic, palette, enabled, menuOpt->text, role);
      return true;
      }

//---------------------------------------------------------
//   pixelMetric
//---------------------------------------------------------

int MStyle::pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget ) const
      {
      switch(metric) {
            // rely on QCommonStyle here
            case PM_SmallIconSize:
            case PM_ButtonIconSize:
                  // return KIconLoader::global()->currentSize(KIconLoader::Small);
            case PM_ToolBarIconSize:
                  // return KIconLoader::global()->currentSize(KIconLoader::Toolbar);
            case PM_LargeIconSize:
                  // return KIconLoader::global()->currentSize(KIconLoader::Dialog);
            case PM_MessageBoxIconSize:
                  // return KIconLoader::SizeHuge;
                  break;

            case PM_DefaultFrameWidth:
                  {
                  if( qobject_cast<const QLineEdit*>(widget) )
                        return LineEdit_FrameWidth;
                  else if( qobject_cast<const QComboBox*>(widget))
                        return ComboBox_FrameWidth;
                  else if( qobject_cast<const QFrame*>(widget) )
                        return Frame_FrameWidth;
                  else if( qstyleoption_cast<const QStyleOptionGroupBox *>(option) )
                        return GroupBox_FrameWidth;
                  else return 1;
                  }

            case PM_LayoutLeftMargin:
            case PM_LayoutTopMargin:
            case PM_LayoutRightMargin:
            case PM_LayoutBottomMargin:
                  {
                  // use either Child margin or TopLevel margin, depending on
                  // widget type
                  if ((option && (option->state & QStyle::State_Window)) || (widget && widget->isWindow())) {
                        return pixelMetric( PM_DefaultTopLevelMargin, option, widget );
                        }
                  else {
                        return pixelMetric( PM_DefaultChildMargin, option, widget );
                        }
                  }

            // push buttons
            case PM_ButtonMargin:
                  return 5;

            case PM_MenuButtonIndicator:
                  if (qstyleoption_cast<const QStyleOptionToolButton*>(option))
                        return ToolButton_MenuIndicatorSize;
                  else
                        return PushButton_MenuIndicatorSize;

            // tabbars
            case PM_TabBarTabHSpace:
                  {
                  if( const QStyleOptionTab* tabOpt = qstyleoption_cast<const QStyleOptionTab*>(option) ) {
                        if( tabOpt->text.isNull() && !tabOpt->icon.isNull())
                              return 0;
                        if( tabOpt->icon.isNull() && !tabOpt->text.isNull())
                              return 0;
                        }
                  else
                        return 4;
                  }

            case PM_ScrollBarExtent:
                  return 10; // OxygenStyleConfigData::scrollBarWidth() + 2;

                        // tooltip label
            case PM_ToolTipLabelFrameWidth:
                  // if (OxygenStyleConfigData::toolTipDrawStyledFrames())
                  //      return 3;
                  // else
                        break;

            case PM_DefaultChildMargin: return 4;
            case PM_DefaultTopLevelMargin: return 11;
            case PM_DefaultLayoutSpacing: return 4;
            case PM_LayoutHorizontalSpacing: return -1;
            case PM_LayoutVerticalSpacing: return -1;

            // buttons
            case PM_ButtonDefaultIndicator: return 0;
            case PM_ButtonShiftHorizontal: return 0;
            case PM_ButtonShiftVertical: return 0;

            // checkboxes: return radiobutton sizes
            case PM_IndicatorWidth: return CheckBox_Size;
            case PM_IndicatorHeight: return CheckBox_Size;
            case PM_ExclusiveIndicatorWidth: return CheckBox_Size;
            case PM_ExclusiveIndicatorHeight: return CheckBox_Size;
            case PM_CheckListControllerSize: return CheckBox_Size;
            case PM_CheckListButtonSize: return CheckBox_Size;

            // splitters and dock widgets
            case PM_SplitterWidth: return Splitter_Width;
            case PM_DockWidgetFrameWidth: return DockWidget_FrameWidth;
            case PM_DockWidgetSeparatorExtent: return DockWidget_SeparatorExtend;
            case PM_DockWidgetTitleMargin: return DockWidget_TitleMargin;

            // progress bar
            case PM_ProgressBarChunkWidth: return 1;

            // menu bars
            case PM_MenuBarPanelWidth: return 0;
            case PM_MenuBarHMargin: return 0;
            case PM_MenuBarVMargin: return 0;
            case PM_MenuBarItemSpacing: return 0;
            case PM_MenuDesktopFrameWidth: return 0;
            case PM_MenuPanelWidth: return 5;

            case PM_MenuScrollerHeight: return 10;
            case PM_MenuTearoffHeight: return 10;

            //! tabbar
            case PM_TabBarTabVSpace: return 0;
            case PM_TabBarBaseHeight: return TabBar_BaseHeight;
            case PM_TabBarBaseOverlap: return TabBar_BaseOverlap;
            case PM_TabBarTabOverlap: return 0;
            case PM_TabBarScrollButtonWidth: return TabBar_ScrollButtonWidth;

            /*
            disable shifts: return because last time I checked it did not work well
            for South and East tabs. Instead the shifts are added directly in
            drawTabBarTabLabel. (Hugo)
            */
            case PM_TabBarTabShiftVertical: return 0;
            case PM_TabBarTabShiftHorizontal: return 0;

            // sliders
            case PM_SliderThickness: return 23;
            case PM_SliderControlThickness: return 23;
            case PM_SliderLength: return 13;

            // spinboxes
            case PM_SpinBoxFrameWidth: return SpinBox_FrameWidth;

            // comboboxes
            case PM_ComboBoxFrameWidth: return ComboBox_FrameWidth;

            // tree view header
            case PM_HeaderMarkSize: return 9;
            case PM_HeaderMargin: return 3;

            // toolbars
            case PM_ToolBarFrameWidth: return 0;
            case PM_ToolBarHandleExtent: return 6;
            case PM_ToolBarSeparatorExtent: return 6;

            case PM_ToolBarExtensionExtent: return 16;
            case PM_ToolBarItemMargin: return 1;
            case PM_ToolBarItemSpacing: return 1;

            // MDI windows titlebars
            case PM_TitleBarHeight: return 20;

            // spacing between widget and scrollbars
            case PM_ScrollView_ScrollBarSpacing: return -2;

            default:
            break;
            }

      // fallback
      return QCommonStyle::pixelMetric( metric, option, widget );
      }

//---------------------------------------------------------
//   expandSize
//    expand size based on margins
//---------------------------------------------------------

QSize MStyle::expandSize(const QSize& size, int main, int left, int top, int right, int bottom) const
      {
      return size + QSize(2*main+left+right, 2*main+top+bottom);
      }

//---------------------------------------------------------
//   sizeFromContents
//---------------------------------------------------------

QSize MStyle::sizeFromContents(ContentsType element, const QStyleOption* option, const QSize& size, const QWidget* widget) const
      {
      switch(element) {
            case CT_CheckBox:       return checkBoxSizeFromContents(option, size, widget);
            case CT_ComboBox:       return comboBoxSizeFromContents(option, size, widget);
            case CT_HeaderSection:  return headerSectionSizeFromContents(option, size, widget);
            case CT_PushButton:     return pushButtonSizeFromContents(option, size, widget);
            case CT_MenuBar:        return size;
            case CT_MenuBarItem:
                    return expandSize(size, MenuBarItem_Margin, MenuBarItem_Margin_Left, 0, MenuBarItem_Margin_Right, 0);

            case CT_MenuItem:       return menuItemSizeFromContents(option, size, widget);
            case CT_RadioButton:    return checkBoxSizeFromContents(option, size, widget);
            case CT_TabBarTab:      return tabBarTabSizeFromContents(option, size, widget);
            case CT_TabWidget:      return expandSize(size, TabWidget_ContentsMargin);

            case CT_ToolButton:     return toolButtonSizeFromContents(option, size, widget);
            default: return QCommonStyle::sizeFromContents(element, option, size, widget);
            }
      }

//---------------------------------------------------------
//   checkBoxSizeFromContents
//---------------------------------------------------------

QSize MStyle::checkBoxSizeFromContents(const QStyleOption*, const QSize& contentsSize, const QWidget*) const
      {
      //Add size for indicator
      const int indicator( CheckBox_Size );

      //Make sure we can fit the indicator
      QSize size( contentsSize );
      size.setHeight(qMax(size.height(), indicator));

      //Add space for the indicator and the icon
      const int spacer( CheckBox_BoxTextSpace );
      size.rwidth() += indicator + spacer;

      return size;
      }

//---------------------------------------------------------
//   comboBoxSizeFromContents
//---------------------------------------------------------

QSize MStyle::comboBoxSizeFromContents( const QStyleOption* option, const QSize& contentsSize, const QWidget*) const
      {
      QSize size = expandSize( contentsSize,
         ComboBox_ContentsMargin,
         ComboBox_ContentsMargin_Left,
         ComboBox_ContentsMargin_Top,
         ComboBox_ContentsMargin_Right,
         ComboBox_ContentsMargin_Bottom );

      // add frame width
      size = expandSize(size, ComboBox_FrameWidth);

      // Add the button width
      size.rwidth() += ComboBox_ButtonWidth;

      // TODO: For some reason the size is not right in the following configurations
      // this is still to be understood and might reveal some deeper issue.
      // notably, should compare to zhqt is done for PushButtons
      const QStyleOptionComboBox *cb = qstyleoption_cast<const QStyleOptionComboBox *>(option);
      if( cb && !cb->editable && (!cb->currentIcon.isNull() || cb->fontMetrics.height() > 13 ) )
            size.rheight()+=1;

      // also expand to account for scrollbar
      size.rwidth() += OxygenStyleConfigData_scrollBarWidth - 6;
      return size;
      }

//---------------------------------------------------------
//   headerSectionSizeFromContents
//---------------------------------------------------------

QSize MStyle::headerSectionSizeFromContents(const QStyleOption* option, const QSize& contentsSize, const QWidget*) const
      {
      const QStyleOptionHeader* headerOpt( qstyleoption_cast<const QStyleOptionHeader *>( option ));
      if( !headerOpt )
            return contentsSize;

      //TODO: check if hardcoded icon size is the right thing to do
      QSize iconSize = headerOpt->icon.isNull() ? QSize(0,0) : QSize(22,22);
      QSize textSize = headerOpt->fontMetrics.size(0, headerOpt->text);

      int iconSpacing = Header_TextToIconSpace;
      int w = iconSize.width() + iconSpacing + textSize.width();
      int h = qMax(iconSize.height(), textSize.height() );

      return expandSize( QSize(w, h), Header_ContentsMargin );
      }

//---------------------------------------------------------
//   menuItemSizeFromContents
//---------------------------------------------------------

QSize MStyle::menuItemSizeFromContents(const QStyleOption* option, const QSize& contentsSize, const QWidget* widget) const
      {
      const QStyleOptionMenuItem* menuItemOption = qstyleoption_cast<const QStyleOptionMenuItem*>(option);
      if( !menuItemOption)
            return contentsSize;

      // First, we calculate the intrinsic size of the item.
      // this must be kept consistent with what's in drawMenuItemContol
      QSize insideSize;

      switch (menuItemOption->menuItemType) {
            case QStyleOptionMenuItem::Normal:
            case QStyleOptionMenuItem::DefaultItem:
            case QStyleOptionMenuItem::SubMenu:
                  {
                  int iconColW = qMax( menuItemOption->maxIconWidth, (int) MenuItem_IconWidth );
                  int leftColW = iconColW;
                  if( menuItemOption->menuHasCheckableItems ) {
                        leftColW += MenuItem_CheckWidth + MenuItem_CheckSpace; }
                        leftColW += MenuItem_IconSpace;
                        int rightColW = MenuItem_ArrowSpace + MenuItem_ArrowWidth;

                        QFontMetrics fm(menuItemOption->font);
                        int textW;
                        int tabPos = menuItemOption->text.indexOf(QLatin1Char('\t'));
                        if( tabPos == -1)
                              textW = contentsSize.width();
                        else {
                              // The width of the accelerator is not included here since
                              // Qt will add that on separately after obtaining the
                              // sizeFromContents() for each menu item in the menu to be shown
                              // ( see QMenuPrivate::calcActionRects() )
                              textW = contentsSize.width() + MenuItem_AccelSpace;
                              }

                        int h = qMax(contentsSize.height(), (int) MenuItem_MinHeight );
                        insideSize = QSize(leftColW + textW + rightColW, h);
                        break;
                        }

            case QStyleOptionMenuItem::Separator:
                  {
                  // separator can have a title and an icon
                  // in that case they are rendered as menubar 'title', which
                  // corresponds to checked toolbuttons.
                  // a rectangle identical to the one of normal items is returned.
                  if( !( menuItemOption->text.isEmpty() && menuItemOption->icon.isNull() ) ) {
                        QStyleOptionMenuItem local( *menuItemOption );
                        local.menuItemType = QStyleOptionMenuItem::Normal;
                        return menuItemSizeFromContents( &local, contentsSize, widget );
                        }
                  else {
                        insideSize = QSize(10, 0);
                        break;
                        }
                  }
            case QStyleOptionMenuItem::Scroller:
            case QStyleOptionMenuItem::TearOff:
            case QStyleOptionMenuItem::Margin:
            case QStyleOptionMenuItem::EmptyArea:
                  return contentsSize;
            }

      // apply the outermost margin.
      return expandSize(insideSize, MenuItem_Margin );
      }

//---------------------------------------------------------
//   pushButtonSizeFromContents
//---------------------------------------------------------

QSize MStyle::pushButtonSizeFromContents(const QStyleOption* option, const QSize& contentsSize, const QWidget*) const
      {
      const QStyleOptionButton* bOpt = qstyleoption_cast<const QStyleOptionButton*>(option);
      if( !bOpt)
            return contentsSize;

      // adjust to handle button margins
      QSize size = expandSize( contentsSize,
         PushButton_ContentsMargin,
         PushButton_ContentsMargin_Left,
         PushButton_ContentsMargin_Top,
         PushButton_ContentsMargin_Right,
         PushButton_ContentsMargin_Bottom );

      if( bOpt->features & QStyleOptionButton::HasMenu) {
            size.rwidth() += PushButton_TextToIconSpace;
            }

      if( !bOpt->text.isEmpty() && !bOpt->icon.isNull()) {
            // Incorporate the spacing between the icon and text. Qt sticks 4 there,
            // but we use PushButton::TextToIconSpace.
            size.rwidth() += PushButton_TextToIconSpace -4;
            }
      return size;
      }

//---------------------------------------------------------
//   tabBarTabSizeFromContents
//---------------------------------------------------------

QSize MStyle::tabBarTabSizeFromContents(const QStyleOption* option, const QSize& contentsSize, const QWidget* widget) const
      {
      const QStyleOptionTab *tabOpt( qstyleoption_cast<const QStyleOptionTab*>(option) );

      QSize size;
      const bool verticalTabs(tabOpt && isVerticalTab(tabOpt));
      if( verticalTabs ) {
            size = expandSize( contentsSize,
            TabBar_TabContentsMargin,
            TabBar_TabContentsMargin_Top,
            TabBar_TabContentsMargin_Right,
            TabBar_TabContentsMargin_Bottom,
            TabBar_TabContentsMargin_Left );
            }
      else {
            size = expandSize( contentsSize,
            TabBar_TabContentsMargin,
            TabBar_TabContentsMargin_Left,
            TabBar_TabContentsMargin_Top,
            TabBar_TabContentsMargin_Right,
            TabBar_TabContentsMargin_Bottom );
            }

      // need to add extra size to match corner buttons
      // try cast parent for tabWidget
      const QTabWidget* tabWidget( widget ? qobject_cast<const QTabWidget*>( widget->parent() ):0 );
      if( !tabWidget )
            return size;

      // try get corner widgets
      const QWidget* leftWidget( tabWidget->cornerWidget( Qt::TopLeftCorner ) );
      const QWidget* rightWidget( tabWidget->cornerWidget( Qt::TopRightCorner ) );
      QSize cornerSize;
      if( leftWidget && leftWidget->isVisible() )
            cornerSize = leftWidget->minimumSizeHint();
      if( rightWidget && rightWidget->isVisible() )
            cornerSize = cornerSize.expandedTo( rightWidget->minimumSizeHint() );
      if( !cornerSize.isValid() )
            return size;

      // expand size
      // note: the extra pixels added to the relevant dimension are fine-tuned.
      if( verticalTabs )
            size.setWidth( qMax( size.width(), cornerSize.width() + 6 ) );
      else
            size.setHeight( qMax( size.height(), cornerSize.height() + 4 ) );
      return size;
      }

//---------------------------------------------------------
//   toolButtonSizeFromContents
//---------------------------------------------------------

QSize MStyle::toolButtonSizeFromContents(const QStyleOption* option, const QSize& contentsSize, const QWidget* widget) const
      {
      QSize size = contentsSize;
      const QStyleOptionToolButton* tbOpt = qstyleoption_cast<const QStyleOptionToolButton*>(option);
      if( tbOpt && !tbOpt->icon.isNull() && !tbOpt->text.isEmpty() && tbOpt->toolButtonStyle == Qt::ToolButtonTextUnderIcon) {
            size.rheight() -= 5;
            }

      // We want to avoid super-skiny buttons, for things like "up" when icons + text
      // For this, we would like to make width >= height.
      // However, once we get here, QToolButton may have already put in the menu area
      // (PM_MenuButtonIndicator) into the width. So we may have to take it out, fix things
      // up, and add it back in. So much for class-independent rendering...
      int menuAreaWidth = 0;
      if( tbOpt ) {
            if( tbOpt->features & QStyleOptionToolButton::MenuButtonPopup) {
                  menuAreaWidth = pixelMetric(QStyle::PM_MenuButtonIndicator, option, widget);
                  }
            else if( tbOpt->features & QStyleOptionToolButton::HasMenu) {
                  // TODO: something wrong here: The size is not properly accounted for
                  // when drawing the slab.
                  size.rwidth() += ToolButton_InlineMenuIndicatorSize;
                  }
            }
      size.rwidth() -= menuAreaWidth;
      if( size.width() < size.height())
            size.setWidth(size.height());

      size.rwidth() += menuAreaWidth;

      const QToolButton* t( qobject_cast<const QToolButton*>(widget) );
      if( t && t->autoRaise() )
            return expandSize( size, ToolButton_ContentsMargin ); // these are toolbutton margins
      else
            return expandSize( size, PushButton_ContentsMargin, 0, PushButton_ContentsMargin_Top, 0,
                  PushButton_ContentsMargin_Bottom );
      }

//---------------------------------------------------------
//   isVerticalTab
//---------------------------------------------------------

bool MStyle::isVerticalTab( const QTabBar::Shape& shape ) const
      {
      return shape == QTabBar::RoundedEast
         || shape == QTabBar::RoundedWest
         || shape == QTabBar::TriangularEast
         || shape == QTabBar::TriangularWest;
      }

//---------------------------------------------------------
//   polish
//---------------------------------------------------------

void MStyle::polish(QWidget* widget)
      {
      if(!widget)
            return;

      // register widget to animations
      animations().registerWidget( widget );
      transitions().registerWidget( widget );
      windowManager().registerWidget( widget );
      frameShadowFactory().registerWidget( widget, _helper );

      // scroll areas
      if (QAbstractScrollArea* scrollArea = qobject_cast<QAbstractScrollArea*>(widget)) {
            polishScrollArea(scrollArea );
            }

      // several widgets set autofill background to false, which effectively breaks the background
      // gradient rendering. Instead of patching all concerned applications,
      // we change the background here
      if (widget->inherits( "MessageList::Core::Widget")) {
            widget->setAutoFillBackground(false);
            }

      // adjust flags for windows and dialogs
      switch(widget->windowFlags() & Qt::WindowType_Mask) {
            case Qt::Window:
            case Qt::Dialog:
                  widget->setAttribute(Qt::WA_StyledBackground);
                  break;

            case Qt::ToolTip:
                  if(!widget->autoFillBackground() ) {
                        widget->setAttribute(Qt::WA_TranslucentBackground);
#ifdef Q_WS_WIN
                        //FramelessWindowHint is needed on windows to make WA_TranslucentBackground work properly
                        widget->setWindowFlags(widget->windowFlags() | Qt::FramelessWindowHint);
#endif
                        }

                  break;

            default:
                  break;
            }

      if (qobject_cast<QAbstractItemView*>(widget)
         || qobject_cast<QAbstractSpinBox*>(widget)
         || qobject_cast<QCheckBox*>(widget)
         || qobject_cast<QComboBox*>(widget)
         || qobject_cast<QDial*>(widget)
         || qobject_cast<QLineEdit*>(widget)
         || qobject_cast<QPushButton*>(widget)
         || qobject_cast<QRadioButton*>(widget)
         || qobject_cast<QScrollBar*>(widget)
         || qobject_cast<QSlider*>(widget)
         || qobject_cast<QSplitterHandle*>(widget)
         || qobject_cast<QTabBar*>(widget)
         || qobject_cast<QTextEdit*>(widget)
         || qobject_cast<QToolButton*>(widget)
         ) {
            widget->setAttribute(Qt::WA_Hover);
            }

      // also enable hover effects in itemviews' viewport
      if( QAbstractItemView *itemView = qobject_cast<QAbstractItemView*>(widget)) {
            itemView->viewport()->setAttribute(Qt::WA_Hover);
            }

      // checkable group boxes
      if( QGroupBox* groupBox = qobject_cast<QGroupBox*>(widget)) {
            if (groupBox->isCheckable()) {
                  groupBox->setAttribute( Qt::WA_Hover );
                  }
            }
      else if( qobject_cast<QAbstractButton*>(widget) && qobject_cast<QDockWidget*>( widget->parent() ) ) {
            widget->setAttribute(Qt::WA_Hover);
            }
      else if( qobject_cast<QAbstractButton*>(widget) && qobject_cast<QToolBox*>( widget->parent() ) ) {
            widget->setAttribute(Qt::WA_Hover);
            }

      /*
      add extra margins for widgets in toolbars
      this allows to preserve alignment with respect to actions
      */
      if( qobject_cast<QToolBar*>(widget->parent()) ) {
            widget->setContentsMargins(0,0,0,1);
            }

      if (qobject_cast<QToolButton*>(widget)) {
            if ( qobject_cast<QToolBar*>( widget->parent() ) ) {
                  // this hack is needed to have correct text color
                  // rendered in toolbars. This does not really update nicely when changing styles
                  // but is the best I can do for now since setting the palette color at painting
                  // time is not doable
                  QPalette palette( widget->palette() );
                  palette.setColor( QPalette::Disabled, QPalette::ButtonText, palette.color( QPalette::Disabled, QPalette::WindowText ) );
                  palette.setColor( QPalette::Active, QPalette::ButtonText, palette.color( QPalette::Active, QPalette::WindowText ) );
                  palette.setColor( QPalette::Inactive, QPalette::ButtonText, palette.color( QPalette::Inactive, QPalette::WindowText ) );
                  widget->setPalette( palette );
                  }
            widget->setBackgroundRole(QPalette::NoRole);
            }
      else if( qobject_cast<QMenuBar*>(widget)) {
            widget->setBackgroundRole(QPalette::NoRole);
            }
      else if( widget->inherits( "KMultiTabBar" ) ) {
            // kMultiTabBar margins are set to unity for alignment
            // with (usually sunken) neighbor frames
            widget->setContentsMargins( 1, 1, 1, 1 );
            }
      else if( widget->inherits("Q3ToolBar") || qobject_cast<QToolBar*>(widget) ) {
            widget->setBackgroundRole(QPalette::NoRole);
            widget->setAttribute(Qt::WA_TranslucentBackground);
            widget->installEventFilter(this);

#ifdef Q_WS_WIN
            //FramelessWindowHint is needed on windows to make WA_TranslucentBackground work properly
            widget->setWindowFlags(widget->windowFlags() | Qt::FramelessWindowHint);
#endif

            }
      else if( qobject_cast<QTabBar*>(widget) ) {
            widget->installEventFilter( this );
            }
      else if( widget->inherits( "QTipLabel" ) ) {
            widget->setBackgroundRole(QPalette::NoRole);
            widget->setAttribute(Qt::WA_TranslucentBackground);

#ifdef Q_WS_WIN
            //FramelessWindowHint is needed on windows to make WA_TranslucentBackground work properly
            widget->setWindowFlags(widget->windowFlags() | Qt::FramelessWindowHint);
#endif
            }
      else if( qobject_cast<QScrollBar*>(widget) ) {
            widget->setAttribute(Qt::WA_OpaquePaintEvent, false);
            // when painted in konsole, one needs to paint the window background below
            // the scrollarea, otherwise an ugly flat background is used
            if( widget->parent() && widget->parent()->inherits( "Konsole::TerminalDisplay" ) ) {
                  widget->installEventFilter( this );
                  }
            }
      else if( qobject_cast<QDockWidget*>(widget)) {
            widget->setBackgroundRole(QPalette::NoRole);
            widget->setAttribute(Qt::WA_TranslucentBackground);
            widget->setContentsMargins(3,3,3,3);
            widget->installEventFilter(this);
            }
      else if( qobject_cast<QMdiSubWindow*>(widget) ) {
            widget->setAutoFillBackground( false );
            widget->installEventFilter( this );
            }
      else if( qobject_cast<QToolBox*>(widget)) {
            widget->setBackgroundRole(QPalette::NoRole);
            widget->setAutoFillBackground(false);
            widget->setContentsMargins(5,5,5,5);
            widget->installEventFilter(this);
            }
      else if( widget->parentWidget() && widget->parentWidget()->parentWidget() && qobject_cast<QToolBox*>(widget->parentWidget()->parentWidget()->parentWidget())) {
            widget->setBackgroundRole(QPalette::NoRole);
            widget->setAutoFillBackground(false);
            widget->parentWidget()->setAutoFillBackground(false);
            }
      else if( qobject_cast<QMenu*>(widget) ) {
            widget->setAttribute(Qt::WA_TranslucentBackground);
#ifdef Q_WS_WIN
            //FramelessWindowHint is needed on windows to make WA_TranslucentBackground work properly
            widget->setWindowFlags(widget->windowFlags() | Qt::FramelessWindowHint);
#endif
            }
      else if( widget->inherits("QComboBoxPrivateContainer")) {
            widget->installEventFilter(this);
            widget->setAttribute(Qt::WA_TranslucentBackground);
#ifdef Q_WS_WIN
            //FramelessWindowHint is needed on windows to make WA_TranslucentBackground work properly
            widget->setWindowFlags(widget->windowFlags() | Qt::FramelessWindowHint);
#endif
            }
      else if( widget->inherits( "KWin::GeometryTip" ) ) {
            // special handling of kwin geometry tip widget
            widget->installEventFilter(this);
            widget->setAttribute(Qt::WA_NoSystemBackground);
            widget->setAttribute(Qt::WA_TranslucentBackground);
            if( QLabel* label = qobject_cast<QLabel*>( widget ) ) {
                  label->setFrameStyle( QFrame::NoFrame );
                  label->setMargin(5);
                  }

#ifdef Q_WS_WIN
            widget->setWindowFlags(widget->windowFlags() | Qt::FramelessWindowHint);
#endif
            }
      else if( qobject_cast<QFrame*>( widget ) && widget->parent() && widget->parent()->inherits( "KTitleWidget" ) ) {
            widget->setAutoFillBackground( false );
            widget->setBackgroundRole( QPalette::Window );
            }

      // base class polishing
      QCommonStyle::polish(widget);
      }

//---------------------------------------------------------
//   unpolish
//---------------------------------------------------------

void MStyle::unpolish(QWidget* widget)
      {
      // register widget to animations
      animations().unregisterWidget( widget );
      transitions().unregisterWidget( widget );
      windowManager().unregisterWidget( widget );
      frameShadowFactory().unregisterWidget( widget );

      // event filters
      switch (widget->windowFlags() & Qt::WindowType_Mask) {
            case Qt::Window:
            case Qt::Dialog:
                  widget->removeEventFilter(this);
                  widget->setAttribute(Qt::WA_StyledBackground, false);
                  break;
            default:
                  break;
            }

      // checkable group boxes
      if( QGroupBox* groupBox = qobject_cast<QGroupBox*>(widget) ) {
            if( groupBox->isCheckable() ) {
                  groupBox->setAttribute( Qt::WA_Hover, false );
                  }
            }

      // hover flags
      if(
         qobject_cast<QAbstractItemView*>(widget)
         || qobject_cast<QAbstractSpinBox*>(widget)
         || qobject_cast<QCheckBox*>(widget)
         || qobject_cast<QComboBox*>(widget)
         || qobject_cast<QDial*>(widget)
         || qobject_cast<QLineEdit*>(widget)
         || qobject_cast<QPushButton*>(widget)
         || qobject_cast<QRadioButton*>(widget)
         || qobject_cast<QScrollBar*>(widget)
         || qobject_cast<QSlider*>(widget)
         || qobject_cast<QSplitterHandle*>(widget)
         || qobject_cast<QTabBar*>(widget)
         || qobject_cast<QTextEdit*>(widget)
         || qobject_cast<QToolButton*>(widget)
         ) {
            widget->setAttribute(Qt::WA_Hover, false);
            }

      // checkable group boxes
      if( QGroupBox* groupBox = qobject_cast<QGroupBox*>(widget) ) {
            if( groupBox->isCheckable() ) {
                  groupBox->setAttribute( Qt::WA_Hover, false );
                  }
            }

      if( qobject_cast<QMenuBar*>(widget)
         || (widget && widget->inherits("Q3ToolBar"))
         || qobject_cast<QToolBar*>(widget)
         || (widget && qobject_cast<QToolBar *>(widget->parent()))
         || qobject_cast<QToolBox*>(widget)) {
            widget->setBackgroundRole(QPalette::Button);
            widget->removeEventFilter(this);
            widget->clearMask();
            }

      if( qobject_cast<QTabBar*>(widget) ) {
            widget->removeEventFilter( this );
            }
      else if( widget->inherits( "QTipLabel" ) ) {
            widget->setAttribute(Qt::WA_PaintOnScreen, false);
            widget->setAttribute(Qt::WA_NoSystemBackground, false);
            widget->clearMask();
            }
      else if( qobject_cast<QScrollBar*>(widget) ) {
            widget->setAttribute(Qt::WA_OpaquePaintEvent);
            }
      else if( qobject_cast<QDockWidget*>(widget) ) {
            widget->setContentsMargins(0,0,0,0);
            widget->clearMask();
            }
      else if( qobject_cast<QToolBox*>(widget) ) {
            widget->setBackgroundRole(QPalette::Button);
            widget->setContentsMargins(0,0,0,0);
            widget->removeEventFilter(this);
            }
      else if( qobject_cast<QMenu*>(widget) ) {
            widget->setAttribute(Qt::WA_PaintOnScreen, false);
            widget->setAttribute(Qt::WA_NoSystemBackground, false);
            widget->clearMask();
            }
      else if( widget->inherits("QComboBoxPrivateContainer") )
            widget->removeEventFilter(this);
      QCommonStyle::unpolish(widget);
      }

//---------------------------------------------------------
//   polishScrollArea
//---------------------------------------------------------

void MStyle::polishScrollArea( QAbstractScrollArea* scrollArea ) const
      {
      if (!scrollArea)
            return;

      // check frame style and background role
      if( scrollArea->frameShape() != QFrame::NoFrame )
            return;
      if( scrollArea->backgroundRole() != QPalette::Window )
            return;

      // get viewport and check background role
      QWidget* viewport( scrollArea->viewport() );
      if( !( viewport && viewport->backgroundRole() == QPalette::Window ) )
            return;

      // change viewport autoFill background.
      // do the same for children if the background role is QPalette::Window
      viewport->setAutoFillBackground( false );
      QList<QWidget*> children( viewport->findChildren<QWidget*>() );
      foreach( QWidget* child, children ) {
            if( child->parent() == viewport && child->backgroundRole() == QPalette::Window ) {
                  child->setAutoFillBackground( false );
                  }
            }
      }

//---------------------------------------------------------
//   drawPrimitive
//---------------------------------------------------------

void MStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option,
   QPainter* painter, const QWidget* widget) const
      {
      painter->save();
      StylePrimitive fcn(0);

      switch(element) {
            case PE_PanelTipLabel: fcn = &MStyle::drawPanelTipLabelPrimitive; break;
            default:
                  break;
            }

      // try find primitive in map, and run.
      // exit if result is true, otherwise fallback to generic case
      if (!(fcn && (this->*fcn)(option, painter, widget))) {
            QCommonStyle::drawPrimitive( element, option, painter, widget );
            }
      painter->restore();
      }

bool MStyle::drawPanelTipLabelPrimitive( const QStyleOption* option, QPainter* painter, const QWidget* widget) const
      {
      // parent style painting if frames should not be styled
      if (!OxygenStyleConfigData_toolTipDrawStyledFrames)
            return false;

      const QRect& r( option->rect );
      const QColor color( option->palette.brush(QPalette::ToolTipBase).color() );
      QColor topColor( _helper.backgroundTopColor(color) );
      QColor bottomColor( _helper.backgroundBottomColor(color) );

      // make tooltip semi transparents when possible
      // alpha is copied from "kdebase/apps/dolphin/tooltips/filemetadatatooltip.cpp"
      const bool hasAlpha( _helper.hasAlphaChannel( widget ) );
      if ( hasAlpha && OxygenStyleConfigData_toolTipTransparent) {
            topColor.setAlpha(220);
            bottomColor.setAlpha(220);
            }

      QLinearGradient gr( 0, r.top(), 0, r.bottom() );
      gr.setColorAt(0, topColor );
      gr.setColorAt(1, bottomColor );

      // contrast pixmap
      QLinearGradient gr2( 0, r.top(), 0, r.bottom() );
      gr2.setColorAt(0.5, _helper.calcLightColor( bottomColor ) );
      gr2.setColorAt(0.9, bottomColor );

      painter->save();

      if( hasAlpha ) {
            painter->setRenderHint(QPainter::Antialiasing);
            QRectF local( r );
            local.adjust( 0.5, 0.5, -0.5, -0.5 );

            painter->setPen( Qt::NoPen );
            painter->setBrush( gr );
            painter->drawRoundedRect( local, 4, 4 );

            painter->setBrush( Qt::NoBrush );
            painter->setPen(QPen( gr2, 1.1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter->drawRoundedRect( local, 4, 4 );
            }
      else {
            painter->setPen( Qt::NoPen );
            painter->setBrush( gr );
            painter->drawRect( r );

            painter->setBrush( Qt::NoBrush );
            painter->setPen(QPen( gr2, 1.1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter->drawRect( r );
            }
      painter->restore();
      return true;
      }


