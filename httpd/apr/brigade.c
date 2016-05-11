#include <apr-1.0/apr_general.h>
#include <apr-1.0/apr_buckets.h>
#include <stdio.h>

int main(void)
{
	apr_initialize();

	apr_pool_t *pool;
	apr_pool_create(&pool, NULL);

	apr_bucket_alloc_t *list = apr_bucket_alloc_create(pool);
	#define TITLE_PART_1 "We will read "
	#define TITLE_PART_2 "this source file \"buckets.c\" "
	#define TITLE_PART_3 "into buckets.\n"
	apr_bucket *title_part1 = apr_bucket_heap_create(
												TITLE_PART_1,
												strlen(TITLE_PART_1),
												NULL,
												list
											);
	apr_bucket *title_part2 = apr_bucket_heap_create(
												TITLE_PART_2,
												strlen(TITLE_PART_2),
												NULL,
												list
											);
	apr_bucket *title_part3 = apr_bucket_heap_create(
												TITLE_PART_3,
												strlen(TITLE_PART_3),
												NULL,
												list
											);

	
	apr_bucket_brigade *bb1 = apr_brigade_create(pool, list);
	APR_BRIGADE_INSERT_TAIL(bb1, title_part1);
	APR_BRIGADE_INSERT_TAIL(bb1, title_part2);
	APR_BRIGADE_INSERT_TAIL(bb1, title_part3);

	apr_bucket_brigade *bb2 = apr_brigade_create(pool, list);
	apr_file_t *fd;
	apr_file_open(&fd, __FILE__, APR_READ, 0, pool);
	APR_BRIGADE_INSERT_TAIL(bb2, apr_bucket_file_create(fd, 0, 20, pool, list));
	APR_BRIGADE_INSERT_TAIL(bb2, apr_bucket_file_create(fd, 20, 40, pool, list));
	APR_BRIGADE_INSERT_TAIL(bb2, apr_bucket_file_create(fd, 180, 40, pool, list));

	char *all;
	apr_size_t len;

	apr_brigade_pflatten(bb1, &all, &len, pool);
	printf("total len = %zd, Title:\n", len);
	printf("%s", all);

	apr_brigade_pflatten(bb2, &all, &len, pool);
	printf("total len = %zd, Content:\n", len);
	printf("%s", all);

	APR_BRIGADE_CONCAT(bb1, bb2);
	apr_brigade_pflatten(bb1, &all, &len, pool);
	printf("total len = %zd, Title:\n", len);
	printf("%s", all);




	apr_pool_destroy(pool);
	apr_terminate();
	return 0;
}
