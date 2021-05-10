/*
 * @Author: xtt
 * @Date: 2021-04-28 15:17:42
 * @Description: ...
 * @LastEditTime: 2021-05-08 19:01:49
 */
#include "serialPort.h"
#include <stdio.h>

unsigned short crc16_code(const unsigned char *buf, const int len)
{
        if (len <= 0)
                return 0x0000;

        unsigned short tmp = 0xffff;
        unsigned short ret1 = 0;

        for (int n = 0; n < len; n++)
        {
                tmp = buf[n] ^ tmp;
                for (int i = 0; i < 8; i++)
                {
                        if (tmp & 0x01)
                        {
                                tmp = tmp >> 1;
                                tmp = tmp ^ 0xa001;
                        }
                        else
                        {
                                tmp = tmp >> 1;
                        }
                }
        }
        return tmp;
}

SerialPort::SerialPort(const PortParam &param) : portParam(param), QObject(nullptr)
{
}

bool SerialPort::open()
{
        port.setPortName(portParam.name);
        port.setBaudRate(QSerialPort::BaudRate(portParam.baudrate));
        port.setDataBits(QSerialPort::DataBits(portParam.databits));
        port.setStopBits(QSerialPort::StopBits(portParam.stopbits));
        port.setParity(QSerialPort::NoParity);
        port.setFlowControl(QSerialPort::NoFlowControl);

        bool res = port.open(QIODevice::ReadWrite);
        if (res)
        {
                if (!port.clear()) fprintf(stderr, "port clear failed");
                connect(&port, &QSerialPort::readyRead, this, &SerialPort::readData);
                connect(&port, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error), this, &SerialPort::portError);
                connect(this, &SerialPort::sigSend, this, &SerialPort::sendData);
        }
        return res;
}

void SerialPort::readData()
{
        auto size = port.bytesAvailable();
        if (size > BUFFER_SIZE)
        {
                fprintf(stderr, "port buffer size less than buffer size:%d,%lld\n", BUFFER_SIZE, size);
                assert(false);
        }
        int read_size = port.read((char *)buffer, size);
        pf_callback(buffer, read_size, call);
}

void SerialPort::sendData(QByteArray data)
{
        port.write(data);
}

void SerialPort::send(const char *_p, int _len)
{
        emit sigSend(QByteArray(_p, _len));
}

void SerialPort::portError(QSerialPort::SerialPortError error)
{
        if (error == QSerialPort::ResourceError)
        {
                fprintf(stderr, "%s\n", port.errorString().toStdString().c_str());
                close();
        }
}

void SerialPort::close()
{
        port.close();
}