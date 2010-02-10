#include "stl_fte.h"

#ifndef CONFIG_FTE_USE_STL

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FTE_BEGIN_NAMESPACE;

static char xempty_string[] = ""; // used as empty string - shall never be overwritten

char* string::empty_string = xempty_string;

string::string(char s)
{
    str = new char[2];
    str[0] = s;
    str[1] = 0;
}

string::string(const char* s)
{
    if (s)
    {
	size_t slen = string::slen(s);

	if (slen)
	{
	    str = new char[slen + 1];
	    memcpy(str, s, slen);
	    str[slen] = 0;
	    return;
	}
    }
    str = empty_string;
}

string::string(const char* s, size_type len)
{
    if (s)
    {
	size_t slen = string::slen(s);

	if (slen > len)
	    slen = len;

	if (slen)
	{
	    str = new char[slen + 1];
	    memcpy(str, s, slen);
	    str[slen] = 0;
	    return;
	}
    }
    str = empty_string;
}

string::string(const string& s)
{
    size_type slen = s.size();

    if (slen)
    {
	str = new char[slen + 1];
	memcpy(str, s.str, slen);
	str[slen] = 0;
    }
    else
	str = empty_string;
}

string::string(const string& s, size_type len)
{
    size_type slen = s.size();

    if (slen > len)
	slen = len;

    if (slen)
    {
	str = new char[slen + 1];
	memcpy(str, s.str, slen);
	str[slen] = 0;
    }
    else
	str = empty_string;
}

string::string(const char* s1, size_type sz1, const char* s2, size_type sz2) :
    str(new char[sz1 + ++sz2])
{
    memcpy(str, s1, sz1);
    memcpy(str + sz1, s2, sz2);
}

string::~string()
{
    if (str != empty_string)
	delete[] str;
}

bool string::operator==(const char* s) const
{
    return (s) ? !strcmp(str, s) : empty();
}

bool string::operator<(const string& s) const
{
    return (strcmp(str, s.str)<0);
}

string& string::operator=(const char* s)
{
    string tmp(s);
    swap(tmp);
    return *this;
}

string string::operator+(const char* s) const
{
    return string(str) += s;
}

string string::operator+(const string& s) const
{
    return string(str) += s;
}

string& string::append(const char* s)
{
    if (s)
    {
	string tmp(str, size(), s, slen(s));
	swap(tmp);
    }
    return *this;
}

string& string::erase(size_type pos, size_type n)
{
    char* p = str + pos;
    if (n != npos && n > 0)
    {
	// add check for size() ???
	for (char* i = p + n; *i; ++i)
	    *p++ = *i;
    }
    if (p != str)
	*p = 0;
    else
	clear();

    return *this;
}

string::size_type string::find(const string& s, size_type startpos) const
{
    const char* p = strstr(str + startpos, s.str);
    return (p) ? p - str : npos;
}

string::size_type string::find(char c) const
{
    const char* p = strchr(str, c);
    return (p) ? p - str : npos;
}

string::size_type string::rfind(char c) const
{
    const char* p = strrchr(str, c);
    return (p) ? p - str : npos;
}

void string::insert(size_type pos, const string& s)
{
    size_type k = size();
    size_type l = s.size();
    char* p = new char[k + l + 1];

    if (pos > k)
	pos = k;

    memcpy(p, str, pos);
    memcpy(p + pos, s.str, l);
    memcpy(p + pos + l, str + pos, k - pos + 1);

    if (str != empty_string)
	delete[] str;

    str = p;
}

int string::sprintf(const char* fmt, ...)
{
    int r;
    va_list ap;
    va_start(ap, fmt);

#ifdef _GNU_SOURCE
    char* s = 0;
    r = vasprintf(&s, fmt, ap);
#else
    // a bit poor hack but should be sufficient
    // eventually write full implementation
    char s[1000];
    r = vsnprintf(s, sizeof(s), fmt, ap);
#endif

    va_end(ap);

    if (s && r > 0)
    {
	string tmp(s, r);
	swap(tmp);
    }
    else
    {
	string tmp;
	swap(tmp);
	r = 0;
    }

#ifdef _GNU_SOURCE
    free(s);
#endif

    return r;
}

string& string::tolower()
{
    for (char *p = str; *p;  p++)
	*p = (char)::tolower(*p);

    return *this;
}

string& string::toupper()
{
    for (char *p = str; *p;  p++)
	*p = (char)::toupper(*p);

    return *this;
}

FTE_END_NAMESPACE;

#endif // CONFIG_FTE_USE_STL
