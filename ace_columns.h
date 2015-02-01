#pragma once

#include <assert.h>

#include "ace_types.h"

#define COLUMN_STRLEN 16

typedef char column_string[COLUMN_STRLEN];

/* copies src into dest */
static __inline void column_cpy(column_string dest, column_string src)
{
	u16 i = 0;
	assert(dest);
	assert(src);
	while ((dest[i]) && (src[i]) && (i < COLUMN_STRLEN)) {
		dest[i] = src[i];
		i++;
	}
	dest[i] = 0;
}

static __inline u16 column_len(column_string sz)
{
	u16 i = 0;
	assert(sz);
	while ((sz[i]) && (i < COLUMN_STRLEN)) {
		i++;
	}
	return i;
}

typedef enum _align
{
	ALIGN_LEFT,
	ALIGN_RIGHT,
	ALIGN_CENTER
} align_t;

typedef struct _column
{
	column_string *columns;
	column_string name;
	u16 nrows;
} column_t;

typedef struct _table
{
	column_t keys;
	column_t* columns;

	u16 ncols;
	u16 nrows;
} table_t;


extern table_t* table_create(u16 r, u16 c);
extern void table_destroy(table_t* table);

/**
 * populates column with the given data.  The data is printed based on format, which is
 * equivalent to printf, taking a single parameter.  That parameter must match the type
 * of data[i]
 */
extern void column_from_int(column_t* column, align_t alignment, const char *format, int *data);
extern void column_from_float(column_t* column, align_t alignment, const char *format, float *data);
extern void column_from_str(column_t* column, align_t alignment, const char *format, column_string *data);

extern void table_set_key(table_t* table, u16 row, column_string name);

extern void table_print(table_t* table);
