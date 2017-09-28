#include "IOService.h"
#include <csignal>
#include <iostream> 
using namespace boost::asio;
using namespace boost::asio::ip;
using namespace std;

#ifndef SIGPIPE
#define SIGPIPE 13
#endif

UnityGame::IOService::Connection::Connection(IOService& host, SOCKET_SPTR sock) : m_Host(host)
{
    m_ReceiveHandler = nullptr;
    m_ExitHandler = nullptr;
    m_pSocket = sock;
}

bool UnityGame::IOService::Connection::SetReceiveCallBack(RECEIVE_CALLBACK_T func)
{
    if (!m_bIsLive)
    {
        return false;
    }

    m_ReceiveHandler = func;
    return true;
}

bool UnityGame::IOService::Connection::SetExitCallBack(EXIT_CALLBACK_T func)
{
    if (!m_bIsLive)
    {
        return false;
    }

    m_ExitHandler = func;
    return true;
}

void UnityGame::IOService::Connection::ReceiveCallBack(STRING_SPTR data)
{
    if (m_bIsLive && m_ReceiveHandler)
    {
        m_ReceiveHandler(*this, *data);
    }
}

void UnityGame::IOService::Connection::ExitCallBack()
{
    if (m_bIsLive && m_ExitHandler)
    {
        m_ExitHandler(*this);
    }
    m_bIsLive = false;
}

bool UnityGame::IOService::Connection::SendData(const string& data)
{
    if (!m_bIsLive)
    {
        return false;
    }

    STRING_SPTR pstrData = STRING_SPTR(new string(data));
    m_Host.AsyncWrite(this, *pstrData);
    return true;
}

void UnityGame::IOService::Connection::SendOkCallBack()
{

}

bool UnityGame::IOService::Connection::CloseConnection()
{
    if (!m_bIsLive)
    {
        return false;
    }

    m_bIsLive = false;
    m_ReceiveHandler = nullptr;
    boost::system::error_code ec;
    m_pSocket->close();
    return true;
}

UnityGame::IOService::~IOService()
{

}

UnityGame::IOService::IOService(ACCEPT_CALLBACK_T accept_handle) :
    m_ServiceAcceptor(m_IOService, tcp::endpoint(tcp::v4(), 8880))
{
    m_AcceptHandler = accept_handle;
    m_bIsRun = false;
    m_nPortNumber = 8880;
}
UnityGame::IOService::IOService(ACCEPT_CALLBACK_T accept_handle, int nPortNumber) :
    m_ServiceAcceptor(m_IOService, tcp::endpoint(tcp::v4(), nPortNumber))
{
    m_AcceptHandler = accept_handle;
    m_bIsRun = false;
    m_nPortNumber = nPortNumber;
}

void UnityGame::IOService::ReadCallBack(CONNECTION_SPTR Connect, const boost::system::error_code& err, size_t bytes_transferred)
{
    if (err)
    {
        cout << "ReadCallBack : " << err << endl;
        Connect->ExitCallBack();
        return;
    }

    STRING_SPTR buffer = STRING_SPTR(new string(Connect->m_szReadBuffer, bytes_transferred));

    Connect->ReceiveCallBack(buffer);

    auto readFuc = boost::bind(&IOService::ReadCallBack, this, Connect, _1, _2);
    Connect->m_pSocket->async_read_some(boost::asio::buffer(Connect->m_szReadBuffer, 255), readFuc);
}

void UnityGame::IOService::WriteCallBack(Connection* Connect, const boost::system::error_code& err, size_t bytes_transferred)
{
    if (err)
    {
        cout << "IOService::WriteCallBack : " << err << endl;
        return;
    }
    Connect->SendOkCallBack();
}

void UnityGame::IOService::AcceptCallBack(SOCKET_SPTR psock, const boost::system::error_code& err)
{
    if (err)
    {
        cout << "IOService::AcceptCallBack : " << err << endl;
        return;
    }

    CONNECTION_SPTR Connect = CONNECTION_SPTR(new Connection(*this, psock));

    m_AcceptHandler(Connect);

    auto handler = boost::bind(&IOService::ReadCallBack, this, Connect, _1, _2);
    Connect->m_pSocket->async_read_some(boost::asio::buffer(Connect->m_szReadBuffer, 255), handler);

    AsyncAccept();
}

void UnityGame::IOService::AsyncAccept()
{
    SOCKET_SPTR sock = SOCKET_SPTR(new tcp::socket(this->m_IOService));

    auto acceptHandle = boost::bind(&IOService::AcceptCallBack, this, sock, _1);

    m_ServiceAcceptor.async_accept(*sock, acceptHandle);
}

void UnityGame::IOService::StartEventLoop()
{
    AsyncAccept();
    this->m_IOService.run();
}

bool UnityGame::IOService::StartIOServer()
{
    if (!m_bIsRun)
    {
        signal(SIGPIPE, SIG_IGN);
        StartEventLoop();
        return true;
    }
    return false;
}

void UnityGame::IOService::StopIOServer()
{
    m_bIsRun = false;
    m_IOService.stop();
}

void UnityGame::IOService::AsyncWrite(Connection* Connect, const string& strData)
{
    SOCKET_SPTR sock = Connect->m_pSocket;

    boost::asio::async_write(*sock,
        boost::asio::buffer(strData),
        boost::asio::transfer_all(),
        boost::bind(&IOService::WriteCallBack, this, Connect, _1, _2));
}
