
#include "Trace.h"

Trace::Trace(Print* swSer) {
#ifdef TRACE
	_swSer = swSer;
	swSer->println(F("*"));
	swSer->println(F("*"));
	swSer->println(F("TraceInit ready"));
#endif
}

template <typename Generic> void Trace::Traceln(Generic text)
{
#ifdef TRACE
	_swSer->println(text);
#endif
};

template <typename Generic> void Trace::Trace(Generic text)
{
#ifdef TRACE
	_swSer->print(text);
#endif
};