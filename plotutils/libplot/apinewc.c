/* This file is part of the GNU plotutils package.  Copyright (C) 1995,
   1996, 1997, 1998, 1999, 2000, 2005, 2008, Free Software Foundation, Inc.

   The GNU plotutils package is free software.  You may redistribute it
   and/or modify it under the terms of the GNU General Public License as
   published by the Free Software foundation; either version 2, or (at your
   option) any later version.

   The GNU plotutils package is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with the GNU plotutils package; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin St., Fifth Floor,
   Boston, MA 02110-1301, USA. */

/* This file is specific to libplot, rather than libplotter.  It defines
   the new (i.e., thread-safe) C API.  The new C API contains wrappers
   around the operations that may be applied to any Plotter object, plus
   two additional functions (pl_newpl_r, pl_deletepl_r) that are specific
   to libplot.

   pl_newpl_r/pl_deletepl_r construct and destroy Plotter instances.  Their
   names resemble the C++ operations `new' and `delete'.  When a new
   Plotter of any type is constructed, the appropriate `default_init'
   structure, which contains function pointers, is copied into it.  Then
   its `initialize' method is invoked.  Before the Plotter is destroyed,
   its `terminate' method is invoked similarly.

   The C API also includes the functions pl_newplparams/pl_deleteplparams,
   which create/destroy PlotterParams instances, and wrappers around the
   methods that may be applied to any PlotterParams object.  A pointer to a
   PlotterParams object is passed to pl_newpl_r().  It specifies the
   device-driver parameters of the Plotter that will be created. */

#include "sys-defines.h"
#include "extern.h"
#include "plot.h"		/* header file for C API */

/* Known Plotter types, indexed into by a short mnemonic case-insensitive
   string: "generic"=generic (i.e. base Plotter class), "bitmap"=bitmap,
   "meta"=metafile, "tek"=Tektronix, "regis"=ReGIS, "hpgl"=HP-GL/2,
   "pcl"=PCL 5, "fig"=xfig, "cgm"=CGM, "ps"=PS, "ai"="AI", "svg"=SVG,
   "gif"=GIF, "pnm"=PNM (i.e. PBM/PGM/PPM), "z"=PNG, "X"=X11,
   "Xdrawable"=X11Drawable.  */

typedef struct 
{
  const char *name;
  const Plotter *default_init;
}
Plotter_data;

/* Initializations for the function-pointer part of each type of Plotter.
   Each of the initializing structures listed here is defined in the
   corresponding ?_defplot.c file. */
static const Plotter_data _plotter_data[] = 
{
  {"generic", &_pl_g_default_plotter},
  {"bitmap", &_pl_b_default_plotter},
  {"meta", &_pl_m_default_plotter},
  {"tek", &_pl_t_default_plotter},
  {"regis", &_pl_r_default_plotter},
  {"hpgl", &_pl_h_default_plotter},
  {"pcl", &_pl_q_default_plotter},
  {"fig", &_pl_f_default_plotter},
  {"cgm", &_pl_c_default_plotter},
  {"ps", &_pl_p_default_plotter},
  {"ai", &_pl_a_default_plotter},
  {"svg", &_pl_s_default_plotter},
  {"gif", &_pl_i_default_plotter},
  {"pnm", &_pl_n_default_plotter},
#ifdef INCLUDE_PNG_SUPPORT
  {"png", &_pl_z_default_plotter},
#endif
#ifndef X_DISPLAY_MISSING
  {"Xdrawable", &_pl_x_default_plotter},
  {"X", &_pl_y_default_plotter},
#endif /* not X_DISPLAY_MISSING */
  {(const char *)NULL, (const Plotter *)NULL}
};

/* forward references */
static bool _string_to_plotter_data (const char *type, int *position);
static void _api_warning (const char *msg);

/* These are two user-callable functions that are specific to the new
   (i.e., thread-safe) C binding: pl_newpl_r, pl_deletepl_r. */

Plotter *
pl_newpl_r (const char *type, FILE *infile, FILE *outfile, FILE *errfile, const PlotterParams *plotter_params)
{
  bool found;
  int position;
  Plotter *_plotter;
  
  /* determine initialization for specified plotter type */
  found = _string_to_plotter_data (type, &position);
  if (!found)
    {
      _api_warning ("ignoring request to create plotter of unknown type");
      return NULL;
    }

  /* create Plotter, copy function pointers to it */
  _plotter = (Plotter *)_pl_xmalloc (sizeof(Plotter));
  memcpy (_plotter, _plotter_data[position].default_init, sizeof(Plotter));

  /* create PlotterData structure, install it in Plotter */
  _plotter->data = (plPlotterData *)_pl_xmalloc (sizeof(plPlotterData));

  /* copy parameters to it */
  _plotter->data->infp = infile;  
  _plotter->data->outfp = outfile;
  _plotter->data->errfp = errfile;
  _pl_g_copy_params_to_plotter (_plotter, plotter_params);

  /* do any additional needed initializiations of the Plotter (e.g.,
     initialize data members of the PlotterData structure in a
     device-dependent way); also add the Plotter to the _plotters[] array */
  _plotter->initialize (_plotter);

  return _plotter;
}

/* utility function, used above; keys into table of Plotter types by a
   short mnemonic string */
static bool
_string_to_plotter_data (const char *type, int *position)
{
  const Plotter_data *p = _plotter_data;
  bool found = false;
  int i = 0;
  
  /* search table of known plotter type mnemonics */
  while (p->name)
    {
      if (strcasecmp ((char *)type, (char *)p->name) == 0)
	{
	  found = true;
	  break;
	}
      p++;
      i++;
    }
  /* return pointer to plotter data through pointer */
  if (found)
    *position = i;
  return found;
}

int
pl_deletepl_r (Plotter *_plotter)
{
  if (_plotter == NULL)
    {
      _api_warning ("ignoring request to delete a null Plotter");
      return -1;
    }

  /* if luser left the Plotter open, close it */
  if (_plotter->data->open)
    _API_closepl (_plotter);

  /* Invoke an internal Plotter method before deletion.  At a minimum, this
     private `terminate' method, frees instance-specific copies of class
     parameters, and also removes the pointer to the Plotter instance from
     the _plotters[] array.

     Also, it writes any unwritten graphics to the Plotter's output stream.
     This is the case for PSPlotters in particular, which write graphics
     only when they are deleted.  For a PSPlotter, the terminate method
     emits the Plotter's pages of graphics to its output stream and then
     deallocates associated storage.  For an XPlotter, this method kills
     the forked-off processes that are maintaining its popped-up windows
     (if any), provided that the VANISH_ON_DELETE parameter is set.  */
  _plotter->terminate (_plotter);

  /* tear down the PlotterData structure */
  free (_plotter->data);

  /* tear down the Plotter itself */
  free (_plotter);

  return 0;
}


/* function used in this file to print warning messages */
static void
_api_warning (const char *msg)
{
  if (pl_libplot_warning_handler != NULL)
    (*pl_libplot_warning_handler)(msg);
  else
    fprintf (stderr, "libplot: %s\n", msg);
}


/* These are two user-callable functions that are specific to the new
   (i.e., thread-safe) C binding: pl_newplparams, pl_deleteplparams,
   pl_copyplparams. */

PlotterParams *
pl_newplparams (void)
{
  int i;
  PlotterParams *_plotter_params_p;
  
  /* create PlotterParams, copy function pointers to it */
  _plotter_params_p = (PlotterParams *)_pl_xmalloc (sizeof(PlotterParams));
  memcpy (_plotter_params_p, &_default_plotter_params, sizeof(PlotterParams));

  /* null out all parameters */
  for (i = 0; i < NUM_PLOTTER_PARAMETERS; i++)
    _plotter_params_p->plparams[i] = (void *)NULL;

  return _plotter_params_p;
}

int
pl_deleteplparams (PlotterParams *_plotter_params_p)
{
  int i;
  
  /* free all copied strings, and the structure itself */
  for (i = 0; i < NUM_PLOTTER_PARAMETERS; i++)
    if (_known_params[i].is_string && _plotter_params_p->plparams[i] != NULL)
      free (_plotter_params_p->plparams[i]);
  free (_plotter_params_p);

  return 0;
}

PlotterParams *
pl_copyplparams (const PlotterParams *_plotter_params_p)
{
  int i;
  PlotterParams *new_plotter_params_p;
  
  /* create PlotterParams, copy function pointers to it */
  new_plotter_params_p = (PlotterParams *)_pl_xmalloc (sizeof(PlotterParams));
  memcpy (new_plotter_params_p, &_default_plotter_params, sizeof(PlotterParams));

  /* copy all parameters */
  for (i = 0; i < NUM_PLOTTER_PARAMETERS; i++)
    new_plotter_params_p->plparams[i] = _plotter_params_p->plparams[i];

  return new_plotter_params_p;
}

/* The following are C wrappers around the public functions in the
   PlotterParams class.  Together with the preceding functions, they are
   part of the new (i.e., thread-safe) C API. */

int
pl_setplparam (PlotterParams *plotter_params, const char *parameter, void * value)
{
  return plotter_params->setplparam (plotter_params, parameter, value);
}

/* END OF WRAPPERS AROUND PLOTTERPARAMS METHODS */
