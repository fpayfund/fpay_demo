#ifndef __SOCK_EXCEPTION_H__
#define __SOCK_EXCEPTION_H__

#include <string>
#include <stdexcept>
#include <errno.h>

class Exception : public std::exception
{
public:
    Exception(const std::string& str)
    {
        init(str);
    }
    Exception(int e, const std::string& str)
    {
        init(e, str);
    }
    ~Exception() throw()
    {}

    const std::string& what_str() const throw();

    int what_errno() const throw()
    {
        return m_errno;
    }
    virtual const char* what () const throw()
    {
        return what_str().c_str ();
    }

private:
    Exception()
        : exception()
    {}
    void init(int e, const std::string & str)
    {
        m_errno = e;
        m_what = str;
        m_bstrerror = false;
    }
    void init(const std::string & str)
    {
        init(lasterrno(), str);
    }
    int lasterrno()
    {
        return errno;
    }

    int m_errno;
    mutable std::string m_what;
    mutable bool m_bstrerror;
};

#endif
