#include "mainwindow.h"
#include <QApplication>
#include <QScreen>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    a.setApplicationName("旅行商问题");
    a.setApplicationVersion("1.0");
    qDebug() << "QApplication created";

    MainWindow w;
    w.setWindowTitle("旅行商问题");
    w.resize(800, 800);
    qDebug() << "MainWindow created";

    // 使窗口居中显示
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect screenRect = screen->availableGeometry();
        int x = (screenRect.width() - w.width()) / 2;
        int y = (screenRect.height() - w.height()) / 2;
        w.move(x, y);
    }

    w.show();
    qDebug() << "MainWindow shown";

    return a.exec();
}
