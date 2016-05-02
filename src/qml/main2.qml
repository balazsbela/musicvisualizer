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
    width:1024
    height:768
    color: "#000000"

    Rectangle
    {
        id:root
        color: mainwindow.color
        width: parent.width
        height: parent.height / 2

        ListModel
        {
            id: dataModel
        }

        Connections
        {
            target: visualizationData
            onDataAvailable :
            {
                for (var i=0; i < resultList.length; ++i)
                {
                    dataModel.set(i, {"i": i, "val" : resultList[i] * 300,
                                      "color": Qt.hsla(resultList[i],
                                                       0.85,
                                                       0.45,
                                                       1.0).toString()});
                }
            }
        }


        ListView
        {
            id: dataView
            model:dataModel
            anchors.fill: parent
            width: parent.width
            height: parent.height
            clip:true
            delegate : Item
            {

                height: parent.height
                width: 3

                Rectangle
                {
                    id: rect
                    color: model.color
                    height: model.val

                    PropertyAnimation
                    {
                        target: rect
                        property: "height"
                        from: rect.height
                        to: model.val
                        duration: 100
                    }

                    width: 3
                    y : parent.height - model.val
                }

            }
            orientation: Qt.Horizontal
            focus:true
            spacing: 1
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
