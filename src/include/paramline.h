#if !defined(__PARAMLINE_H__)
#define __PARAMLINE_H__

#ifdef __cplusplus
extern "C" {
#endif

extern int count_params(const char *p, int n);

extern char *get_param(const char **bufpp, int *nleftp);

extern void paramline_to_argcv(const char *cmdp, int *argcp, char ***argvp);

#ifdef __cplusplus
}
#endif

#endif
