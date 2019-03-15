#ifndef PROXYUPSTREAMER_HH
#define PROXYUPSTREAMER_HH

#include <memory>
#include <boost/intrusive_ptr.hpp>
#include <socks6msg/socks6msg.hh>
#include "core/streamreactor.hh"
#include "core/spinlock.hh"

class Proxy;
class ConnectProxyDownstreamer;
class AuthServer;

class ProxyUpstreamer: public StreamReactor
{
	const size_t MSS = 1460; //TODO: don't hardcode

	enum State
	{
		S_HANDSHAKE,
		S_READING_REQ,
		S_READING_TFO_PAYLOAD,
		S_AWAITING_AUTH,
		S_CONNECTING,
		S_STREAM,
	};

	boost::intrusive_ptr<Proxy> proxy;
	
	volatile State state = S_HANDSHAKE;
	volatile bool authenticated = false;
	size_t tfoPayload;
	
	std::shared_ptr<S6M::Request> request;
	S6M::OptionSet replyOptions { S6M::OptionSet::M_OP_REP };

	boost::intrusive_ptr<ConnectProxyDownstreamer> downstreamer;
	
	AuthServer *authServer = nullptr;
	Spinlock honorLock;
	
	bool mustFail = false;
	
	void honorRequest();

	void honorConnect();

	void honorConnectStackOptions();
	
public:
	ProxyUpstreamer(Proxy *proxy, int *pSrcFD, TLSContext *serverCtx);
	
	void start();
	
	void process(int fd, uint32_t events);
	
	void authDone(SOCKS6TokenExpenditureCode expenditureCode);

	std::shared_ptr<S6M::Request> getRequest() const
	{
		return request;
	}
	
	Proxy *getProxy()
	{
		return proxy.get();
	}
	
	void fail()
	{
		mustFail = true;
	}
	
	class SimpleReplyException: public std::exception
	{
		SOCKS6OperationReplyCode code;
		
	public:
		SimpleReplyException(SOCKS6OperationReplyCode code)
			: code(code) {}
		
		SOCKS6OperationReplyCode getCode() const
		{
			return code;
		}
	};
};

#endif // PROXYUPSTREAMER_HH
