#include <QApplication>
#include "deviceshower.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    DeviceShower w;
    w.setWindowTitle("Sensor Monitor");
    w.resize(300, 100);
    w.show();
    return app.exec();
}

