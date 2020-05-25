/* This file is part of the GNU plotutils package. */

/*
 * Copyright (C) 1982-1994, Nicholas B. Tufillaro.  All rights reserved.
 *
 * GNU enhancements Copyright (C) 1996, 1997, 1998, 1999, 2005, 2008, Free
 * Software Foundation, Inc.
 */

/*
 * symbol table space management routines
 *
 */

#include "sys-defines.h"
#include "ode.h"
#include "extern.h"

struct sym *
lookup (const char *nam)
{
  struct sym *sp;
  
  for (sp = symtab; sp != NULL; sp = sp->sy_link)
    if (strncmp (sp->sy_name, nam, NAMMAX) == 0)
      return sp;
  sp = salloc();
  strncpy (sp->sy_name, nam, NAMMAX);
  return sp;
}

struct sym *
salloc (void)
{
  struct sym *sp;
  
  sp = (struct sym *)xmalloc(sizeof(struct sym));
  sp->sy_link = symtab;
  symtab = sp;
  sp->sy_expr = NULL;
  sp->sy_value = sp->sy_prime = 0.0;
  sp->sy_sserr = sp->sy_aberr = sp->sy_acerr = 0.0;
  sp->sy_flags = 0;
  return sp;
}

void
sfree (struct sym *sp)
{
  if (sp != NULL)
    free ((void *)sp);
}
