#include <stdlib.h>

#include "ace_columns.h"

static void column_create(column_t* column, u16 rows)
{
	if (!column) return;

	column->columns = (column_string*)malloc(sizeof(column_string) * rows);
	memset(column->columns, 0, sizeof(column_string) * rows);

	column->nrows = rows;
}

static void column_destroy(column_t* column)
{
	if (column) free(column->columns);
}

table_t* table_create(u16 r, u16 c)
{
	u16 i;
	table_t* t = (table_t*)malloc(sizeof(table_t));

	if (t) {
		column_create(&t->keys, r);

		t->columns = (column_t*)malloc(sizeof(column_t) * c);
		for (i = 0; i < c; i++) {
			column_create(&t->columns[i], r);
		}

		t->ncols = c;
		t->nrows = r;
	}

	return t;
}


void table_destroy(table_t* table)
{
	u16 i;

	if (!table) return;

	column_destroy(&table->keys);
	for (i = 0; i < table->ncols; i++) {
		column_destroy(&table->columns[i]);
	}
	free(table->columns);

	free(table);
}

static void align_in(column_string out, align_t alignment, column_string in, u16 inlen)
{
	u16 pad = COLUMN_STRLEN - inlen - 1;
	u16 i = 0, cnt = 0;
	char *s;

	if (inlen >= COLUMN_STRLEN) {
		inlen = COLUMN_STRLEN - 1;
		alignment = ALIGN_LEFT;
	}

	memset(out, ' ', COLUMN_STRLEN);

	switch (alignment) {
	case ALIGN_CENTER:	/* fall through */
		pad /= 2;
	case ALIGN_RIGHT:
		s = out;
		i = pad;

		while ((i < COLUMN_STRLEN) && (cnt < inlen)) {
			s[i] = in[cnt];
			i++;
			cnt++;
		}
		break;
	case ALIGN_LEFT:
		s = out;

		while (i < inlen) {
			s[i] = in[i];
			i++;
		}
		break;
	}

	out[COLUMN_STRLEN - 1] = 0;
}

void column_from_int(column_t* column, align_t alignment, const char *format, int *data)
{
	column_string buf;
	u16 i, n;

	if (!column) return;

	for (i = 0; i < column->nrows; i++) {
		n = snprintf(buf, COLUMN_STRLEN, format, data[i]);

		align_in(column->columns[i], alignment, buf, n);
	}
}

void column_from_float(column_t* column, align_t alignment, const char *format, float *data)
{
	column_string buf;
	u16 i, n;

	if (!column) return;

	for (i = 0; i < column->nrows; i++) {
		n = snprintf(buf, COLUMN_STRLEN, format, data[i]);

		align_in(column->columns[i], alignment, buf, n);
	}
}

void column_from_str(column_t* column, align_t alignment, const char *format, column_string *data)
{
	column_string buf;
	u16 i, n;

	if (!column) return;

	for (i = 0; i < column->nrows; i++) {
		n = snprintf(buf, COLUMN_STRLEN, format, data[i]);

		align_in(column->columns[i], alignment, buf, n);
	}
}

void table_set_key(table_t* table, u16 row, column_string name)
{
	if (!table) return;
	if (table->nrows < row) return;

	align_in(&table->keys.columns[row], ALIGN_LEFT, name, column_len(name));
}

void table_print(table_t* table)
{
	u16 r, c;
	if (!table) return;

	for (r = 0; r < table->nrows; r++) {
		printf("%s ", table->keys.columns[r]);

		for (c = 0; c < table->ncols; c++) {
			printf("%s ", table->columns[c].columns[r]);
		}

		printf("\n");
	}
}
