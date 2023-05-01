#include "mainwindow.h"
#include <QSerialPort>
#include <QApplication>
#include <QtDebug>

//int main(int argc, char *argv[])
//{
//    QApplication a(argc, argv);
//    MainWindow w;
//    w.show();
//    return a.exec();
//}

QSerialPort serial;
float read_float(QByteArray data) {
  // Преобразуем указатель на первый байт к типу float *
  float  value = *(float *)data.data();
  return value;
}

void readData() {
  QByteArray data = serial.readAll(); // Чтение данных из порта
  if (data.size() == sizeof(float )) { // Проверка размера данных
    float value = read_float(data); // Получение числа
    qDebug() << "Received value: " << value; // Вывод числа в консоль
  }
}

int main(int argc, char *argv[]) {
  // Инициализация последовательного порта
  serial.setPortName("COM3");
  serial.setBaudRate(QSerialPort::Baud115200);
  serial.setDataBits(QSerialPort::Data8);
  serial.setParity(QSerialPort::NoParity);
  serial.setStopBits(QSerialPort::OneStop);
  serial.setFlowControl(QSerialPort::NoFlowControl);

  // Открытие порта для чтения
  if (serial.open(QIODevice::ReadOnly)) {
    QObject::connect(&serial, &QSerialPort::readyRead, readData); // Подключение сигнала readyRead

    // Запуск основного цикла приложения
    QApplication a(argc, argv);
    // ... код интерфейса ...
    return a.exec();
  }
  else {
    qDebug() << "Error opening port: " << serial.errorString();
    return -1;
  }
}
