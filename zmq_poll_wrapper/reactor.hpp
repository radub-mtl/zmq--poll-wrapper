
/*
 * ----------------------------------------------------------
 *
 * Copyright 2010 Radu Braniste
 *
 * ----------------------------------------------------------
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 */


#ifndef reactor_hpp
#define reactor_hpp


#include <zmq.hpp>
#include <zmq_utils.h> 
#include <vector>
#include "OneMethodOnePointerParamInterface.hpp"
#include <algorithm>

typedef OneMthdOneParamPtrInterface<zmq::socket_t> PollEventInterface;


template <class T>
void set (T& t, zmq_pollitem_t& item)
{
	item.fd = t;
}
template <>
void set<zmq::socket_t> (zmq::socket_t& v, zmq_pollitem_t& item)
{
	item.socket = v;
}

template <class T, class V, class R>
void trait(T& t, V* v, R* r)
{
	t(v, r); 
};
template <class T, class V>
void trait(T& t, V* v, void* r)
{
	t(v); 
};


template <class T, class V = zmq::socket_t>
struct reactor
{
	bool add(V& v, short event, T* t, bool checkIfSocketAddedTwice = 0)
	{
		zmq_pollitem_t item = {0,0,0,0};
		set(v, item);
		item.events = event;
		return addImpl(item, v, t, checkIfSocketAddedTwice);
	}
	bool remove(V& v)
	{
		return removeImpl(v);
	}
	template <typename R>
	int operator()(R* r = 0, int timeout = -1)
	{
		int ret = zmq::poll (&items_[0], items_.size(), timeout);
		if ((ret == 0) || (ret == -1) )
			return ret;

		auto c = callbacks_.begin(); //outside "for" to keep VS2010 happy 
		auto sk = socks_.begin();
		for (auto i = items_.begin(); i != items_.end(); ++i, ++c, ++sk)
		{
			if (i->revents & i->events)
			{	
				T& t = *((T*)*c);
				V* v = *sk;
				trait(t,v, r);
			}
		}
		return ret;
	}
	int operator()( int timeout = -1) //  overload to keep VS2010 happy - no defaults for functions
	{
		return this->operator()((void*)0, timeout);
	}
	
	template <typename R>
	int run(R* r = 0,  int timeout = -1, int (*begin)(R*, reactor&, int&) = 0, int (*end)(R*, reactor&, int&, int) = 0)
	{
		while (1)
		{
			int tmout = timeout;
			if (begin)
			{
				int ret = begin(r, *this, tmout);
				if (ret) return ret;
			}
			int ret = this->operator()(r, tmout);
			if (ret == -1)
				return ret;
			if (end)
			{
				int ret = end(r, *this, tmout, ret);
				if (ret) return ret;
			}
		}
	}
	int run(int timeout = -1) //  overload to keep VS2010 happy - no defaults for functions
	{
		return run((void*)0, timeout, 0, 0);
	}
	
private:
	template <class K, class TT>
	static int getIndex(const K& k, TT* t)
	{
	  	auto it = std::find( k.begin(), k.end(), t );
    		return it == k.end() ? -1 : it - k.begin();
	}
	bool addImpl(zmq_pollitem_t& item, V& v, T* t, bool checkIfSocketAddedTwice)
	{
		//if added twice it hangs
		if (checkIfSocketAddedTwice && (getIndex(socks_, &v) > -1) )
			return false;
		items_.push_back(item);
		callbacks_.push_back(t);
		socks_.push_back(&v);
		return true;
	}

	template <class K>
	static void removeImpl( K& k, size_t pos)
	{
		k.erase(k.begin() + pos);
	}
	
	bool removeImpl(V& v)
	{
		int pos = getIndex(socks_, &v);
		if (pos == -1)
			return false;
		removeImpl(items_, pos);
		removeImpl(callbacks_, pos);
		removeImpl(socks_, pos);
		return true;
	}
private:
	std::vector<zmq_pollitem_t> items_;
	std::vector<T*> callbacks_;
	std::vector<V*> socks_;
	bool checkIfSocketAddedTwice_;
};



#endif

