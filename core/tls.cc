#include <stdexcept>
#include "tls.hh"

using namespace std;

TLS::TLS(WOLFSSL_CTX *ctx, int fd)
	: rfd(fd), wfd(fd)
{
	readTLS.assign(wolfSSL_new(ctx));
	if (readTLS == NULL)
		throw runtime_error("Error creating context");
	
}

void TLS::tlsConnect(S6U::SocketAddress addr, StreamBuffer *buf, bool useEarlyData)
{
//	if (useEarlyData)
//		wolfSSL_write_early_data(readTLS,)
		
//	wolfSSL_connect(readTLS);
}

void TLS::tlsAccept()
{
	
}

void TLS::tlsWrite(StreamBuffer *buf)
{
	
}

void TLS::tlsRead(StreamBuffer *buf)
{
	
}