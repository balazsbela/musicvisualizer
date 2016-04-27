#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QThread>

#include <memory>

#include "constants.h"
#include "audioengine.h"


const unsigned bufferSize = 5 * 512;
const unsigned queueSize = 100;

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    engine.addImportPath(":/qml/");
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    // Set up communication through event queue

    Visualizer::Constants::queue_t bufferQueue(100);


    // Start audio thread

    QThread audioThread;
    AudioEngine audioEngine(bufferQueue);
    audioEngine.moveToThread(&audioThread);
    QObject::connect(&audioThread, &QThread::started, &audioEngine, &AudioEngine::startPlayback);
    audioThread.start();


    // Start FFT Thread

    QThread fftThread;
    FFTWrapper fft(bufferQueue);
    fft.moveToThread(&fftThread);
    QObject::connect(&fftThread, &QThread::started, &fft, &FFTWrapper::start);
    fftThread.start();


    // Ensure clean exit

    QObject::connect(&app, &QGuiApplication::aboutToQuit, &audioEngine, &AudioEngine::stop);
    QObject::connect(&app, &QGuiApplication::aboutToQuit, &fft, &FFTWrapper::stop);

    QObject::connect(&audioEngine, &AudioEngine::stopped, [&]()
    {
        audioThread.exit(0);
    });

    QObject::connect(&fft, &FFTWrapper::stopped, [&]()
    {
        fftThread.exit(0);
    });


    int result = app.exec();

    audioThread.wait(5000);
    fftThread.wait(5000);
    return result;
}
