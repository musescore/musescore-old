import QtQuick 1.0

Item {
      id: container

      signal clicked

      width: 32; height: 32
      Image {
            id: buttonImage
            source: "images/playbuttonOff.png"
            }
      MouseArea {
            id: mouseRegion
            anchors.fill: parent
            onClicked:    container.clicked()
            }
      states: [
            State {
                  name: "Pressed"
                  when: mouseRegion.pressed === true
                  PropertyChanges {
                        target: buttonImage
                        opacity: 1
                        }
                  PropertyChanges {
                        target: buttonImage
                        source: "images/playbuttonOn.png"
                        }
                  }
            ]
      }

