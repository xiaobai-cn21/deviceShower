#include "deviceshower.h"
#include "LedDisplay.h"
#include "gauge.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QPushButton>
#include <QLabel>
#include <QMenu>
#include <QAction>
#include <QDebug>
#include <QAbstractSocket>

static QFrame* makeCard(const QString &title) {
    QFrame *card = new QFrame();
    card->setStyleSheet("background:#202538; border-radius:8px;");
    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(8, 8, 8, 8);

    QLabel *label = new QLabel(title);
    label->setStyleSheet("color:white; font-weight:bold; font-size:12px;");
    label->setAlignment(Qt::AlignCenter);

    cardLayout->addWidget(label);
    return card;
}

DeviceShower::DeviceShower(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(12, 12, 12, 12);

    // === Buttons row ===
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(12);

    QPushButton *equipButton = new QPushButton("Change Equipment");
    QPushButton *terminalButton = new QPushButton("Terminal");
    QPushButton *malfunctionButton = new QPushButton("Malfunction Info");

    QString buttonStyle = "QPushButton { background:#202538; color:white; border-radius:8px; font-weight:bold; padding:6px; }"
                          "QPushButton:hover { background:#303654; }";

    equipButton->setStyleSheet(buttonStyle);
    terminalButton->setStyleSheet(buttonStyle);
    malfunctionButton->setStyleSheet(buttonStyle);

    equipButton->setFixedSize(150, 40);
    terminalButton->setFixedSize(150, 40);
    malfunctionButton->setFixedSize(150, 40);

    buttonLayout->addWidget(equipButton);
    buttonLayout->addWidget(terminalButton);
    buttonLayout->addWidget(malfunctionButton);

    // Equipment menu
    QMenu *equipMenu = new QMenu(this);
    equipMenu->setStyleSheet(
        "QMenu { background-color: #202538; border: 1px solid #303654; border-radius: 8px; }"
        "QMenu::item { color: white; padding: 6px 12px; }"
        "QMenu::item:selected { background-color: #303654; }"
    );
    equipMenu->addAction(new QAction("Equipment 1", this));
    equipMenu->addAction(new QAction("Equipment 2", this));
    equipMenu->addAction(new QAction("Equipment 3", this));
    equipButton->setMenu(equipMenu);

    // === First row: Gauges ===
    QWidget *gaugeRowWidget = new QWidget();
    QHBoxLayout *gaugeLayoutRow = new QHBoxLayout(gaugeRowWidget);
    gaugeLayoutRow->setSpacing(12);
    gaugeLayoutRow->setContentsMargins(16, 0, 16, 0);

    gauges.clear();
    for (int i = 0; i < 3; i++) {
        Gauge *g = new Gauge();
        g->setMinMax(0, 200);
        g->setValue(0);
        g->setUnits("--");
        g->setLabel("--");
        g->setColor(QColor(0, 190, 255));

        QFrame *gCard = new QFrame();
        gCard->setStyleSheet("background:#202538; border-radius:8px;");
        QVBoxLayout *gCardLayout = new QVBoxLayout(gCard);
        gCardLayout->setContentsMargins(8, 8, 8, 8);
        gCardLayout->addWidget(g);

        gaugeLayoutRow->addWidget(gCard);
        gauges.append(g);
    }

    // === Second row: LEDs ===
    QWidget *ledRowWidget = new QWidget();
    QHBoxLayout *ledLayoutRow = new QHBoxLayout(ledRowWidget);
    ledLayoutRow->setSpacing(12);
    ledLayoutRow->setContentsMargins(16, 0, 16, 0);

    leds.clear();
    for (int i = 0; i < 3; i++) {
        LedDisplay *led = new LedDisplay();
        led->setText("--");
        led->setColor(QColor(0, 255, 0));
        led->setLabel("--");
        led->setBlinkInterval(2000);

        QFrame *ledCard = new QFrame();
        ledCard->setStyleSheet("background:#202538; border-radius:8px;");
        QVBoxLayout *ledCardLayout = new QVBoxLayout(ledCard);
        ledCardLayout->setContentsMargins(8, 8, 8, 8);
        ledCardLayout->addWidget(led);

        ledLayoutRow->addWidget(ledCard);
        leds.append(led);
    }

    // === Third row: empty cards ===
    QHBoxLayout *emptyLayoutRow = new QHBoxLayout();
    emptyLayoutRow->setSpacing(12);
    for (int i = 0; i < 3; i++) {
        emptyLayoutRow->addWidget(makeCard(QString("Empty %1").arg(i+1)));
    }

    // === Assemble main layout ===
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(gaugeRowWidget);
    mainLayout->addWidget(ledRowWidget);
    mainLayout->addLayout(emptyLayoutRow);

    setStyleSheet("background-color:#0a0f1a;");

    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::readyRead, this, &DeviceShower::onDataReceived);
    connect(socket, &QTcpSocket::connected, this, [](){
        qDebug() << "Connected to server!";
    });
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onSocketError(QAbstractSocket::SocketError)));
    socket->connectToHost("127.0.0.1", 8081); // Change to your server IP if needed
}

SensorData DeviceShower::parseSensorData(const QString& line) {
    SensorData data;
    QStringList fields = line.trimmed().split(";");
    if (fields.size() == 6) {
        auto parseField = [](const QString& field, SensorField& out) {
            QStringList parts = field.split(",");
            if (parts.size() == 3) {
                out.chineseName = parts[0];
                out.unit = parts[1];
                out.value = parts[2].toFloat();
            }
        };
        parseField(fields[0], data.pumpPressure);
        parseField(fields[1], data.pumpVibration);
        parseField(fields[2], data.motorCurrent);
        parseField(fields[3], data.oilLevel);
        parseField(fields[4], data.pumpTemp);
        parseField(fields[5], data.flowRate);
    }
    return data;
}

void DeviceShower::onDataReceived() {
    while (socket->canReadLine()) {
        QString line = socket->readLine();
        qDebug() << "Received from server:" << line;
        SensorData data = parseSensorData(line);
        updateSensorData(data);
    }
}

void DeviceShower::updateSensorData(const SensorData& data) {
    // Gauge 0: Pressure
    // Gauge 1: Vibration
    // Gauge 2: Flow Rate
    if (gauges.size() >= 3) {
        gauges[0]->setValue(data.pumpPressure.value);
        gauges[0]->setUnits(data.pumpPressure.unit);
        gauges[0]->setLabel(data.pumpPressure.chineseName);

        gauges[1]->setValue(data.pumpVibration.value);
        gauges[1]->setUnits(data.pumpVibration.unit);
        gauges[1]->setLabel(data.pumpVibration.chineseName);

        gauges[2]->setValue(data.flowRate.value);
        gauges[2]->setUnits(data.flowRate.unit);
        gauges[2]->setLabel(data.flowRate.chineseName);
    }
    // LED 0: Motor Current
    // LED 1: Oil Level
    // LED 2: Pump Temp
    if (leds.size() >= 3) {
        leds[0]->setText(QString::number(data.motorCurrent.value, 'f', 1));
        leds[0]->setLabel(data.motorCurrent.chineseName + " (" + data.motorCurrent.unit + ")");

        leds[1]->setText(QString::number(data.oilLevel.value, 'f', 1));
        leds[1]->setLabel(data.oilLevel.chineseName + " (" + data.oilLevel.unit + ")");

        leds[2]->setText(QString::number(data.pumpTemp.value, 'f', 1));
        leds[2]->setLabel(data.pumpTemp.chineseName + " (" + data.pumpTemp.unit + ")");
    }
}

void DeviceShower::onSocketError(QAbstractSocket::SocketError socketError) {
    qDebug() << "Socket error:" << socketError << socket->errorString();
}