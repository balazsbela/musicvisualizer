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

    Rectangle
    {
        id:root
        color: mainwindow.color
        width: parent.width
        height: parent.height / 2

        property bool pending: false

        ListModel
        {
            id: dataModel
        }

        Timer
        {
            running: true
            interval: 15
            repeat: true
            onTriggered:
            {
                canvas.requestPaint();
            }
        }

        Canvas
        {
            id: canvas
            anchors.fill: parent
            renderTarget: Canvas.FramebufferObject
            renderStrategy: Canvas.Cooperative
            antialiasing: true
            onPaint:
            {
                if (!root.pending)
                {
                    return;
                }

                var ctx = canvas.getContext("2d");
                ctx.clearRect(0, 0, width, height);
                ctx.lineWidth = 3;
                ctx.globalAlpha = 1.0;

                ctx.save();

                for (var i = 0; i < dataModel.count; ++i)
                {
                    var item = dataModel.get(i);

                    ctx.strokeStyle = Qt.hsla(i / dataModel.count, 0.85, 0.45, 1.0);
                    ctx.fillStyle = ctx.strokeStyle;

                    var x = i * 5;
                    var y = parent.height;

                    ctx.beginPath();
                    ctx.moveTo(x, y);
                    ctx.lineTo(x, parent.height - 300 * item.val);
                    ctx.stroke();
                }

                ctx.restore();

                root.pending = false;

            }

        }

        ShaderEffect
        {
            id: shaderItem
            property variant source: ShaderEffectSource { sourceItem: canvas; smooth: true; hideSource: true}
            property variant recursiveSource: recursiveSource
            anchors.fill: parent
            visible: true

            property real interpolationFactor;

            ParallelAnimation {
                running: true
                loops: Animation.Infinite

                UniformAnimator {
                    uniform: "interpolationFactor"
                    target: shaderItem
                    from: 0.8
                    to: 0.3
                    duration: 10
                    easing.type: Easing.Linear
                }

                UniformAnimator {
                    uniform: "interpolationFactor"
                    target: shaderItem
                    from: 0.8
                    to: 0.2
                    duration: 10
                    easing.type: Easing.Linear
                }
            }

            fragmentShader: "
                varying mediump vec2 qt_TexCoord0;
                uniform highp float qt_Opacity;
                uniform lowp sampler2D source;
                uniform lowp sampler2D recursiveSource;
                uniform lowp float interpolationFactor;
                void main() {
                    vec4 current = texture2D(source, qt_TexCoord0);
                    vec4 previous = texture2D(recursiveSource, qt_TexCoord0);
                    gl_FragColor = mix(previous, current, interpolationFactor) * qt_Opacity * 1.2;
                }"
        }

        ShaderEffectSource
        {
            id: recursiveSource
            sourceItem: shaderItem
            visible: false
            live: true
            smooth: true
            recursive: true
        }


        Connections
        {
            target: visualizationData
            onDataAvailable :
            {
                if (root.pending)
                {
                    return;
                }

                for (var i = 0; i < resultList.length; ++i)
                {
                    dataModel.set(i, {"val" : resultList[i]});
                }

                root.pending = true;
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
