﻿#include "net/Acceptor.h"
#include "net/EventLoop.h"
#include "net/SocketUtil.h"
#include "net/Logger.h"

using namespace xop;

Acceptor::Acceptor(EventLoop* eventLoop, std::string ip, uint16_t port)
    : _eventLoop(eventLoop)
    , _tcpSocket(new TcpSocket)
{	
    _tcpSocket->create();
    _acceptChannel.reset(new Channel(_tcpSocket->fd()));
    SocketUtil::setReuseAddr(_tcpSocket->fd());
    SocketUtil::setReusePort(_tcpSocket->fd());
    SocketUtil::setNonBlock(_tcpSocket->fd());
    _tcpSocket->bind(ip, port);
}

Acceptor::~Acceptor()
{
    _eventLoop->removeChannel(_acceptChannel);
    _eventLoop->quit();
    _tcpSocket->close();
}

int Acceptor::listen()
{
    if (!_tcpSocket->listen(1024))
    {
        return -1;
    }
    _acceptChannel->setReadCallback([this]() { this->handleAccept(); });
    _acceptChannel->enableReading();
    _eventLoop->updateChannel(_acceptChannel);
    return 0;
}

void Acceptor::handleAccept()
{
    int connfd = _tcpSocket->accept();
    if (connfd > 0)
    {
        if (_newConnectionCallback)		
        {
            _newConnectionCallback(connfd);
        }
        else
        {
            SocketUtil::close(connfd);
        }
    }
}

