//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

import QtQuick 1.0
import Qt.labs.gestures 1.0
import MuseScore 1.0
import "mobile" as Mobile

Item {
      id: screen
      property bool inScoreView: false
      width: 1024
      height: 768

      states: [
            State {
                  name: "landscape"
                  when: (runtime.orientation == Orientation.Landscape)
                  PropertyChanges {
                        target: screen
                        width: 1024
                        height: 768
                        }
                  PropertyChanges {
                        target: background
                        color: "blue"
                        }
                  },
            State {
                  name: "landscapeInverted"
                  when: (runtime.orientation == Orientation.LandscapeInverted)
                  PropertyChanges {
                        target: screen
                        width: 1024
                        height: 768
                        }
                  PropertyChanges {
                        target: background
                        color: "red"
                        }
                  },
            State {
                  name: "portrait"
                  when: (runtime.orientation == Orientation.Portrait)
                  PropertyChanges {
                        target: screen
                        width:  768
                        height: 1024
                        }
                  PropertyChanges {
                        target: background
                        color: "green"
                        }
                  },
            State {
                  name: "portraitInverted"
                  when: (runtime.orientation == Orientation.PortraitInverted)
                  PropertyChanges {
                        target: screen
                        width:  768
                        height: 1024
                        }
                  PropertyChanges {
                        target: background
                        color: "yellow"
                        }
                  }
            ]
      Rectangle {
            id: background;
            anchors.fill: parent
            state: "myscores"

//            color: "#343434"
            Image {
                  source: "mobile/images/stripes.png"
                  fillMode: Image.Tile
                  anchors.fill: parent
                  opacity: 0.3
                  }

            states: State {
                  name: "ScoreView"
                  when: screen.inScoreView == true
                  PropertyChanges {
                        target: scoreView
                        x: 0
                        }
                  PropertyChanges {
                        target: scoreListView
//                        x: -(parent.width * 1)
                        x: -width;
                        }
                  PropertyChanges {
                        target: toolBar
                        button1Visible: true
                        button2Visible: true
                        button3Visible: true
                        tempoButtonVisible: true
                        }
                  }
            transitions: Transition {
                  NumberAnimation {
                        properties: "x"
                        easing.type: Easing.InOutQuad
                        duration: 500
                        }
                  }

            ListModel {
                  id: scorelist
                  ListElement {
                        title: "Promenade"
                        author: "Modeste Moussorgsky"
                        path: ":/scores/promenade.mscz"
                        imagePath: "qrc:///scores/promenade.png"
                        }
                  ListElement {
                        title: "Leise rieselt der Schnee"
                        author: "Traditional"
                        path: ":/scores/schnee.mscz"
                        imagePath: "qrc:///scores/schnee.png"
                        }
                  ListElement {
                        title: "Italienisches Konzert"
                        author: "J.S. Bach"
                        path: ":/scores/italian-1.mscz"
                        imagePath: "qrc:///scores/italian-1.png"
                        }
                  }

            Component {
                  id: scorelistdelegate
                  Text {
                        id: label
                        font.pixelSize: 18
                        text: type
                        }
                  }

            ListView {
                  id: scoreListView
                  anchors.top: titleBar.bottom
                  anchors.bottom: toolBar.top

                  width: parent.width
                  height: parent.height
                  model: scorelist
                  delegate: Mobile.ListDelegate { }

                  MouseArea {
                        anchors.fill: parent
                        onClicked: {
                              var idx = scoreListView.indexAt(mouseX, mouseY)
                              if (idx >= 0) {
                                    scoreView.setScore(scorelist.get(idx).path)
                                    screen.inScoreView = true
                                    scoreView.contentY = 0
                                    }
                              }
                        }
                  }

            ScoreView {
                  id: scoreView
                  parentWidth: screen.width
                  parentHeight: screen.height
                  }

            Mobile.TitleBar {
                  id: titleBar
                  height: 40
                  width: parent.width
                  opacity: .9
                  anchors.top: parent.top
                  }

            Mobile.ToolBar {
                  id: toolBar
                  height: 40;
                  width: parent.width;
                  opacity: .9
                  anchors.bottom: parent.bottom

                  button1Label: "MyScores"
                  onButton1Clicked: {
                        if (screen.inScoreView == true)
                              screen.inScoreView = false
                        else
                              screen.inScoreView = true
                        }
                  onButton3Clicked: {
                        scoreView.rewind()
                        }
                  onTempoChanged: {
                        scoreView.setTempo(tempo)
                        }
                  onButton2Clicked: scoreView.play();
                  button1Visible: false
                  button2Visible: false
                  button3Visible: false
                  tempoButtonVisible: false
                  }
            }
      }

