// FWFunctor -- small by-value callback slots. The reference binds C++ member functions with heap-allocated `TFunctor<...>` objects
// (0x20 bytes: vtable, object, member pointer {ptr, adj}; A1 Q15, Q15f). The clone keeps the same call shapes but stores the
// binding by value (object pointer + thunk), so there is nothing to free (the 10.13 `operator delete(NULL)` panic, Q-SDK5, cannot
// be reached) and the owners need no destructors.
#ifndef OA_FWFUNCTOR_H
#define OA_FWFUNCTOR_H

#include <libkern/OSTypes.h>
#include <IOKit/IOReturn.h>

class OSObject;

// (index, value) -> IOReturn : per-element / per-channel setters, ref TFunctor<int(uint,uint)>
struct FWCall2 {
    void *obj;
    IOReturn (*fn)(void *, UInt32, UInt32);
    FWCall2() : obj(0), fn(0) {}
    bool isSet() const { return fn != 0; }
    IOReturn operator()(UInt32 a, UInt32 b) const { return fn ? fn(obj, a, b) : kIOReturnSuccess; }
};
// value -> IOReturn : 32-bit number properties, ref TFunctor<int(uint)>
struct FWCall1 {
    void *obj;
    IOReturn (*fn)(void *, UInt32);
    FWCall1() : obj(0), fn(0) {}
    bool isSet() const { return fn != 0; }
    IOReturn operator()(UInt32 a) const { return fn ? fn(obj, a) : kIOReturnSuccess; }
};
// 64-bit value -> IOReturn : ref TFunctor<int(unsigned long long)>
struct FWCall64 {
    void *obj;
    IOReturn (*fn)(void *, UInt64);
    FWCall64() : obj(0), fn(0) {}
    bool isSet() const { return fn != 0; }
    IOReturn operator()(UInt64 a) const { return fn ? fn(obj, a) : kIOReturnSuccess; }
};
// OSObject* -> IOReturn : function properties, ref TFunctor<int(OSObject*)>
struct FWCallObj {
    void *obj;
    IOReturn (*fn)(void *, OSObject *);
    FWCallObj() : obj(0), fn(0) {}
    bool isSet() const { return fn != 0; }
    IOReturn operator()(OSObject *a) const { return fn ? fn(obj, a) : kIOReturnSuccess; }
};
// () : "all changed" notifications, ref TFunctor<void()>
struct FWCall0 {
    void *obj;
    void (*fn)(void *);
    FWCall0() : obj(0), fn(0) {}
    bool isSet() const { return fn != 0; }
    void operator()() const { if (fn) fn(obj); }
};
// (register byte offset, quadlet count, data) : the batch mixer writer, ref TFunctor<int(uint,uint,void*)>
struct FWCallBatch {
    void *obj;
    IOReturn (*fn)(void *, UInt32, UInt32, void *);
    FWCallBatch() : obj(0), fn(0) {}
    bool isSet() const { return fn != 0; }
    IOReturn operator()(UInt32 a, UInt32 b, void *c) const { return fn ? fn(obj, a, b, c) : kIOReturnSuccess; }
};

template <class T, IOReturn (T::*M)(UInt32, UInt32)>
static IOReturn FWThunk2(void *o, UInt32 a, UInt32 b) { return (static_cast<T *>(o)->*M)(a, b); }
template <class T, IOReturn (T::*M)(UInt32, UInt32)>
static FWCall2 FWBind2(T *o) { FWCall2 c; c.obj = o; c.fn = &FWThunk2<T, M>; return c; }

template <class T, IOReturn (T::*M)(UInt32)>
static IOReturn FWThunk1(void *o, UInt32 a) { return (static_cast<T *>(o)->*M)(a); }
template <class T, IOReturn (T::*M)(UInt32)>
static FWCall1 FWBind1(T *o) { FWCall1 c; c.obj = o; c.fn = &FWThunk1<T, M>; return c; }

template <class T, IOReturn (T::*M)(UInt64)>
static IOReturn FWThunk64(void *o, UInt64 a) { return (static_cast<T *>(o)->*M)(a); }
template <class T, IOReturn (T::*M)(UInt64)>
static FWCall64 FWBind64(T *o) { FWCall64 c; c.obj = o; c.fn = &FWThunk64<T, M>; return c; }

template <class T, IOReturn (T::*M)(OSObject *)>
static IOReturn FWThunkObj(void *o, OSObject *a) { return (static_cast<T *>(o)->*M)(a); }
template <class T, IOReturn (T::*M)(OSObject *)>
static FWCallObj FWBindObj(T *o) { FWCallObj c; c.obj = o; c.fn = &FWThunkObj<T, M>; return c; }

template <class T, void (T::*M)()>
static void FWThunk0(void *o) { (static_cast<T *>(o)->*M)(); }
template <class T, void (T::*M)()>
static FWCall0 FWBind0(T *o) { FWCall0 c; c.obj = o; c.fn = &FWThunk0<T, M>; return c; }

template <class T, IOReturn (T::*M)(UInt32, UInt32, void *)>
static IOReturn FWThunkBatch(void *o, UInt32 a, UInt32 b, void *c) { return (static_cast<T *>(o)->*M)(a, b, c); }
template <class T, IOReturn (T::*M)(UInt32, UInt32, void *)>
static FWCallBatch FWBindBatch(T *o) { FWCallBatch c; c.obj = o; c.fn = &FWThunkBatch<T, M>; return c; }

#endif
