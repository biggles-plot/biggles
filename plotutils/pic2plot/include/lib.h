// -*- C++ -*-
// Declarations/definitions of miscellaneous libgroff functions etc.,
// not declared in any other header file.

extern const char *our_itoa(int);

extern char *strsave(const char *s);

extern int interpret_lf_args(const char *p);

extern char illegal_char_table[];

inline int illegal_input_char(int c)
{
  return c >= 0 && illegal_char_table[c];
}

/* ad_delete deletes an array of objects with destructors; a_delete deletes
   an array of objects without destructors */

#ifdef ARRAY_DELETE_NEEDS_SIZE
/* for 2.0 systems */
#define ad_delete(size) delete [size]
#define a_delete delete
#else /* not ARRAY_DELETE_NEEDS_SIZE */
/* for ARM systems */
#define ad_delete(size) delete []
#define a_delete delete []
#endif /* not ARRAY_DELETE_NEEDS_SIZE */
