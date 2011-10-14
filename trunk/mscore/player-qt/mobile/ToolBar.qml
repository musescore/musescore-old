import QtQuick 1.0

Item {
      id: toolbar
      height: 40

      property alias button1Label: button1.text

      property alias button1Visible: button1.visible
      property alias button2Visible: button2.visible
      property alias button3Visible: button3.visible
      property alias tempoButtonVisible: tempo.visible
      property alias tempo: tempo.value

      signal button1Clicked
      signal button2Clicked
      signal button3Clicked
      signal tempoChanged

      BorderImage {
            source: "images/titlebar.sci"
            width: parent.width; height: parent.height + 14; y: -7
            }

      Button {
            id: button1
            width: 140
            height: 32
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            onClicked: toolbar.button1Clicked()
            }
      Slider {
            id: tempo
            y: 7;       // 32-3-16/2
            anchors.left:  button1.right
            anchors.right: button3.left
            anchors.verticalCenter: parent.verticalCenter
            minimum: 0.2
            maximum: 1.5
            value: 1.0
            onValueChanged: toolbar.tempoChanged()
            }
      RewindButton {
            id: button3
            anchors.right: button2.left
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            onClicked: toolbar.button3Clicked()
            }
      PlayButton {
            id: button2
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            onClicked: toolbar.button2Clicked()
            }
      }

