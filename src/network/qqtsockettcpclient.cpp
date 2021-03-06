#include "qqtsockettcpclient.h"

#include <QTcpSocket>
#include <QHostInfo>

#include "qqtnetwork.h"
#include "qqtcore.h"
#include "qqt.h"

QQtSocketTcpClient::QQtSocketTcpClient ( QObject* parent ) :
    QTcpSocket ( parent )
{
    connect ( this, SIGNAL ( stateChanged ( QAbstractSocket::SocketState ) ), this,
              SLOT ( socketStateChanged ( QAbstractSocket::SocketState ) ) );
    // connected
    connect ( this, SIGNAL ( connected() ), this, SLOT ( socketConnected() ) );
    // disconnected
    connect ( this, SIGNAL ( disconnected() ), this, SLOT ( socketDisconnect() ) );
    // domain
    connect ( this, SIGNAL ( hostFound() ), this, SLOT ( domainHostFound() ) );
    // error
    connect ( this, SIGNAL ( error ( QAbstractSocket::SocketError ) ), this,
              SLOT ( socketErrorOccured ( QAbstractSocket::SocketError ) ) );

    connect ( this, SIGNAL ( readyRead() ), this, SLOT ( readyReadData() ) );

    connect ( this, SIGNAL ( bytesWritten ( qint64 ) ), this, SLOT ( updateProgress ( qint64 ) ) );

    connect ( this, SIGNAL ( bytesWritten ( qint64 ) ), this, SIGNAL ( signalUpdateProgress ( qint64 ) ) );

    /*
     * 心情很好，但是给自己带来麻烦顾虑，也不能给系统节约多少什么资源, Qt::QueuedConnection);
     */

    setSocketOption ( QAbstractSocket::LowDelayOption, 0 );
    setSocketOption ( QAbstractSocket::KeepAliveOption, 0 );
    setReadBufferSize ( _TCP_RECVBUFF_SIZE );

    m_PORT = 0;
    /*
     * 启动连接
     */
    eConType = 0;
    m_protocol = NULL;
}

QQtSocketTcpClient::~QQtSocketTcpClient()
{
}

void QQtSocketTcpClient::installProtocol ( QQtProtocol* stack )
{
    if ( m_protocol )
        return;

    m_protocol = stack;
    connect ( m_protocol, SIGNAL ( write ( const QByteArray& ) ),
              this, SLOT ( write ( const QByteArray& ) ) );
}

void QQtSocketTcpClient::uninstallProtocol ( QQtProtocol* stack )
{
    Q_UNUSED ( stack )

    if ( !m_protocol )
        return;

    disconnect ( m_protocol, SIGNAL ( write ( const QByteArray& ) ),
                 this, SLOT ( write ( const QByteArray& ) ) );
    m_protocol = NULL;
}

QQtProtocol* QQtSocketTcpClient::installedProtocol()
{
    return m_protocol;
}

void QQtSocketTcpClient::SendConnectMessage()
{
    pline() << isValid() << isOpen() << state();

    if ( !isValid() && !isOpen() )
    {
        connectToSingelHost();
        return;
    }

    if ( state() == HostLookupState ||
         state() == ConnectingState )
    {
        emit signalConnecting();
        return;
    }

    if ( state() == ConnectedState )
        emit signalConnectSucc();

    return;
}


int QQtSocketTcpClient::SendDisConnectFromHost()
{
    pline() << isValid() << isOpen() << state();

    if ( isValid() || isOpen() )
    {
#if defined(__WIN32__) || defined (__WIN64__)
        ;
#else
        shutdown ( this->socketDescriptor(), SHUT_RDWR );
#endif
        disconnectFromHost();
        waitForDisconnected();
        close();
        emit signalDisConnectSucc();
    }

    return true;
}

void QQtSocketTcpClient::domainHostFound()
{
    pline();
}

/**
 * @brief QQtSocketTcpClient::socketStateChanged
 * @param eSocketState
 * 状态函数
 */
void QQtSocketTcpClient::socketStateChanged ( QAbstractSocket::SocketState eSocketState )
{
    pline() << eSocketState;

    switch ( eSocketState )
    {
    case QAbstractSocket::HostLookupState:
    case QAbstractSocket::ConnectingState:
        break;

    case QAbstractSocket::ConnectedState:
        break;

    case QAbstractSocket::ClosingState:
        break;

    case QAbstractSocket::UnconnectedState:
        eConType++;
        break;

    default:
        break;
    }
}

/**
 * @brief QQtSocketTcpClient::socketErrorOccured
 * @param e
 * 状态函数
 */
void QQtSocketTcpClient::socketErrorOccured ( QAbstractSocket::SocketError e )
{
    /*
     * 在错误状态下重新连接其他热点，直到确定连接类型，写入配置文件
     */
    pline() << e;

    switch ( e )
    {
    case QAbstractSocket::RemoteHostClosedError:
        break;

    case QAbstractSocket::HostNotFoundError:
    default:
        emit signalConnectFail();
        break;
    }
}

/**
 * @brief QQtSocketTcpClient::socketConnected
 * 功能接口
 */
void QQtSocketTcpClient::socketConnected()
{
    pline() << peerName() << peerAddress().toString() << peerPort();
    /*
     * 这个步骤，socket重建，资源重新开始
     */
    emit signalConnectSucc();
}

/**
 * @brief QQtSocketTcpClient::socketDisconnect
 * 功能接口
 */
void QQtSocketTcpClient::socketDisconnect()
{
    pline();
}

void QQtSocketTcpClient::updateProgress ( qint64 bytes )
{
    Q_UNUSED ( bytes )
    //pline() << bytes;
}

void QQtSocketTcpClient::connectToSingelHost()
{
    int contype = eConType % m_serverIP.size();
    QString ip = m_serverIP.at ( contype );
    connectToHost ( QHostAddress ( ip ), m_PORT );

    pline() << peerName() << m_PORT;
}


void QQtSocketTcpClient::readyReadData()
{
    QByteArray bytes;
    bytes = readAll();
    m_protocol->translator ( bytes );
}
