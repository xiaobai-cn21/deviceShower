#include "deviceshower.h"
#include <QVBoxLayout>

DeviceShower::DeviceShower(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);

    tempLabel = new QLabel("Temperature: --", this);
    pressureLabel = new QLabel("Pressure: --", this);

    layout->addWidget(tempLabel);
    layout->addWidget(pressureLabel);

    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::readyRead, this, &DeviceShower::onDataReceived);

    // Change IP if server is on another computer
    socket->connectToHost("192.168.245.129", 8081);
}

void DeviceShower::onDataReceived() {
    while (socket->canReadLine()) {
        QByteArray line = socket->readLine().trimmed();
        QList<QByteArray> parts = line.split(',');
        if (parts.size() == 2) {
            QString temp = parts[0];
            QString pressure = parts[1];
            tempLabel->setText("Temperature: " + temp + " °C");
            pressureLabel->setText("Pressure: " + pressure + " hPa");
        }
    }
}
