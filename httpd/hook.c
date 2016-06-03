#include <apr-1.0/apr_hooks.h>

/**
 * Implement an Apache core hook that runs until one of the functions
 * returns something other than ok or decline. That return value is
 * then returned from the hook runner. If the hooks run to completion,
 * then ok is returned. Note that if no hook runs it would probably be
 * more correct to return decline, but this currently does not do
 * so. The implementation is called ap_run_<i>name</i>.
 *
 * @param ret The return type of the hook (and the hook runner)
 * @param name The name of the hook
 * @param args_decl The declaration of the arguments for the hook, for example
 * "(int x,void *y)"
 * @param args_use The arguments for the hook as used in a call, for example
 * "(x,y)"
 * @param ok The "ok" return value
 * @param decline The "decline" return value
 * @return ok, decline or an error.
 * @note If IMPLEMENTing a hook that is not linked into the Apache core,
 * (e.g. within a dso) see APR_IMPLEMENT_EXTERNAL_HOOK_RUN_ALL.
 */
#define AP_IMPLEMENT_HOOK_RUN_ALL(ret,name,args_decl,args_use,ok,decline) \
	APR_IMPLEMENT_EXTERNAL_HOOK_RUN_ALL(ap,AP,ret,name,args_decl, \
                                            args_use,ok,decline)

/**
 * Implement an Apache core hook that has no return code, and
 * therefore runs all of the registered functions. The implementation
 * is called ap_run_<i>name</i>.
 *
 * @param name The name of the hook
 * @param args_decl The declaration of the arguments for the hook, for example
 * "(int x,void *y)"
 * @param args_use The arguments for the hook as used in a call, for example
 * "(x,y)"
 * @note If IMPLEMENTing a hook that is not linked into the Apache core,
 * (e.g. within a dso) see APR_IMPLEMENT_EXTERNAL_HOOK_VOID.
 */
#define AP_IMPLEMENT_HOOK_VOID(name,args_decl,args_use) \
	APR_IMPLEMENT_EXTERNAL_HOOK_VOID(ap,AP,name,args_decl,args_use)

/**
 * Implement a hook that runs until a function returns something other than
 * decline. If all functions return decline, the hook runner returns decline.
 * The implementation is called ap_run_<i>name</i>.
 *
 * @param ret The return type of the hook (and the hook runner)
 * @param name The name of the hook
 * @param args_decl The declaration of the arguments for the hook, for example
 * "(int x,void *y)"
 * @param args_use The arguments for the hook as used in a call, for example
 * "(x,y)"
 * @param decline The "decline" return value
 * @return decline or an error.
 * @note If IMPLEMENTing a hook that is not linked into the Apache core
 * (e.g. within a dso) see APR_IMPLEMENT_EXTERNAL_HOOK_RUN_FIRST.
 */
#define AP_IMPLEMENT_HOOK_RUN_FIRST(ret,name,args_decl,args_use,decline) \
	APR_IMPLEMENT_EXTERNAL_HOOK_RUN_FIRST(ap,AP,ret,name,args_decl, \
                                              args_use,decline)

#define AP_DECLARE(type)            type

APR_HOOK_STRUCT(
           APR_HOOK_LINK(header_parser)
           APR_HOOK_LINK(pre_config)
           APR_HOOK_LINK(post_config)
           APR_HOOK_LINK(open_logs)
           APR_HOOK_LINK(child_init)
           APR_HOOK_LINK(handler)
           APR_HOOK_LINK(quick_handler)
           APR_HOOK_LINK(optional_fn_retrieve)
           APR_HOOK_LINK(test_config)
)

AP_IMPLEMENT_HOOK_RUN_ALL(int, header_parser,
                          (request_rec *r), (r), OK, DECLINED)

AP_IMPLEMENT_HOOK_RUN_ALL(int, pre_config,
                          (apr_pool_t *pconf, apr_pool_t *plog,
                           apr_pool_t *ptemp),
                          (pconf, plog, ptemp), OK, DECLINED)

AP_IMPLEMENT_HOOK_VOID(test_config,
                       (apr_pool_t *pconf, server_rec *s),
                       (pconf, s))

AP_IMPLEMENT_HOOK_RUN_ALL(int, post_config,
                          (apr_pool_t *pconf, apr_pool_t *plog,
                           apr_pool_t *ptemp, server_rec *s),
                          (pconf, plog, ptemp, s), OK, DECLINED)

AP_IMPLEMENT_HOOK_RUN_ALL(int, open_logs,
                          (apr_pool_t *pconf, apr_pool_t *plog,
                           apr_pool_t *ptemp, server_rec *s),
                          (pconf, plog, ptemp, s), OK, DECLINED)

AP_IMPLEMENT_HOOK_VOID(child_init,
                       (apr_pool_t *pchild, server_rec *s),
                       (pchild, s))

AP_IMPLEMENT_HOOK_RUN_FIRST(int, handler, (request_rec *r),
                            (r), DECLINED)

AP_IMPLEMENT_HOOK_RUN_FIRST(int, quick_handler, (request_rec *r, int lookup),
                            (r, lookup), DECLINED)

AP_IMPLEMENT_HOOK_VOID(optional_fn_retrieve, (void), ())


#define AP_DECLARE_HOOK(ret,name,args) \
	APR_DECLARE_EXTERNAL_HOOK(ap,AP,ret,name,args)

/**
 * Run the header parser functions for each module
 * @param r The current request
 * @return OK or DECLINED
 */
AP_DECLARE_HOOK(int,header_parser,(request_rec *r))

/**
 * Run the pre_config function for each module
 * @param pconf The config pool
 * @param plog The logging streams pool
 * @param ptemp The temporary pool
 * @return OK or DECLINED on success anything else is a error
 */
AP_DECLARE_HOOK(int,pre_config,(apr_pool_t *pconf,apr_pool_t *plog,
                                apr_pool_t *ptemp))

/**
 * Run the test_config function for each module; this hook is run
 * only if the server was invoked to test the configuration syntax.
 * @param pconf The config pool
 * @param s The list of server_recs
 */
AP_DECLARE_HOOK(void,test_config,(apr_pool_t *pconf, server_rec *s))

/**
 * Run the post_config function for each module
 * @param pconf The config pool
 * @param plog The logging streams pool
 * @param ptemp The temporary pool
 * @param s The list of server_recs
 * @return OK or DECLINED on success anything else is a error
 */
AP_DECLARE_HOOK(int,post_config,(apr_pool_t *pconf,apr_pool_t *plog,
                                 apr_pool_t *ptemp,server_rec *s))

/**
 * Run the open_logs functions for each module
 * @param pconf The config pool
 * @param plog The logging streams pool
 * @param ptemp The temporary pool
 * @param s The list of server_recs
 * @return OK or DECLINED on success anything else is a error
 */
AP_DECLARE_HOOK(int,open_logs,(apr_pool_t *pconf,apr_pool_t *plog,
                               apr_pool_t *ptemp,server_rec *s))

/**
 * Run the child_init functions for each module
 * @param pchild The child pool
 * @param s The list of server_recs in this server
 */
AP_DECLARE_HOOK(void,child_init,(apr_pool_t *pchild, server_rec *s))

/**
 * Run the handler functions for each module
 * @param r The request_rec
 * @remark non-wildcard handlers should HOOK_MIDDLE, wildcard HOOK_LAST
 */
AP_DECLARE_HOOK(int,handler,(request_rec *r))

/**
 * Run the quick handler functions for each module. The quick_handler
 * is run before any other requests hooks are called (location_walk,
 * directory_walk, access checking, et. al.). This hook was added
 * to provide a quick way to serve content from a URI keyed cache.
 *
 * @param r The request_rec
 * @param lookup_uri Controls whether the caller actually wants content or not.
 * lookup is set when the quick_handler is called out of
 * ap_sub_req_lookup_uri()
 */
AP_DECLARE_HOOK(int,quick_handler,(request_rec *r, int lookup_uri))

/**
 * Retrieve the optional functions for each module.
 * This is run immediately before the server starts. Optional functions should
 * be registered during the hook registration phase.
 */
AP_DECLARE_HOOK(void,optional_fn_retrieve,(void))







// ====================================================
static struct
{
  apr_array_header_t *link_header_parser;
  apr_array_header_t *link_pre_config;
  apr_array_header_t *link_post_config;
  apr_array_header_t *link_open_logs;
  apr_array_header_t *link_child_init;
  apr_array_header_t *link_handler;
  apr_array_header_t *link_quick_handler;
  apr_array_header_t *link_optional_fn_retrieve;
  apr_array_header_t *link_test_config;
} _hooks;

void
ap_hook_header_parser (ap_HOOK_header_parser_t * pf,
		       const char *const *aszPre, const char *const *aszSucc,
		       int nOrder)
{
  ap_LINK_header_parser_t *pHook;
  if (!_hooks.link_header_parser)
    {
      _hooks.link_header_parser =
	apr_array_make (apr_hook_global_pool, 1,
			sizeof (ap_LINK_header_parser_t));
      apr_hook_sort_register ("header_parser", &_hooks.link_header_parser);
    }
  pHook = apr_array_push (_hooks.link_header_parser);
  pHook->pFunc = pf;
  pHook->aszPredecessors = aszPre;
  pHook->aszSuccessors = aszSucc;
  pHook->nOrder = nOrder;
  pHook->szName = apr_hook_debug_current;
  if (apr_hook_debug_enabled)
    apr_hook_debug_show ("header_parser", aszPre, aszSucc);
}

apr_array_header_t *
ap_hook_get_header_parser (void)
{
  return _hooks.link_header_parser;
}

int
ap_run_header_parser (request_rec * r)
{
  ap_LINK_header_parser_t *pHook;
  int n;
  int rv;
  if (!_hooks.link_header_parser)
    return OK;
  pHook = (ap_LINK_header_parser_t *) _hooks.link_header_parser->elts;
  for (n = 0; n < _hooks.link_header_parser->nelts; ++n)
    {
      rv = pHook[n].pFunc (r);
      if (rv != OK && rv != DECLINED)
	return rv;
    }
  return OK;
}


void
ap_hook_pre_config (ap_HOOK_pre_config_t * pf, const char *const *aszPre,
		    const char *const *aszSucc, int nOrder)
{
  ap_LINK_pre_config_t *pHook;
  if (!_hooks.link_pre_config)
    {
      _hooks.link_pre_config =
	apr_array_make (apr_hook_global_pool, 1,
			sizeof (ap_LINK_pre_config_t));
      apr_hook_sort_register ("pre_config", &_hooks.link_pre_config);
    }
  pHook = apr_array_push (_hooks.link_pre_config);
  pHook->pFunc = pf;
  pHook->aszPredecessors = aszPre;
  pHook->aszSuccessors = aszSucc;
  pHook->nOrder = nOrder;
  pHook->szName = apr_hook_debug_current;
  if (apr_hook_debug_enabled)
    apr_hook_debug_show ("pre_config", aszPre, aszSucc);
}

apr_array_header_t *
ap_hook_get_pre_config (void)
{
  return _hooks.link_pre_config;
}

int
ap_run_pre_config (apr_pool_t * pconf, apr_pool_t * plog, apr_pool_t * ptemp)
{
  ap_LINK_pre_config_t *pHook;
  int n;
  int rv;
  if (!_hooks.link_pre_config)
    return OK;
  pHook = (ap_LINK_pre_config_t *) _hooks.link_pre_config->elts;
  for (n = 0; n < _hooks.link_pre_config->nelts; ++n)
    {
      rv = pHook[n].pFunc (pconf, plog, ptemp);
      if (rv != OK && rv != DECLINED)
	return rv;
    }
  return OK;
}




void
ap_hook_test_config (ap_HOOK_test_config_t * pf, const char *const *aszPre,
		     const char *const *aszSucc, int nOrder)
{
  ap_LINK_test_config_t *pHook;
  if (!_hooks.link_test_config)
    {
      _hooks.link_test_config =
	apr_array_make (apr_hook_global_pool, 1,
			sizeof (ap_LINK_test_config_t));
      apr_hook_sort_register ("test_config", &_hooks.link_test_config);
    }
  pHook = apr_array_push (_hooks.link_test_config);
  pHook->pFunc = pf;
  pHook->aszPredecessors = aszPre;
  pHook->aszSuccessors = aszSucc;
  pHook->nOrder = nOrder;
  pHook->szName = apr_hook_debug_current;
  if (apr_hook_debug_enabled)
    apr_hook_debug_show ("test_config", aszPre, aszSucc);
}

apr_array_header_t *
ap_hook_get_test_config (void)
{
  return _hooks.link_test_config;
}

void
ap_run_test_config (apr_pool_t * pconf, server_rec * s)
{
  ap_LINK_test_config_t *pHook;
  int n;
  if (!_hooks.link_test_config)
    return;
  pHook = (ap_LINK_test_config_t *) _hooks.link_test_config->elts;
  for (n = 0; n < _hooks.link_test_config->nelts; ++n)
    pHook[n].pFunc (pconf, s);
}



void
ap_hook_post_config (ap_HOOK_post_config_t * pf, const char *const *aszPre,
		     const char *const *aszSucc, int nOrder)
{
  ap_LINK_post_config_t *pHook;
  if (!_hooks.link_post_config)
    {
      _hooks.link_post_config =
	apr_array_make (apr_hook_global_pool, 1,
			sizeof (ap_LINK_post_config_t));
      apr_hook_sort_register ("post_config", &_hooks.link_post_config);
    }
  pHook = apr_array_push (_hooks.link_post_config);
  pHook->pFunc = pf;
  pHook->aszPredecessors = aszPre;
  pHook->aszSuccessors = aszSucc;
  pHook->nOrder = nOrder;
  pHook->szName = apr_hook_debug_current;
  if (apr_hook_debug_enabled)
    apr_hook_debug_show ("post_config", aszPre, aszSucc);
}

apr_array_header_t *
ap_hook_get_post_config (void)
{
  return _hooks.link_post_config;
}

int
ap_run_post_config (apr_pool_t * pconf, apr_pool_t * plog, apr_pool_t * ptemp,
		    server_rec * s)
{
  ap_LINK_post_config_t *pHook;
  int n;
  int rv;
  if (!_hooks.link_post_config)
    return OK;
  pHook = (ap_LINK_post_config_t *) _hooks.link_post_config->elts;
  for (n = 0; n < _hooks.link_post_config->nelts; ++n)
    {
      rv = pHook[n].pFunc (pconf, plog, ptemp, s);
      if (rv != OK && rv != DECLINED)
	return rv;
    }
  return OK;
}




void
ap_hook_open_logs (ap_HOOK_open_logs_t * pf, const char *const *aszPre,
		   const char *const *aszSucc, int nOrder)
{
  ap_LINK_open_logs_t *pHook;
  if (!_hooks.link_open_logs)
    {
      _hooks.link_open_logs =
	apr_array_make (apr_hook_global_pool, 1,
			sizeof (ap_LINK_open_logs_t));
      apr_hook_sort_register ("open_logs", &_hooks.link_open_logs);
    }
  pHook = apr_array_push (_hooks.link_open_logs);
  pHook->pFunc = pf;
  pHook->aszPredecessors = aszPre;
  pHook->aszSuccessors = aszSucc;
  pHook->nOrder = nOrder;
  pHook->szName = apr_hook_debug_current;
  if (apr_hook_debug_enabled)
    apr_hook_debug_show ("open_logs", aszPre, aszSucc);
}

apr_array_header_t *
ap_hook_get_open_logs (void)
{
  return _hooks.link_open_logs;
}

int
ap_run_open_logs (apr_pool_t * pconf, apr_pool_t * plog, apr_pool_t * ptemp,
		  server_rec * s)
{
  ap_LINK_open_logs_t *pHook;
  int n;
  int rv;
  if (!_hooks.link_open_logs)
    return OK;
  pHook = (ap_LINK_open_logs_t *) _hooks.link_open_logs->elts;
  for (n = 0; n < _hooks.link_open_logs->nelts; ++n)
    {
      rv = pHook[n].pFunc (pconf, plog, ptemp, s);
      if (rv != OK && rv != DECLINED)
	return rv;
    }
  return OK;
}




void
ap_hook_child_init (ap_HOOK_child_init_t * pf, const char *const *aszPre,
		    const char *const *aszSucc, int nOrder)
{
  ap_LINK_child_init_t *pHook;
  if (!_hooks.link_child_init)
    {
      _hooks.link_child_init =
	apr_array_make (apr_hook_global_pool, 1,
			sizeof (ap_LINK_child_init_t));
      apr_hook_sort_register ("child_init", &_hooks.link_child_init);
    }
  pHook = apr_array_push (_hooks.link_child_init);
  pHook->pFunc = pf;
  pHook->aszPredecessors = aszPre;
  pHook->aszSuccessors = aszSucc;
  pHook->nOrder = nOrder;
  pHook->szName = apr_hook_debug_current;
  if (apr_hook_debug_enabled)
    apr_hook_debug_show ("child_init", aszPre, aszSucc);
}

apr_array_header_t *
ap_hook_get_child_init (void)
{
  return _hooks.link_child_init;
}

void
ap_run_child_init (apr_pool_t * pchild, server_rec * s)
{
  ap_LINK_child_init_t *pHook;
  int n;
  if (!_hooks.link_child_init)
    return;
  pHook = (ap_LINK_child_init_t *) _hooks.link_child_init->elts;
  for (n = 0; n < _hooks.link_child_init->nelts; ++n)
    pHook[n].pFunc (pchild, s);
}



void
ap_hook_handler (ap_HOOK_handler_t * pf, const char *const *aszPre,
		 const char *const *aszSucc, int nOrder)
{
  ap_LINK_handler_t *pHook;
  if (!_hooks.link_handler)
    {
      _hooks.link_handler =
	apr_array_make (apr_hook_global_pool, 1, sizeof (ap_LINK_handler_t));
      apr_hook_sort_register ("handler", &_hooks.link_handler);
    }
  pHook = apr_array_push (_hooks.link_handler);
  pHook->pFunc = pf;
  pHook->aszPredecessors = aszPre;
  pHook->aszSuccessors = aszSucc;
  pHook->nOrder = nOrder;
  pHook->szName = apr_hook_debug_current;
  if (apr_hook_debug_enabled)
    apr_hook_debug_show ("handler", aszPre, aszSucc);
}

apr_array_header_t *
ap_hook_get_handler (void)
{
  return _hooks.link_handler;
}

int
ap_run_handler (request_rec * r)
{
  ap_LINK_handler_t *pHook;
  int n;
  int rv;
  if (!_hooks.link_handler)
    return DECLINED;
  pHook = (ap_LINK_handler_t *) _hooks.link_handler->elts;
  for (n = 0; n < _hooks.link_handler->nelts; ++n)
    {
      rv = pHook[n].pFunc (r);
      if (rv != DECLINED)
	return rv;
    }
  return DECLINED;
}


void
ap_hook_quick_handler (ap_HOOK_quick_handler_t * pf,
		       const char *const *aszPre, const char *const *aszSucc,
		       int nOrder)
{
  ap_LINK_quick_handler_t *pHook;
  if (!_hooks.link_quick_handler)
    {
      _hooks.link_quick_handler =
	apr_array_make (apr_hook_global_pool, 1,
			sizeof (ap_LINK_quick_handler_t));
      apr_hook_sort_register ("quick_handler", &_hooks.link_quick_handler);
    }
  pHook = apr_array_push (_hooks.link_quick_handler);
  pHook->pFunc = pf;
  pHook->aszPredecessors = aszPre;
  pHook->aszSuccessors = aszSucc;
  pHook->nOrder = nOrder;
  pHook->szName = apr_hook_debug_current;
  if (apr_hook_debug_enabled)
    apr_hook_debug_show ("quick_handler", aszPre, aszSucc);
}

apr_array_header_t *
ap_hook_get_quick_handler (void)
{
  return _hooks.link_quick_handler;
}

int
ap_run_quick_handler (request_rec * r, int lookup)
{
  ap_LINK_quick_handler_t *pHook;
  int n;
  int rv;
  if (!_hooks.link_quick_handler)
    return DECLINED;
  pHook = (ap_LINK_quick_handler_t *) _hooks.link_quick_handler->elts;
  for (n = 0; n < _hooks.link_quick_handler->nelts; ++n)
    {
      rv = pHook[n].pFunc (r, lookup);
      if (rv != DECLINED)
	return rv;
    }
  return DECLINED;
}


void
ap_hook_optional_fn_retrieve (ap_HOOK_optional_fn_retrieve_t * pf,
			      const char *const *aszPre,
			      const char *const *aszSucc, int nOrder)
{
  ap_LINK_optional_fn_retrieve_t *pHook;
  if (!_hooks.link_optional_fn_retrieve)
    {
      _hooks.link_optional_fn_retrieve =
	apr_array_make (apr_hook_global_pool, 1,
			sizeof (ap_LINK_optional_fn_retrieve_t));
      apr_hook_sort_register ("optional_fn_retrieve",
			      &_hooks.link_optional_fn_retrieve);
    }
  pHook = apr_array_push (_hooks.link_optional_fn_retrieve);
  pHook->pFunc = pf;
  pHook->aszPredecessors = aszPre;
  pHook->aszSuccessors = aszSucc;
  pHook->nOrder = nOrder;
  pHook->szName = apr_hook_debug_current;
  if (apr_hook_debug_enabled)
    apr_hook_debug_show ("optional_fn_retrieve", aszPre, aszSucc);
}

apr_array_header_t *
ap_hook_get_optional_fn_retrieve (void)
{
  return _hooks.link_optional_fn_retrieve;
}

void
ap_run_optional_fn_retrieve (void)
{
  ap_LINK_optional_fn_retrieve_t *pHook;
  int n;
  if (!_hooks.link_optional_fn_retrieve)
    return;
  pHook =
    (ap_LINK_optional_fn_retrieve_t *) _hooks.link_optional_fn_retrieve->elts;
  for (n = 0; n < _hooks.link_optional_fn_retrieve->nelts; ++n)
    pHook[n].pFunc ();
}

#120 "hook.c"
typedef int ap_HOOK_header_parser_t (request_rec * r);
void ap_hook_header_parser (ap_HOOK_header_parser_t * pf,
			    const char *const *aszPre,
			    const char *const *aszSucc, int nOrder);
int ap_run_header_parser (request_rec * r);
apr_array_header_t *ap_hook_get_header_parser (void);
typedef struct ap_LINK_header_parser_t
{
  ap_HOOK_header_parser_t *pFunc;
  const char *szName;
  const char *const *aszPredecessors;
  const char *const *aszSuccessors;
  int nOrder;
} ap_LINK_header_parser_t;
#129 "hook.c"
typedef int ap_HOOK_pre_config_t (apr_pool_t * pconf, apr_pool_t * plog,
				  apr_pool_t * ptemp);
void ap_hook_pre_config (ap_HOOK_pre_config_t * pf, const char *const *aszPre,
			 const char *const *aszSucc, int nOrder);
int ap_run_pre_config (apr_pool_t * pconf, apr_pool_t * plog,
		       apr_pool_t * ptemp);
apr_array_header_t *ap_hook_get_pre_config (void);
typedef struct ap_LINK_pre_config_t
{
  ap_HOOK_pre_config_t *pFunc;
  const char *szName;
  const char *const *aszPredecessors;
  const char *const *aszSuccessors;
  int nOrder;
} ap_LINK_pre_config_t;
#138 "hook.c"
typedef void ap_HOOK_test_config_t (apr_pool_t * pconf, server_rec * s);
void ap_hook_test_config (ap_HOOK_test_config_t * pf,
			  const char *const *aszPre,
			  const char *const *aszSucc, int nOrder);
void ap_run_test_config (apr_pool_t * pconf, server_rec * s);
apr_array_header_t *ap_hook_get_test_config (void);
typedef struct ap_LINK_test_config_t
{
  ap_HOOK_test_config_t *pFunc;
  const char *szName;
  const char *const *aszPredecessors;
  const char *const *aszSuccessors;
  int nOrder;
} ap_LINK_test_config_t;
#148 "hook.c"
typedef int ap_HOOK_post_config_t (apr_pool_t * pconf, apr_pool_t * plog,
				   apr_pool_t * ptemp, server_rec * s);
void ap_hook_post_config (ap_HOOK_post_config_t * pf,
			  const char *const *aszPre,
			  const char *const *aszSucc, int nOrder);
int ap_run_post_config (apr_pool_t * pconf, apr_pool_t * plog,
			apr_pool_t * ptemp, server_rec * s);
apr_array_header_t *ap_hook_get_post_config (void);
typedef struct ap_LINK_post_config_t
{
  ap_HOOK_post_config_t *pFunc;
  const char *szName;
  const char *const *aszPredecessors;
  const char *const *aszSuccessors;
  int nOrder;
} ap_LINK_post_config_t;
#159 "hook.c"
typedef int ap_HOOK_open_logs_t (apr_pool_t * pconf, apr_pool_t * plog,
				 apr_pool_t * ptemp, server_rec * s);
void ap_hook_open_logs (ap_HOOK_open_logs_t * pf, const char *const *aszPre,
			const char *const *aszSucc, int nOrder);
int ap_run_open_logs (apr_pool_t * pconf, apr_pool_t * plog,
		      apr_pool_t * ptemp, server_rec * s);
apr_array_header_t *ap_hook_get_open_logs (void);
typedef struct ap_LINK_open_logs_t
{
  ap_HOOK_open_logs_t *pFunc;
  const char *szName;
  const char *const *aszPredecessors;
  const char *const *aszSuccessors;
  int nOrder;
} ap_LINK_open_logs_t;







typedef void ap_HOOK_child_init_t (apr_pool_t * pchild, server_rec * s);
void ap_hook_child_init (ap_HOOK_child_init_t * pf, const char *const *aszPre,
			 const char *const *aszSucc, int nOrder);
void ap_run_child_init (apr_pool_t * pchild, server_rec * s);
apr_array_header_t *ap_hook_get_child_init (void);
typedef struct ap_LINK_child_init_t
{
  ap_HOOK_child_init_t *pFunc;
  const char *szName;
  const char *const *aszPredecessors;
  const char *const *aszSuccessors;
  int nOrder;
} ap_LINK_child_init_t;






typedef int ap_HOOK_handler_t (request_rec * r);
void ap_hook_handler (ap_HOOK_handler_t * pf, const char *const *aszPre,
		      const char *const *aszSucc, int nOrder);
int ap_run_handler (request_rec * r);
apr_array_header_t *ap_hook_get_handler (void);
typedef struct ap_LINK_handler_t
{
  ap_HOOK_handler_t *pFunc;
  const char *szName;
  const char *const *aszPredecessors;
  const char *const *aszSuccessors;
  int nOrder;
} ap_LINK_handler_t;
#187 "hook.c"
typedef int ap_HOOK_quick_handler_t (request_rec * r, int lookup_uri);
void ap_hook_quick_handler (ap_HOOK_quick_handler_t * pf,
			    const char *const *aszPre,
			    const char *const *aszSucc, int nOrder);
int ap_run_quick_handler (request_rec * r, int lookup_uri);
apr_array_header_t *ap_hook_get_quick_handler (void);
typedef struct ap_LINK_quick_handler_t
{
  ap_HOOK_quick_handler_t *pFunc;
  const char *szName;
  const char *const *aszPredecessors;
  const char *const *aszSuccessors;
  int nOrder;
} ap_LINK_quick_handler_t;






typedef void ap_HOOK_optional_fn_retrieve_t (void);
void ap_hook_optional_fn_retrieve (ap_HOOK_optional_fn_retrieve_t * pf,
				   const char *const *aszPre,
				   const char *const *aszSucc, int nOrder);
void ap_run_optional_fn_retrieve (void);
apr_array_header_t *ap_hook_get_optional_fn_retrieve (void);
typedef struct ap_LINK_optional_fn_retrieve_t
{
  ap_HOOK_optional_fn_retrieve_t *pFunc;
  const char *szName;
  const char *const *aszPredecessors;
  const char *const *aszSuccessors;
  int nOrder;
} ap_LINK_optional_fn_retrieve_t;
