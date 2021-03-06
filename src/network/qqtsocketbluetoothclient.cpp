#include "qqtsocketbluetoothclient.h"

#include <qqt.h>

QQtSocketBluetoothClient::QQtSocketBluetoothClient ( QBluetoothServiceInfo::Protocol socketType, QObject* parent ) :
    QBluetoothSocket ( socketType, parent )
{
    initSocket();
}

QQtSocketBluetoothClient::QQtSocketBluetoothClient ( QObject* parent ) : QBluetoothSocket ( parent )
{
    initSocket();
}

void QQtSocketBluetoothClient::initSocket()
{
    connect ( this, SIGNAL ( stateChanged ( QBluetoothSocket::SocketState ) ),
              this, SLOT ( socketStateChanged ( QBluetoothSocket::SocketState ) ) );
    // connected
    connect ( this, SIGNAL ( connected() ),
              this, SLOT ( socketConnected() ) );
    // disconnected
    connect ( this, SIGNAL ( disconnected() ),
              this, SLOT ( socketDisconnect() ) );
    // error
    connect ( this, SIGNAL ( error ( QBluetoothSocket::SocketError ) ),
              this, SLOT ( socketErrorOccured ( QBluetoothSocket::SocketError ) ) );

    connect ( this, SIGNAL ( readyRead() ),
              this, SLOT ( readyReadData() ) );

    connect ( this, SIGNAL ( bytesWritten ( qint64 ) ),
              this, SLOT ( updateProgress ( qint64 ) ) );

    connect ( this, SIGNAL ( bytesWritten ( qint64 ) ),
              this, SIGNAL ( signalUpdateProgress ( qint64 ) ) );

    m_PORT = 0;
    m_protocol = nullptr;
}

void QQtSocketBluetoothClient::installProtocol ( QQtProtocol* stack )
{
    if ( m_protocol )
        return;

    m_protocol = stack;
    connect ( m_protocol, SIGNAL ( write ( const QByteArray& ) ),
              this, SLOT ( write ( const QByteArray& ) ) );
}

void QQtSocketBluetoothClient::uninstallProtocol ( QQtProtocol* stack )
{
    Q_UNUSED ( stack )

    if ( !m_protocol )
        return;

    disconnect ( m_protocol, SIGNAL ( write ( const QByteArray& ) ),
                 this, SLOT ( write ( const QByteArray& ) ) );
    m_protocol = NULL;
}

QQtProtocol* QQtSocketBluetoothClient::installedProtocol()
{
    return m_protocol;
}

void QQtSocketBluetoothClient::sendConnectMessage()
{
    pline() << isOpen() << state();

    if ( !isOpen() )
    {
        connectToSingelHost();
        return;
    }

    if ( state() == ServiceLookupState ||
         state() == ConnectingState )
    {
        emit signalConnecting();
        return;
    }

    if ( state() == ConnectedState )
        emit signalConnectSucc();

    return;
}


int QQtSocketBluetoothClient::sendDisConnectFromHost()
{
    pline() << isOpen() << state();

    if ( isOpen() )
    {
#if defined(__WIN32__) || defined (__WIN64__)
        ;
#else
        shutdown ( this->socketDescriptor(), SHUT_RDWR );
#endif
        disconnectFromService();
        close();
        emit signalDisConnectSucc();
    }

    return true;
}

/**
 * @brief QQtSocketBluetoothClient::socketStateChanged
 * @param eSocketState
 * 状态函数
 */
void QQtSocketBluetoothClient::socketStateChanged ( QBluetoothSocket::SocketState eSocketState )
{
    pline() << eSocketState;

    switch ( eSocketState )
    {
    case ServiceLookupState:
    case ConnectingState:
        break;

    case ConnectedState:
        break;

    case ClosingState:
        break;

    case UnconnectedState:
        break;

    default:
        break;
    }
}

/**
 * @brief QQtSocketBluetoothClient::socketErrorOccured
 * @param e
 * 状态函数
 */
void QQtSocketBluetoothClient::socketErrorOccured ( QBluetoothSocket::SocketError e )
{
    /*
     * 在错误状态下重新连接其他热点，直到确定连接类型，写入配置文件
     */
    pline() << e << errorString();

    switch ( e )
    {
    case HostNotFoundError:
    default:
        emit signalConnectFail();
        break;
    }
}

/**
 * @brief QQtSocketBluetoothClient::socketConnected
 * 功能接口
 */
void QQtSocketBluetoothClient::socketConnected()
{
    pline() << peerName() << peerAddress().toString() << peerPort();
    /*
     * 这个步骤，socket重建，资源重新开始
     */
    emit signalConnectSucc();
}

/**
 * @brief QQtSocketBluetoothClient::socketDisconnect
 * 功能接口
 */
void QQtSocketBluetoothClient::socketDisconnect()
{
    pline();
}

void QQtSocketBluetoothClient::updateProgress ( qint64 bytes )
{
    Q_UNUSED ( bytes )
    //pline() << bytes;
}

void QQtSocketBluetoothClient::connectToSingelHost()
{
    if ( !m_uuid.isNull() )
    {
        connectToService ( m_serverIP, m_uuid );
        pline() << peerName() << m_uuid;
    }
    else if ( m_serviceInfo.isValid() )
    {
        connectToService ( m_serviceInfo );
        pline() << m_serviceInfo;
    }
    else if ( m_PORT != 0 )
    {
        connectToService ( m_serverIP, m_PORT );
        pline() << peerName() << m_PORT;
    }
}



void QQtSocketBluetoothClient::readyReadData()
{
    QByteArray bytes;
    bytes = readAll();
    m_protocol->translator ( bytes );
}
