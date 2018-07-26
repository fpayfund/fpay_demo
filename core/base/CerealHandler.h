#ifndef __CEREAL_HANDLER_H__
#define __CEREAL_HANDLER_H__

#include <cereal/types/memory.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>

#include "ServiceContext.h"
#include "logging.h"

template<class T>
class CerealMsgHandle : public IFormHandle
{
public:
     virtual void *handlePacket(proto::BinReadStream &up)
     {
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8)
         std::unique_ptr<T> obj(new T); 
#else
         std::auto_ptr<T> obj(new T); 
#endif
         try {
              std::istringstream is(std::string(up.data(), up.size()));
              cereal::JSONInputArchive archive(is);
              archive(*obj);
         } catch (proto::ReadStreamExn & e) {
             throw e;
         }   

         up.reset(NULL, 0); 
         return obj.release();
     }   
     virtual void destroyForm(void *cmd)
     {
         T *obj = (T *)cmd;
         if (obj) {
             delete obj;
         }
     }   
};

#define J_ON_REQUEST(hClass, fp)  {hClass::uri, WrapFormHandle(new CerealMsgHandle<hClass>()), fpt_vc, \
        (TargetFunc)(static_cast<void (ProtoHandlerClass::*)(hClass *)>(fp)), NULL},

#define J_ON_UREQUEST(hClass, fp)  {hClass::uri, WrapFormHandle(new CerealMsgHandle<hClass>()), fpt_vuc, \
        (TargetFunc)(static_cast<void (ProtoHandlerClass::*)(uid_t, hClass *)>(fp)), NULL},

#define J_ON_SREQUEST(hClass, fp)  {hClass::uri, WrapFormHandle(new CerealMsgHandle<hClass>()), fpt_vkc, \
        (TargetFunc)(static_cast<void (ProtoHandlerClass::*)(key_t, hClass *)>(fp)), NULL},

#define J_ON_LINK(hClass, fp) {hClass::uri, WrapFormHandle(new CerealMsgHandle<hClass>()), fpt_vcc, \
        (TargetFunc)(static_cast<void (ProtoHandlerClass::*)(hClass *, IConn*)>(fp)), NULL},
#endif
