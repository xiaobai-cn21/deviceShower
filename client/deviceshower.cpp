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

// Helper function to create a styled card with a label
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
        g->setValue(0); // initial value
        g->setUnits("RPM");
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
        led->setLabel(QString("LED %1").arg(i+1));
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

    // === TCP Socket setup ===
    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::readyRead, this, &DeviceShower::onDataReceived);

    // Change IP if server is on another computer
    socket->connectToHost("192.168.245.129", 8081); // <-- Set your server IP here
}

SensorData DeviceShower::parseSensorData(const QString& csv) {
    SensorData data;
    QStringList parts = csv.trimmed().split(",");
    if (parts.size() == 6) {
        data.pumpPressure = parts[0].toFloat();
        data.pumpVibration = parts[1].toFloat();
        data.motorCurrent = parts[2].toFloat();
        data.oilLevel = parts[3].toFloat();
        data.pumpTemp = parts[4].toFloat();
        data.flowRate = parts[5].toFloat();
    }
    return data;
}

void DeviceShower::onDataReceived() {
    while (socket->canReadLine()) {
        QString line = socket->readLine();
        SensorData data = parseSensorData(line);
        updateSensorData(data);
    }
}

void DeviceShower::updateSensorData(const SensorData& data) {
    // Example mapping:
    // Gauge 0: Pressure
    // Gauge 1: Vibration
    // Gauge 2: Flow Rate
    if (gauges.size() >= 3) {
        gauges[0]->setValue(data.pumpPressure);
        gauges[1]->setValue(data.pumpVibration);
        gauges[2]->setValue(data.flowRate);
    }
    // LED 0: Motor Current
    // LED 1: Oil Level
    // LED 2: Pump Temp
    if (leds.size() >= 3) {
        leds[0]->setText(QString::number(data.motorCurrent, 'f', 1));
        leds[1]->setText(QString::number(data.oilLevel, 'f', 1));
        leds[2]->setText(QString::number(data.pumpTemp, 'f', 1));
    }
}