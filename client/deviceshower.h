#ifndef CLIENTWIDGET_H
#define CLIENTWIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QLabel>

class DeviceShower : public QWidget {
    Q_OBJECT

public:
    explicit DeviceShower(QWidget *parent = nullptr);

private slots:
    void onDataReceived();

private:
    QLabel *tempLabel;
    QLabel *pressureLabel;
    QTcpSocket *socket;
};

#endif // DEVICESHOWER_H

