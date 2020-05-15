#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow w;
    QFile dark_style{":qdarkstyle/style.qss"};
    if(dark_style.exists()) {
        dark_style.open(QFile::ReadOnly | QFile::Text);
        QTextStream text_stream{&dark_style};
        app.setStyleSheet(text_stream.readAll());
        qInfo() << "Dark mode loaded";
    }

    w.setFixedSize(QSize{400, 400});
    w.show();
    return app.exec();
}
