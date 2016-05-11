#include <apr-1.0/apr_general.h>
#include <apr-1.0/apr_tables.h>
#include <stdio.h>

void print_arr(apr_array_header_t *arr)
{
	printf("elt_size	= %d\n", arr->elt_size);
	printf("nelts		= %d\n", arr->nelts);
	printf("nalloc		= %d\n", arr->nalloc);
	int i;
	uint64_t *elts = (uint64_t *)arr->elts;
	for (i = 0; i < arr->nalloc; i++) {	
		printf("%d: %#lx\n", i, elts[i]);
	}
}

int main(void)
{
	apr_initialize();

	apr_pool_t *pool;
	apr_pool_create(&pool, NULL);

	apr_array_header_t *arr = apr_array_make(pool, 10, sizeof(uint64_t));
	print_arr(arr);

	uint64_t *elt1 = apr_array_push(arr);
	*elt1 = 0xeeee;
	uint64_t *elt2 = apr_array_push(arr);
	*elt2 = 0xdddd;
	print_arr(arr);

	apr_array_pop(arr);
	print_arr(arr);

	APR_ARRAY_PUSH(arr, uint64_t) = 0xcccc;
	print_arr(arr);



	apr_pool_destroy(pool);
	apr_terminate();
	return 0;
}
