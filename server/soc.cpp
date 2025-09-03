#include <iostream>
#include <string>
#include <random>
#include <unistd.h>
#include <arpa/inet.h>

struct SensorField {
    std::string chineseName;
    std::string unit;
    float value;
};

struct SensorData {
    SensorField pumpPressure   = {"泵出口压力", "MPa", 0.0f};
    SensorField pumpVibration  = {"泵轴振动", "mm/s", 0.0f};
    SensorField motorCurrent   = {"泵电机电流", "A", 0.0f};
    SensorField oilLevel       = {"油箱液位", "%", 0.0f};
    SensorField pumpTemp       = {"泵体温度", "°C", 0.0f};
    SensorField flowRate       = {"流体流量", "m³/h", 0.0f};
};

SensorData generateSensorData(std::mt19937& gen) {
    std::uniform_real_distribution<float> pumpPressure(1.2, 2.0);
    std::uniform_real_distribution<float> pumpVibration(0.3, 1.5);
    std::uniform_real_distribution<float> motorCurrent(80.0, 120.0);
    std::uniform_real_distribution<float> oilLevel(30.0, 80.0);
    std::uniform_real_distribution<float> pumpTemp(30.0, 55.0);
    std::uniform_real_distribution<float> flowRate(50.0, 100.0);

    SensorData data;
    data.pumpPressure.value  = pumpPressure(gen);
    data.pumpVibration.value = pumpVibration(gen);
    data.motorCurrent.value  = motorCurrent(gen);
    data.oilLevel.value      = oilLevel(gen);
    data.pumpTemp.value      = pumpTemp(gen);
    data.flowRate.value      = flowRate(gen);
    return data;
}

// Send as: name,unit,value;name,unit,value;...
std::string formatSensorData(const SensorData& data) {
    char buffer[256];
    snprintf(buffer, sizeof(buffer),
        "%s,%s,%.2f;%s,%s,%.2f;%s,%s,%.2f;%s,%s,%.2f;%s,%s,%.2f;%s,%s,%.2f\n",
        data.pumpPressure.chineseName.c_str(), data.pumpPressure.unit.c_str(), data.pumpPressure.value,
        data.pumpVibration.chineseName.c_str(), data.pumpVibration.unit.c_str(), data.pumpVibration.value,
        data.motorCurrent.chineseName.c_str(), data.motorCurrent.unit.c_str(), data.motorCurrent.value,
        data.oilLevel.chineseName.c_str(), data.oilLevel.unit.c_str(), data.oilLevel.value,
        data.pumpTemp.chineseName.c_str(), data.pumpTemp.unit.c_str(), data.pumpTemp.value,
        data.flowRate.chineseName.c_str(), data.flowRate.unit.c_str(), data.flowRate.value
    );
    return buffer;
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    std::random_device rd;
    std::mt19937 gen(rd());

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error\n";
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8081);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported\n";
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed\n";
        return -1;
    }

    std::cout << "Connected to server. Sending sensor data...\n";
    while (true) {
        SensorData data = generateSensorData(gen);
        std::string message = formatSensorData(data);
        send(sock, message.c_str(), message.size(), 0);
        std::cout << "Sent: " << message;
        sleep(1);
    }

    close(sock);
    return 0;
}