#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <zmq.hpp>
#include <zmq_utils.h> 

#include <map>
#include <string>

#include "reactor.hpp"

using namespace ZMQ_REACTOR; 

typedef void (EVT_OP)(zmq::socket_t* s);

struct IReactorEvent
{
virtual void operator()(zmq::socket_t* s)  = 0;
virtual ~IReactorEvent(){};
};

void op1 (zmq::socket_t* s)
{
	zmq::message_t query;
	s->recv (&query);
 	 
	const char *query_string = (const char *)query.data ();
	printf ("Received stage 1: '%s'\n", query_string);
	std::string r("from stage 1");

	zmq::message_t  resultset(r.size());
	memcpy (resultset.data (),  &r[0], r.size());
	 
	s->send (resultset);
}

void op2 (zmq::socket_t* s)
{
	zmq::message_t query;
	s->recv (&query);

	const char *query_string = (const char *)query.data ();
	printf ("Received stage 2: '%s'\n", query_string);

	std::string r("from stage 2");
	zmq::message_t  resultset(r.size());
	memcpy (resultset.data (),  &r[0], r.size());

	s->send (resultset);
}

struct ReactorEvent1 : IReactorEvent
{
	virtual void operator() (zmq::socket_t* s) 
	{
		op1(s);
	}
};


struct ReactorEvent2 : IReactorEvent
{
	virtual void operator() (zmq::socket_t* s) 
	{
		op2(s);
	}
};

void runOp(zmq::socket_t& s, zmq::socket_t& s1)
{
	reactor<EVT_OP> r;
	r.add(s, ZMQ_POLLIN, &op1);
	r.add(s1, ZMQ_POLLIN, &op2);
	while (1) {
		r();
	}
}

void runInterface(zmq::socket_t& s, zmq::socket_t& s1)
{
	reactor<IReactorEvent> r;
	ReactorEvent1 e1;
	r.add(s, ZMQ_POLLIN, &e1);
	ReactorEvent2 e2;
	r.add(s1, ZMQ_POLLIN, &e2);
	while (1) {
		r();
	}
}

typedef std::map<char, bool> BITS;
int main (int argc, const char* argv[]) 
{
 
 try
 {
	/* Initialise 0MQ context, requesting a single I/O thread */
	BITS bits;
	zmq::context_t ctx (1);

	/* Create a ZMQ_REP socket to receive requests and send replies */
	zmq::socket_t s (ctx, ZMQ_REP);
	zmq::socket_t s1 (ctx, ZMQ_REP);

	/* Bind to the TCP transport and port 5555 on the 'lo' interface */
	s.bind ("tcp://lo:5555");
	s1.bind ("tcp://lo:5556");

	argc > 1 ? runInterface(s, s1) : runOp(s, s1);
}
	catch (std::exception &e) {
	// 0MQ throws standard exceptions just like any other C++ API
	printf ("An error occurred: %s\n", e.what());
	return 1;
}
}
