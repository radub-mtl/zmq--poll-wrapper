
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

#ifndef OneMethodOnePointerParamInterface_hpp
#define OneMethodOnePointerParamInterface_hpp


template <class S>
struct OneMthdOneParamPtrInterface
{
	template<class T>
	OneMthdOneParamPtrInterface(T* x) : x_(x), table_(vTable_<T>::table)
	{}
	void operator()(S* s)
	{
		table_.event(x_, s);
	}
private:
	struct VTable
	{
		void (*event)(void*, S* s);
	};
	template <class TT>
	struct vTable_
	{
		static VTable table;
		static void event(void* x, S* s)
		{
			static_cast<TT*>(x)->operator()(s);
		}
	};

private:
	VTable table_;
	void* x_;
};
template <class S> template <class TT> typename OneMthdOneParamPtrInterface<S>::VTable
	  OneMthdOneParamPtrInterface<S>::vTable_<TT>::table = 
{&OneMthdOneParamPtrInterface<S>::vTable_< TT>::event};


#endif

