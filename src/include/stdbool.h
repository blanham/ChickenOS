#ifndef C_OS_STDBOOL_H
#define C_OS_STDBOOL_H

#define	__bool_true_false_are_defined	1

#ifndef __cplusplus

#define	bool	_Bool
#if __STDC_VERSION__ < 199901L && __GNUC__ < 3
typedef	int	_Bool;
#endif

#define	false	0
#define	true	1
#endif

#endif
