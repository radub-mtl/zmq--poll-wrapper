#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <zmq.hpp>
#include <zmq_utils.h> 

#include <map>
#include <string>

#include "reactor.hpp"



struct State
{
	int k;
};

typedef void (EVT_OP)(zmq::socket_t* , State* );

void op1 (zmq::socket_t* s, State* state)
{
	zmq::message_t query;
	s->recv (&query);

	const char *query_string = (const char *)query.data ();
	printf ("Received stage 1: '%s'\n", query_string);
	std::string r("from stage 1");

	zmq::message_t  resultset(r.size());
	memcpy (resultset.data (),  &r[0], r.size());

	state->k++;
	s->send (resultset);
}

void op2 (zmq::socket_t* s, State* state)
{
	zmq::message_t query;
	s->recv (&query);

	const char *query_string = (const char *)query.data ();
	printf ("Received stage 2: '%s'\n", query_string);

	printf ("state: '%d'\n", state->k);

	std::string r("return from stage 2");
	zmq::message_t  resultset(r.size());
	memcpy (resultset.data (),  &r[0], r.size());

	s->send (resultset);
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

	reactor<EVT_OP> r;
	r.add(s, ZMQ_POLLIN, &op1);
	r.add(s1, ZMQ_POLLIN, &op2);
	
	State state;
	state.k = 10;
	
	while (1) 
	{
		r(&state);
	}
}
	catch (std::exception &e) {
	// 0MQ throws standard exceptions just like any other C++ API
	printf ("An error occurred: %s\n", e.what());
	return 1;
}
}
