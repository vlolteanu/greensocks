#include <stdlib.h>
#include "../core/poller.hh"
#include "proxyupstreamer.hh"
#include "proxy.hh"

using namespace std;

void Proxy::handleNewConnection(int fd)
{
	try
	{
		poller->assign(new ProxyUpstreamer(this, &fd, serverCtx));
	}
	catch (...)
	{
		close(fd); // tolerable error
	}
}

SyncedTokenBank *Proxy::createBank(const string &user, uint32_t size)
{
	ScopedSpinlock lock(&bankLock); (void)lock;
	SyncedTokenBank *bank = new SyncedTokenBank((uint32_t)rand(), size, 0, size / 2);
	
	banks[user] = unique_ptr<SyncedTokenBank>(bank);
	return bank;
}

SyncedTokenBank *Proxy::getBank(const string &user)
{
	ScopedSpinlock lock(&bankLock); (void)lock;
	
	return banks[user].get();
}
