// -*- C++ -*-
// Declarations etc. related to the cset class, defined in libgroff/cset.cc.

#ifdef HAVE_LIMITS_H
#include <limits.h>
#else /* not HAVE_LIMITS_H */
#ifndef UCHAR_MAX
#define UCHAR_MAX 255
#endif
#endif /* not HAVE_LIMITS_H */ 

enum cset_builtin { CSET_BUILTIN };

class cset 
{
public:
  // ctors
  cset();
  cset(cset_builtin);
  cset(const char *);
  cset(const unsigned char *);

  int operator()(unsigned char) const;

  cset &operator|=(const cset &);
  cset &operator|=(unsigned char);

  friend class cset_init;
private:
  char v[UCHAR_MAX+1];
  void clear();
};

inline int
cset::operator()(unsigned char c) const
{
  return v[c];
}

inline cset &
cset::operator|=(unsigned char c)
{
  v[c] = 1;
  return *this;
}

extern cset csalpha;
extern cset csupper;
extern cset cslower;
extern cset csdigit;
extern cset csxdigit;
extern cset csspace;
extern cset cspunct;
extern cset csalnum;
extern cset csprint;
extern cset csgraph;
extern cset cscntrl;

static class cset_init 
{
public:
  cset_init();
private:
  static int initialised;
} _cset_init;
