#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <zmq.hpp>
#include <zmq_utils.h> 

#include <map>
#include <string>

#include "reactor.hpp"

using namespace ZMQ_REACTOR; 


template <class T>
struct ReactorEvent1
{
ReactorEvent1(T& t) : bits_(t)
{}
	void operator() (zmq::socket_t* s) 
	{
		zmq::message_t query;
		s->recv (&query);
		const char *query_string = (const char *)query.data ();
		printf ("Received query: '%s'\n", query_string);
		std::string r;
		if((bits_.count(query_string[0]) > 0) && bits_[query_string[0]])
		{
			printf ("Hit an existing one '%s'\n", query_string);
			r = "already set";
		}
		else
		{
			r = "set";
			bits_[query_string[0]] = true;
		}
		zmq::message_t  resultset(r.size());
		memcpy (resultset.data (),  &r[0], r.size());
		 
		s->send (resultset);
	}
private:
T& bits_;
};


template <class T>
struct ReactorEvent2
{
ReactorEvent2(T& t) : bits_(t)
{}
	void operator() (zmq::socket_t* s) 
	{
		zmq::message_t query;
		s->recv (&query);
 		const char *query_string = (const char *)query.data ();
		printf ("Reactor2 receives: '%s'\n", query_string);

	 	bits_[query_string[0]] = false;
	 	zmq::message_t t ;
		 
		s->send (t);
	}
private:
T& bits_;
};



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

	reactor<PollEventInterface> r;


	ReactorEvent1<BITS> e1(bits);
	PollEventInterface pe1(&e1);
	ReactorEvent2<BITS> e2(bits);
	PollEventInterface pe2(&e2);
	r.add(s, ZMQ_POLLIN, &pe1);
	r.add(s1, ZMQ_POLLIN, &pe2);
	
	while (1) 
	{
		if (bits.count('a') && bits['a'] )
		{
			bool t = r.add(s1, ZMQ_POLLIN, &pe2, true);
			printf ("added pe2: %d\n", t);
		}
		if (bits.count('x') && !bits['x'] )
		{
			bits['x'] = true;
			r.remove(s1);
			printf ("removed \n");
		}
		int ret = r();
		printf ("ret: '%d'\n", ret);
		if (ret == -1)
			break;
	}
}
catch (std::exception &e) {
        printf ("An error occurred: %s\n", e.what());
        return 1;
    }
}
