#ifndef FTE_STL_H
#define FTE_STL_H

// some basic types for our C++ usage
// we are replacing overbloated STL
#include <inttypes.h>
#include <sys/types.h>
#include <assert.h>

//#include <stdio.h>

#define FTE_BEGIN_NAMESPACE namespace fte {
#define FTE_END_NAMESPACE }

#if defined(__GNUC__) && \
    ((__GNUC__ > 2) || ((__GNUC__ == 2) && (__GNUC_MINOR__ > 4)))
#   define _fte_printf_attr(a,b) __attribute((__format__(__printf__,a,b)))
#else
#   define _fte_printf_attr(a,b)
#endif

FTE_BEGIN_NAMESPACE;

/**
 * Simple class for storing SmartPointers
 */
template <class T, bool ARRAY = false>
    class aptr
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
    string();
    string(char s);
    string(const char* s, size_type len = npos);
    string(const string& s, size_type len = npos);
    ~string();
    size_type size() const { size_type i = 0; while (str[i]) i++; return i; }
    char operator[](size_type i) const { return str[i]; }
    char& operator[](size_type i) { return str[i]; }
    bool operator==(const char* s) const;
    bool operator==(const string& s) const { return operator==(s.str); }
    bool operator!=(const char* s) const { return !operator==(s); }
    bool operator!=(const string& s) const { return !operator==(s); }
    bool operator<(const string& s) const;
    const char* c_str() const { return str; }
    string& operator=(const char* s);
    string& operator=(const string& s) { return operator=(s.str); }
    string& operator+=(const char* s);
    string& operator+=(const string& s) { return operator+=(s.str); }
    string operator+(const char* s) const { return string(str) += s; }
    string operator+(const string& s) const { return string(str) += s; }
    string substr(size_type from = 0, size_type to = npos) const { return string(str + from, to); };
    bool empty() const { return str[0] == 0; }
    size_type find(const string& s, size_type startpos = 0) const;
    size_type find(char c) const;
    size_type rfind(char c) const;
    void insert(size_type pos, const string& s);
    string& erase(size_type from = 0, size_type to = npos);
    int sprintf(const char* fmt, ...) _fte_printf_attr(2, 3); // allocates size
    // it will use just 1024 bytes for non _GNU_SOURCE compilation!!
    string& tolower();
    string& toupper();

private:
    char* str;
};

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

    vector<Type>()
	:m_type(0), m_capacity(0), m_size(0)
    {
    }

    vector<Type>(size_type prealloc)
	:m_type(0), m_capacity(prealloc), m_size(prealloc)
    {
	if (m_capacity > 0)
	    m_type = new Type[m_capacity];
    }

    // we will not count references - we have to program with this in mind!
    vector<Type>(const vector<Type>& t) :m_type(0)
    {
	operator=(t);
    }
    vector<Type>& operator=(const vector<Type>& t)
    {
	//printf("operator=    %p   %d	%d\n", t.m_type, t.m_size, t.m_capacity);
	if (this != &t)
	    copy(t.m_type, t.m_size, t.m_capacity);
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
    void erase(iterator pos);
    size_type find(const Type& t) const
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
	m_size--;
	if ((m_capacity >= 8) && (m_size < m_capacity / 4))
	    copy(m_type, m_size, m_capacity / 2);
    }
    void pop_front()
    {
	assert(m_size > 0);
	for (size_type i = 1; i < m_size; i++)
	    m_type[i - 1] = m_type[i];
	pop_back();
    }
    void push_back(const_reference m)
    {
	if (m_size + 1 >= m_capacity)
	    copy(m_type, m_size, m_capacity * 2);
	m_type[m_size++] = m;
    }
    void remove(const_reference t);
    void reserve(size_type sz)
    {
	if (sz > m_capacity)
	    copy(m_type, m_size, sz);
    }
    void resize(size_type sz)
    {
	copy(m_type, (sz < m_size) ? sz : m_size, sz);
	m_size = sz;
    }
    size_type size() const { return m_size; }

protected:
    Type* m_type;
    size_type m_capacity;
    size_type m_size;
    void copy(const_iterator in, size_type size, size_type alloc);
};

#define vector_const_iterate(type, var, i) for (fte::vector<type>::const_iterator i = var.begin(); i != var.end(); ++i)
#define vector_iterate(type, var, i) for (fte::vector<type>::iterator i = var.begin(); i != var.end(); ++i)

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
void vector<Type>::copy(const Type* in, size_type sz, size_type alloc)
{
    Type* tmp = m_type;
    m_capacity = (alloc < 4) ? 4 : alloc;
    //printf("COPY VECT %d  %d\n", sz, alloc);
    assert(sz <= m_capacity);
    m_type = new Type[m_capacity];
    for (size_type i = 0; i < sz; ++i)
	m_type[i] = in[i];
    m_size = sz;
    delete[] tmp;
}

template <class Type>
void vector<Type>::erase(iterator pos)
{
    assert(m_size > 0);
    if (m_size > 0)
    {
	while ((pos + 1) != end())
	{
	    pos[0] = pos[1];
	    pos++;
	}
	pop_back();
    }
}
template <class Type>
void vector<Type>::remove(const Type& t)
{
    unsigned d = 0;
    iterator from = begin();
    for (iterator it = from; it != end(); it++)
    {
	if (t == *it)
	{
	    d++;
	    continue;
	}
	if (from != it)
	    *from++ = *it;
    }
    while (d-- > 0)
	pop_back();
    //printf("REMOVE VECT %d\n", m_size);
}

FTE_END_NAMESPACE;

#endif // FTE_STL_H
