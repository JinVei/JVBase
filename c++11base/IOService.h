#ifndef __UNITYGAME_IOSERVICE_H_
#define __UNITYGAME_IOSERVICE_H_

#include <boost/asio.hpp>
#include <boost/bind.hpp>

namespace UnityGame {
    typedef std::shared_ptr<boost::asio::ip::tcp::socket>        SOCKET_SPTR;
    typedef std::weak_ptr<boost::asio::ip::tcp::socket>          SOCKET_WPTR;
    typedef std::shared_ptr<std::string>                         STRING_SPTR;

    class IOService {
    public:
        class Connection;
        typedef std::shared_ptr<Connection>                        CONNECTION_SPTR;
        typedef std::weak_ptr<Connection>                          CONNECTION_WPTR;
        typedef std::function<void(CONNECTION_SPTR)>               ACCEPT_CALLBACK_T;
    private:
        boost::asio::io_service             m_IOService;
        boost::asio::ip::tcp::acceptor      m_ServiceAcceptor;
        ACCEPT_CALLBACK_T                   m_AcceptHandler;
        int                                 m_nPortNumber;
        bool                                m_bIsRun;

        void AcceptCallBack(SOCKET_SPTR sock, const boost::system::error_code& err);
        void StartEventLoop();
        void AsyncAccept();
        void WriteCallBack(Connection* Connect, const boost::system::error_code& err, size_t bytes_transferred);
        void ReadCallBack(CONNECTION_SPTR Connect, const boost::system::error_code& err, size_t bytes_transferred);
    public:
        ~IOService();
        IOService(ACCEPT_CALLBACK_T accept_handle);
        IOService(ACCEPT_CALLBACK_T accept_handle, int nPortNumber);
        bool StartIOServer();
        void StopIOServer();
        void AsyncWrite(Connection* Connect, const std::string& strData);
    };

    class IOService::Connection {
    public:
        typedef std::function<void(Connection& Connect)>                     EXIT_CALLBACK_T;
        typedef std::function<void(Connection& Connect, const std::string&)> RECEIVE_CALLBACK_T;
    private:
        friend class IOService;
        IOService&                m_Host;
        SOCKET_SPTR               m_pSocket;

        RECEIVE_CALLBACK_T        m_ReceiveHandler;
        EXIT_CALLBACK_T           m_ExitHandler;

        char                      m_szReadBuffer[256];
        const int                 m_nReadBufferSize   = 256;
        bool                      m_bIsLive           = true;
        void SendOkCallBack();
        void ReceiveCallBack(STRING_SPTR data);
        void ExitCallBack();
        Connection(IOService& host, SOCKET_SPTR sock);
    public:
        bool SendData(const std::string& data);
        bool SetReceiveCallBack(RECEIVE_CALLBACK_T fuc);
        bool SetExitCallBack(EXIT_CALLBACK_T fuc);
        bool CloseConnection();
    public:
        void* userdata = nullptr;
    };
}

#endif
