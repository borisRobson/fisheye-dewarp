#include <QtGui>
#include <QtGui/QApplication>
#include <QCoreApplication>
#include <QApplication>
#include "stream.h"
#include "mainwindow.h"

stream *strm;

int main(int argc, char *argv[])
{
    //QCoreApplication app(argc, argv);
    QApplication app(argc,argv);

    strm = new stream();

    Task* task = new Task();

    MainWindow window;

    window.show();

    QObject::connect(task, SIGNAL(finished()),&app, SLOT(quit()));

    gst_init(&argc, &argv);

    QTimer::singleShot(10, task, SLOT(run()));

    return app.exec();
}


void Task::run()
{
    bool pipelinebuilt = false;

    pipelinebuilt = strm->buildpipeline();

    if(pipelinebuilt){
        strm->startstream();
    }

    QCoreApplication::instance()->quit();

    return;
}
