#include "stl_fte.h"

#ifndef CONFIG_FTE_USE_STL

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FTE_BEGIN_NAMESPACE;

static char empty_string[] = ""; // used as empty string - shall never be overwritten

string::string()
{
    //str = new char[1];
    //str[0] = 0;
    str = empty_string;
}

string::string(char s)
{
    str = new char[2];
    str[0] = s;
    str[1] = 0;
}

string::string(const char* s, size_type len)
{
    if (s)
    {
	size_t slen = strlen(s);

        if (len > slen)
	    len = slen;

	str = new char[len + 1];
	memcpy(str, s, len);
	str[len] = 0;
    }
    else
        str = empty_string;
}

string::string(const string& s, size_type len)
{
    if (len > s.size())
	len = s.size();

    str = new char[len + 1];
    memcpy(str, s.str, len);
    str[len] = 0;
}

string::~string()
{
    if (str != empty_string)
	delete[] str;
}

bool string::operator==(const char* s) const
{
    if (s)
	return !strcmp(str, s);
    return (size() == 0);
}

bool string::operator<(const string& s) const
{
    return (strcmp(str, s.str)<0);
}

string& string::operator=(const char* s)
{
    if (str != s)
    {
	if (str != empty_string)
	    delete[] str;

	size_t sz = s ? strlen(s) + 1 : 0;

	if (sz > 1)
	{
	    str = new char[sz];
	    memcpy(str, s, sz);
	}
	else
	    str = empty_string;
    }
    return *this;
}

string& string::operator+=(const char* s)
{
    if (s)
    {
	size_type s1 = size();
	size_t s2 = strlen(s) + 1; // with '0'
	if (s2 > 1)
	{
	    char* p = new char[s1 + s2];
	    memcpy(p, str, s1);
	    memcpy(p + s1, s, s2);

	    if (str != empty_string)
		delete[] str;

	    str = p;
	}
    }
    return *this;
}

string& string::erase(size_type from, size_type to)
{
    if (str != empty_string)
    {
	char* p = str + from;
	if (to != npos && to > 0 && to < size())
	{
	    // add check for size() ???
	    char* i = p + to;
	    while (*i)
		*p++ = *i++;
	}
	if (p == str)
	{
	    delete[] str;
	    str = empty_string;
	}
	else
	    *p = 0;
    }
    return *this;
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

int string::sprintf(const char* fmt, ...)
{
    int r;
    char* s = 0;
    va_list ap;
    va_start(ap, fmt);

    if (str != empty_string)
	delete[] str;

#ifdef _GNU_SOURCE
    r = vasprintf(&s, fmt, ap);
#else
    // a bit poor hack but should be sufficient
    // eventually write full implementation
    s = malloc(1000);
    r = vsnprintf(s, 999, fmt, ap);
#endif

    va_end(ap);

    if (r < 0) {
	free(s);
	s = 0;
	r = 0;
    }

    if (s)
    {
	str = new char[r + 1];
	memcpy(str, s, r);
	str[r] = 0;
	free(s);
    }
    else
    {
	str = empty_string;
	r = 0;
    }

    return r;
}

string& string::tolower()
{
    char* p = str;

    while (*p) {
	*p = (char)::tolower(*p);
	p++;
    }

    return *this;
}

string& string::toupper()
{
    char* p = str;

    while (*p) {
	*p = (char)::toupper(*p);
	p++;
    }

    return *this;
}

FTE_END_NAMESPACE;

#endif // CONFIG_FTE_USE_STL
