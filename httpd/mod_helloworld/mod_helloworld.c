#include <httpd.h>
#include <http_protocol.h>
#include <http_config.h>
#include <apr_strings.h>
#include <apr_hash.h>

static int printitem(void *rec, const char *key, const char *value)
{
    request_rec *r = rec;
    apr_bucket_brigade *bb = apr_brigade_create(r->pool, r->connection->bucket_alloc);
    char *tr = apr_psprintf(r->pool, "<tr>\n"
                                     "  <th>%s</th>\n"
                                     "  <td>%s</td>\n"
                                     "</tr>\n",
                                    ap_escape_html(r->pool, key),
                                    ap_escape_html(r->pool, value));
    APR_BRIGADE_INSERT_TAIL(bb, apr_bucket_immortal_create(tr, strlen(tr), bb->bucket_alloc));
    ap_pass_brigade(r->output_filters, bb);
    return 1;
}

static void printtable(request_rec *r, apr_table_t *t, const char *caption, const char *keyhead, const char *valhead)
{
    apr_bucket_brigade *bb = apr_brigade_create(r->pool, r->connection->bucket_alloc);
    char *table_start = apr_psprintf(r->pool, "<table width=\"100%%\" border=\"1\">\n"
                                                "   <caption>%s</caption>\n"
                                                "   <thead>\n"
                                                "       <tr>\n"
                                                "           <th width=\"30%%\">%s</th>\n"
                                                "           <th>%s</th>\n"
                                                "       </tr>\n"
                                                "   </thead>\n"
                                                "   <tbody>\n",
                                                caption, keyhead, valhead);
    APR_BRIGADE_INSERT_TAIL(bb, apr_bucket_immortal_create(table_start, strlen(table_start), bb->bucket_alloc));
    ap_pass_brigade(r->output_filters, bb);

    apr_table_do(printitem, r, t, NULL);

    char *table_end = "</tbody>\n</table>\n\n\n";
    bb = apr_brigade_create(r->pool, r->connection->bucket_alloc);
    APR_BRIGADE_INSERT_TAIL(bb, apr_bucket_immortal_create(table_end, strlen(table_end), bb->bucket_alloc));
    ap_pass_brigade(r->output_filters, bb);
}

#define BUFLEN 8192
static int check_postdata(request_rec *r)
{
    int has_input = 0;
    const char *hdr = apr_table_get(r->headers_in, "Content-Length");
    if (hdr) {
        has_input = 1;
    }
    hdr = apr_table_get(r->headers_in, "Transfer-Encoding");
    if (hdr) {
        if (strcasecmp(hdr, "chunked") == 0) {
            has_input = 1;
        } else {
            ap_rprintf(r, "<p>Unsupported Transfer Encoding: %s</p>", ap_escape_html(r->pool, hdr));
            return OK;
        }
    }
    if (!has_input) {
        ap_rputs("<p>No request body.</p>\n", r);
        return OK;
    }

    apr_bucket_brigade *bb = apr_brigade_create(r->pool, r->connection->bucket_alloc);
    int end = 0;
    apr_status_t status;
    apr_bucket *b;
    apr_size_t bytes, count = 0;
    const char *buf;
    do {
        status = ap_get_brigade(r->input_filters, bb, AP_MODE_READBYTES, APR_BLOCK_READ, BUFLEN);
        if (status == APR_SUCCESS) {
            for (b = APR_BRIGADE_FIRST(bb); b != APR_BRIGADE_SENTINEL(bb); b = APR_BUCKET_NEXT(b)) {
                if (APR_BUCKET_IS_EOS(b)) {
                    end = 1;
                    break;
                }
                if (APR_BUCKET_IS_METADATA(b)) {
                    continue;
                }
                bytes = BUFLEN;
                status = apr_bucket_read(b, &buf, &bytes, APR_BLOCK_READ);
                count += bytes;
            }
        }
        apr_brigade_cleanup(bb);
    } while (!end && (status == APR_SUCCESS));

    if (status == APR_SUCCESS) {
        ap_rprintf(r, "<p>Got %ld bytes of request body data.</p>\n", count);
    } else {
        ap_rputs("<p>Error reading request body.</p>", r);
    }
    return OK;
}

static apr_hash_t *parse_form_from_string(request_rec *r, char *args)
{
    apr_hash_t *form;
    apr_array_header_t *values;
    char *pair;
    char *eq;
    const char *delim = "&";
    char *last;
    char **ptr;

    if (args == NULL) {
        return NULL;
    }

    form = apr_hash_make(r->pool);

    for (pair = apr_strtok(args, delim, &last); pair != NULL; pair = apr_strtok(NULL, delim, &last)) {
        for (eq = pair; *eq; ++eq) {
            if (*eq == '+') {
                *eq = ' ';
            }
        }
        eq = strchr(pair, '=');
        if (eq) {
            *eq = 0;
            eq++;
            ap_unescape_url(pair);
            ap_unescape_url(eq);
        } else {
            eq = "";
            ap_unescape_url(pair);
        }

        values = apr_hash_get(form, pair, APR_HASH_KEY_STRING);
        if (values == NULL) {
            values = apr_array_make(r->pool, 1, sizeof(const char *));
            apr_hash_set(form, pair, APR_HASH_KEY_STRING, values);
        }
        ptr = apr_array_push(values);
        *ptr = apr_pstrdup(r->pool, eq);
    }

    return form;
}

static int print_form_element(void *userdata, const void *key, apr_ssize_t klen, const void *value)
{
    request_rec *r = (request_rec *)userdata;
    ap_rprintf(r, "%s: %s<br>", (char *)key, apr_array_pstrcat(r->pool, (apr_array_header_t *)value, ','));
}

static void print_form(apr_hash_t *form, request_rec *r)
{
    apr_hash_do(print_form_element, r, form);
}

static apr_hash_t *parse_form_from_GET(request_rec *r)
{
    return parse_form_from_string(r, r->args);
}

static int helloworld_handler(request_rec *r)
{
    if (!r->handler || strcmp(r->handler, "helloworld")) {
        return DECLINED;
    }
    if (r->method_number != M_GET && r->method_number != M_POST) {
        return HTTP_METHOD_NOT_ALLOWED;
    }

    ap_set_content_type(r, "text/html;charset=utf8");

    printtable(r, r->headers_in, "Request Headers", "Header", "Value");
    printtable(r, r->headers_out, "Response Headers", "Header", "Value");
    printtable(r, r->subprocess_env, "Environment", "Variable", "Value");

    check_postdata(r);
/*
    print_form(parse_form_from_GET(r), r);
*/
    return OK;
}

static void helloworld_hooks(apr_pool_t *pool)
{
    ap_hook_handler(helloworld_handler, NULL, NULL, APR_HOOK_MIDDLE);
}

module AP_MODULE_DECLARE_DATA helloworld_module = {
    STANDARD20_MODULE_STUFF,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    helloworld_hooks
};
