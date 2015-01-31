#include <ctype.h>

#include "ace_str.h"

int isnonzero(const char c)
{
	return ((c >= '1') && (c <= '9'));
}


int isrank(const char c)
{
	return (isnonzero(c) && (c != '9'));
}


int isfile(const char c)
{
	return ((c >= 'a') && (c <= 'h'));
}


int iszero(const char c)
{
	return (c == '0');
}


int iswhite(const char c)
{
	return ((c == 'P') || (c == 'N') || (c == 'B') ||
			(c == 'R') || (c == 'Q') || (c == 'K'));
}


int isblack(const char c)
{
	return ((c == 'p') || (c == 'n') || (c == 'b') ||
			(c == 'r') || (c == 'q') || (c == 'k'));
}

int ismaterial(const char c)
{
	return ((c == 'b') || (c == 'n') || (c == 'Q') || (c == 'r') ||
			(c == 'B') || (c == 'N') || (c == 'q') || (c == 'R'));
}


void str_trim_ex(char** sz, const char *pattern)
{
	str_ltrim_ex(sz, pattern);
	str_rtrim_ex(*sz, pattern);
}


void str_ltrim_ex(char** sz, const char* pattern)
{
	assert(sz);
	assert(*sz);

	char* s = *sz;

	while (*s) {
		if (str_contains(pattern, *s)) {
			s++;
			*sz++;
		}
		else { break; }
	}
}


void str_rtrim_ex(char* sz, const char* pattern)
{
	char* s = sz;
	char* last = NULL;

	assert(sz);

	while (*s) {
		if (!str_contains(pattern, *s)) {
			/* keep track of the last non-trimmable character */
			last = s;
		}
		s++;
	}

	if (last) {
		/* move the null character */
		last++;
		*last = 0;
	} else {
		/* string was all trimmable */
		*sz = 0;
	}
}


int str_icompare(const char* a, const char *b)
{
	int r = 0;
	assert(a);
	assert(b);

	while (1) {
		r = tolower(*a) - tolower(*b);
		
		if ((r) || (*a == 0)) return r;

		a++;
		b++;
	}

	return r;
}


int str_begin(const char* prefix, const char *sz)
{
	while (*prefix) {
		if (*sz == 0) return 0;
		if (*sz != *prefix) return 0;
		prefix++;
		sz++;
	}

	return 1;
}


int str_ibegin(const char* prefix, const char *sz)
{
	while (*prefix) {
		if (*sz == 0) return 0;
		if (tolower(*sz) != tolower(*prefix)) return 0;
		prefix++;
		sz++;
	}

	return 1;
}


piece_t piece_from_char(const char c)
{
	piece_t p = INVALID_PIECE;
	set_piece_color(p, WHITE);

	/* is c a valid character */
	switch (c) {
	case 'p': set_piece_color(p, BLACK);
	case 'P': set_piece_type(p, PAWN); break;
	case 'n': set_piece_color(p, BLACK);
	case 'N': set_piece_type(p, KNIGHT); break;
	case 'b': set_piece_color(p, BLACK);
	case 'B': set_piece_type(p, BISHOP); break;
	case 'r': set_piece_color(p, BLACK);
	case 'R': set_piece_type(p, ROOK); break;
	case 'q': set_piece_color(p, BLACK);
	case 'Q': set_piece_type(p, QUEEN); break;
	case 'k': set_piece_color(p, BLACK);
	case 'K': set_piece_type(p, KING); break;
	default: break;
	}

	return p;
}


board_file_t file_from_char(const char c)
{
	board_file_t f = FA;

	if (isfile(c)) {
		f = (board_file_t)(c - 'a');
	}

	return f;
}


board_rank_t rank_from_char(const char c)
{
	board_rank_t r = R1;

	if (isrank(c)) {
		r = (board_rank_t)(c - '1');
	}

	return r;
}


char char_from_piece(piece_t p)
{
	static const char sPIECES[6] = { 'P', 'N', 'B', 'R', 'Q', 'K' };

	if (piece_color(p) == BLACK) {
		return tolower(sPIECES[piece_type(p)]);
	}

	return sPIECES[piece_type(p)];
}


char char_from_file(board_file_t f)
{
	return ('a' + f);
}


char char_from_rank(board_rank_t r)
{
	return ('1' + r);
}
