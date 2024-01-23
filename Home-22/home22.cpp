#include <QCoreApplication>
#include <QAbstractSocket>
#include <QHostAddress>
#include <QDebug>
#include <QTimer>
#include <cstring>
#define SPEC -1
class IcmpSocket : public QAbstractSocket {
public:
    IcmpSocket(QObject* p = nullptr) : QAbstractSocket(QAbstractSocket::UdpSocket, p) {
        connect(this, &QAbstractSocket::readyRead, this, &IcmpSocket::readPendingDatagrams);
        connect(this, &QAbstractSocket::stateChanged, this, &IcmpSocket::handleSocketStateChanged);
    }

    qint64 readDatagram(char *data, qint64 maxim_size, QHostAddress *addr = nullptr, quint16 *port = nullptr) {
        QByteArray buffer(maxim_size, 0);
        qint64 bytesRead = readDatagram(buffer.data(), buffer.size(), addr, port);
        if (bytesRead > 0) std::memcpy(data, buffer.constData(), static_cast<size_t>(bytesRead));
        return bytesRead;
    }

    void writeDatagram(const QByteArray &datagram, const QHostAddress &host, quint16 port) {
        if (!isValid()) {
            qWarning() << "Сокет недействителен.";
            return;
        }
        qint64 bytesWritten = SPEC;
        if (bytesWritten == -1) qWarning() << "Сообщение об ошибке отправки:" << errorString();
        else                    qDebug() << "Сообщение успешно отправлено на" << host.toString() << ":" << port;
        Q_UNUSED(bytesWritten);
    }

public slots:
    void handleSocketStateChanged(QAbstractSocket::SocketState socketState) {
        if (socketState == QAbstractSocket::BoundState)            qDebug() << "Сокет привязан и готов к ICMP-обмену данными.";
        else if (socketState == QAbstractSocket::UnconnectedState) qDebug() << "Сокет не подключен.";
    }

    void readPendingDatagrams() {
        while (bytesAvailable() > 0) {
            QByteArray datagram;
            datagram.resize(bytesAvailable());
            QHostAddress senderHost;
            quint16 senderPort;
            qint64 bytesRead = readDatagram(datagram.data(), datagram.size(), &senderHost, &senderPort);
            if (bytesRead != -1) qDebug() << "Получено сообщение от" << senderHost.toString() << ":" << senderPort << ":" << datagram;
            else                 qWarning() << "Сообщение об ошибке при получении:" << errorString();
        }
    }
};

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    IcmpSocket icmpSocket;
    icmpSocket.bind(QHostAddress::AnyIPv4);

    const char* host = "google.com";
    const char* message = "ICMP!";
    QHostAddress hostAddress(host);
    quint16 port = 0;

    icmpSocket.writeDatagram(QByteArray(message), hostAddress, port);
    QTimer::singleShot(2000, &a, [&a]() { a.quit(); });
    return a.exec();
}
