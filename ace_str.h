#pragma once

#undef NDEBUG
#include <assert.h>

#include "ace_types.h"

static const char WHITESPACE[6] = " \t\r\n";

/* in-place string trimming functions */
static void str_trim_ex(char** sz, const char *pattern);
static void str_ltrim_ex(char** sz, const char* pattern);
static void str_rtrim_ex(char* sz, const char* pattern);

/* these macros provide a convient wrapper for trimming whitespace */
#define str_trim(sz) (str_trim_ex((sz), WHITESPACE))
#define str_ltrim(sz) (str_ltrim_ex((sz), WHITESPACE))
#define str_rtrim(sz) (str_rtrim_ex((sz), WHITESPACE))

/* returns 1 if sz contains ch, 0 otherwise */
static __inline int str_contains(const char* sz, char ch)
{
	while (*sz) {
		if (*sz == ch) return 1;
		sz++;
	}
	return 0;
}

/* returns non-zero if c is a valid value for rank */
int isrank(const char c);

/* returns non-zero if c is a valid value for file */
int isfile(const char c);

/* returns non-zero if c is non-zero (1-9) */
int isnonzero(const char c);

/* returns non-zero if c is zero (0) */
int iszero(const char c);

/* returns non-zero if c is a white piece */
int iswhite(const char c);

/* returns non-zero if c is a black piece */
int isblack(const char c);

/* returns true if a major or minor piece */
int ismaterial(const char c);

#define isnumber(c) (isnonzero(c) || iszero(c))

/* piece, file, rank from char */
piece_t piece_from_char(const char c);
board_file_t file_from_char(const char c);
board_rank_t rank_from_char(const char c);

char char_from_piece(piece_t p);
char char_from_file(board_file_t f);
char char_from_rank(board_rank_t r);

/* string comparison */
#define str_compare(a, b) (strcmp((a), (b)))
#define str_equal(a, b) (strcmp((a), (b)) == 0)

extern int str_icompare(const char* a, const char *b);

/* returns non-zero if sz begins with prefix */
extern int str_begin(const char* prefix, const char *sz);
extern int str_ibegin(const char* prefix, const char *sz);
