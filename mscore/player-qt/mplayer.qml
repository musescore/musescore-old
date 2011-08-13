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
import MuseScore 1.0
import "mobile" as Mobile

Item {
      id: screen
      width: 1024; height: 768
      state: "myscores"
      property bool inScoreView: false;

      Rectangle {
            id: background;
            anchors.fill: parent;

            Image {
                  source: "mobile/images/paper5.png";
                  fillMode: Image.Tile;
                  anchors.fill: parent;
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
                        x: -(parent.width * 1.5)
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
                        path: ":/scores/promenade.mscz"
                        }
                  ListElement {
                        title: "Leise rieselt der Schnee"
                        path: ":/scores/schnee.mscz"
                        }
                  ListElement {
                        title: "Italienisches Konzert"
                        path: ":/scores/italian-1.mscz"
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
                  width: parent.width;
                  height: parent.height;

                  clip: true
                  model: scorelist

                  delegate: Mobile.ListDelegate { }
                  header: myScoresBanner

                  footer: Rectangle {
                        width: parent.width; height: 30
                        gradient: scorecolors
                        }
                  highlight: Rectangle {
                        width: parent.width
                        color: "lightgray"
                        }
                  MouseArea {
                        anchors.fill: parent
                        onClicked: {
                              var idx = scoreListView.indexAt(mouseX, mouseY)
                              if (idx >= 0) {
                                    scoreListView.currentIndex = idx
                                    console.log(idx)
                                    console.log(scorelist.get(idx).type)
                                    scoreView.setScore(scorelist.get(idx).path)
                                    screen.inScoreView = true
                                    }
                              }
                        }
                  }
            Component {
                  id: myScoresBanner
                  Rectangle {
                        id: banner
                        width: parent.width; height: 50
                        gradient: scorecolors
                        border { color: "#9eddf2"; width: 2 }
                        Text {
                              anchors.centerIn: parent
                              text: "My Scores"
                              font.pixelSize: 32
                              }
                        }
                  }
            Gradient {
                  id: scorecolors
                  GradientStop { position: 0.0; color:  "#8ee2fe" }
                  GradientStop { position: 0.66; color: "#7ed2ee" }
                  }

            ScoreView {
                  id: scoreView
                  width: parent.width;
                  height: parent.height;
                  x: -(parent.width * 1.5);

/*                  Image {
                        source: "mobile/images/paper5.png";
                        fillMode: Image.Tile;
                        anchors.fill: parent;
                        }
  */
                  MouseArea {
                        state: "normal"
                        states: [
                              State { name: "normal" },
                              State { name: "pressed" },
                              State { name: "drag" }
                              ]
                        anchors.fill: parent
                        drag.target: scoreView
                        drag.axis: Drag.XandYAxis
                        drag.minimumX: 0
                        drag.maximumX: 1000
                        drag.minimumY: 0
                        drag.maximumY: 1000

                        onPositionChanged: {
                              // parent.drag(mouseX, mouseY)
                              state = "drag"
                              }
                        onPressed:         {
                              state = "pressed"
                             // parent.startDrag(mouseX, mouseY)
                              }
                        onReleased: {
                              if (state == "pressed"
                                 && (mouseX > width * .3)
                                 && (mouseX < width * .6)) {
                                    if (screen.state == "normal")
                                          screen.state = "toolbar1"
                                    else if (player.state == "toolbar1")
                                          screen.state = "normal"
                                    }
                              // state = "normal";
                              }
                        onClicked: {
                              if (mouseX < width * .3)
                                    parent.prevPage()
                              else if (mouseX > width * .6)
                                    parent.nextPage()
                              }
                        }
                  }

            Mobile.ToolBar {
                  id: toolBar
                  height: 40; anchors.bottom: parent.bottom;
                  width: parent.width; opacity: 0.9
                  button1Label: "MyScores"; button2Label: "Play"
                  onButton1Clicked: {
                        if (screen.inScoreView == true)
                              screen.inScoreView = false;
                        else
                              screen.inScoreView = true;
                        }
                  onButton2Clicked: scoreView.play();
                  }
            }
      }

