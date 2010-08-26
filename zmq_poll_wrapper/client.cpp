#include <stdio.h>
#include <zmq.hpp>
#include <string>
#include <zmq_utils.h> 
 
int main (int argc, const char* argv[])
{
    try {
        // Initialise 0MQ context with one I/O thread
        zmq::context_t ctx (1);
        // Create a ZMQ_REQ socket to send requests and receive replies
        zmq::socket_t s (ctx, ZMQ_REQ);
        zmq::socket_t s1 (ctx, ZMQ_REQ);
        // Connect it to port 5555 on localhost using the TCP transport
 
        // Construct an example zmq::message_t with our query
        const char *query_string = argv[1];

        s.connect ("tcp://localhost:5555");

	char cc = '0';
        
        do 
        {
		zmq::message_t query (strlen (query_string) + 1);
		memcpy (query.data (), query_string, strlen (query_string) + 1);
		// Send the query
		s.send (query);
	 	//printf ("send: '%s'\n", query_string);
		// Receive and display the result
		zmq::message_t resultset;
		s.recv (&resultset);
		const char *resultset_string = (const char *)resultset.data ();
		printf ("Received response: '%s'\n", resultset_string);
		cc = resultset_string[0];
        }while ( cc == '1');
        
        zmq_sleep(1);
        
        zmq::message_t w (strlen (query_string) + 1);
        memcpy (w.data (), query_string, strlen (query_string) + 1);
        // Send the query
        s1.connect ("tcp://localhost:5556");
        s1.send (w);
        zmq::message_t resultset;
	s1.recv (&resultset);
	printf ("Received response: '%s'\n", (const char *)resultset.data ());
        
    }
    catch (std::exception &e) {
        // 0MQ throws standard exceptions just like any other C++ API
        printf ("An error occurred: %s\n", e.what());
        return 1;
    }
 
    return 0;
}
