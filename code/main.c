/*

 FILES →
 a  b  c  d  e  f  g  h
+-------------------------+
8 | ♜  ♞  ♝  ♛  ♚  ♝  ♞  ♜ | ← Rank 8
7 | ♙  ♙  ♙  ♙  ♙  ♙  ♙  ♙ | ← Rank 7
6 | .   .  .  .  .  .  .  .     |
5 | .   .  .  .  .  .  .  .     |
4 | .   .  .  .  .  .  .  .     |
3 | .   .  .  .  .  .  .  .     |
2 | ♙  ♙  ♙  ♙  ♙  ♙  ♙  ♙ | ← Rank 2
1 | ♖  ♘  ♗  ♕  ♔  ♗  ♘  ♖ | ← Rank 1
+-------------------------+

*/


#include <stdint.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float    f32;
typedef double   f64;

// ::--- Board Representation ---::

u64 White_Pawns  = 0xffULL << 48;
u64 Black_Pawns  = 0xffULL << 8;
u64 White_Bishop = 0x24ULL << 56;
u64 Black_Bishop = 0x24ULL;
u64 White_Knight = 0x42ULL << 56;
u64 Black_Knight = 0x42ULL;
u64 White_Rook   = 0x81ULL << 56;
u64 Black_Rook   = 0x81ULL;
u64 White_Queen  = 0x10ULL << 56;
u64 Black_Queen  = 0x10ULL;
u64 White_King   = 0x8ULL << 56;
u64 Black_King   = 0x8ULL;

// ::--- Game Rules ---::

int White_Turn;         /* White=1, Black=-1 */
u8  Castling_Rights;
int En_Passant_Square;

// ::--- Attack Masks ---::
//
// Fixed-move pieces (That don't depend on blocks)
u64 Pawn_Attacks[2][64];
u64 Knight_Attacks[64];
u64 King_Attacks[64];

void Pawn_Attacks_Init() {
    for(u8 color=0; color<2; color++) {
        for(u8 rank=0; rank<8; rank++) {
            for(u8 file=0; file<8; file++) {
                u64 file_mask   = (0x0101010101010101 << (file));
                u64 rank_mask   = (0xff << (rank * 8));
                u64 square_mask = (1 << ((rank * 8) + file));
                Pawn_Attacks[color][rank + file] = (file_mask | rank_mask) & ~square_mask;
            }
        }
    }
}

// Sliding pieces
// u64 Bishop_Attack(square, occupancy);
// u64 Rook_Attack(square, occupancy);
// u64 Queen_Attack(square, occupancy); // bishop ^ rook attacks


// ::--- Evaluation ---::

int BitSet256[256];

void BitSet256_Init() {
    BitSet256[0] = 0;
    for(u8 i=1; i<255; i++) {
        BitSet256[i] = (i & 1) + BitSet256[i >> 1];
    }
}

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
    u64 all     = White_Pawns | Black_Pawns;
    u64 blocked = White_Turn == 1
        ? pawns & (all >> 8)
        : pawns & (all << 8);
    return Count_Bits(blocked);
}


/* This evaluated relative to the side being evaluated */
f32 Evaluate_Board() {
    // TODO: for pieces that are just 1, only check different than 0
    int King_Score           = 200 * White_Turn * (Count_Bits(White_King)    - Count_Bits(Black_King));
    int Queen_Score          =   9 * White_Turn * (Count_Bits(White_Queen)   - Count_Bits(Black_Queen));
    int Rook_Score           =   5 * White_Turn * (Count_Bits(White_Rook)    - Count_Bits(Black_Rook));
    int Bishops_Knight_Score =   3 * White_Turn * ((Count_Bits(White_Bishop) - Count_Bits(Black_Bishop))
                                   + (Count_Bits(White_Knight)  - Count_Bits(Black_Knight)));
    int Pawns_Score          =   1 * White_Turn * (Count_Bits(White_Pawns)   - Count_Bits(Black_Pawns));

    int White_Doubled_Pawns  = Count_Doubled_Pawns(White_Pawns);
    int Black_Doubled_Pawns  = Count_Doubled_Pawns(Black_Pawns);

    int White_Isolated_Pawns = Count_Isolated_Pawns(White_Pawns);
    int Black_Isolated_Pawns = Count_Isolated_Pawns(Black_Pawns);

    int White_Blocked_Pawns  = Count_Blocked_Pawns(White_Pawns);
    int Black_Blocked_Pawns  = Count_Blocked_Pawns(Black_Pawns);


    /* ::--- Mobility ---:: */
    {
        u64 occupancy = White_Pawns | White_Bishop | White_Knight
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
