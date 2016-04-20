import QtQuick 2.3
import QtQuick.Window 2.2


// Currently heavily based on this:
// http://quitcoding.com/download/QUIt_GForceMeter_1.0.tgz
// That may change

// Also currently not connected to the frequency data

Window
{
    visible: true
    id: mainwindow
    width:640
    height:480

    color: "#000000"

    Rectangle
    {
        id:root
        color: mainwindow.color

        width:640
        height:240

        property alias ledWidth: ledScreen.ledWidth
        property alias ledHeight: ledScreen.ledHeight

        property real value: 0.5
        property real _peakValue: root.value

        property int  _updateFrequencyCounter: 0
        property int  updateFrequency: 2

        SequentialAnimation on value {
            running: true
            loops: Animation.Infinite
            NumberAnimation { from: 0.5; to: 1; duration: 300; easing.type: Easing.InOutQuad }
            NumberAnimation { from: 1.0; to: 0.5; duration: 1000; easing.type: Easing.OutElastic }
            NumberAnimation { from: 0.5; to: 0.2; duration: 2000; easing.type: Easing.InOutElastic }
        }

        onValueChanged: {
            root.value = Math.min(1.0, root.value)
            root._peakValue = Math.max(root._peakValue, root.value);
        }

        // This area is the source, one pixel is one led
        Rectangle
        {
            id: sourceArea
            width: Math.floor(root.width / ledScreen.ledWidth)
            height: Math.floor(root.height / ledScreen.ledHeight)
            color:"#000000"
            visible: false

            Image
            {
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                width: 1
                height: parent.height * root.value
                source: "images/eqline.png"
                smooth: true
            }

            Rectangle
            {
                anchors.right: parent.right
                color: "#ffffff"
                width: 1
                height: 1
                y: Math.floor(parent.height * (1 - root._peakValue))
            }

        }

        ShaderEffectSource
        {
            id: recursiveSource
            sourceItem: inputItem
            hideSource: true
            visible: false
            live: root.updateFrequency == 1
            smooth: true
            recursive: true
        }

        LedScreen
        {
            id: ledScreen
            anchors.fill: parent
            sourceItem: inputItem
            useSourceColors: true
            ledColor: Qt.rgba(0.4, 0.4, 0.4, 0.2)
            threshold: 0.01
            ledWidth: 24
            ledHeight: 12
        }

        ShaderEffect
        {
            id: inputItem
            property variant source: ShaderEffectSource { sourceItem: sourceArea; smooth: true }
            property variant recursiveSource: recursiveSource
            property real ledWidthPercent: ledScreen.ledWidth / ledScreen.width
            anchors.fill: sourceArea
            visible: false

            fragmentShader: "
                varying mediump vec2 qt_TexCoord0;
                uniform highp float qt_Opacity;
                uniform highp float ledWidthPercent;
                uniform lowp sampler2D source;
                uniform lowp sampler2D recursiveSource;
                void main() {
                    highp vec4 pix;
                    if (qt_TexCoord0.x > (1.0 - ledWidthPercent)) {
                        pix = texture2D(source, qt_TexCoord0);
                    } else {
                        highp vec2 pos = vec2(qt_TexCoord0.x+ledWidthPercent, qt_TexCoord0.y);
                        pix = texture2D(recursiveSource, pos) * (1.0-ledWidthPercent*2.0);
                    }
                    gl_FragColor = pix * qt_Opacity;
                }
            "
        }

        // Dummy element for syncing updates
        Item {
            id: dummyItem
            NumberAnimation on rotation {
                from:0
                to: 360
                duration: 1000
                loops: Animation.Infinite
            }
            onRotationChanged: {
                root._updateFrequencyCounter++;
                if (root._updateFrequencyCounter == 1) {
                    // Reduce peak in first iteratio after repaint
                    var reduced_peakValue = root._peakValue - (root._peakValue - root.value)/2;
                    root._peakValue = Math.max(root.value, reduced_peakValue);
                }
                if (root._updateFrequencyCounter >= root.updateFrequency) {
                    recursiveSource.scheduleUpdate();
                    root._updateFrequencyCounter = 0;
                }
            }
        }
    }


    // Add mirror effect

    ShaderEffect {
        property variant source: ShaderEffectSource { sourceItem: root; hideSource: false }

        anchors.top: root.bottom
        width: root.width
        height: root.height

        fragmentShader: "
            varying highp vec2 qt_TexCoord0;
            uniform highp float qt_Opacity;
            uniform sampler2D source;
            void main(void) {
                highp vec2 pos = vec2(qt_TexCoord0.x, (1.0 - qt_TexCoord0.y*0.8));
                pos.x += (qt_TexCoord0.y*0.2) * (pos.x * -1.0 + 1.0);
                highp vec4 pix = texture2D(source, pos);
                pix *= (0.4 - qt_TexCoord0.y*0.5) * min(qt_TexCoord0.y*5.0, 1.0);
                gl_FragColor = pix * qt_Opacity;
            }"
    }
}
