import QtQuick 2.3
import QtQuick.Window 2.2

/* VideoSynth Synth
 *
 * Use the audio data to modulate the parameters of a video synth.
 *
 * Based on Julian Parker's Videosynth shader.
 * julian dot parker at cantab dot net
 * 2016
 *
 */


Rectangle
{
    id:root
    color: mainwindow.color
    anchors.fill: parent

    property variant currentBuffer : []
    property variant newBuffer : []
    property int bufferSize: 0

    Connections
    {
        target: sampleData.getModel()
        onModelReset:
        {
          bufferSize = sampleData.getModel().count;
          for (var i = 0; i < bufferSize; ++i)
          {
            var item = sampleData.getModel().get(i);
            newBuffer[i] = item.val;
          }

          image.reload();
        }
    }

    Image
    {
        id: image;
        source: "image://buffer/"
        cache: false
        function reload()
        {
            var oldSource = source;
            source = "";
            source = oldSource;
        }
    }

    ShaderEffectSource
    {
        id: audioTexture
        hideSource: true
        sourceItem: image
    }


    ShaderEffect
    {
        id: shader
        z: 0
        property variant prevSource: shaderOutput
        property variant feedbackGain:    0.7
        property real freq : 0.1
        property real time : 0

        property variant audioDataImage: audioTexture

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

        anchors.fill: parent
        visible:true
        blending:true
        smooth: true

        property real frameWidth:  parent.width
        property real frameHeight: parent.height
        antialiasing: true
        fragmentShader: "
                          #define lines 500.0

                          varying highp vec2 qt_TexCoord0;
                          uniform highp float time;
                          uniform highp float freq;
                          uniform sampler2D prevSource;
                          uniform lowp  float qt_Opacity;
                          uniform highp float feedbackGain;
                          uniform highp float frameWidth;
                          uniform highp float frameHeight;
                          uniform highp float currentValue;
                          uniform sampler2D audioDataImage;

                          highp float DecodeFloatRGBA( vec4 val )
                          {
                            // Same as the bit shifting
                            return dot( val, vec4(1.0, 1.0 / 255.0, 1.0 / 65025.0, 1.0 / 160581375.0) );
                          }

                          void main()
                          {
                              highp vec2 uv = qt_TexCoord0;

                              highp float timebase = (floor(lines * uv.y) + uv.x)/lines;

                              highp float colorVal = DecodeFloatRGBA(texture2D(audioDataImage, vec2(timebase, 0.0)).abgr);
                              highp float sampleValue = (colorVal * 2.0) - 1.0;
                              highp float absSample = abs(sampleValue);
                              highp vec4 feedback = texture2D(prevSource, 0.999 * uv - vec2(0.0,0.004));
                              highp float osc1 = 0.5 + 0.5 * sin(6.283185 * (0.02 * time + 1.0 * timebase + currentValue * 1.0 * length(feedback.rgb)));
                              highp float osc2 = 0.5 + 0.5 * sin(6.283185 * (0.01 * time + 2.0 * lines * timebase + 0.9 * osc1));
                              highp float osc3 = 0.5 + 0.5 * sin(6.283185 * (0.1 * time + 2.999 * lines * timebase + 0.9 * osc2));
                              gl_FragColor = mix(1.75 * vec4(colorVal * osc3, colorVal * osc2, colorVal * osc1, 0.9), feedback, feedbackGain);

//                              Inspect image and values for debugging
//                              gl_FragColor = vec4(1.0, 0.0, colorVal, 1.0);
//                              gl_FragColor = texture2D(audioDataImage, vec2(timebase, 0.0)).abgr;
//
//                            // Soundwave, for debugging
//                            float wave = texture2D(audioDataImage, vec2(uv.x, 0)).w;
//                            vec3 col = vec3(0, 0, 0);
//                            col += 1.0 -  smoothstep( 0.0, 0.15, abs(wave - uv.y) );
//                            gl_FragColor = qt_Opacity * vec4(col, 1.0);
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
            currentBuffer = newBuffer;
            counterAnimation.start()
            shaderOutput.scheduleUpdate()
        }
    }

}

