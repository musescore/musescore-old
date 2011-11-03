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

import QtQuick 1.1
import MuseScore 1.0
import Qt.labs.gestures 1.0
import "mobile" as Mobile

Item {
      id: screen
      property string state: "ListView"

      Rectangle {
            id: background;
            anchors.fill: parent
            state: "myscores"

            color: "#343434"
            Image {
                  source: "mobile/images/stripes.png"
                  fillMode: Image.Tile
                  anchors.fill: parent
                  opacity: 0.3
                  }

            Timer {
                  id: plainTimer
                  interval: 3000
                  repeat: false
                  onTriggered: {
                        if (screen.state == "ScoreView")
                              screen.state = "PlainScoreView";
                        }
                  }

            states: [
                  State {
                        name: "ListView"
                        when: screen.state == "ListView"
                        PropertyChanges {
                              target: scoreView
                              x: -width;
                              }
                        PropertyChanges {
                              target: scoreListView
                              x: 0
                              }
                        PropertyChanges {
                              target: toolBar
                              visible: false
                              }
                        StateChangeScript {
                              name: plainTimer.stop()
                              }
                        },
                  State {
                        name: "ScoreView"
                        when: screen.state == "ScoreView"
                        PropertyChanges {
                              target: scoreView
                              x: 0
                              }
                        PropertyChanges {
                              target: scoreListView
                              x: -width;
                              }
                        PropertyChanges {
                              target: toolBar
                              visible: true
                              }
                        StateChangeScript {
                              name: plainTimer.start()
                              }
                        },
                  State {
                        name: "PlainScoreView"
                        when: screen.state == "PlainScoreView"
                        PropertyChanges {
                              target: scoreView
                              x: 0
                              }
                        PropertyChanges {
                              target: scoreListView
                              x: -width;
                              }
                        PropertyChanges {
                              target: toolBar
                              visible: false
                              }
                        PropertyChanges {
                              target: titleBar
                              visible: false
                              }
                        }
                  ]
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
                              console.log("list clicked");
                              if (idx >= 0) {
                                    console.log(scorelist.get(idx).path);
                                    scoreView.setScore(scorelist.get(idx).path)
                                    screen.state = "ScoreView"
                                    }
                              }
                        }
                  }

            ScoreView {
                  id: scoreView
                  parentWidth: screen.width
                  parentHeight: screen.height
                  x: -width;
                  MouseArea {
                        anchors.fill: parent
                        anchors.bottomMargin: 60
                        onClicked: {
                              if (mouseX > parent.width * .8)
                                    scoreView.nextPage();
                              else if (mouseX < parent.width * .2)
                                    scoreView.prevPage();
                              else {
                                    if (screen.state == "ScoreView") {
                                          screen.state = "PlainScoreView"
                                          plainTimer.stop()
                                          }
                                    else {
                                          screen.state = "ScoreView"
                                          plainTimer.start()
                                          }
                                    }
                              }
                        onPressAndHold: {
                              scoreView.seek(mouseX, mouseY)
                              }
                        }
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
                  height: 60;
                  width: parent.width;
                  opacity: .9
                  anchors.bottom: parent.bottom

                  button1Label: "MyScores"
                  onButton1Clicked: {
                        if (screen.state == "ScoreView")
                              screen.state = "ListView"
                        else
                              screen.state = "ScoreView"
                        }
                  onButton3Clicked: {
                        scoreView.rewind()
                        }
                  onTempoChanged: {
                        scoreView.setTempo(tempo)
                        plainTimer.restart()
                        }
                  onButton2Clicked: scoreView.play();
                  }
            }
      }

