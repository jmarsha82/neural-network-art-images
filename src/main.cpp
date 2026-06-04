#include "artnet/MainWindow.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    artnet::MainWindow window;
    window.show();

    return QApplication::exec();
}
