#ifndef PROXIFIERUPSTREAMER_HH
#define PROXIFIERUPSTREAMER_HH

#include "../core/streamreactor.hh"

class Proxifier;
class ProxifierDownstreamer;

class ProxifierUpstreamer: public StreamReactor
{
	enum State
	{
		S_READING_INIT_DATA,
		S_SENDING_REQ,
		S_STREAM,
	};
	
	ssize_t reqBytesLeft;
	
	Proxifier *owner;
	
	State state;
	
public:
	ProxifierUpstreamer(Proxifier *owner, int srcFD);

	void process();
	
	Proxifier *getOwner() const
	{
		return owner;
	}
};

#endif // PROXIFIERUPSTREAMER_HH
