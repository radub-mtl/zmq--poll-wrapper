#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <zmq.hpp>
#include <zmq_utils.h> 

#include <map>
#include <string>

#include "reactor.hpp"

typedef std::map<char, bool> BITS;

struct State
{
	typedef void (*EVT_H) (zmq::socket_t* s, State* bits_);
	EVT_H op;
	zmq::socket_t* s1;
	BITS bits_;
};
template <class STATE>
void eventH1 (zmq::socket_t* s, STATE* state) 
{
	zmq::message_t query;
	s->recv (&query);
	const char *query_string = (const char *)query.data ();
	printf ("Received query: '%s'\n", query_string);
	std::string r;
	if((state->bits_.count(query_string[0]) > 0) && state->bits_[query_string[0]])
	{
		printf ("Hit an existing one '%s'\n", query_string);
		r = "already set";
	}
	else
	{
		r = "set";
		state->bits_[query_string[0]] = true;
	}
	zmq::message_t  resultset(r.size());
	memcpy (resultset.data (),  &r[0], r.size());
	 
	s->send (resultset);
}


template <class STATE>
void eventH2 (zmq::socket_t* s, STATE* state) 
{
	zmq::message_t query;
	s->recv (&query);
	const char *query_string = (const char *)query.data ();
	printf ("Reactor2 receives: '%s'\n", query_string);

 	state->bits_[query_string[0]] = false;
 	zmq::message_t t ;
	 
	s->send (t);
}

template <class STATE, class  R>
int begin (STATE* state, R& r, int& unused) 
{
	if (state->bits_.count('a') && state->bits_['a'] )
	{
		bool t = r.add((zmq::socket_t&)*state->s1, ZMQ_POLLIN, state->op);
		printf ("added pe2: %d\n", t);
	}
	if (state->bits_.count('x') && !state->bits_['x'] )
	{
		state->bits_['x'] = true;
		r.remove((zmq::socket_t&)*state->s1);
		printf ("removed \n");
	}
	return 0;
}





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
	
	typedef void (eventH) (zmq::socket_t* s, State* bits_);
	typedef reactor<eventH> REACTOR;
	typedef int (*BEGIN)(State*, REACTOR&, int&);

	REACTOR r;

	r.add(s, ZMQ_POLLIN, &eventH1);
	
	State state;
	state.s1 = &s1;
	state.op = &eventH2;
	BEGIN b = &begin;
	
	int ret = r.run( &state, -1,  b);
	printf ("ret: '%d'\n", ret);
}
catch (std::exception &e) {
        printf ("An error occurred: %s\n", e.what());
        return 1;
    }
}
