/* This file is part of the GNU plotutils package. */

/*
 * Copyright (C) 1982-1994, Nicholas B. Tufillaro.  All rights reserved.
 *
 * GNU enhancements Copyright (C) 1996, 1997, 2005, 2008, Free Software
 * Foundation, Inc.
 */

#include "sys-defines.h"
#include <signal.h>
#include "ode.h"
#include "extern.h"

/*
 * arithmetic exceptions (e.g., floating point errors) come here
 */
RETSIGTYPE
fptrap (int sig)
{
  rterror ("arithmetic exception");
}

void
setflt (void)
{
  if (signal (SIGFPE, SIG_IGN) != SIG_IGN)
    signal (SIGFPE, fptrap);
}

void
resetflt (void)
{
  if (signal (SIGFPE, SIG_IGN) != SIG_IGN)
    signal (SIGFPE, SIG_DFL);
}
