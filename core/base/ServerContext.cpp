#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "ByteStream.h"
#include "logging.h"
#include "ServerContext.h"

using namespace std;
using namespace proto;

void ServerContext::addEntry(FormEntry *es, void *target, IWriterAware *inf)
{
    if (inf) {
        inf->setWriter(getWriter());
    }       

    int i = 0;
    //target maybe null
    while (es[i].uri != 0) {
        if (entries.find(es[i].uri) != entries.end()) {
            assert(false);
        }
        LOG_INFO << "add entry uri:" << (es[i].uri >> 8) << "|" << (0xff & es[i].uri);
        entries[es[i].uri] = &es[i];
        es[i].target = (ProtoHandlerClass *)target;
        ++i;
    }   
}

void ServerContext::DefaultDispatch(Request &request, IConn *conn)
{
    LOG_TRACE;
    int i = request.getUri();

    LOG_WARN <<  "not found request entry: " << i;
}

IWriter * ServerContext::requestDispatch(Request &req, IConn *conn)
{
    LOG_TRACE << "uri:" << req.getUri();
    entry_map_t::iterator it = entries.find((uri_t)req.getUri());//找合适的消息映射入口
    do {
        if (it == entries.end()) {
            DefaultDispatch(req, conn);
            break;
        }
        //找到对应的消息映射入口
        FormEntry *entry = it->second;
        try { 
            req.setFormHandler(entry->requestForm.get());
        } catch (ReadStreamExn& ex) {   
            LOG_WARN << "unpack error exn:" << ex.what() << " uri:" << req.getUri() << " remote_addr:" <<  conn->remote_addr();
            return getWriter();
        }   
        try {   
            dispatchMessage(entry, req, conn);
        } catch(ReadStreamExn& ex) {   
            LOG_WARN << "dispatch uri:" << req.getUri() << " remote_addr:" <<  conn->remote_addr();
            return getWriter();
        }   
    } while (0);

    return getWriter();
}
void * ServerContext::fetchPtr(FormEntry *entry, Request &request, IConn* conn)
{
    return entry->target;
}

void ServerContext::dispatchMessage(FormEntry* entry, Request &req, IConn* conn)
{
    LOG_TRACE;
    ProtoHandlerClass *target  = (ProtoHandlerClass *)fetchPtr(entry, req, conn);
    if(target == NULL) {
        LOG_WARN << "fetch null ptr uri:" << req.getUri();
        return;
    }

    TargetProc prc;
    prc.mf_oo = entry->proc;
    void *cmd = req.getCmd();
    switch(entry->type)
    {
        //fpt_vc,           // void ::(Cmd*);
        case fpt_vc:
            if (cmd) {
                (target->*prc.mf_vc)(cmd);
            } else {
                LOG_WARN << "cmd is null, uri:" << req.getUri();
            }
            break;
            //fpt_vcc,            // void ::(Cmd *, Conn* );
        case fpt_vcc:
            if (cmd) {
                (target->*prc.mf_vcc)(cmd, (ConnPtr_t)conn);
            } else {
                LOG_WARN << "cmd is null, uri:" << req.getUri();
            }
            break;
        case fpt_vkc:
            if(cmd) {
                (target->*prc.mf_vkc)(req.getKey(), cmd);
            } else {
                LOG_WARN << "cmd is null, uri:" << req.getUri();
            }
            break;
        case fpt_vuc:
            if (cmd) {
                (target->*prc.mf_vuc)(req.getUid(), cmd);
            } else {
                LOG_WARN << "cmd is null, uri:" << req.getUri();
            }
            break;
        default:
            break;
    }
}

void ServerContext::clear()
{
    entries.clear();
}
