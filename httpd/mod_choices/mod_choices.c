#include <httpd.h>
#include <http_protocol.h>
#include <http_config.h>
#include <apr_strings.h>
#include <apr_hash.h>

typedef struct choices_cfg {
    int choices;
    apr_hash_t *transforms;
} choices_cfg;

typedef struct choices_transform {
    const char *ctype;
    const char *xslt;
} choices_transform;

/*
<LocationMatch "/reports*">
    Choices On
    ChoicesTransform html text/html;charset=utf8 transforms/html.xslt
    ChoicesTransform rdf application/rdf+xml transforms/earl.xslt
</LocationMatch>
*/

module AP_MODULE_DECLARE_DATA choices_module;

static int choices_select(request_rec *r)
{
    if (!r->handler || strcmp(r->handler, "choices")) {
        return DECLINED;
    }

    choices_cfg *cfg = ap_get_module_config(r->per_dir_config, &choices_module);
    if (!cfg->choices) {
        return DECLINED;
    }

    if (r->method_number != M_GET) {
        return DECLINED;
    }

    if (!r->filename) {
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    ap_set_content_type(r, "text/plain;charset=utf8");

    char *ext = strrchr(r->filename, '.');
    if (!ext) {
        ap_rprintf(r, "r->filename = %s\n", r->filename);
        return OK;
    }

    ext++;
    choices_transform *fmt = apr_hash_get(cfg->transforms, ext, APR_HASH_KEY_STRING);
    if (!fmt) {
        return HTTP_NOT_FOUND;
    }

    ap_rprintf(r, "r->filename = %s\ntransform->ctype = %s\ntransform->xslt = %s\n", r->filename, fmt->ctype, fmt->xslt);
    return OK;
}

static void choices_hooks(apr_pool_t *pool)
{
    ap_hook_handler(choices_select, NULL, NULL, APR_HOOK_MIDDLE);
}

static void *choices_cr_cfg(apr_pool_t *pool, char *x)
{
    choices_cfg *ret = apr_pcalloc(pool, sizeof(choices_cfg));
    ret->transforms = apr_hash_make(pool);
    return ret;
}

static const char *choices_transform_set(cmd_parms *cmd, void *cfg, const char *ext, const char *ctype, const char *xslt)
{
    apr_hash_t *transforms = ((choices_cfg *)cfg)->transforms;
    choices_transform *t = apr_palloc(cmd->pool, sizeof(choices_transform));
    t->ctype = ctype;
    t->xslt = xslt;
    apr_hash_set(transforms, ext, APR_HASH_KEY_STRING, t);
    return NULL;
}

static const command_rec choices_cmds[] = {
    AP_INIT_FLAG("Choices", ap_set_flag_slot, (void *)APR_OFFSETOF(choices_cfg, choices), ACCESS_CONF, "Choices On/Off"),
    AP_INIT_TAKE3("ChoicesTransform", choices_transform_set, NULL, ACCESS_CONF, "ext-transform"),
    {NULL}
};

module AP_MODULE_DECLARE_DATA choices_module = {
    STANDARD20_MODULE_STUFF,
    choices_cr_cfg,
    NULL,
    NULL,
    NULL,
    choices_cmds,
    choices_hooks
};
