#ifndef TLS_HH
#define TLS_HH

#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include <socks6util/socks6util.hh>
#include "tlscontext.hh"
#include "tlssession.hh"
#include "streambuffer.hh"

class Proxifier;

class TLS: public boost::intrusive_ref_counter<TLS>
{
	int rfd;
	int wfd;

	WOLFSSL *readTLS;
	WOLFSSL *writeTLS;

	bool connectCalled;
	
	static int sessionTicketCallback(WOLFSSL* ssl, const unsigned char* ticket, int ticketSz, void* ctx);

	TLSSession *session;

public:
	TLS(TLSContext *ctx, int fd, TLSSession *session);
	
	~TLS();
	
	void setReadFD(int fd);
	
	void setWriteFD(int fd);
	
	void tlsConnect(S6U::SocketAddress *addr, StreamBuffer *buf, bool useEarlyData);
	
	void tlsAccept(StreamBuffer *buf);
	
	size_t tlsWrite(StreamBuffer *buf);
	
	size_t tlsRead(StreamBuffer *buf);
};

#endif // TLS_HH
