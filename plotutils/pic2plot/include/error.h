// -*- C++ -*-
// Declarations of error-handling functions, defined in libgroff/error.cc.

extern void fatal_with_file_and_line(const char *filename, int lineno, const char *format, const errarg &arg1 = empty_errarg, const errarg &arg2 = empty_errarg, const errarg &arg3 = empty_errarg);

extern void error_with_file_and_line(const char *filename, int lineno, const char *format, const errarg &arg1 = empty_errarg, const errarg &arg2 = empty_errarg, const errarg &arg3 = empty_errarg);
			 
extern void warning_with_file_and_line(const char *filename, int lineno, const char *format, const errarg &arg1 = empty_errarg, const errarg &arg2 = empty_errarg, const errarg &arg3 = empty_errarg);

extern void fatal(const char *, const errarg &arg1 = empty_errarg, const errarg &arg2 = empty_errarg, const errarg &arg3 = empty_errarg);
      
extern void error(const char *, const errarg &arg1 = empty_errarg, const errarg &arg2 = empty_errarg, const errarg &arg3 = empty_errarg);

extern void warning(const char *, const errarg &arg1 = empty_errarg, const errarg &arg2 = empty_errarg, const errarg &arg3 = empty_errarg);


extern const char *program_name;
extern const char *current_filename;
extern int current_lineno;
