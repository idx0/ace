Another Chess Engine (ACE)
===

ACE is a simple bitboard based chess engine written in C.  The initial goal is to get a working engine that can play chess via a GUI application such as [Arena](http://www.playwitharena.com).  Eventually, I hope to expand and optimize the program.  Maybe one day it will even be able to compete for a decent ranking against other chess engines.

Board Representation
---
The board is represented using a bitboard.  I bitboard is a 64-bit unsigned integer where each bit represents a square on the chess board.  The rank and file map to index bit positions as shown:

```
     a  b  c  d  e  f  g  h      a  b  c  d  e  f  g  h
   +--+--+--+--+--+--+--+--+   +--+--+--+--+--+--+--+--+
 8 |A8|B8|C8|D8|E8|F8|G8|H8| 8 |56|57|58|59|60|61|62|63| 8
   +--+--+--+--+--+--+--+--+   +--+--+--+--+--+--+--+--+ 
 7 |A7|B7|C7|D7|E7|F7|G7|H7| 7 |48|49|50|51|52|53|54|55| 7
   +--+--+--+--+--+--+--+--+   +--+--+--+--+--+--+--+--+ 
 6 |A6|B6|C6|D6|E6|F6|G6|H6| 6 |40|41|42|43|44|45|46|47| 6
   +--+--+--+--+--+--+--+--+   +--+--+--+--+--+--+--+--+ 
 5 |A5|B5|C5|D5|E5|F5|G5|H5| 5 |32|33|34|35|36|37|38|39| 5
   +--+--+--+--+--+--+--+--+   +--+--+--+--+--+--+--+--+ 
 4 |A4|B4|C4|D4|E4|F4|G4|H4| 4 |24|25|26|27|28|29|30|31| 4
   +--+--+--+--+--+--+--+--+   +--+--+--+--+--+--+--+--+ 
 3 |A3|B3|C3|D3|E3|F3|G3|H3| 3 |16|17|18|19|20|21|22|23| 3
   +--+--+--+--+--+--+--+--+   +--+--+--+--+--+--+--+--+ 
 2 |A2|B2|C2|D2|E2|F2|G2|H2| 2 | 8| 9|10|11|12|13|14|15| 2
   +--+--+--+--+--+--+--+--+   +--+--+--+--+--+--+--+--+ 
 1 |A1|B1|C1|D1|E1|F1|G1|H1| 1 | 0| 1| 2| 3| 4| 5| 6| 7| 1
   +--+--+--+--+--+--+--+--+   +--+--+--+--+--+--+--+--+
     a  b  c  d  e  f  g  h      a  b  c  d  e  f  g  h
```

I keep both a list of pieces at each index (packed into the lower 4 bytes of and unsigned char) and a set of bitboards representing pieces by color.

Moves are generated using a set of precomputed movelists and magic tables.  Movelists can be AND'd with an occupancy bitboard to generate captures or moves for pawns, knights, and kings.  For rooks, bishops, or queens, the moves are generated from a precomputed magic table by using a hashing method.  The implementation of the magic table comes from Pradyumna Kannan.  A paper about the technique can be [read at his website](http://www.pradu.us/old/Nov27_2008/Buzz/research/magic/Bitboards.pdf).