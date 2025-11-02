/*
 FILES →
    a  b  c  d  e  f  g  h
+-------------------------+
8 | ♜ ♞ ♝ ♛  ♚  ♝  ♞  ♜ | ← Rank 8
7 | ♙ ♙ ♙ ♙  ♙  ♙  ♙  ♙ | ← Rank 7
6 | .  .  .  .  .   .  .   . |
5 | .  .  .  .  .   .  .   . |
4 | .  .  .  .  .   .  .   . |
3 | .  .  .  .  .   .  .   . |
2 | ♙ ♙ ♙ ♙  ♙  ♙  ♙  ♙ | ← Rank 2
1 | ♖ ♘ ♗ ♕  ♔  ♗  ♘  ♖ | ← Rank 1
+-------------------------+

| Rank\File | a  | b  | c  | d  | e  | f  | g  | h  |
| --------- | -- | -- | -- | -- | -- | -- | -- | -- |
| 8         | 56 | 57 | 58 | 59 | 60 | 61 | 62 | 63 |
| 7         | 48 | 49 | 50 | 51 | 52 | 53 | 54 | 55 |
| 6         | 40 | …  | …  | …  | …  | …  | …  | 47 |
| …         |    |    |    |    |    |    |    |    |
| 1         | 0  | 1  | 2  | 3  | 4  | 5  | 6  | 7  |

Bits     0,  1,  2,  3, ...
Squares a1, b1, c1, d1,

5 6 7 8 9 10 11  << left  >> right
3 2 1 0 1 2 3
*/

#include <stdint.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float    f32;
typedef double   f64;

#define WHITE 0
#define BLACK 1

#define NOT_FILE_A  0xfefefefefefefefe
#define NOT_FILE_AB 0xfcfcfcfcfcfcfcfc
#define NOT_FILE_H  0x7f7f7f7f7f7f7f7f
#define NOT_FILE_GH 0x3f3f3f3f3f3f3f3f

// ::--- Board Representation ---::

u64 Black_Pawn   = 0x00FF000000000000ULL;  // rank 7
u64 Black_Rook   = 0x8100000000000000ULL;  // a8, h8
u64 Black_Knight = 0x4200000000000000ULL;  // b8, g8
u64 Black_Bishop = 0x2400000000000000ULL;  // c8, f8
u64 Black_Queen  = 0x0800000000000000ULL;  // d8
u64 Black_King   = 0x1000000000000000ULL;  // e8

u64 White_Pawn   = 0x000000000000FF00ULL;  // rank 2
u64 White_Rook   = 0x0000000000000081ULL;  // a1, h1
u64 White_Knight = 0x0000000000000042ULL;  // b1, g1
u64 White_Bishop = 0x0000000000000024ULL;  // c1, f1
u64 White_Queen  = 0x0000000000000008ULL;  // d1
u64 White_King   = 0x0000000000000010ULL;  // e1

// ::--- Game Rules ---::

u8 White_Turn;
u8 Castling_Rights;
u8 En_Passant_Square;

// ::--- Attack Masks ---::

// Fixed-move pieces (That don't depend on blocks)
u64 Pawn_Attacks[2][64];
u64 Knight_Attacks[64];
u64 King_Attacks[64];

void Pawn_Attacks_Init() {
    for(u8 square = 0; square < 64; square++) {
        u64 bit = 1ULL << square;

        // --- White Attacks ---
        u64 white = 0ULL;
        if(bit & NOT_FILE_A) white |= bit << 7; // NW
        if(bit & NOT_FILE_H) white |= bit << 9; // NE

        // --- Black Attacks ---
        u64 black = 0ULL;
        if((bit >> 7) & NOT_FILE_H) black |= bit >> 7; // SE
        if((bit >> 9) & NOT_FILE_A) black |= bit >> 9; // SW

        Pawn_Attacks[WHITE][square] = white;
        Pawn_Attacks[BLACK][square] = black;
    }
}

void Knight_Attacks_Init() {
    for(u8 square = 0; square < 64; square++) {
        u8   bit = 1ULL << square;
        u64 mask = 0ULL;
        if(bit & NOT_FILE_AB) {
            mask |= bit << 15; // 2Top  1Left
            mask |= bit << 6;  // 2Left 1Top
            mask |= bit >> 15; // 2Down 1Left
            mask |= bit >> 6;  // 2Left 1Down
        }
        if(bit & NOT_FILE_GH) {
            mask |= bit << 17; // 2Top   1Right
            mask |= bit << 10; // 2Right 1Top
            mask |= bit >> 17; // 2Down  1Right
            mask |= bit >> 10; // 2Right 1Down
        }
        Knight_Attacks[square] = mask;
    }
}

void King_Attacks_Init() {
    for(u8 square = 0; square < 64; square++) {
        u8   bit = 1ULL << square;
        u64 mask = 0ULL;
        if(bit & NOT_FILE_A) {
            mask |= bit << 1; // 1left
            mask |= bit << 7; // 1top 1left
            mask |= bit << 8; // 1top
            mask |= bit << 9; // 1top 1right
        }
        if(bit & NOT_FILE_H) {
            mask |= bit >> 1; // 1right
            mask |= bit >> 7; // 1down 1left
            mask |= bit >> 8; // 1down
            mask |= bit >> 9; // 1down 1right
        }
        King_Attacks[square] = mask;
    }
}

// Sliding pieces
// u64 Bishop_Attack(square, occupancy);
// u64 Rook_Attack(square, occupancy);
// u64 Queen_Attack(square, occupancy); // bishop ^ rook attacks


// ::--- Evaluation ---::

int BitSet256[256];  /* Used for Bit Counting */

void BitSet256_Init() {
    BitSet256[0] = 0;
    for(u8 i=1; i<255; i++) {
        BitSet256[i] = (i & 1) + BitSet256[i >> 1];
    }
}

// Count the amount of bits activated in a 64-bit number
int Count_Bits(u64 n) {
    return (BitSet256[n         & 0xff] +
            BitSet256[(n >> 8)  & 0xff] +
            BitSet256[(n >> 16) & 0xff] +
            BitSet256[(n >> 24) & 0xff] +
            BitSet256[(n >> 32) & 0xff] +
            BitSet256[(n >> 40) & 0xff] +
            BitSet256[(n >> 48) & 0xff] +
            BitSet256[(n >> 56) & 0xff]);
}

// Two or more pawns of the same color on the same file.
int Count_Doubled_Pawns(u64 pawns) {
    int count = 0;
    u64 file_mask = 0x0101010101010101;
    for(u8 file=0; file<8; file++) {
        int pawns_in_file = Count_Bits(pawns & (file_mask << file));
        if(pawns_in_file > 1) {
            count += pawns_in_file - 1;
        }
    }
    return count;
}

// A pawn that has no friendly pawns on the adjacent files.
int Count_Isolated_Pawns(u64 pawns) {
    u64 file_a = 0x0101010101010101;
    u64 file_h = file_a << 7;
    u64 full   = ~0ULL; // 0xFFFFFFFFFFFFFFFF

    // 'file' bitboard mask for each file that contains at least 1 pawn.
    // Set every bit on that file
    u64 files = 0ULL;
    for(u8 file=0; file<8; file++) {
       u64 file_mask = file_a << file;      // mask for current file
       if(pawns & file_mask) {              // if any pawns in the current file
           files |= file_mask;              // mask the whole file as occupied
       }
    }

    // Compute files adjacent to any occupied file.
    // Avoid wrap-around when shifting
    u64 left_files     = (files << 1) & ~file_a;
    u64 right_files    = (files >> 1) & ~file_h;
    u64 neighbor_files = left_files | right_files;

    // Pawns that are in files that have no neighbors
    u64 isolated_mask = pawns & ~neighbor_files;

    return Count_Bits(isolated_mask);;
}

// A pawn whose forward square (in front of it) is occupied by any pawn (typically an enemy pawn).
int Count_Blocked_Pawns(u64 pawns) {
    u64 all     = White_Pawn | Black_Pawn;
    u64 blocked = White_Turn == 1
        ? pawns & (all >> 8)
        : pawns & (all << 8);
    return Count_Bits(blocked);
}


// This evaluated relative to the side being evaluated
f32 Evaluate_Board() {
    // TODO: for pieces that are just 1, only check different than 0
    int King_Score           = 200 * White_Turn * (Count_Bits(White_King)    - Count_Bits(Black_King));
    int Queen_Score          =   9 * White_Turn * (Count_Bits(White_Queen)   - Count_Bits(Black_Queen));
    int Rook_Score           =   5 * White_Turn * (Count_Bits(White_Rook)    - Count_Bits(Black_Rook));
    int Bishops_Knight_Score =   3 * White_Turn * ((Count_Bits(White_Bishop) - Count_Bits(Black_Bishop))
                                   + (Count_Bits(White_Knight)  - Count_Bits(Black_Knight)));
    int Pawns_Score          =   1 * White_Turn * (Count_Bits(White_Pawn)   - Count_Bits(Black_Pawn));

    int White_Doubled_Pawns  = Count_Doubled_Pawns(White_Pawn);
    int Black_Doubled_Pawns  = Count_Doubled_Pawns(Black_Pawn);

    int White_Isolated_Pawns = Count_Isolated_Pawns(White_Pawn);
    int Black_Isolated_Pawns = Count_Isolated_Pawns(Black_Pawn);

    int White_Blocked_Pawns  = Count_Blocked_Pawns(White_Pawn);
    int Black_Blocked_Pawns  = Count_Blocked_Pawns(Black_Pawn);


    /* ::--- Mobility ---:: */
    {
        u64 occupancy = White_Pawn | White_Bishop | White_Knight
                      | White_Rook  | White_Queen  | White_King;

    }


    int White_Mobility = 0.0; // TODO: fix this
    int Black_Mobility = 0.0;

    return (King_Score
          + Queen_Score
          + Rook_Score
          + Bishops_Knight_Score
          + Pawns_Score
        - 0.5*((White_Doubled_Pawns  - Black_Doubled_Pawns)
              +(White_Blocked_Pawns  - Black_Blocked_Pawns)
              +(White_Isolated_Pawns - Black_Isolated_Pawns))
        + 0.1*(White_Mobility - Black_Mobility)
    );
}

int main() {
    BitSet256_Init();
    Pawn_Attacks_Init();

    // Evaluate_Board();

    return 0;
}
