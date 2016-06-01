#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QThread>
#include <QtQml>
#include <QSurfaceFormat>

#include <memory>

#include "constants.h"
#include "audioengine.h"
#include "fftwrapper.h"
#include "visualizationdata.h"


int main(int argc, char *argv[])
{
    // Disable vsync

    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setSwapInterval(0);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    format.setOptions(QSurfaceFormat::StereoBuffers);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setVersion(2, 9);
    QSurfaceFormat::setDefaultFormat(format);

    QGuiApplication app(argc, argv);

    // Set up communication through event queue

    Visualizer::Common::sample_queue_t bufferQueue(0);
    Visualizer::Common::fft_result_queue_t fftResultQueue(0);

    // Start audio thread

    QThread audioThread;
    AudioEngine audioEngine(bufferQueue);
    audioEngine.moveToThread(&audioThread);
    QObject::connect(&audioThread, &QThread::started, &audioEngine, &AudioEngine::startToneGenerator);
//    QObject::connect(&audioThread, &QThread::started, &audioEngine, &AudioEngine::startPlayback);
    audioThread.start();

    // Start FFT Thread

    QThread fftThread;
    FFTWrapper fft(bufferQueue, fftResultQueue);
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


    QQmlVariantListModel model;
    VisualizationData visualization(fftResultQueue, model);

    qmlRegisterType<QQmlVariantListModel>("visualizer.models", 1, 0, "QQmlVariantListModel");

    QQmlApplicationEngine engine;
    engine.addImportPath(":/qml/");
    engine.rootContext()->setContextProperty(QStringLiteral("dataModel"), &model);
    engine.load(QUrl(QStringLiteral("qrc:/qml/main2.qml")));

    int result = app.exec();

    audioThread.wait(5000);
    fftThread.wait(5000);
    return result;
}
