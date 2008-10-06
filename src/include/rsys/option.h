#if !defined (_RSYS_OPTION_H_)
#define _RSYS_OPTION_H_

typedef enum option_kind
{
  /* this option has no argument; it evaluates to `def' if provided */
  opt_no_arg,
  /* option is provided if next element in argv does not start with `-' */
  opt_optional,
  /* argument is part of argument, ie., -p4 */
  opt_sticky,
  /* argument is next value in argv */
  opt_sep,
  /* ignore this option */
  opt_ignore,
  /* ignore this option and its argument */
  opt_sep_ignore,
} option_kind_t;

typedef struct option
{
  /* text of the option.  does not include the `-' prefix */
  char *text;

  /* description of the option */
  char *desc;

  /* kind of argument */
  option_kind_t kind;

#if 0  
  /* NOTE: this was never implemented, so every time someone added an option
     to the database, there was potential to get totally confused.  The whole
     way this is set up is error prone and ugly, and the fact that we had a
     misleading unused field in here for so long is just a testament to
     these problems. */
  /* default value of argument */
  char *def;
#endif

  /* value of option if it is specified with no argument */
  char *opt_val;
} option_t;

typedef enum priority
{
  pri_appl_config_file,
  pri_command_line,
  pri_env_var,
  pri_x_resource,
  pri_dwrite,
  pri_default,
} priority_t;

typedef struct opt_val
{
  char *text;

  /* temporary value (ie., those specified in by an application
     specific configuration file */
  char *t_val;
  priority_t t_pri;
  
  char *val;
  priority_t pri;
} opt_val_t;

typedef struct opt_database
{
  opt_val_t *opt_vals;
  int n_opt_vals;
  int max_opt_vals;
} opt_database_t;

/* common options */
extern option_t opt_common[];
extern opt_database_t *common_db;

void opt_init (void);
void opt_shutdown (void);

void opt_register (char *new_interface, option_t *opts, int n_opts);

/* provide a function to parse specified arguments */
opt_database_t *opt_alloc_db (void);
int opt_parse (opt_database_t *db, option_t *opts, int n_opts,
	       int *argc, char *argv[]);

/* returns TRUE if options was specified */
int opt_int_val (opt_database_t *db, char *opt, int *retval,
		 boolean_t *parse_error_p);

#if !defined (OPENSTEP)
int opt_val (opt_database_t *db, char *opt, char **retval);
void opt_put_val (opt_database_t *db, char *opt, char *value,
		  priority_t pri, int temp_val_p);
#else
int opt_val (opt_database_t *db, char *opt, const char **retval);
void opt_put_val (opt_database_t *db, char *opt, const char *value,
		  priority_t pri, int temp_val_p);
#endif

void opt_put_int_val (opt_database_t *db, char *opt, int value,
		      priority_t pri, int temp_val_p);


char *opt_help_message (void);
void opt_register_pre_note (char *note);

extern uint32 parse_drive_opt (const char *opt_name, const char *opt_value);
extern FILE * executor_dir_fopen (const char *file, const char *perm);
extern int executor_dir_remove (const char *file);

#endif /* !_RSYS_OPTION_H_ */
