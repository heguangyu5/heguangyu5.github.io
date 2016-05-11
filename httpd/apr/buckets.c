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

	
	APR_BUCKET_INSERT_BEFORE(title_part1, title_part2);
	APR_BUCKET_INSERT_BEFORE(title_part1, title_part3);

	apr_bucket *cur = title_part1;
	do {
		const char *str;
		apr_size_t len;
		apr_bucket_read(cur, &str, &len, 0);
		printf("%s", str);
		cur = APR_BUCKET_NEXT(cur);
	} while (cur != title_part1);

	APR_BUCKET_INSERT_BEFORE(title_part1, apr_bucket_eos_create(list));
	cur = title_part1;
	while (!APR_BUCKET_IS_EOS(cur)) {
		const char *str;
		apr_size_t len;
		apr_bucket_read(cur, &str, &len, 0);
		printf("%s", str);
		cur = APR_BUCKET_NEXT(cur);
	}


	apr_pool_destroy(pool);
	apr_terminate();
	return 0;
}
