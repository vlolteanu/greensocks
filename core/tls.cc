#include <stdexcept>
#include "rescheduleexception.hh"
#include "tlsexception.hh"
#include "poller.hh"
#include "tls.hh"

using namespace std;

static void tlsHandleErr(WOLFSSL *tls, int rc, int fd)
{
	int err = wolfSSL_get_error((tls), (rc));
	if (err == WOLFSSL_ERROR_WANT_READ)
		throw RescheduleException((fd), Poller::IN_EVENTS);
	if (err == WOLFSSL_ERROR_WANT_WRITE)
		throw RescheduleException((fd), Poller::OUT_EVENTS);
	throw TLSException(err);
}

int TLS::sessionTicketCallback(WOLFSSL *ssl, const unsigned char *ticket, int ticketSz, void *ctx)
{
	(void)ticket; (void)ticketSz;

	TLSSession *session = reinterpret_cast<TLSSession *>(ctx);
	session->update(ssl); //TODO: maybe session hasn't changed; need to memcpy new ticket

	return 0;
}

TLS::TLS(TLSContext *ctx, int fd)
	: rfd(fd), wfd(fd)
{
	readTLS = wolfSSL_new(ctx->get());
	if (readTLS == NULL)
		throw runtime_error("Error creating context");
	writeTLS = readTLS;
		
	wolfSSL_SetIOWriteFlags(readTLS, MSG_NOSIGNAL);
	
	static const int CERT_VERIFY_DEPTH = 3;
	if (ctx->isClient())
	{
		wolfSSL_set_verify_depth(readTLS, CERT_VERIFY_DEPTH);
		session->apply(readTLS);
	}
		
	try
	{
		int rc = wolfSSL_set_fd(readTLS, fd);
		if (rc != SSL_SUCCESS)
			throw TLSException(readTLS, rc);

//		if (ctx->isClient())
//		{
//			rc = wolfSSL_set_SessionTicket_cb(readTLS, sessionTicketCallback, session);
//			if (rc != SSL_SUCCESS)
//				throw TLSException(readTLS, rc);
//		}
	}
	catch (std::exception &ex)
	{
		wolfSSL_free(readTLS);
		throw ex;
	}
}

TLS::~TLS()
{
	if (readTLS == writeTLS)
		writeTLS = NULL;
	
	wolfSSL_free(writeTLS);
	wolfSSL_free(readTLS);
}

void TLS::setReadFD(int fd)
{
	if (readTLS == writeTLS)
	{
		writeTLS = wolfSSL_write_dup(readTLS);
		if (writeTLS == NULL)
			throw runtime_error("Error duplicating WOLFSSL");
	}
	rfd = fd;
	int rc = wolfSSL_set_read_fd(readTLS, fd);
	if (rc != SSL_SUCCESS)
		throw TLSException(readTLS, rc);
}

void TLS::setWriteFD(int fd)
{
	if (readTLS == writeTLS)
	{
		writeTLS = wolfSSL_write_dup(readTLS);
		if (writeTLS == NULL)
			throw runtime_error("Error duplicating WOLFSSL");
	}
	wfd = fd;
	int rc = wolfSSL_set_write_fd(writeTLS, fd);
	if (rc != SSL_SUCCESS)
		throw TLSException(writeTLS, rc);
}

void TLS::tlsConnect(S6U::SocketAddress *addr, StreamBuffer *buf, bool useEarlyData)
{
	int rc;
	int earlyDataWritten = 0;
	
	useEarlyData = useEarlyData && addr != NULL && buf != NULL;
	
	bool firstConnnect = !connectCalled;
	connectCalled = true;

	if (firstConnnect && addr != NULL)
		wolfSSL_SetTFOAddr(readTLS, &addr->storage, addr->size());

	if (firstConnnect && useEarlyData && buf->usedSize() > 0)
	{
		rc = wolfSSL_write_early_data(readTLS, buf->getHead(), buf->usedSize(), &earlyDataWritten);
		if (rc < 0)
			tlsHandleErr(readTLS, rc, rfd);
	}
	else
	{
		rc = wolfSSL_connect(readTLS);
		if (rc != SSL_SUCCESS)
			tlsHandleErr(readTLS, rc, rfd);
	}
	
	if (useEarlyData)
		buf->unuse(earlyDataWritten);
}

void TLS::tlsAccept(StreamBuffer *buf)
{
	//SECStatusCheck(SSL_OptionSetDefault(SSL_ENABLE_0RTT_DATA, PR_TRUE)); //TODO: move to TLSConnect
	
	int earlyDataRead = 0;
	int rc = wolfSSL_read_early_data(readTLS, buf->getTail(), buf->availSize(), &earlyDataRead);
	if (rc < 0)
		tlsHandleErr(writeTLS, rc, rfd);
	
	buf->use(earlyDataRead);
}

size_t TLS::tlsWrite(StreamBuffer *buf)
{
	int bytes = wolfSSL_write(writeTLS, buf->getHead(), buf->usedSize());
	if (bytes < 0)
		tlsHandleErr(writeTLS, bytes, wfd);
	
	buf->unuse(bytes);
	return bytes;
}

size_t TLS::tlsRead(StreamBuffer *buf)
{
	int bytes = wolfSSL_read(readTLS, buf->getTail(), buf->availSize());
	if (bytes < 0)
		tlsHandleErr(readTLS, bytes, rfd);

//	if (session && !wolfSSL_session_reused(readTLS))
//		session->update(readTLS);
	session = NULL;
	
	buf->use(bytes);
	return bytes;
}

TLS::Descriptor::Descriptor(int fd)
{
	fileDescriptor = PR_ImportTCPSocket(fd);
	if (!fileDescriptor)
		throw TLSException();
	PRFileDesc *newDesc = SSL_ImportFD(NULL, fileDescriptor);
	if (!newDesc)
	{
		PR_Close(fileDescriptor); //might return error
		throw TLSException();
	}
	fileDescriptor = newDesc;
}

TLS::Descriptor::~Descriptor()
{
	PR_Close(fileDescriptor); //might return error
}
