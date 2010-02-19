#ifndef FTE_STL_H
#define FTE_STL_H

// some basic types for our C++ usage
// we are replacing overbloated STL

//#define CONFIG_FTE_USE_STL

#ifndef CONFIG_FTE_USE_STL
#include <inttypes.h>
#include <sys/types.h>
#include <assert.h>

#define FTE_BEGIN_NAMESPACE namespace fte {
#define FTE_END_NAMESPACE }

#if defined(__GNUC__) && \
    ((__GNUC__ > 2) || ((__GNUC__ == 2) && (__GNUC_MINOR__ > 4)))
#   define _fte_printf_attr(a,b) __attribute((__format__(__printf__,a,b)))
#else
#   define _fte_printf_attr(a,b)
#endif

//#include <stdio.h>

FTE_BEGIN_NAMESPACE;

/**
 * Simple class for storing SmartPointers
 */
template <class T, bool ARRAY = false> class aptr
{
public:
    explicit aptr(T* p) : pointer(p) {}
    ~aptr()
    {
	if (ARRAY)
	    delete [] pointer ;
	else
	    delete pointer;
    }
    T& operator*() const { return *pointer; }
    T* operator->() const { return pointer; }
private:
    T* pointer;
};

/**
 * Class to be used for swapping two variable of the same type
 */
template <class T>
inline void swap(T& a, T& b)
{
    const T tmp = a;
    a = b;
    b = tmp;
}

/**
 * Class to be used instead of line sequence:
 *   delete p;
 *   p = 0;
 */
//template <class T> inline void destroy(T*& p) { delete p; p = 0; }


/**
 * Simple class for storing char*
 *
 *
 * The main reason for existance of this class is faster compilation.
 * We do not need overcomplicated  std::string class for our purpose.
 * The behaviour of implemented methods should mostly match them 1:1
 */
class string
{
public:
    typedef size_t size_type;
    static const size_type npos = ~0U;

    string() : str(empty_string) {}
    string(char s);
    string(const char* s);
    string(const char* s, size_type len);
    string(const string& s);
    string(const string& s, size_type len);
    ~string();
    char operator[](size_type i) const { return str[i]; }
    char& operator[](size_type i) { return str[i]; }
    bool operator==(const char* s) const;
    bool operator==(const string& s) const { return operator==(s.str); }
    bool operator!=(const char* s) const { return !operator==(s); }
    bool operator!=(const string& s) const { return !operator==(s); }
    bool operator<(const string& s) const;
    string operator+(char c) const;
    string operator+(const char* s) const;
    string operator+(const string& s) const;
    string& operator=(char c);
    string& operator=(const char* s);
    string& operator=(const string& s) { return (this == &s) ? *this : operator=(s.str); }
    string& operator+=(char c) { return append(c); }
    string& operator+=(const char* s) { return append(s); }
    string& operator+=(const string& s) { return append(s.str); }
    string& append(char c);
    string& append(const char* s);
    string& append(const string& s) { return append(s.str); }
    char* begin() { return str; }
    const char* begin() const { return str; }
    void clear() { string tmp; swap(tmp); }
    const char* c_str() const { return str; }
    char* end() { return str + size(); }
    const char* end() const { return str + size(); }
    bool empty() const { return str[0] == 0; }
    size_type find(const string& s, size_type startpos = 0) const;
    size_type find(char c) const;
    size_type rfind(char c) const;
    void insert(size_type pos, const string& s);
    string& erase(size_type pos = 0, size_type n = npos);
    size_type size() const { return slen(str); }
    string substr(size_type from = 0, size_type to = npos) const { return string(str + from, to); };
    void swap(string& s) { fte::swap(str, s.str); }

    /* string extensions */
    int sprintf(const char* fmt, ...) _fte_printf_attr(2, 3); // allocates size
    // it will use just 1024 bytes for non _GNU_SOURCE compilation!!
    string& tolower();
    string& toupper();

private:
    char* str;
    static char* empty_string;
    static inline size_type slen(const char* s) { size_type i = 0; while (s[i]) ++i; return i; }
    string(const char* s1, size_type sz1, const char* s2, size_type sz2);
};

template<> inline void swap(fte::string& a, fte::string& b) { a.swap(b); }

/*
 * without this operator attempt to compare const char* with string will give quite unexpected
 * results because of implicit usage of operator const char*() with the right operand
 */
inline bool operator==(const char* s1, const string& s2) { return s2 == s1; }


/**
 * Simple vector class
 *
 * Implemented methods behaves like std::vector
 */
template <class Type> class vector
{
public:
    typedef const Type* const_iterator;
    typedef Type* iterator;
    typedef const Type& const_reference;
    typedef Type& reference;
    typedef size_t size_type;

    static const size_type invalid = ~0U;

    vector<Type>() :
	m_type(0), m_capacity(0), m_size(0)
    {
    }

    vector<Type>(size_type prealloc) :
	m_type(0), m_capacity(prealloc), m_size(0)
    {
	if (m_capacity > 0)
	    m_type = new Type[m_capacity];
    }

    // we will not count references - we have to program with this in mind!
    vector<Type>(const vector<Type>& t) :
	m_type(0), m_capacity(0), m_size(0)
    {
	copy(t.m_type, t.m_size, t.m_size);
    }
    vector<Type>& operator=(const vector<Type>& t)
    {
	//printf("operator=    %p   %d	%d\n", t.m_type, t.m_size, t.m_capacity);
	if (this != &t) {
	    vector<Type> tmp(t);
	    swap(tmp);
	}
	return *this;
    }
    ~vector();
    const_reference operator[](size_type i) const { return m_type[i]; }
    reference operator[](size_type i) { return m_type[i]; }
    const_iterator begin() const { return m_type; }
    iterator begin() { return m_type; }
    const_reference front() const { return *begin(); }
    reference front() { return *begin(); }
    const_iterator end() const { return m_type + m_size; }
    iterator end() { return m_type + m_size; }
    const_reference back() const { return *(end() - 1); }
    reference back() { return *(end() - 1); }
    size_type capacity() const { return m_capacity; }
    void clear();
    iterator erase(iterator pos);
    iterator erase(iterator first, iterator last);
    iterator insert(iterator pos, const Type& t);
    void insert(iterator pos, const_iterator from, const_iterator to);
    size_type find(reference t) const
    {
	for (size_type i = 0; i < m_size; ++i)
	    if (t == m_type[i])
		return i;
	return invalid;
    }
    void pop_back()
    {
	//printf("vector pop_back %d\n", m_size);
	assert(m_size > 0);
	if (--m_size < m_capacity / 4)
	    internal_copy(m_type, m_size, m_capacity / 2);
    }
    void pop_front()
    {
	assert(m_size > 0);
	if (--m_size >= m_capacity / 4)
	    for (size_type i = 1; i <= m_size; ++i)
		internal_swap(m_type + (i - 1), m_type[i]);
        else
	    internal_copy(m_type + 1, m_size, m_capacity / 2);
    }
    void push_back(const_reference m)
    {
	if (m_size + 1 >= m_capacity)
	    internal_copy(m_type, m_size, m_capacity * 2);
	m_type[m_size++] = m;
    }
    void reserve(size_type sz)
    {
	if (sz > m_capacity)
	    internal_copy(m_type, m_size, sz);
    }
    void resize(size_type sz)
    {
	internal_copy(m_type, (sz < m_size) ? sz : m_size, sz);
	m_size = sz;
    }
    size_type size() const { return m_size; }
    void swap(vector<Type>& v)
    {
	fte::swap(m_type, v.m_type);
	fte::swap(m_capacity, v.m_capacity);
	fte::swap(m_size, v.m_size);
    }

    /* vector extensions */
    void remove(const_reference t);

protected:
    static const size_type min_capacity = 4;
    Type* m_type;
    size_type m_capacity;
    size_type m_size;
    void copy(const_iterator in, size_type size, size_type alloc);
    void internal_copy(iterator in, size_type size, size_type alloc);
    void internal_swap(iterator pos, Type& t) { *pos = t; }
};

template <class Type>
vector<Type>::~vector()
{
    delete[] m_type;
}

template <class Type>
void vector<Type>::clear()
{
    delete[] m_type;
    m_type = 0;
    m_capacity = 0;
    m_size = 0;
}

template <class Type>
void vector<Type>::copy(const_iterator in, size_type sz, size_type alloc)
{
    assert(sz <= alloc);
    vector<Type> tmp((alloc < min_capacity) ? min_capacity : alloc);
    for (size_type i = 0; i < sz; ++i)
	tmp[i] = in[i];
    swap(tmp);
    m_size = sz;
}

template <class Type>
void vector<Type>::internal_copy(iterator in, size_type sz, size_type alloc)
{
    assert(sz <= alloc);
    vector<Type> tmp((alloc < min_capacity) ? min_capacity : alloc);
    for (size_type i = 0; i < sz; ++i)
	internal_swap(tmp.begin() + i, in[i]);
    swap(tmp);
    m_size = sz;
}

template <class Type>
typename vector<Type>::iterator vector<Type>::erase(iterator pos)
{
    return erase(pos, pos + 1);
}

template <class Type>
typename vector<Type>::iterator vector<Type>::erase(iterator first, iterator last)
{
    assert(last <= end());
    m_size -= last - first;
    for (iterator p = first; p < end(); ++last, ++p)
	internal_swap(p, *last);
    return first;
}

template <class Type>
typename vector<Type>::iterator vector<Type>::insert(iterator pos, const Type& t)
{
    const size_type n = pos - begin();
    insert(pos, &t, &t + 1);
    return begin() + n;
}

template <class Type>
void vector<Type>::insert(iterator pos, const_iterator from, const_iterator to)
{
    const size_type isz = to - from;
    if (!isz)
	return;

    const size_type n = pos - begin();
    Type* tmp;

    if (m_size + isz < m_capacity)
	tmp = begin();
    else
    {
	const size_type nc = ((m_size + isz > min_capacity)
			      ? m_size + isz : min_capacity) * 2;
	tmp = new Type[nc];
	m_capacity = nc;
    }

    // works also for m_size == 0
    for (size_type i = m_size - 1; i + 1 >= n + isz; --i)
	internal_swap(tmp + (i + isz), m_type[i]);

    if (tmp != m_type)
    {
	for (size_type i = 0; i < n; ++i)
	    internal_swap(tmp + i, m_type[i]);
	delete[] m_type;
	m_type = tmp;
    }

    for (size_type i = n; from != to; ++i, ++from)
	m_type[i] = *from;

    m_size += isz;
}

template <class Type>
void vector<Type>::remove(const_reference t)
{
    iterator from = begin();
    for (iterator it = from; it != end(); ++it)
	if (t != *it)
	{
	    if (from != it)
		internal_swap(from, *it);
	    ++from;
	}
    m_size -= (end() - from);
}

template <class Type>
inline void swap(fte::vector<Type>& a, fte::vector<Type>& b) { a.swap(b); }

/*
 * partial specialization for some common types
 * for more effective transfers in copy constructions
 * instead of heaving two copies - we allow to swap between field values
 * thus we can maintain vector<string> without reallocation of string with
 * every size change of vector<>
 */
template <>
inline void vector<fte::string>::internal_swap(vector<fte::string>::iterator pos, fte::string& t) { (*pos).swap(t); }


#define StlString   fte::string
#define StlVector   fte::vector

FTE_END_NAMESPACE;

#else

#include <string>
#include <vector>

#define StlString   std::string
#define StlVector   std::vector

#endif // CONFIG_FTE_USE_STL

#define vector_const_iterate(type, var, i) for (StlVector<type>::const_iterator i = var.begin(); i != var.end(); ++i)
#define vector_iterate(type, var, i) for (StlVector<type>::iterator i = var.begin(); i != var.end(); ++i)

#endif // FTE_STL_H
