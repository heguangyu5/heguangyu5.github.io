#include <apr-1.0/apr_general.h>
#include <apr-1.0/apr_tables.h>
#include <stdio.h>

int print_table_item(void *userdata, const char *key, const char *value)
{
	printf("%s\n", (char *)userdata);
	printf("	key = \"%s\", value = \"%s\"\n", key, value);
	return 1;
}

int main(void)
{
	apr_initialize();

	apr_pool_t *pool;
	apr_pool_create(&pool, NULL);

	apr_table_t *table = apr_table_make(pool, 10);
	apr_table_setn(table, "php", "Hello, PHP");
	apr_table_setn(table, "apache", "Hello, Apache");
	apr_table_addn(table, "apache", "hello, apache");
	apr_table_do(print_table_item, "first_print",  table, NULL);

	printf("apr_table_get(talbe, \"php\") = %s\n", apr_table_get(table, "php"));
	printf("apr_table_get(talbe, \"apache\") = %s\n", apr_table_get(table, "apache"));

	apr_pool_destroy(pool);
	apr_terminate();
	return 0;
}
