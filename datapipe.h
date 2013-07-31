// connect.h

#ifndef CONNECT_H
#define CONNECT_H


// === data source ==========================================================

// The inheritor must define ReadLast and Read
template <class T>
class CSource
{
	virtual T ReadLast() = 0;
	virtual T Read() = 0;
public:
	virtual ~CSource() {}

	template <class S> friend class CSink;
};

// Null source for not connected sinks
template <class T>
class CNullSource : public CSource<T>
{
protected:
	CNullSource() {}
	CNullSource(const CNullSource&) {}
	~CNullSource() {}
	T ReadLast() { throw "no connection"; }
	T Read()     { return ReadLast();     }
	template <class S> friend class CSink;
};


// === data sink ============================================================

template <class T>
class CSink
{
public:
	CSource<T> *src;
	static CNullSource<T> null;
public:
	CSink() : src(&null) {}

	T GetLast() { return src->ReadLast(); }
	T Get()     { return src->Read();     }
	void GetAll() { while (true) Get(); }
};

template <class T> CNullSource<T> CSink<T>::null;


// === data in -> out (pipe) ================================================

template <class TI, class TO=TI>
class CDataPipe : public CSink<TI>, public CSource<TO> {};


// === operators to connect pipes ===========================================

// source -> sink; source -> datapipe
template <class TI, class TO>
void operator >> (CSource<TI> &in, CSink<TO> &out)
{
	out.src = &in;
}

// source -> datapipe -> datapipe -> sink
template <class TI, class TO>
CSource<TO>& operator >> (CSource<TI> &in, CDataPipe<TI,TO> &out)
{
	out.src = &in;
	return out;
}


#endif
