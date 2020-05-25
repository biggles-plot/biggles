/* This file is part of the GNU plotutils package. */

/*
 * Copyright (C) 1982-1994, Nicholas B. Tufillaro.  All rights reserved.
 *
 * GNU enhancements Copyright (C) 1996, 1997, 1998, 1999 Free Software 
 * Foundation, Inc.
 */

/*
 * print queue memory management
 *
 */

#include "sys-defines.h"
#include "ode.h"
#include "extern.h"

struct prt *
palloc (void)
{
  struct prt *pp;
  
  pp = (struct prt *)xmalloc (sizeof(struct prt));
  pp->pr_sym = NULL;
  pp->pr_link = NULL;
  pp->pr_which = P_VALUE;	/* default */
  return pp;
}

void
pfree (struct prt *pp)
{
  if (pp != NULL) 
    {
      pfree (pp->pr_link);
      free ((void *)pp);
    }
}
