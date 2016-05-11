#include <apr-1.0/apr_general.h>
#include <apr-1.0/apr_hash.h>
#include <stdio.h>

int print_hash_item(void *userdata, const void *key, apr_ssize_t klen, const void *value)
{
	printf("%s\n", (char *)userdata);
	printf("	key = \"%s\", klen=\"%zd\", value = \"%s\"\n", (char *)key, klen, (char *)value);
	return 1;
}

int main(void)
{
	apr_initialize();

	apr_pool_t *pool;
	apr_pool_create(&pool, NULL);

	apr_hash_t *hash = apr_hash_make(pool);
	apr_hash_set(hash, "php", APR_HASH_KEY_STRING, "Hello, PHP");
	apr_hash_set(hash, "apache", APR_HASH_KEY_STRING, "Hello, Apache");
	apr_hash_set(hash, "mysql", APR_HASH_KEY_STRING, "hello, mysql");
	apr_hash_do(print_hash_item, "first_print",  hash);

	printf("apr_hash_count(hash) = %d\n", apr_hash_count(hash));
	printf("apr_hash_get(hash, \"apache\", APR_HASH_KEY_STRING) = %s\n", apr_hash_get(hash, "apache", APR_HASH_KEY_STRING));


	apr_pool_destroy(pool);
	apr_terminate();
	return 0;
}
