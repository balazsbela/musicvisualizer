import QtQuick 2.3
import QtQuick.Window 2.2


Window
{
    visible: true
    id: mainwindow
    width:1024
    height:768
    color: "#000000"
    visibility: "FullScreen"

    Item
    {
        anchors.fill: parent
        focus: true

        Keys.onRightPressed:
        {
            loader.source = "videosynth.qml";
        }

        Keys.onLeftPressed:
        {
            loader.source = "spectogram.qml";
        }

        Loader
        {
            id: loader
            source: "spectogram.qml"
            anchors.fill: parent
        }
    }

}
