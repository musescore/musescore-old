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

      states: [
            State {
                  name: "normal"
                  PropertyChanges { target: toolbar; opacity: 0 }
                  PropertyChanges { target: scores; width: 0 }
                  },
            State {
                  name: "toolbar1"
                  PropertyChanges { target: toolbar; opacity: 1 }
                  PropertyChanges { target: scores;  width: 0 }
                  },
            State {
                  name: "myscores"
                  PropertyChanges { target: toolbar; opacity: 1 }
                  PropertyChanges { target: scores;  width: 250 }
                  }
            ]
      transitions: Transition {
            PropertyAnimation {
                  property: "width"
                  easing.type: Easing.Linear
                  duration: 500
                  }
            PropertyAnimation {
                  property: "opacity"
                  easing.type: Easing.Linear
                  duration: 500
                  }
            }

      ListModel {
            id: scorelist
            ListElement {
                  type: "Promenade"
                  path: ":/scores/promenade.mscz"
                  }
            ListElement {
                  type: "Leise rieselt der Schnee"
                  path: ":/scores/schnee.mscz"
                  }
            ListElement {
                  type: "Italienisches Konzert"
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
            id: scores
            anchors.left: screen.left
            anchors.top: screen.top
            anchors.bottom: screen.bottom
            anchors.margins: 10
            clip: true

            model: scorelist
            delegate: scorelistdelegate
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
                        var idx = scores.indexAt(mouseX, mouseY)
                        if (idx >= 0) {
                              scores.currentIndex = idx
                              console.log(idx)
                              console.log(scorelist.get(idx).type)
                              scoreview.setScore(scorelist.get(idx).path)
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

      Rectangle {
            id: view
            anchors.left:  scores.right
            anchors.right: screen.right
            anchors.top:   screen.top
            anchors.bottom: screen.bottom

            ScoreView {
                  id: scoreview
                  x: 0
                  y: 0

                  MouseArea {
                        state: "normal"
                        states: [
                              State { name: "normal" },
                              State { name: "pressed" },
                              State { name: "drag" }
                              ]
                        anchors.fill: parent
                        drag.target: scoreview
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
            }

      Mobile.ToolBar {
            id: toolBar
            height: 40; anchors.bottom: parent.bottom;
            width: parent.width; opacity: 0.9
            button1Label: "MyScores"; button2Label: "Play"
            onButton1Clicked: {
                  if (screen.state == "toolbar1")
                        screen.state = "myscores"
                  else
                        screen.state = "toolbar1"
                        }
            onButton2Clicked: scoreview.play();
            }
      }

