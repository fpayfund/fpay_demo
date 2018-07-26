#ifndef __SERVICE_CONTEXT_H__
#define __SERVICE_CONTEXT_H__

#include <memory>
#include "base.h"
#include "typedef.h"
#include "ByteStream.h"

struct FormEntry;
struct IConn;
struct Request;

namespace proto {
    class Marshallable;
    class BinReadStream;
}

struct IWriter
{
    virtual ~IWriter(){}
    virtual void answer(uri_t uri, const proto::Marshallable &obj) = 0;

    virtual void stop() = 0;
    virtual int flush(IConn *conn) = 0;
};

struct IWriterAware
{
protected:
    IWriter *_writer;
public:
    IWriterAware()
        : _writer(NULL)
    {}
    void setWriter(IWriter *w)
    {
        _writer = w;
    }
    inline IWriter *getWriter()
    {
        return _writer;
    }
};

struct ProtoHandlerClass
{
};

struct IFormTarget: public IWriterAware
{
    void answer(uri_t u, const proto::Marshallable &obj)
    {
        _writer->answer(u, obj);
    }
};

typedef void (ProtoHandlerClass::*TargetFunc)();
enum FormProcType {
    fpt_vc,           // void ::(Cmd*);
    fpt_vuc,           // void ::(uid_t, Cmd*);
    fpt_vkc,          // void ::(rkey_t, Cmd*);
    fpt_vcc,          // void ::(Cmd *, Conn* );
    fpt_null,         // void ::(Cmd *, Conn* );
    ftp_last,
};

#define ON_REQUEST(hClass, fp)  {hClass::uri, WrapFormHandle(new FormHandleT<hClass>()), fpt_vc, \
        (TargetFunc)(static_cast<void (ProtoHandlerClass::*)(hClass *)>(fp)), NULL},

#define ON_UREQUEST(hClass, fp)  {hClass::uri, WrapFormHandle(new FormHandleT<hClass>()), fpt_vuc, \
        (TargetFunc)(static_cast<void (ProtoHandlerClass::*)(uid_t, hClass *)>(fp)), NULL},

#define ON_KREQUEST(hClass, fp)  {hClass::uri, WrapFormHandle(new FormHandleT<hClass>()), fpt_vkc, \
        (TargetFunc)(static_cast<void (ProtoHandlerClass::*)(rkey_t, hClass *)>(fp)), NULL},

#define ON_LINK(hClass, fp) {hClass::uri, WrapFormHandle(new FormHandleT<hClass>()), fpt_vcc, \
        (TargetFunc)(static_cast<void (ProtoHandlerClass::*)(hClass *, IConn*)>(fp)), NULL},

union TargetProc
{
    TargetFunc mf_oo;
    void (ProtoHandlerClass::*mf_vc)(void *); 
    void (ProtoHandlerClass::*mf_vuc)(uid_t, void *); 
    void (ProtoHandlerClass::*mf_vkc)(rkey_t, void *); 
    void (ProtoHandlerClass::*mf_vcc)(void *, IConn*);
};

struct IFormHandle 
{
    virtual ~IFormHandle() {}
    virtual void *handlePacket(proto::BinReadStream &up) = 0; 
    virtual void destroyForm(void *form) = 0;
};

struct WrapFormHandle
{
    IFormHandle *_f;
    WrapFormHandle(IFormHandle *f)
        : _f(f)
    {}
    WrapFormHandle()
    {
        _f = NULL;
    }
    ~WrapFormHandle()
    {
        delete _f;
    }
    inline IFormHandle * get()
    {
        return _f;
    }
    inline void reset()
    {
        _f = NULL;
    }
};

struct FormEntry
{
    uint32_t uri;
    WrapFormHandle requestForm;
    int type;
    TargetFunc proc;
    ProtoHandlerClass *target;
    void reset()
    {
        requestForm.reset();
    }
};

template<class T> class FormHandleT : public IFormHandle
{
public:
    virtual void *handlePacket(proto::BinReadStream &up)
    {
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8)
        std::unique_ptr<T> obj(new T);
#else
        std::auto_ptr<T> obj(new T);
#endif
        up >> *obj;
        return obj.release();
    }
    virtual void destroyForm(void *cmd)
    {
        T *obj = (T *)cmd;
        if(obj)
            delete obj;
    }
};

#define DECLARE_FORM_MAP static FormEntry *getFormEntries(); \
                        static FormEntry formEntries[];

#define DECLARE_LOCAL_FORM_MAP FormEntry *getLocalFormEntries(); \
                               FormEntry *localFormEntries; \
                               void copyLocalFormEntries(); \
                               void deleteLocalFormEntires();

#define BEGIN_FORM_MAP(theClass) \
    FormEntry* theClass::getFormEntries()\
{ return theClass::formEntries; } \
    FormEntry theClass::formEntries[] = \
{ \


#define END_FORM_MAP() \
{0, WrapFormHandle(NULL), fpt_null, NULL, NULL} \
}; \

class ServiceContext
{
public:
    virtual ~ServiceContext () {}
    virtual IWriter *requestDispatch(Request &request, IConn *conn) = 0;
    virtual void setParent(ServiceContext *p) = 0;
    virtual void addEntry(FormEntry *entries, void *target, IWriterAware *inf) {}
};

struct ServiceContextAware
{
protected:
    ServiceContext *_appContext;
public:
    ServiceContextAware(): _appContext(NULL){}
    virtual ~ServiceContextAware() {}
    virtual void setAppContext(ServiceContext *c)
    {
        _appContext = c;
    }
};

#endif
