import QtQuick 1.1

Item {
      id: container
      y: 20

      signal clicked

      width: 60; height: 60
      Image {
            id: buttonImage
            anchors.fill: parent
            source: "images/playbuttonOff.png"
            }
      MouseArea {
            id: mouseRegion
            anchors.fill: parent
            onClicked:    container.clicked()
            preventStealing: true
            }
      states: [
            State {
                  name: "Pressed"
                  when: mouseRegion.containsMouse == true
                  PropertyChanges {
                        target: buttonImage
                        source: "images/playbuttonOn.png"
                        opacity: 1
                        }
                  }
            ]
      }

