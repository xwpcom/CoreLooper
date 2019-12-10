﻿/*
 * MIT License
 *
 * Copyright (c) 2016-2019 xiongziliang <771730766@qq.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef TCPSERVER_TCPSERVER_H
#define TCPSERVER_TCPSERVER_H

#include <mutex>
#include <memory>
#include <exception>
#include <functional>
#include <unordered_map>
#include "TcpSession.h"
#include "Util/mini.h"
#include "Util/util.h"
#include "Util/logger.h"
#include "Util/uv_errno.h"
#include "Poller/Timer.h"
#include "Thread/semaphore.h"

using namespace std;

namespace toolkit {

//全局的TcpSession记录对象，方便后面管理
//线程安全的
class SessionMap {
public:
    friend class TcpServer;
    //单例
    static SessionMap &Instance();
    //获取Session
    TcpSession::Ptr get(const string &tag){
        lock_guard<mutex> lck(_mtx_session);
        auto it = _map_session.find(tag);
        if(it == _map_session.end()){
            return nullptr;
        }
        return it->second.lock();
    }
    void for_each_session(const function<void(const string &id,const TcpSession::Ptr &session)> &cb){
        lock_guard<mutex> lck(_mtx_session);
        for(auto it = _map_session.begin() ; it != _map_session.end() ; ){
            auto session = it->second.lock();
            if(!session){
                it = _map_session.erase(it);
                continue;
            }
            cb(it->first,session);
            ++it;
        }
    }
private:
    SessionMap(){};
    ~SessionMap(){};
    //添加Session
    bool add(const string &tag,const TcpSession::Ptr &session){
        //InfoL ;
        lock_guard<mutex> lck(_mtx_session);
        return _map_session.emplace(tag,session).second;
    }
    //移除Session
    bool remove(const string &tag){
        //InfoL ;
        lock_guard<mutex> lck(_mtx_session);
        return _map_session.erase(tag);
    }
private:
    unordered_map<string, weak_ptr<TcpSession> > _map_session;
    mutex _mtx_session;
};

class TcpServer;
class TcpSessionHelper {
public:
	typedef std::shared_ptr<TcpSessionHelper> Ptr;

	TcpSessionHelper(const std::weak_ptr<TcpServer> &server,TcpSession::Ptr &&session){
		_server = server;
		_session = std::move(session);
	}
	~TcpSessionHelper(){
		if(!_server.lock()){
			_session->onError(SockException(Err_other,"Tcp server shutdown!"));
		}
	}

	const TcpSession::Ptr &session() const{
		return _session;
	}
private:
	std::weak_ptr<TcpServer> _server;
	TcpSession::Ptr _session;
};


//TCP服务器，可配置的；配置通过TcpSession::attachServer方法传递给会话对象
//该对象是非线程安全的，务必在主线程中操作
class TcpServer : public mINI , public std::enable_shared_from_this<TcpServer>{
public:
	typedef std::shared_ptr<TcpServer> Ptr;

	/**
	 * 创建Tcp服务器，父文件描述符的accept事件在某固定的poller循环中触发
     * 但是子文件描述符的所有事件、数据读取、数据处理都是在从EventPollerPool中获取的poller线程中执行
     * 所以这种方式网络事件的触发会派发到多个poller线程中执行
     * 这种方式网络吞吐量最大
	 */

    TcpServer(const EventPoller::Ptr &poller = nullptr) {
		_poller = poller;
		if(!_poller){
			_poller =  EventPollerPool::Instance().getPoller();
		}
		_socket = std::make_shared<Socket>(_poller);
        _socket->setOnAccept(bind(&TcpServer::onAcceptConnection_l, this, placeholders::_1));
		_socket->setOnBeforeAccept(bind(&TcpServer::onBeforeAcceptConnection_l, this,std::placeholders::_1));
    }

	virtual ~TcpServer() {
		_timer.reset();
        //先关闭socket监听，防止收到新的连接
		_socket.reset();

        if(!_cloned) {
            TraceL << "start clean tcp server...";
        }
        //务必通知TcpSession已从TcpServer脱离
		for (auto &pr : _sessionMap) {
            //从全局的map中移除记录
            SessionMap::Instance().remove(pr.first);
		}
		_sessionMap.clear();
		_clonedServer.clear();
        if(!_cloned){
            TraceL << "clean tcp server completed!";
        }
	}

    //开始监听服务器
    template <typename SessionType>
	void start(uint16_t port, const std::string& host = "0.0.0.0", uint32_t backlog = 1024) {
		start_l<SessionType>(port,host,backlog);
		EventPollerPool::Instance().for_each([&](const TaskExecutor::Ptr &executor){
			EventPoller::Ptr poller = dynamic_pointer_cast<EventPoller>(executor);
			if(poller == _poller || !poller){
				return;
			}
			auto &serverRef = _clonedServer[poller.get()];
			if(!serverRef){
				serverRef = onCreatServer(poller);
			}
			if(serverRef){
				serverRef->cloneFrom(*this);
			}
		});
	}

	uint16_t getPort(){
		if(!_socket){
			return 0;
		}
		return _socket->get_local_port();
	}
protected:
	virtual TcpServer::Ptr onCreatServer(const EventPoller::Ptr &poller){
		return std::make_shared<TcpServer>(poller);
    }

    virtual Socket::Ptr onBeforeAcceptConnection(const EventPoller::Ptr &poller){
    	/**
    	 * 服务器器模型socket是线程安全的，所以为了提高性能，关闭互斥锁
    	 */
		return std::make_shared<Socket>(poller,false);
	}

	virtual void cloneFrom(const TcpServer &that){
		if(!that._socket){
			throw std::invalid_argument("TcpServer::cloneFrom other with null socket!");
		}
		_sessionMaker = that._sessionMaker;
		_socket->cloneFromListenSocket(*(that._socket));
		_timer = std::make_shared<Timer>(2, [this]()->bool {
			this->onManagerSession();
			return true;
		},_poller);
		this->mINI::operator=(that);
		_cloned = true;
	}

    // 接收到客户端连接请求
    virtual void onAcceptConnection(const Socket::Ptr & sock) {
		weak_ptr<TcpServer> weakSelf = shared_from_this();
        //创建一个TcpSession;这里实现创建不同的服务会话实例
		auto sessionHelper = _sessionMaker(weakSelf,sock);
		auto &session = sessionHelper->session();
        //把本服务器的配置传递给TcpSession
        session->attachServer(*this);

        //TcpSession的唯一识别符，可以是guid之类的
        auto sessionId = session->getIdentifier();
        //记录该TcpSession
        if(!SessionMap::Instance().add(sessionId,session)){
            //有同名session，说明getIdentifier生成的标识符有问题
            WarnL << "SessionMap::add failed:" << sessionId;
            return;
        }
        //SessionMap中没有相关记录，那么_sessionMap更不可能有相关记录了；
        //所以_sessionMap::emplace肯定能成功
        auto success = _sessionMap.emplace(sessionId, sessionHelper).second;
        assert(success == true);

        weak_ptr<TcpSession> weakSession(session);
		//会话接收数据事件
		sock->setOnRead([weakSession](const Buffer::Ptr &buf, struct sockaddr *, int ){
			//获取会话强应用
			auto strongSession=weakSession.lock();
			if(!strongSession) {
				//会话对象已释放
				return;
			}
			strongSession->onRecv(buf);
		});


		//会话接收到错误事件
		sock->setOnErr([weakSelf,weakSession,sessionId](const SockException &err){
		    //在本函数作用域结束时移除会话对象
            //目的是确保移除会话前执行其onError函数
            //同时避免其onError函数抛异常时没有移除会话对象
		    onceToken token(nullptr,[&](){
                //移除掉会话
                SessionMap::Instance().remove(sessionId);
                auto strongSelf = weakSelf.lock();
                if(!strongSelf) {
                    return;
                }
                //在TcpServer对应线程中移除map相关记录
                strongSelf->_poller->async([weakSelf,sessionId](){
                    auto strongSelf = weakSelf.lock();
                    if(!strongSelf){
                        return;
                    }
                    strongSelf->_sessionMap.erase(sessionId);
                });
		    });
			//获取会话强应用
			auto strongSession=weakSession.lock();
            if(strongSession) {
                //触发onError事件回调
				strongSession->onError(err);
			}
		});
	}

private:
	Socket::Ptr onBeforeAcceptConnection_l(const EventPoller::Ptr &poller){
		return onBeforeAcceptConnection(poller);
	}
    // 接收到客户端连接请求
    void onAcceptConnection_l(const Socket::Ptr & sock) {
        onAcceptConnection(sock);
    }


	template <typename SessionType>
	void start_l(uint16_t port, const std::string& host = "0.0.0.0", uint32_t backlog = 1024) {
		//TcpSession创建器，通过它创建不同类型的服务器
		_sessionMaker = [](const weak_ptr<TcpServer> &server,const Socket::Ptr &sock){
			return std::make_shared<TcpSessionHelper>(server,std::make_shared<SessionType>(sock));
		};

		if (!_socket->listen(port, host.c_str(), backlog)) {
			//创建tcp监听失败，可能是由于端口占用或权限问题
			string err = (StrPrinter << "listen on " << host << ":" << port << " failed:" << get_uv_errmsg(true));
			throw std::runtime_error(err);
		}

		//新建一个定时器定时管理这些tcp会话
		_timer = std::make_shared<Timer>(2, [this]()->bool {
			this->onManagerSession();
			return true;
		},_poller);
		InfoL << "TCP Server listening on " << host << ":" << port;
	}

	//定时管理Session
	void onManagerSession() {
		for (auto &pr : _sessionMap) {
			weak_ptr<TcpSession> weakSession = pr.second->session();
			pr.second->session()->async([weakSession]() {
				auto strongSession=weakSession.lock();
				if(!strongSession) {
					return;
				}
				strongSession->onManager();
			}, false);
		}
	}
private:
    EventPoller::Ptr _poller;
    Socket::Ptr _socket;
    std::shared_ptr<Timer> _timer;
	unordered_map<string, TcpSessionHelper::Ptr > _sessionMap;
    function<TcpSessionHelper::Ptr(const weak_ptr<TcpServer> &server,const Socket::Ptr &)> _sessionMaker;
	unordered_map<EventPoller *,Ptr> _clonedServer;
    bool _cloned = false;
};

} /* namespace toolkit */

#endif /* TCPSERVER_TCPSERVER_H */
