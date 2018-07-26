#include <iostream>
#include <string.h>
#include <stdio.h>

#include "Exception.h"

using namespace std;

const std::string & Exception::what_str() const throw()
{
    if (m_bstrerror)
        return m_what;

    m_bstrerror = true;

    if (0 == what_errno())
        return m_what;

    m_what += " - ";
    char str[128] = {0};
    snprintf(str, sizeof(str) -1, "%d %s", errno, strerror(errno));
    m_what += str;

    return m_what;
}
