/*
   +----------------------------------------------------------------------+
   | PHP Version 7                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2017 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@zend.com>                                |
   |          Sascha Schumann <sascha@schumann.cx>                        |
   |          Pierre Joye <pierre@php.net>                                |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>

#include "zend.h"
#include "zend_virtual_cwd.h"
#include "tsrm_strtok_r.h"



#ifndef HAVE_REALPATH
#define realpath(x,y) strcpy(y,x)
#endif

#define VIRTUAL_CWD_DEBUG 0

#include "TSRM.h"

/* Only need mutex for popen() in Windows and NetWare because it doesn't chdir() on UNIX */

virtual_cwd_globals cwd_globals;

cwd_state main_cwd_state; /* True global */

#include <unistd.h>

#define TOKENIZER_STRING "/"

/* default macros */

#ifndef IS_DIRECTORY_UP
#define IS_DIRECTORY_UP(element, len) \
	(len == 2 && element[0] == '.' && element[1] == '.')
#endif

#ifndef IS_DIRECTORY_CURRENT
#define IS_DIRECTORY_CURRENT(element, len) \
	(len == 1 && element[0] == '.')
#endif

/* define this to check semantics */
#define IS_DIR_OK(s) (1)



#define CWD_STATE_COPY(d, s)				\
	(d)->cwd_length = (s)->cwd_length;		\
	(d)->cwd = (char *) emalloc((s)->cwd_length+1);	\
	memcpy((d)->cwd, (s)->cwd, (s)->cwd_length+1);

#define CWD_STATE_FREE(s)			\
	efree((s)->cwd);

# define CWD_STATE_FREE_ERR(state) CWD_STATE_FREE(state)


static int php_is_dir_ok(const cwd_state *state)  /* {{{ */
{
	zend_stat_t buf;

	if (php_sys_stat(state->cwd, &buf) == 0 && S_ISDIR(buf.st_mode))
		return (0);

	return (1);
}
/* }}} */

static int php_is_file_ok(const cwd_state *state)  /* {{{ */
{
	zend_stat_t buf;

	if (php_sys_stat(state->cwd, &buf) == 0 && S_ISREG(buf.st_mode))
		return (0);

	return (1);
}
/* }}} */

static void cwd_globals_ctor(virtual_cwd_globals *cwd_g) /* {{{ */
{
	CWD_STATE_COPY(&cwd_g->cwd, &main_cwd_state);
	cwd_g->realpath_cache_size = 0;
	cwd_g->realpath_cache_size_limit = REALPATH_CACHE_SIZE;
	cwd_g->realpath_cache_ttl = REALPATH_CACHE_TTL;
	memset(cwd_g->realpath_cache, 0, sizeof(cwd_g->realpath_cache));
}
/* }}} */

static void cwd_globals_dtor(virtual_cwd_globals *cwd_g) /* {{{ */
{
	realpath_cache_clean();
}
/* }}} */

CWD_API void virtual_cwd_startup(void) /* {{{ */
{
	char cwd[MAXPATHLEN];
	char *result;

	result = getcwd(cwd, sizeof(cwd));
	if (!result) {
		cwd[0] = '\0';
	}

	main_cwd_state.cwd_length = (int)strlen(cwd);
	main_cwd_state.cwd = strdup(cwd);

	cwd_globals_ctor(&cwd_globals);

}
/* }}} */

CWD_API void virtual_cwd_shutdown(void) /* {{{ */
{
	cwd_globals_dtor(&cwd_globals);

	free(main_cwd_state.cwd); /* Don't use CWD_STATE_FREE because the non global states will probably use emalloc()/efree() */
}
/* }}} */

CWD_API int virtual_cwd_activate(void) /* {{{ */
{
	if (CWDG(cwd).cwd == NULL) {
		CWD_STATE_COPY(&CWDG(cwd), &main_cwd_state);
	}
	return 0;
}
/* }}} */

CWD_API int virtual_cwd_deactivate(void) /* {{{ */
{
	if (CWDG(cwd).cwd != NULL) {
		CWD_STATE_FREE(&CWDG(cwd));
		CWDG(cwd).cwd = NULL;
	}
	return 0;
}
/* }}} */

CWD_API char *virtual_getcwd_ex(size_t *length) /* {{{ */
{
	cwd_state *state;

	state = &CWDG(cwd);

	if (state->cwd_length == 0) {
		char *retval;

		*length = 1;
		retval = (char *) emalloc(2);
		if (retval == NULL) {
			return NULL;
		}
		retval[0] = DEFAULT_SLASH;
		retval[1] = '\0';
		return retval;
	}

	if (!state->cwd) {
		*length = 0;
		return NULL;
	}

	*length = state->cwd_length;
	return estrdup(state->cwd);
}
/* }}} */

/* Same semantics as UNIX getcwd() */
CWD_API char *virtual_getcwd(char *buf, size_t size) /* {{{ */
{
	size_t length;
	char *cwd;

	cwd = virtual_getcwd_ex(&length);

	if (buf == NULL) {
		return cwd;
	}
	if (length > size-1) {
		efree(cwd);
		errno = ERANGE; /* Is this OK? */
		return NULL;
	}
	if (!cwd) {
		return NULL;
	}
	memcpy(buf, cwd, length+1);
	efree(cwd);
	return buf;
}
/* }}} */

static inline zend_ulong realpath_cache_key(const char *path, int path_len) /* {{{ */
{
	register zend_ulong h;
	const char *e = path + path_len;

	for (h = Z_UL(2166136261); path < e;) {
		h *= Z_UL(16777619);
		h ^= *path++;
	}

	return h;
}
/* }}} */

CWD_API void realpath_cache_clean(void) /* {{{ */
{
	uint32_t i;

	for (i = 0; i < sizeof(CWDG(realpath_cache))/sizeof(CWDG(realpath_cache)[0]); i++) {
		realpath_cache_bucket *p = CWDG(realpath_cache)[i];
		while (p != NULL) {
			realpath_cache_bucket *r = p;
			p = p->next;
			free(r);
		}
		CWDG(realpath_cache)[i] = NULL;
	}
	CWDG(realpath_cache_size) = 0;
}
/* }}} */

CWD_API void realpath_cache_del(const char *path, int path_len) /* {{{ */
{
	zend_ulong key = realpath_cache_key(path, path_len);
	zend_ulong n = key % (sizeof(CWDG(realpath_cache)) / sizeof(CWDG(realpath_cache)[0]));
	realpath_cache_bucket **bucket = &CWDG(realpath_cache)[n];

	while (*bucket != NULL) {
		if (key == (*bucket)->key && path_len == (*bucket)->path_len &&
					memcmp(path, (*bucket)->path, path_len) == 0) {
			realpath_cache_bucket *r = *bucket;
			*bucket = (*bucket)->next;

			/* if the pointers match then only subtract the length of the path */
		   	if(r->path == r->realpath) {
				CWDG(realpath_cache_size) -= sizeof(realpath_cache_bucket) + r->path_len + 1;
			} else {
				CWDG(realpath_cache_size) -= sizeof(realpath_cache_bucket) + r->path_len + 1 + r->realpath_len + 1;
			}

			free(r);
			return;
		} else {
			bucket = &(*bucket)->next;
		}
	}
}
/* }}} */

static inline void realpath_cache_add(const char *path, int path_len, const char *realpath, int realpath_len, int is_dir, time_t t) /* {{{ */
{
	zend_long size = sizeof(realpath_cache_bucket) + path_len + 1;
	int same = 1;

	if (realpath_len != path_len ||
		memcmp(path, realpath, path_len) != 0) {
		size += realpath_len + 1;
		same = 0;
	}

	if (CWDG(realpath_cache_size) + size <= CWDG(realpath_cache_size_limit)) {
		realpath_cache_bucket *bucket = malloc(size);
		zend_ulong n;

		if (bucket == NULL) {
			return;
		}

		bucket->key = realpath_cache_key(path, path_len);
		bucket->path = (char*)bucket + sizeof(realpath_cache_bucket);
		memcpy(bucket->path, path, path_len+1);
		bucket->path_len = path_len;
		if (same) {
			bucket->realpath = bucket->path;
		} else {
			bucket->realpath = bucket->path + (path_len + 1);
			memcpy(bucket->realpath, realpath, realpath_len+1);
		}
		bucket->realpath_len = realpath_len;
		bucket->is_dir = is_dir;
		bucket->expires = t + CWDG(realpath_cache_ttl);
		n = bucket->key % (sizeof(CWDG(realpath_cache)) / sizeof(CWDG(realpath_cache)[0]));
		bucket->next = CWDG(realpath_cache)[n];
		CWDG(realpath_cache)[n] = bucket;
		CWDG(realpath_cache_size) += size;
	}
}
/* }}} */

static inline realpath_cache_bucket* realpath_cache_find(const char *path, int path_len, time_t t) /* {{{ */
{
	zend_ulong key = realpath_cache_key(path, path_len);
	zend_ulong n = key % (sizeof(CWDG(realpath_cache)) / sizeof(CWDG(realpath_cache)[0]));
	realpath_cache_bucket **bucket = &CWDG(realpath_cache)[n];

	while (*bucket != NULL) {
		if (CWDG(realpath_cache_ttl) && (*bucket)->expires < t) {
			realpath_cache_bucket *r = *bucket;
			*bucket = (*bucket)->next;

			/* if the pointers match then only subtract the length of the path */
		   	if(r->path == r->realpath) {
				CWDG(realpath_cache_size) -= sizeof(realpath_cache_bucket) + r->path_len + 1;
			} else {
				CWDG(realpath_cache_size) -= sizeof(realpath_cache_bucket) + r->path_len + 1 + r->realpath_len + 1;
			}
			free(r);
		} else if (key == (*bucket)->key && path_len == (*bucket)->path_len &&
					memcmp(path, (*bucket)->path, path_len) == 0) {
			return *bucket;
		} else {
			bucket = &(*bucket)->next;
		}
	}
	return NULL;
}
/* }}} */

CWD_API realpath_cache_bucket* realpath_cache_lookup(const char *path, int path_len, time_t t) /* {{{ */
{
	return realpath_cache_find(path, path_len, t);
}
/* }}} */

CWD_API zend_long realpath_cache_size(void)
{
	return CWDG(realpath_cache_size);
}

CWD_API zend_long realpath_cache_max_buckets(void)
{
	return (sizeof(CWDG(realpath_cache)) / sizeof(CWDG(realpath_cache)[0]));
}

CWD_API realpath_cache_bucket** realpath_cache_get_buckets(void)
{
	return CWDG(realpath_cache);
}


#undef LINK_MAX
#define LINK_MAX 32

static int tsrm_realpath_r(char *path, int start, int len, int *ll, time_t *t, int use_realpath, int is_dir, int *link_is_dir) /* {{{ */
{
	int i, j, save;
	int directory = 0;
	zend_stat_t st;
	realpath_cache_bucket *bucket;
	char *tmp;
	ALLOCA_FLAG(use_heap)

	while (1) {
		if (len <= start) {
			if (link_is_dir) {
				*link_is_dir = 1;
			}
			return start;
		}

		i = len;
		while (i > start && !IS_SLASH(path[i-1])) {
			i--;
		}

		if (i == len ||
			(i == len - 1 && path[i] == '.')) {
			/* remove double slashes and '.' */
			len = i - 1;
			is_dir = 1;
			continue;
		} else if (i == len - 2 && path[i] == '.' && path[i+1] == '.') {
			/* remove '..' and previous directory */
			is_dir = 1;
			if (link_is_dir) {
				*link_is_dir = 1;
			}
			if (i - 1 <= start) {
				return start ? start : len;
			}
			j = tsrm_realpath_r(path, start, i-1, ll, t, use_realpath, 1, NULL);
			if (j > start) {
				j--;
				while (j > start && !IS_SLASH(path[j])) {
					j--;
				}
				if (!start) {
					/* leading '..' must not be removed in case of relative path */
					if (j == 0 && path[0] == '.' && path[1] == '.' &&
							IS_SLASH(path[2])) {
						path[3] = '.';
						path[4] = '.';
						path[5] = DEFAULT_SLASH;
						j = 5;
					} else if (j > 0 &&
							path[j+1] == '.' && path[j+2] == '.' &&
							IS_SLASH(path[j+3])) {
						j += 4;
						path[j++] = '.';
						path[j++] = '.';
						path[j] = DEFAULT_SLASH;
					}
				}
			} else if (!start && !j) {
				/* leading '..' must not be removed in case of relative path */
				path[0] = '.';
				path[1] = '.';
				path[2] = DEFAULT_SLASH;
				j = 2;
			}
			return j;
		}

		path[len] = 0;

		save = (use_realpath != CWD_EXPAND);

		if (start && save && CWDG(realpath_cache_size_limit)) {
			/* cache lookup for absolute path */
			if (!*t) {
				*t = time(0);
			}
			if ((bucket = realpath_cache_find(path, len, *t)) != NULL) {
				if (is_dir && !bucket->is_dir) {
					/* not a directory */
					return -1;
				} else {
					if (link_is_dir) {
						*link_is_dir = bucket->is_dir;
					}
					memcpy(path, bucket->realpath, bucket->realpath_len + 1);
					return bucket->realpath_len;
				}
			}
		}

		if (save && php_sys_lstat(path, &st) < 0) {
			if (use_realpath == CWD_REALPATH) {
				/* file not found */
				return -1;
			}
			/* continue resolution anyway but don't save result in the cache */
			save = 0;
		}

		tmp = do_alloca(len+1, use_heap);
		memcpy(tmp, path, len+1);

		if (save && S_ISLNK(st.st_mode)) {
			if (++(*ll) > LINK_MAX || (j = php_sys_readlink(tmp, path, MAXPATHLEN)) < 0) {
				/* too many links or broken symlinks */
				free_alloca(tmp, use_heap);
				return -1;
			}
			path[j] = 0;
			if (IS_ABSOLUTE_PATH(path, j)) {
				j = tsrm_realpath_r(path, 1, j, ll, t, use_realpath, is_dir, &directory);
				if (j < 0) {
					free_alloca(tmp, use_heap);
					return -1;
				}
			} else {
				if (i + j >= MAXPATHLEN-1) {
					free_alloca(tmp, use_heap);
					return -1; /* buffer overflow */
				}
				memmove(path+i, path, j+1);
				memcpy(path, tmp, i-1);
				path[i-1] = DEFAULT_SLASH;
				j = tsrm_realpath_r(path, start, i + j, ll, t, use_realpath, is_dir, &directory);
				if (j < 0) {
					free_alloca(tmp, use_heap);
					return -1;
				}
			}
			if (link_is_dir) {
				*link_is_dir = directory;
			}
		} else {
			if (save) {
				directory = S_ISDIR(st.st_mode);
				if (link_is_dir) {
					*link_is_dir = directory;
				}
				if (is_dir && !directory) {
					/* not a directory */
					free_alloca(tmp, use_heap);
					return -1;
				}
			}
			if (i - 1 <= start) {
				j = start;
			} else {
				/* some leading directories may be unaccessable */
				j = tsrm_realpath_r(path, start, i-1, ll, t, save ? CWD_FILEPATH : use_realpath, 1, NULL);
				if (j > start) {
					path[j++] = DEFAULT_SLASH;
				}
			}
			if (j < 0 || j + len - i >= MAXPATHLEN-1) {
				free_alloca(tmp, use_heap);
				return -1;
			}
			memcpy(path+j, tmp+i, len-i+1);
			j += (len-i);
		}

		if (save && start && CWDG(realpath_cache_size_limit)) {
			/* save absolute path in the cache */
			realpath_cache_add(tmp, len, path, j, directory, *t);
		}

		free_alloca(tmp, use_heap);
		return j;
	}
}
/* }}} */

/* Resolve path relatively to state and put the real path into state */
/* returns 0 for ok, 1 for error */
CWD_API int virtual_file_ex(cwd_state *state, const char *path, verify_path_func verify_path, int use_realpath) /* {{{ */
{
	int path_length = (int)strlen(path);
	char resolved_path[MAXPATHLEN];
	int start = 1;
	int ll = 0;
	time_t t;
	int ret;
	int add_slash;
	void *tmp;

	if (path_length <= 0 || path_length >= MAXPATHLEN-1) {
		errno = EINVAL;
		return 1;
	}


	/* cwd_length can be 0 when getcwd() fails.
	 * This can happen under solaris when a dir does not have read permissions
	 * but *does* have execute permissions */
	if (!IS_ABSOLUTE_PATH(path, path_length)) {
		if (state->cwd_length == 0) {
			/* resolve relative path */
			start = 0;
			memcpy(resolved_path , path, path_length + 1);
		} else {
			int state_cwd_length = state->cwd_length;

			if (path_length + state_cwd_length + 1 >= MAXPATHLEN-1) {
				return 1;
			}
			memcpy(resolved_path, state->cwd, state_cwd_length);
			if (resolved_path[state_cwd_length-1] == DEFAULT_SLASH) {
				memcpy(resolved_path + state_cwd_length, path, path_length + 1);
				path_length += state_cwd_length;
			} else {
				resolved_path[state_cwd_length] = DEFAULT_SLASH;
				memcpy(resolved_path + state_cwd_length + 1, path, path_length + 1);
				path_length += state_cwd_length + 1;
			}
		}
	} else {
		memcpy(resolved_path, path, path_length + 1);
	}



	add_slash = (use_realpath != CWD_REALPATH) && path_length > 0 && IS_SLASH(resolved_path[path_length-1]);
	t = CWDG(realpath_cache_ttl) ? 0 : -1;
	path_length = tsrm_realpath_r(resolved_path, start, path_length, &ll, &t, use_realpath, 0, NULL);

	if (path_length < 0) {
		errno = ENOENT;
		return 1;
	}

	if (!start && !path_length) {
		resolved_path[path_length++] = '.';
	}
	if (add_slash && path_length && !IS_SLASH(resolved_path[path_length-1])) {
		if (path_length >= MAXPATHLEN-1) {
			return -1;
		}
		resolved_path[path_length++] = DEFAULT_SLASH;
	}
	resolved_path[path_length] = 0;

	if (verify_path) {
		cwd_state old_state;

		CWD_STATE_COPY(&old_state, state);
		state->cwd_length = path_length;

		tmp = erealloc(state->cwd, state->cwd_length+1);
		if (tmp == NULL) {
			CWD_STATE_FREE(&old_state);
			return 1;
		}
		state->cwd = (char *) tmp;

		memcpy(state->cwd, resolved_path, state->cwd_length+1);
		if (verify_path(state)) {
			CWD_STATE_FREE(state);
			*state = old_state;
			ret = 1;
		} else {
			CWD_STATE_FREE(&old_state);
			ret = 0;
		}
	} else {
		state->cwd_length = path_length;
		tmp = erealloc(state->cwd, state->cwd_length+1);
		if (tmp == NULL) {
			return 1;
		}
		state->cwd = (char *) tmp;

		memcpy(state->cwd, resolved_path, state->cwd_length+1);
		ret = 0;
	}

	return (ret);
}
/* }}} */

CWD_API int virtual_chdir(const char *path) /* {{{ */
{
	return virtual_file_ex(&CWDG(cwd), path, php_is_dir_ok, CWD_REALPATH)?-1:0;
}
/* }}} */

CWD_API int virtual_chdir_file(const char *path, int (*p_chdir)(const char *path)) /* {{{ */
{
	int length = (int)strlen(path);
	char *temp;
	int retval;
	ALLOCA_FLAG(use_heap)

	if (length == 0) {
		return 1; /* Can't cd to empty string */
	}
	while(--length >= 0 && !IS_SLASH(path[length])) {
	}

	if (length == -1) {
		/* No directory only file name */
		errno = ENOENT;
		return -1;
	}

	if (length == COPY_WHEN_ABSOLUTE(path) && IS_ABSOLUTE_PATH(path, length+1)) { /* Also use trailing slash if this is absolute */
		length++;
	}
	temp = (char *) do_alloca(length+1, use_heap);
	memcpy(temp, path, length);
	temp[length] = 0;
	retval = p_chdir(temp);
	free_alloca(temp, use_heap);
	return retval;
}
/* }}} */

CWD_API char *virtual_realpath(const char *path, char *real_path) /* {{{ */
{
	cwd_state new_state;
	char *retval;
	char cwd[MAXPATHLEN];

	/* realpath("") returns CWD */
	if (!*path) {
		new_state.cwd = (char*)emalloc(1);
		if (new_state.cwd == NULL) {
			retval = NULL;
			goto end;
		}
		new_state.cwd[0] = '\0';
		new_state.cwd_length = 0;
		if (VCWD_GETCWD(cwd, MAXPATHLEN)) {
			path = cwd;
		}
	} else if (!IS_ABSOLUTE_PATH(path, strlen(path))) {
		CWD_STATE_COPY(&new_state, &CWDG(cwd));
	} else {
		new_state.cwd = (char*)emalloc(1);
		if (new_state.cwd == NULL) {
			retval = NULL;
			goto end;
		}
		new_state.cwd[0] = '\0';
		new_state.cwd_length = 0;
	}

	if (virtual_file_ex(&new_state, path, NULL, CWD_REALPATH)==0) {
		int len = new_state.cwd_length>MAXPATHLEN-1?MAXPATHLEN-1:new_state.cwd_length;

		memcpy(real_path, new_state.cwd, len);
		real_path[len] = '\0';
		retval = real_path;
	} else {
		retval = NULL;
	}

	CWD_STATE_FREE(&new_state);
end:
	return retval;
}
/* }}} */

CWD_API int virtual_filepath_ex(const char *path, char **filepath, verify_path_func verify_path) /* {{{ */
{
	cwd_state new_state;
	int retval;

	CWD_STATE_COPY(&new_state, &CWDG(cwd));
	retval = virtual_file_ex(&new_state, path, verify_path, CWD_FILEPATH);

	*filepath = new_state.cwd;

	return retval;

}
/* }}} */

CWD_API int virtual_filepath(const char *path, char **filepath) /* {{{ */
{
	return virtual_filepath_ex(path, filepath, php_is_file_ok);
}
/* }}} */

CWD_API FILE *virtual_fopen(const char *path, const char *mode) /* {{{ */
{
	cwd_state new_state;
	FILE *f;

	if (path[0] == '\0') { /* Fail to open empty path */
		return NULL;
	}

	CWD_STATE_COPY(&new_state, &CWDG(cwd));
	if (virtual_file_ex(&new_state, path, NULL, CWD_EXPAND)) {
		CWD_STATE_FREE_ERR(&new_state);
		return NULL;
	}

	f = fopen(new_state.cwd, mode);

	CWD_STATE_FREE_ERR(&new_state);

	return f;
}
/* }}} */

CWD_API int virtual_access(const char *pathname, int mode) /* {{{ */
{
	cwd_state new_state;
	int ret;

	CWD_STATE_COPY(&new_state, &CWDG(cwd));
	if (virtual_file_ex(&new_state, pathname, NULL, CWD_REALPATH)) {
		CWD_STATE_FREE_ERR(&new_state);
		return -1;
	}

	ret = access(new_state.cwd, mode);

	CWD_STATE_FREE_ERR(&new_state);

	return ret;
}
/* }}} */

#if HAVE_UTIME
CWD_API int virtual_utime(const char *filename, struct utimbuf *buf) /* {{{ */
{
	cwd_state new_state;
	int ret;

	CWD_STATE_COPY(&new_state, &CWDG(cwd));
	if (virtual_file_ex(&new_state, filename, NULL, CWD_REALPATH)) {
		CWD_STATE_FREE_ERR(&new_state);
		return -1;
	}

	ret = utime(new_state.cwd, buf);

	CWD_STATE_FREE_ERR(&new_state);
	return ret;
}
/* }}} */
#endif

CWD_API int virtual_chmod(const char *filename, mode_t mode) /* {{{ */
{
	cwd_state new_state;
	int ret;

	CWD_STATE_COPY(&new_state, &CWDG(cwd));
	if (virtual_file_ex(&new_state, filename, NULL, CWD_REALPATH)) {
		CWD_STATE_FREE_ERR(&new_state);
		return -1;
	}

	ret = chmod(new_state.cwd, mode);

	CWD_STATE_FREE_ERR(&new_state);
	return ret;
}
/* }}} */

CWD_API int virtual_chown(const char *filename, uid_t owner, gid_t group, int link) /* {{{ */
{
	cwd_state new_state;
	int ret;

	CWD_STATE_COPY(&new_state, &CWDG(cwd));
	if (virtual_file_ex(&new_state, filename, NULL, CWD_REALPATH)) {
		CWD_STATE_FREE_ERR(&new_state);
		return -1;
	}

	if (link) {
#if HAVE_LCHOWN
		ret = lchown(new_state.cwd, owner, group);
#else
		ret = -1;
#endif
	} else {
		ret = chown(new_state.cwd, owner, group);
	}

	CWD_STATE_FREE_ERR(&new_state);
	return ret;
}
/* }}} */

CWD_API int virtual_open(const char *path, int flags, ...) /* {{{ */
{
	cwd_state new_state;
	int f;

	CWD_STATE_COPY(&new_state, &CWDG(cwd));
	if (virtual_file_ex(&new_state, path, NULL, CWD_FILEPATH)) {
		CWD_STATE_FREE_ERR(&new_state);
		return -1;
	}

	if (flags & O_CREAT) {
		mode_t mode;
		va_list arg;

		va_start(arg, flags);
		mode = (mode_t) va_arg(arg, int);
		va_end(arg);

		f = open(new_state.cwd, flags, mode);
	} else {
		f = open(new_state.cwd, flags);
	}
	CWD_STATE_FREE_ERR(&new_state);
	return f;
}
/* }}} */

CWD_API int virtual_creat(const char *path, mode_t mode) /* {{{ */
{
	cwd_state new_state;
	int f;

	CWD_STATE_COPY(&new_state, &CWDG(cwd));
	if (virtual_file_ex(&new_state, path, NULL, CWD_FILEPATH)) {
		CWD_STATE_FREE_ERR(&new_state);
		return -1;
	}

	f = creat(new_state.cwd,  mode);

	CWD_STATE_FREE_ERR(&new_state);
	return f;
}
/* }}} */

CWD_API int virtual_rename(const char *oldname, const char *newname) /* {{{ */
{
	cwd_state old_state;
	cwd_state new_state;
	int retval;

	CWD_STATE_COPY(&old_state, &CWDG(cwd));
	if (virtual_file_ex(&old_state, oldname, NULL, CWD_EXPAND)) {
		CWD_STATE_FREE_ERR(&old_state);
		return -1;
	}
	oldname = old_state.cwd;

	CWD_STATE_COPY(&new_state, &CWDG(cwd));
	if (virtual_file_ex(&new_state, newname, NULL, CWD_EXPAND)) {
		CWD_STATE_FREE_ERR(&old_state);
		CWD_STATE_FREE_ERR(&new_state);
		return -1;
	}
	newname = new_state.cwd;

	/* rename on windows will fail if newname already exists.
	   MoveFileEx has to be used */
	retval = rename(oldname, newname);

	CWD_STATE_FREE_ERR(&old_state);
	CWD_STATE_FREE_ERR(&new_state);

	return retval;
}
/* }}} */

CWD_API int virtual_stat(const char *path, zend_stat_t *buf) /* {{{ */
{
	cwd_state new_state;
	int retval;

	CWD_STATE_COPY(&new_state, &CWDG(cwd));
	if (virtual_file_ex(&new_state, path, NULL, CWD_REALPATH)) {
		CWD_STATE_FREE_ERR(&new_state);
		return -1;
	}

	retval = php_sys_stat(new_state.cwd, buf);

	CWD_STATE_FREE_ERR(&new_state);
	return retval;
}
/* }}} */

CWD_API int virtual_lstat(const char *path, zend_stat_t *buf) /* {{{ */
{
	cwd_state new_state;
	int retval;

	CWD_STATE_COPY(&new_state, &CWDG(cwd));
	if (virtual_file_ex(&new_state, path, NULL, CWD_EXPAND)) {
		CWD_STATE_FREE_ERR(&new_state);
		return -1;
	}

	retval = php_sys_lstat(new_state.cwd, buf);

	CWD_STATE_FREE_ERR(&new_state);
	return retval;
}
/* }}} */

CWD_API int virtual_unlink(const char *path) /* {{{ */
{
	cwd_state new_state;
	int retval;

	CWD_STATE_COPY(&new_state, &CWDG(cwd));
	if (virtual_file_ex(&new_state, path, NULL, CWD_EXPAND)) {
		CWD_STATE_FREE_ERR(&new_state);
		return -1;
	}

	retval = unlink(new_state.cwd);

	CWD_STATE_FREE_ERR(&new_state);
	return retval;
}
/* }}} */

CWD_API int virtual_mkdir(const char *pathname, mode_t mode) /* {{{ */
{
	cwd_state new_state;
	int retval;

	CWD_STATE_COPY(&new_state, &CWDG(cwd));
	if (virtual_file_ex(&new_state, pathname, NULL, CWD_FILEPATH)) {
		CWD_STATE_FREE_ERR(&new_state);
		return -1;
	}

	retval = mkdir(new_state.cwd, mode);
	CWD_STATE_FREE_ERR(&new_state);
	return retval;
}
/* }}} */

CWD_API int virtual_rmdir(const char *pathname) /* {{{ */
{
	cwd_state new_state;
	int retval;

	CWD_STATE_COPY(&new_state, &CWDG(cwd));
	if (virtual_file_ex(&new_state, pathname, NULL, CWD_EXPAND)) {
		CWD_STATE_FREE_ERR(&new_state);
		return -1;
	}

	retval = rmdir(new_state.cwd);
	CWD_STATE_FREE_ERR(&new_state);
	return retval;
}
/* }}} */


CWD_API DIR *virtual_opendir(const char *pathname) /* {{{ */
{
	cwd_state new_state;
	DIR *retval;

	CWD_STATE_COPY(&new_state, &CWDG(cwd));
	if (virtual_file_ex(&new_state, pathname, NULL, CWD_REALPATH)) {
		CWD_STATE_FREE_ERR(&new_state);
		return NULL;
	}

	retval = opendir(new_state.cwd);

	CWD_STATE_FREE_ERR(&new_state);
	return retval;
}
/* }}} */

CWD_API FILE *virtual_popen(const char *command, const char *type) /* {{{ */
{
	size_t command_length;
	int dir_length, extra = 0;
	char *command_line;
	char *ptr, *dir;
	FILE *retval;

	command_length = strlen(command);

	dir_length = CWDG(cwd).cwd_length;
	dir = CWDG(cwd).cwd;
	while (dir_length > 0) {
		if (*dir == '\'') extra+=3;
		dir++;
		dir_length--;
	}
	dir_length = CWDG(cwd).cwd_length;
	dir = CWDG(cwd).cwd;

	ptr = command_line = (char *) emalloc(command_length + sizeof("cd '' ; ") + dir_length + extra+1+1);
	if (!command_line) {
		return NULL;
	}
	memcpy(ptr, "cd ", sizeof("cd ")-1);
	ptr += sizeof("cd ")-1;

	if (CWDG(cwd).cwd_length == 0) {
		*ptr++ = DEFAULT_SLASH;
	} else {
		*ptr++ = '\'';
		while (dir_length > 0) {
			switch (*dir) {
			case '\'':
				*ptr++ = '\'';
				*ptr++ = '\\';
				*ptr++ = '\'';
				/* fall-through */
			default:
				*ptr++ = *dir;
			}
			dir++;
			dir_length--;
		}
		*ptr++ = '\'';
	}

	*ptr++ = ' ';
	*ptr++ = ';';
	*ptr++ = ' ';

	memcpy(ptr, command, command_length+1);
	retval = popen(command_line, type);

	efree(command_line);
	return retval;
}
/* }}} */

CWD_API char *tsrm_realpath(const char *path, char *real_path) /* {{{ */
{
	cwd_state new_state;
	char cwd[MAXPATHLEN];

	/* realpath("") returns CWD */
	if (!*path) {
		new_state.cwd = (char*)emalloc(1);
		if (new_state.cwd == NULL) {
			return NULL;
		}
		new_state.cwd[0] = '\0';
		new_state.cwd_length = 0;
		if (VCWD_GETCWD(cwd, MAXPATHLEN)) {
			path = cwd;
		}
	} else if (!IS_ABSOLUTE_PATH(path, strlen(path)) &&
					VCWD_GETCWD(cwd, MAXPATHLEN)) {
		new_state.cwd = estrdup(cwd);
		new_state.cwd_length = (int)strlen(cwd);
	} else {
		new_state.cwd = (char*)emalloc(1);
		if (new_state.cwd == NULL) {
			return NULL;
		}
		new_state.cwd[0] = '\0';
		new_state.cwd_length = 0;
	}

	if (virtual_file_ex(&new_state, path, NULL, CWD_REALPATH)) {
		efree(new_state.cwd);
		return NULL;
	}

	if (real_path) {
		int copy_len = new_state.cwd_length>MAXPATHLEN-1 ? MAXPATHLEN-1 : new_state.cwd_length;
		memcpy(real_path, new_state.cwd, copy_len);
		real_path[copy_len] = '\0';
		efree(new_state.cwd);
		return real_path;
	} else {
		return new_state.cwd;
	}
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
