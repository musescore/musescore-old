import QtQuick 1.0

Item {
      id: toolbar

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

      Row {
            anchors.right: parent.right; anchors.rightMargin: 32
            y: 3; height: 32; spacing: 30
            Button {
                  id: button1
                  width: 140; height: 32
                  onClicked: toolbar.button1Clicked()
                  }
            Slider {
                  y: 7;       // 32-3-16/2
                  id: tempo
                  minimum: 0.2
                  maximum: 1.5
                  value: 1.0
                  onValueChanged: toolbar.tempoChanged()
                  }
            RewindButton {
                  id: button3
                  onClicked: toolbar.button3Clicked()
                  }
            PlayButton {
                  id: button2
                  onClicked: toolbar.button2Clicked()
                  }
            }
      }

