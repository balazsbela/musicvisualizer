import QtQuick 2.3
import QtQuick.Window 2.2

Rectangle
{
    id:root
    color: mainwindow.color
    anchors.fill: parent

    property variant currentBuffer : []
    property int bufferSize: 0

    Connections
    {
        target: sampleData
        onModelReset:
        {
          bufferSize = sampleData.count;
          for (var i = 0; i < sampleData.count; ++i)
          {
            var item = sampleData.get(i);
            currentBuffer[i] = item.val;

          }
        }
    }


    ShaderEffect
    {
        id: shader
        z: 0
        property variant prevSource: shaderOutput
        property variant feedbackGain:    0.3
        property real freq : 0.5
        property real time


        property real currentValue
        property int bufferIndex

        onBufferIndexChanged:
        {
            var value = root.currentBuffer[bufferIndex];
            currentValue = value ? value : 0;
        }

        NumberAnimation on bufferIndex
        {
            id: counterAnimation
            running:false
            loops: 1
            from: 0
            to: bufferSize
            duration: 16
        }

        ParallelAnimation
        {
            running: true
            loops: Animation.Infinite
            UniformAnimator
            {
                uniform: "time"
                target: shader
                loops: Animation.Infinite
                from: 0.0
                to: 100.0
                duration: 100000
            }
        }

        ParallelAnimation
        {
            running: true
            loops: Animation.Infinite

            UniformAnimator {
                uniform: "freq"
                target: shader
                from: 0.05
                to: 0.99
                duration: 100000
                easing.type: Easing.Linear
            }

            UniformAnimator {
                uniform: "freq"
                target: shader
                from: 0.99
                to: 0.05
                duration: 100000
                easing.type: Easing.Linear
            }
        }

        anchors.fill: parent
        visible:true
        blending:true
        smooth: true

        property real frameWidth:  parent.width
        property real frameHeight: parent.height
        antialiasing: true
        fragmentShader: "
                          #define lines 1000.0

                          varying highp vec2 qt_TexCoord0;
                          uniform highp float time;
                          uniform highp float freq;
                          uniform sampler2D prevSource;
                          uniform lowp  float qt_Opacity;
                          uniform highp float feedbackGain;
                          uniform highp float frameWidth;
                          uniform highp float frameHeight;
                          uniform highp float currentValue;

                          void main()
                          {
                              highp float modulationValue = currentValue + 0.5;
                              highp vec2 uv = qt_TexCoord0;
                              highp vec4 feedback = texture2D(prevSource, 0.999 * uv - vec2(0.0,0.004));
                              highp float timebase = (floor(lines * uv.y) + uv.x)/lines;
                              highp float osc1 = 0.5 + 0.5 * sin(6.283185 * (0.1 * time + 1.0 * timebase + modulationValue * 1.0 * length(feedback.rgb)));
                              highp float osc2 = 0.5 + 0.5 * sin(6.283185 * (0.01 * time + 4.0 * lines * timebase + 0.9 * modulationValue * osc1));
                              highp float osc3 = 0.5 + 0.5 * sin(6.283185 * (0.01 * time + 2.999 * lines * timebase + 0.9 * modulationValue * osc2));
                              gl_FragColor = mix(vec4(modulationValue * osc3 , modulationValue * osc1, modulationValue * osc2, 0.9), feedback, 0.54);
                          }"
    }

    ShaderEffectSource
    {
        id: shaderOutput
        hideSource: false
        anchors.fill: parent
        sourceItem: shader
        recursive: true
        visible: false
        live: false
    }

    Timer
    {
        interval: 16
        running: true
        repeat: true
        onTriggered:
        {
            counterAnimation.start()
            shaderOutput.scheduleUpdate()
        }

    }

}

