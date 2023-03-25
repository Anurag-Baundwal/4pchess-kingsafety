#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <ostream>
#include <sstream>
#include <unordered_map>
#include <utility>
#include <vector>

#include "board.h"

namespace chess {

const BoardLocation kRedInitialRookLocationKingside(13, 10);
const BoardLocation kRedInitialRookLocationQueenside(13, 3);
const BoardLocation kBlueInitialRookLocationKingside(10, 0);
const BoardLocation kBlueInitialRookLocationQueenside(3, 0);
const BoardLocation kYellowInitialRookLocationKingside(0, 3);
const BoardLocation kYellowInitialRookLocationQueenside(0, 10);
const BoardLocation kGreenInitialRookLocationKingside(13, 3);
const BoardLocation kGreenInitialRookLocationQueenside(13, 10);

const Player kRedPlayer = Player(RED);
const Player kBluePlayer = Player(BLUE);
const Player kYellowPlayer = Player(YELLOW);
const Player kGreenPlayer = Player(GREEN);

const Piece kRedPawn(kRedPlayer, PAWN);
const Piece kRedKnight(kRedPlayer, KNIGHT);
const Piece kRedBishop(kRedPlayer, BISHOP);
const Piece kRedRook(kRedPlayer, ROOK);
const Piece kRedQueen(kRedPlayer, QUEEN);
const Piece kRedKing(kRedPlayer, KING);

const Piece kBluePawn(kBluePlayer, PAWN);
const Piece kBlueKnight(kBluePlayer, KNIGHT);
const Piece kBlueBishop(kBluePlayer, BISHOP);
const Piece kBlueRook(kBluePlayer, ROOK);
const Piece kBlueQueen(kBluePlayer, QUEEN);
const Piece kBlueKing(kBluePlayer, KING);

const Piece kYellowPawn(kYellowPlayer, PAWN);
const Piece kYellowKnight(kYellowPlayer, KNIGHT);
const Piece kYellowBishop(kYellowPlayer, BISHOP);
const Piece kYellowRook(kYellowPlayer, ROOK);
const Piece kYellowQueen(kYellowPlayer, QUEEN);
const Piece kYellowKing(kYellowPlayer, KING);

const Piece kGreenPawn(kGreenPlayer, PAWN);
const Piece kGreenKnight(kGreenPlayer, KNIGHT);
const Piece kGreenBishop(kGreenPlayer, BISHOP);
const Piece kGreenRook(kGreenPlayer, ROOK);
const Piece kGreenQueen(kGreenPlayer, QUEEN);
const Piece kGreenKing(kGreenPlayer, KING);

const Piece* kPieceSet[4][6] = {
  &kRedPawn,
  &kRedKnight,
  &kRedBishop,
  &kRedRook,
  &kRedQueen,
  &kRedKing,

  &kBluePawn,
  &kBlueKnight,
  &kBlueBishop,
  &kBlueRook,
  &kBlueQueen,
  &kBlueKing,

  &kYellowPawn,
  &kYellowKnight,
  &kYellowBishop,
  &kYellowRook,
  &kYellowQueen,
  &kYellowKing,

  &kGreenPawn,
  &kGreenKnight,
  &kGreenBishop,
  &kGreenRook,
  &kGreenQueen,
  &kGreenKing,
};



namespace {

int64_t rand64() {
  int32_t t0 = rand();
  int32_t t1 = rand();
  return (((int64_t)t0) << 32) + (int64_t)t1;
}


void AddPawnMoves(
    std::vector<Move>& moves,
    const BoardLocation& from,
    const BoardLocation& to,
    const PlayerColor color,
    const Piece* capture = nullptr,
    const std::optional<BoardLocation>& en_passant_location = std::nullopt,
    const Piece* en_passant_capture = nullptr) {
  bool is_promotion = false;

  constexpr int kRedPromotionRow = 3;
  constexpr int kYellowPromotionRow = 10;
  constexpr int kBluePromotionCol = 10;
  constexpr int kGreenPromotionCol = 3;

  switch (color) {
  case RED:
    is_promotion = to.GetRow() == kRedPromotionRow;
    break;
  case BLUE:
    is_promotion = to.GetCol() == kBluePromotionCol;
    break;
  case YELLOW:
    is_promotion = to.GetRow() == kYellowPromotionRow;
    break;
  case GREEN:
    is_promotion = to.GetCol() == kGreenPromotionCol;
    break;
  default:
    assert(false);
    break;
  }

  if (is_promotion) {
    moves.emplace_back(from, to, capture, en_passant_location, en_passant_capture, KNIGHT);
    moves.emplace_back(from, to, capture, en_passant_location, en_passant_capture, BISHOP);
    moves.emplace_back(from, to, capture, en_passant_location, en_passant_capture, ROOK);
    moves.emplace_back(from, to, capture, en_passant_location, en_passant_capture, QUEEN);
  } else {
    moves.emplace_back(from, to, capture, en_passant_location, en_passant_capture, std::nullopt);
  }
}

}  // namespace

void Board::GetPawnMoves(
    std::vector<Move>& moves,
    const BoardLocation& from,
    const Piece& piece) const {
  PlayerColor color = piece.GetColor();
  Team team = piece.GetTeam();

  // Move forward
  int delta_rows = 0;
  int delta_cols = 0;
  bool not_moved = false;
  switch (color) {
  case RED:
    delta_rows = -1;
    not_moved = from.GetRow() == 12;
    break;
  case BLUE:
    delta_cols = 1;
    not_moved = from.GetCol() == 1;
    break;
  case YELLOW:
    delta_rows = 1;
    not_moved = from.GetRow() == 1;
    break;
  case GREEN:
    delta_cols = -1;
    not_moved = from.GetCol() == 12;
    break;
  default:
    assert(false);
    break;
  }

  BoardLocation to = from.Relative(delta_rows, delta_cols);
  if (IsLegalLocation(to)) {
    const auto* other_piece = GetPiece(to);
    if (other_piece == nullptr) {
      // Advance once square
      AddPawnMoves(moves, from, to, piece.GetColor());
      // Initial move (advance 2 squares)
      if (not_moved) {
        to = from.Relative(delta_rows * 2, delta_cols * 2);
        const auto* other_piece = GetPiece(to);
        if (other_piece == nullptr) {
          AddPawnMoves(moves, from, to, piece.GetColor());
        }
      }
    } else {
      // En-passant
      if (other_piece->GetPieceType() == PAWN
          && piece.GetTeam() != other_piece->GetTeam()) {
        if (!moves_.empty()) {
          const auto& last_move = moves_.back();
          if (last_move.To() == to
              && last_move.ManhattanDistance() == 2) {
            const BoardLocation& moved_from = last_move.From();
            int delta_row = to.GetRow() - moved_from.GetRow();
            int delta_col = to.GetCol() - moved_from.GetCol();
            BoardLocation enpassant_to = moved_from.Relative(
                delta_row / 2, delta_col / 2);
            AddPawnMoves(moves, from, enpassant_to, piece.GetColor(),
                         nullptr, to, other_piece);
          }
        }
      }
    }
  }

  // Non-enpassant capture
  bool check_cols = team == RED_YELLOW;
  int capture_row, capture_col;
  for (int incr = 0; incr < 2; ++incr) {
    capture_row = from.GetRow() + delta_rows;
    capture_col = from.GetCol() + delta_cols;
    if (check_cols) {
      capture_col += incr == 0 ? -1 : 1;
    } else {
      capture_row += incr == 0 ? -1 : 1;
    }
    if (IsLegalLocation(capture_row, capture_col)) {
      const auto* other_piece = GetPiece(capture_row, capture_col);
      if (other_piece != nullptr
          && other_piece->GetTeam() != team) {
        AddPawnMoves(moves, from, BoardLocation(capture_row, capture_col),
            piece.GetColor(), other_piece);
      }
    }
  }

}

void Board::GetKnightMoves(
    std::vector<Move>& moves,
    const BoardLocation& from,
    const Piece& piece) const {

  int delta_row, delta_col;
  for (int pos_row_sign = 0; pos_row_sign < 2; ++pos_row_sign) {
    for (int abs_delta_row = 1; abs_delta_row < 3; ++abs_delta_row) {
      delta_row = pos_row_sign > 0 ? abs_delta_row : -abs_delta_row;
      for (int pos_col_sign = 0; pos_col_sign < 2; ++pos_col_sign) {
        int abs_delta_col = abs_delta_row == 1 ? 2 : 1;
        delta_col = pos_col_sign > 0 ? abs_delta_col : -abs_delta_col;
        BoardLocation to = from.Relative(delta_row, delta_col);
        if (IsLegalLocation(to)) {
          const auto* capture = GetPiece(to);
          if (capture == nullptr
              || capture->GetTeam() != piece.GetTeam()) {
            moves.emplace_back(from, to, capture);
          }
        }
      }
    }
  }

}

void Board::AddMovesFromIncrMovement(
    std::vector<Move>& moves,
    const Piece& piece,
    const BoardLocation& from,
    int incr_row,
    int incr_col,
    const std::optional<CastlingRights>& initial_castling_rights,
    const std::optional<CastlingRights>& castling_rights) const {
  BoardLocation to = from.Relative(incr_row, incr_col);
  while (IsLegalLocation(to)) {
    const auto* capture = GetPiece(to);
    if (capture == nullptr) {
      moves.emplace_back(from, to, nullptr, initial_castling_rights,
          castling_rights);
    } else {
      if (capture->GetTeam() != piece.GetTeam()) {
        moves.emplace_back(from, to, capture, initial_castling_rights,
            castling_rights);
      }
      break;
    }
    to = to.Relative(incr_row, incr_col);
  }
}

void Board::GetBishopMoves(
    std::vector<Move>& moves,
    const BoardLocation& from,
    const Piece& piece) const {

  for (int pos_row = 0; pos_row < 2; ++pos_row) {
    for (int pos_col = 0; pos_col < 2; ++pos_col) {
      AddMovesFromIncrMovement(
          moves, piece, from, pos_row ? 1 : -1, pos_col ? 1 : -1);
    }
  }
}

void Board::GetRookMoves(
    std::vector<Move>& moves,
    const BoardLocation& from,
    const Piece& piece) const {

  // Update castling rights
  std::optional<CastlingRights> initial_castling_rights;
  std::optional<CastlingRights> castling_rights;
  std::optional<CastlingType> castling_type = GetRookLocationType(
      piece.GetPlayer(), from);
  if (castling_type.has_value()) {
    const auto& castling_it = castling_rights_.find(piece.GetPlayer());
    if (castling_it != castling_rights_.end()) {
      const auto& curr_rights = castling_it->second;
      if (curr_rights.Kingside() || curr_rights.Queenside()) {
        if (castling_type == KINGSIDE) {
          if (curr_rights.Kingside()) {
            initial_castling_rights = curr_rights;
            castling_rights = CastlingRights(false, curr_rights.Queenside());
          }
        } else {
          if (curr_rights.Queenside()) {
            initial_castling_rights = curr_rights;
            castling_rights = CastlingRights(curr_rights.Kingside(), false);
          }
        }
      }
    }
  }

  for (int do_pos_incr = 0; do_pos_incr < 2; ++do_pos_incr) {
    int incr = do_pos_incr > 0 ? 1 : -1;
    for (int do_incr_row = 0; do_incr_row < 2; ++do_incr_row) {
      int incr_row = do_incr_row > 0 ? incr : 0;
      int incr_col = do_incr_row > 0 ? 0 : incr;
      AddMovesFromIncrMovement(moves, piece, from, incr_row, incr_col,
          initial_castling_rights, castling_rights);
    }
  }
}

void Board::GetQueenMoves(
    std::vector<Move>& moves,
    const BoardLocation& from,
    const Piece& piece) const {
  GetBishopMoves(moves, from, piece);
  GetRookMoves(moves, from, piece);
}

void Board::GetKingMoves(
    std::vector<Move>& moves,
    const BoardLocation& from,
    const Piece& piece) const {

  std::optional<CastlingRights> initial_castling_rights;
  std::optional<CastlingRights> castling_rights;
  const auto& castling_it = castling_rights_.find(piece.GetPlayer());
  if (castling_it != castling_rights_.end()) {
    const auto& curr_rights = castling_it->second;
    if (curr_rights.Kingside() || curr_rights.Queenside()) {
      initial_castling_rights = curr_rights;
      castling_rights = CastlingRights(false, false);
    }
  }


  for (int delta_row = -1; delta_row < 2; ++delta_row) {
    for (int delta_col = -1; delta_col < 2; ++delta_col) {
      if (delta_row == 0 && delta_col == 0) {
        continue;
      }
      BoardLocation to = from.Relative(delta_row, delta_col);
      if (IsLegalLocation(to)) {
        const auto* capture = GetPiece(to);
        if (capture == nullptr
            || capture->GetTeam() != piece.GetTeam()) {
          moves.emplace_back(from, to, capture, initial_castling_rights,
              castling_rights);
        }
      }
    }
  }

  if (castling_it != castling_rights_.end()) {
    const auto& curr_rights = castling_it->second;
    Team other_team = OtherTeam(piece.GetTeam());
    for (int is_kingside = 0; is_kingside < 2; ++is_kingside) {
      bool allowed = is_kingside ? curr_rights.Kingside() :
        curr_rights.Queenside();
      if (allowed) {
        std::vector<BoardLocation> squares_between;
        BoardLocation rook_location;

        switch (piece.GetColor()) {
        case RED:
          if (is_kingside) {
            squares_between = {
              from.Relative(0, 1),
              from.Relative(0, 2),
            };
            rook_location = from.Relative(0, 3);
          } else {
            squares_between = {
              from.Relative(0, -1),
              from.Relative(0, -2),
              from.Relative(0, -3),
            };
            rook_location = from.Relative(0, -4);
          }
          break;
        case BLUE:
          if (is_kingside) {
            squares_between = {
              from.Relative(1, 0),
              from.Relative(2, 0),
            };
            rook_location = from.Relative(3, 0);
          } else {
            squares_between = {
              from.Relative(-1, 0),
              from.Relative(-2, 0),
              from.Relative(-3, 0),
            };
            rook_location = from.Relative(-4, 0);
          }
          break;
        case YELLOW:
          if (is_kingside) {
            squares_between = {
              from.Relative(0, -1),
              from.Relative(0, -2),
            };
            rook_location = from.Relative(0, -3);
          } else {
            squares_between = {
              from.Relative(0, -1),
              from.Relative(0, -2),
              from.Relative(0, -3),
            };
            rook_location = from.Relative(0, -4);
          }
          break;
        case GREEN:
          if (is_kingside) {
            squares_between = {
              from.Relative(-1, 0),
              from.Relative(-2, 0),
            };
            rook_location = from.Relative(-3, 0);
          } else {
            squares_between = {
              from.Relative(1, 0),
              from.Relative(2, 0),
              from.Relative(3, 0),
            };
            rook_location = from.Relative(4, 0);
          }
          break;
        default:
          assert(false);
          break;
        }

        // Make sure that there are no pieces between the king and rook
        bool piece_between = false;
        for (const auto& loc : squares_between) {
          if (GetPiece(loc) != nullptr) {
            piece_between = true;
            break;
          }
        }

        if (!piece_between) {
          // Make sure the king is not currently in or would pass through check
          if (!IsAttackedByTeam(other_team, squares_between[0])
              && !IsAttackedByTeam(other_team, from)) {
            // Additionally move the castle
            const auto* rook = GetPiece(rook_location);
            // Sanity check: the rook should be there
            if (rook != nullptr
                && rook->GetPieceType() == ROOK
                && rook->GetTeam() == piece.GetTeam()) {
              SimpleMove rook_move(rook_location, squares_between[0], rook);
              moves.emplace_back(from, squares_between[1], rook_move,
                  initial_castling_rights, castling_rights);
            }
          }
        }
      }
    }
  }
}

bool Board::RookAttacks(
    const BoardLocation& rook_loc,
    const BoardLocation& other_loc) const {
  if (rook_loc.GetRow() == other_loc.GetRow()) {
    bool piece_between = false;
    for (int col = std::min(rook_loc.GetCol(), other_loc.GetCol()) + 1;
         col < std::max(rook_loc.GetCol(), other_loc.GetCol());
         ++col) {
      if (GetPiece(rook_loc.GetRow(), col) != nullptr) {
        piece_between = true;
        break;
      }
    }
    if (!piece_between) {
      return true;
    }
  }
  if (rook_loc.GetCol() == other_loc.GetCol()) {
    bool piece_between = false;
    for (int row = std::min(rook_loc.GetRow(), other_loc.GetRow()) + 1;
         row < std::max(rook_loc.GetRow(), other_loc.GetRow());
         ++row) {
      if (GetPiece(row, rook_loc.GetCol()) != nullptr) {
        piece_between = true;
        break;
      }
    }
    if (!piece_between) {
      return true;
    }
  }
  return false;
}

bool Board::BishopAttacks(
    const BoardLocation& bishop_loc,
    const BoardLocation& other_loc) const {
  int delta_row = bishop_loc.GetRow() - other_loc.GetRow();
  int delta_col = bishop_loc.GetCol() - other_loc.GetCol();
  if (std::abs(delta_row) == std::abs(delta_col)) {
    int row;
    int col;
    int col_incr;
    int row_max;
    if (bishop_loc.GetRow() < other_loc.GetRow()) {
      row = bishop_loc.GetRow();
      col = bishop_loc.GetCol();
      row_max = other_loc.GetRow();
      col_incr = bishop_loc.GetCol() < other_loc.GetCol() ? 1 : -1;
    } else {
      row = other_loc.GetRow();
      col = other_loc.GetCol();
      row_max = bishop_loc.GetRow();
      col_incr = other_loc.GetCol() < bishop_loc.GetCol() ? 1 : -1;
    }
    row++;
    col += col_incr;
    bool piece_between = false;
    while (row < row_max) {
      if (GetPiece(row, col) != nullptr) {
        piece_between = true;
        break;
      }

      ++row;
      col += col_incr;
    }
    return !piece_between;
  }
  return false;
}

bool Board::KingAttacks(
    const BoardLocation& king_loc,
    const BoardLocation& other_loc) const {
  if ((std::abs(king_loc.GetRow() - other_loc.GetRow())
        + std::abs(king_loc.GetCol() - other_loc.GetCol())) < 2) {
    return true;
  }
  return false;
}

bool Board::KnightAttacks(
    const BoardLocation& knight_loc,
    const BoardLocation& other_loc) const {
  int abs_row_diff = std::abs(knight_loc.GetRow() - other_loc.GetRow());
  int abs_col_diff = std::abs(knight_loc.GetCol() - other_loc.GetCol());
  return (abs_row_diff == 1 && abs_col_diff == 2)
    || (abs_row_diff == 2 && abs_col_diff == 1);
}

bool Board::PawnAttacks(
    const BoardLocation& pawn_loc,
    PlayerColor pawn_color,
    const BoardLocation& other_loc) const {
  int row_diff = other_loc.GetRow() - pawn_loc.GetRow();
  int col_diff = other_loc.GetCol() - pawn_loc.GetCol();
  switch (pawn_color) {
  case RED:
    return row_diff == -1 && (std::abs(col_diff) == 1);
  case BLUE:
    return col_diff == 1 && (std::abs(row_diff) == 1);
  case YELLOW:
    return row_diff == 1 && (std::abs(col_diff) == 1);
  case GREEN:
    return col_diff == -1 && (std::abs(row_diff) == 1);
  default:
    assert(false);
    return false;
  }
}

std::vector<PlacedPiece> Board::GetAttackers(
    Team team, const BoardLocation& location, bool return_early) const {
  std::vector<PlacedPiece> attackers;

  int loc_row = location.GetRow();
  int loc_col = location.GetCol();

  // Rooks & queens
  for (int do_incr_row = 0; do_incr_row < 2; ++do_incr_row) {
    for (int pos_incr = 0; pos_incr < 2; ++pos_incr) {
      int row_incr = do_incr_row ? (pos_incr ? 1 : -1) : 0;
      int col_incr = do_incr_row ? 0 : (pos_incr ? 1 : -1);
      int row = loc_row + row_incr;
      int col = loc_col + col_incr;
      while (row >= 0 && row < 14 && col >= 0 && col < 14) {
        const Piece* piece = GetPiece(row, col);
        if (piece != nullptr) {
          if (piece->GetTeam() == team
              && (piece->GetPieceType() == ROOK
                  || piece->GetPieceType() == QUEEN)) {
            attackers.emplace_back(BoardLocation(row, col), piece);
            if (return_early) {
              return attackers;
            }
          }
          break;
        }
        row += row_incr;
        col += col_incr;
      }
    }
  }

  // Bishops & queens
  for (int pos_row = 0; pos_row < 2; ++pos_row) {
    int row_incr = pos_row ? 1 : -1;
    for (int pos_col = 0; pos_col < 2; ++pos_col) {
      int col_incr = pos_col ? 1 : -1;
      int row = loc_row + row_incr;
      int col = loc_col + col_incr;
      while (IsLegalLocation(row, col)) {
        const auto* piece = GetPiece(row, col);
        if (piece != nullptr) {
          if (piece->GetTeam() == team
              && (piece->GetPieceType() == BISHOP
                  || piece->GetPieceType() == QUEEN)) {
            attackers.emplace_back(BoardLocation(row, col), piece);
            if (return_early) {
              return attackers;
            }
          }
          break;
        }
        row += row_incr;
        col += col_incr;
      }
    }
  }

  // Knights
  for (int row_less = 0; row_less < 2; ++row_less) {
    for (int pos_row = 0; pos_row < 2; ++pos_row) {
      int row = loc_row + (row_less ? (pos_row ? 1 : -1) : (pos_row ? 2: -2));
      for (int pos_col = 0; pos_col < 2; ++pos_col) {
        int col = loc_col + (row_less ? (pos_col ? 2: -2): (pos_col ? 1 : -1));
        if (IsLegalLocation(row, col)) {
          const auto* piece = GetPiece(row, col);
          if (piece != nullptr
              && piece->GetTeam() == team
              && piece->GetPieceType() == KNIGHT) {
            attackers.emplace_back(BoardLocation(row, col), piece);
            if (return_early) {
              return attackers;
            }
          }
        }
      }
    }
  }

  // Pawns
  for (int pos_row = 0; pos_row < 2; ++pos_row) {
    int row = pos_row ? loc_row + 1 : loc_row - 1;
    if (row >= 0 && row < 14) {
      for (int pos_col = 0; pos_col < 2; ++pos_col) {
        int col = pos_col ? loc_col + 1 : loc_col - 1;
        if (col >= 0 && col < 14) {
          const auto* piece = GetPiece(row, col);
          if (piece != nullptr
              && piece->GetTeam() == team
              && piece->GetPieceType() == PAWN) {
            bool attacks = false;
            switch (piece->GetColor()) {
            case RED:
              if (pos_row) {
                attacks = true;
              }
              break;
            case BLUE:
              if (!pos_col) {
                attacks = true;
              }
              break;
            case YELLOW:
              if (!pos_row) {
                attacks = true;
              }
              break;
            case GREEN:
              if (pos_col) {
                attacks = true;
              }
              break;
            default:
              assert(false);
              break;
            }

            if (attacks) {
              attackers.emplace_back(BoardLocation(row, col), piece);
              if (return_early) {
                return attackers;
              }
            }

          }
        }
      }
    }
  }

  // Kings
  for (int delta_row = -1; delta_row < 2; ++delta_row) {
    int row = loc_row + delta_row;
    for (int delta_col = -1; delta_col < 2; ++delta_col) {
      if (delta_row == 0 && delta_col == 0) {
        continue;
      }
      int col = loc_col + delta_col;
      if (IsLegalLocation(row, col)) {
        const auto* piece = GetPiece(row, col);
        if (piece != nullptr
            && piece->GetTeam() == team
            && piece->GetPieceType() == KING) {
          attackers.emplace_back(BoardLocation(row, col), piece);
          if (return_early) {
            return attackers;
          }
        }
      }
    }
  }

  return attackers;
}

bool Board::IsAttackedByTeam(Team team, const BoardLocation& location) const {
  auto attackers = GetAttackers(team, location, /*return_early=*/true);
  return attackers.size() > 0;
}

bool Board::IsOnPathBetween(
    const BoardLocation& from,
    const BoardLocation& to,
    const BoardLocation& between) const {
  int delta_row = from.GetRow() - to.GetRow();
  int delta_col = from.GetCol() - to.GetCol();
  int delta_row_between = from.GetRow() - between.GetRow();
  int delta_col_between = from.GetCol() - between.GetCol();
  return delta_row * delta_col_between == delta_col * delta_row_between;
}

bool Board::DiscoversCheck(
    const BoardLocation& king_location,
    const BoardLocation& move_from,
    const BoardLocation& move_to,
    Team attacking_team) const {
  int delta_row = move_from.GetRow() - king_location.GetRow();
  int delta_col = move_from.GetCol() - king_location.GetCol();
  if (std::abs(delta_row) != std::abs(delta_col)
      && delta_row != 0
      && delta_col != 0) {
    return false;
  }

  int incr_col = delta_col == 0 ? 0 : delta_col > 0 ? 1 : -1;
  int incr_row = delta_row == 0 ? 0 : delta_row > 0 ? 1 : -1;
  int row = king_location.GetRow() + incr_row;
  int col = king_location.GetCol() + incr_col;
  while (IsLegalLocation(row, col)) {
    if (row != move_from.GetRow() || col != move_from.GetCol()) {
      if (row == move_to.GetRow() && col == move_to.GetCol()) {
        return false;
      }
      const auto* piece = GetPiece(row, col);
      if (piece != nullptr) {
        if (piece->GetTeam() == attacking_team) {
          if (delta_row == 0 || delta_col == 0) {
            if (piece->GetPieceType() == QUEEN
                || piece->GetPieceType() == ROOK) {
              return true;
            }
          } else {
            if (piece->GetPieceType() == QUEEN
                || piece->GetPieceType() == BISHOP) {
              return true;
            }
          }
        }
        break;
      }
    }

    row += incr_row;
    col += incr_col;
  }
  return false;
}

std::vector<Move> Board::GetPseudoLegalMoves() {
  move_buffer_.clear();

  std::optional<BoardLocation> king_location = GetKingLocation(turn_);
  if (!king_location.has_value()) {
    return move_buffer_;
  }

  for (const auto& placed_piece : piece_list_[turn_.GetColor()]) {
    const auto& location = placed_piece.GetLocation();
    const auto* piece = placed_piece.GetPiece();
    switch (piece->GetPieceType()) {
      case PAWN:
        GetPawnMoves(move_buffer_, location, *piece);
        break;
      case KNIGHT:
        GetKnightMoves(move_buffer_, location, *piece);
        break;
      case BISHOP:
        GetBishopMoves(move_buffer_, location, *piece);
        break;
      case ROOK:
        GetRookMoves(move_buffer_, location, *piece);
        break;
      case QUEEN:
        GetQueenMoves(move_buffer_, location, *piece);
        break;
      case KING:
        GetKingMoves(move_buffer_, location, *piece);
        king_location = location;
        break;
      default:
       assert(false);
    }
  }

  return move_buffer_;
}

GameResult Board::GetGameResult() {
  Player player = turn_;
  for (const auto& move : GetPseudoLegalMoves()) {
    MakeMove(move);
    GameResult king_capture_result = CheckWasLastMoveKingCapture();
    if (king_capture_result != IN_PROGRESS) {
      UndoMove();
      return king_capture_result;
    }
    bool legal = !IsKingInCheck(player);
    UndoMove();
    if (legal) {
      return IN_PROGRESS;
    }
  }
  // No legal moves
  PlayerColor color = player.GetColor();
  if (color == RED || color == YELLOW) {
    return WIN_BG;
  }
  return WIN_RY;
}

bool Board::IsKingInCheck(const Player& player) const {
  // note: this can be faster by updating the king location in Make[Undo]Move
  const auto king_location = GetKingLocation(player);
//  if (!king_location.has_value()) { // debug
//    std::cout << "turn: " << turn_ << std::endl;
//    for (const auto& move : moves_) {
//      std::cout << "move: " << move << std::endl;
//    }
//  }
//  assert(king_location.has_value());

  if (!king_location.has_value()) {
    return false;
  }

  return IsAttackedByTeam(OtherTeam(player.GetTeam()), *king_location);
}

GameResult Board::CheckWasLastMoveKingCapture() const {
  // King captured last move
  if (!moves_.empty()) {
    const auto& last_move = moves_.back();
    const auto* capture = last_move.GetStandardCapture();
    if (capture != nullptr && capture->GetPieceType() == KING) {
      return capture->GetTeam() == RED_YELLOW ? WIN_BG : WIN_RY;
    }
  }
  return IN_PROGRESS;
}

//std::pair<GameResult, std::vector<Move>> Board::GetLegalMoves(
//    bool check_legal) {
//  // First, check if the game is over
//
//  // King captured last move
//  if (!moves_.empty()) {
//    const auto& last_move = moves_.back();
//    const auto* capture = last_move.GetStandardCapture();
//    if (capture != nullptr && capture->GetPieceType() == KING) {
//      return std::make_pair(
//          capture->GetTeam() == RED_YELLOW ? WIN_BG : WIN_RY,
//          std::vector<Move>());
//    }
//  }
//
//  // Get the pieces of the current player
//  // For each piece, find the legal moves
//
//  // Special rules:
//  // 1) If a player is in check, they can only either capture an opposing
//  //    team's king or perform a move that gets them out of check.
//  // 2) If a player can not get out of check other than capturing an opposing
//  //    king, then their team loses. Moves that would place the king in check
//  //    are not allowed.
//
//  //std::vector<Move> moves;
//  move_buffer_.clear();
//
//  std::optional<BoardLocation> king_location = GetKingLocation(turn_);
//  for (const auto& placed_piece : piece_list_[turn_.GetColor()]) {
//    const auto& location = placed_piece.GetLocation();
//    const auto* piece = placed_piece.GetPiece();
//    switch (piece->GetPieceType()) {
//      case PAWN:
//        GetPawnMoves(move_buffer_, location, *piece);
//        break;
//      case KNIGHT:
//        GetKnightMoves(move_buffer_, location, *piece);
//        break;
//      case BISHOP:
//        GetBishopMoves(move_buffer_, location, *piece);
//        break;
//      case ROOK:
//        GetRookMoves(move_buffer_, location, *piece);
//        break;
//      case QUEEN:
//        GetQueenMoves(move_buffer_, location, *piece);
//        break;
//      case KING:
//        GetKingMoves(move_buffer_, location, *piece);
//        king_location = location;
//        break;
//      default:
//       assert(false);
//    }
//  }
//
//  // NOTE: We could speed this up by not copying the moves to a second vector
//  std::vector<Move> legal_moves;
//  legal_moves.reserve(move_buffer_.size());
//  Team other_team = OtherTeam(TeamToPlay());
//
////  std::vector<PlacedPiece> king_attackers;
////  if (king_location.has_value()) {
////    king_attackers = GetAttackers(other_team, king_location.value());
////  }
//
//  for (const auto& move : move_buffer_) {
//    // King captures are explicitly allowed
//    const auto* capture = move.GetStandardCapture();
//    bool king_captured = false;
//    if (capture != nullptr && capture->GetPieceType() == KING) {
//      king_captured = true;
//    } else {
//      const auto* en_passant_capture = move.GetEnpassantCapture();
//      if (en_passant_capture != nullptr
//          && en_passant_capture->GetPieceType() == KING) {
//        king_captured = true;
//      }
//    }
//    if (king_captured) {
//      legal_moves.push_back(std::move(move));
//      continue;
//    }
//
////    // verify that the move doesn't leave the king in check
////    if (king_attackers.size() == 2
////        || (king_location.has_value()
////            && king_location.value() == move.From())) {
////      if (!IsAttackedByTeam(other_team, move.To())) {
////        legal_moves.push_back(move);
////      }
////    } else if (king_attackers.size() == 1) {
////      const PlacedPiece& attacker = king_attackers[0];
////      if ((move.To() == attacker.GetLocation()
////            || IsOnPathBetween(king_location.value(), attacker.GetLocation(),
////                               move.To()))
////          && !DiscoversCheck(king_location.value(), move.From(), move.To(), other_team)) {
////        legal_moves.push_back(move);
////      }
////    } else {
////      if (!DiscoversCheck(king_location.value(), move.From(), move.To(), other_team)) {
////        legal_moves.push_back(move);
////      }
////    }
//
//    if (!check_legal) {
//      legal_moves.push_back(move);
//    } else {
//      const auto* piece = GetPiece(move.From());
//      Player t = turn_;
//      board.MakeMove(move);
//      bool legal;
//      if (piece->GetPieceType() == KING) {
//        std::optional<BoardLocation> loc = GetKingLocation(t);
//        legal = !IsAttackedByTeam(other_team, *loc);
//      } else {
//        legal = !IsAttackedByTeam(other_team, *king_location);
//      }
//      board.UndoMove();
//      if (legal) {
//        legal_moves.push_back(move);
//      }
//    }
//
//  }
//
//  // If in check, this team loses. Otherwise it's a stalemate.
//  if (legal_moves.empty()) {
//    const auto king_location = GetKingLocation(turn_);
//    if (!king_location.has_value()) { // debug
//      std::cout << "turn: " << turn_ << std::endl;
//      for (const auto& move : moves_) {
//        std::cout << "move: " << move << std::endl;
//      }
//    }
//    assert(king_location.has_value());
//    GameResult game_result = STALEMATE;
//    if (IsAttackedByTeam(other_team, *king_location)) {
//      // Checkmate
//      game_result = TeamToPlay() == RED_YELLOW ? WIN_BG : WIN_RY;
//    }
//    return std::make_pair(game_result, std::move(legal_moves));
//  }
//
//  return std::make_pair(IN_PROGRESS, std::move(legal_moves));
//}

void Board::SetPiece(
    const BoardLocation& location,
    const Piece& piece) {
  location_to_piece_[location.GetRow()][location.GetCol()] = &piece;
  // Add to piece_list_
  piece_list_[piece.GetColor()].emplace_back(location, &piece);
  UpdatePieceHash(piece, location);
}

void Board::RemovePiece(const BoardLocation& location) {
  const auto* piece = GetPiece(location);
  UpdatePieceHash(*piece, location);
  location_to_piece_[location.GetRow()][location.GetCol()] = nullptr;
  assert(piece != nullptr);
  auto& placed_pieces = piece_list_[piece->GetColor()];
  for (auto it = placed_pieces.begin(); it != placed_pieces.end();) {
    const auto& placed_piece = *it;
    if (placed_piece.GetLocation() == location) {
      placed_pieces.erase(it);
      break;
    }
    ++it;
  }
}

void Board::InitializeHash() {
  for (int color = 0; color < 4; color++) {
    for (const auto& placed_piece : piece_list_[color]) {
      UpdatePieceHash(*placed_piece.GetPiece(), placed_piece.GetLocation());
    }
  }
  UpdateTurnHash(static_cast<int>(turn_.GetColor()));
}

void Board::MakeMove(const Move& move) {
  // Cases:
  // 1. Move
  // 2. Capture
  // 3. En-passant
  // 4. Promotion
  // 5. Castling (rights, rook move)

  const auto* piece = GetPiece(move.From());

  // Capture
  const auto* capture = GetPiece(move.To());
  if (capture != nullptr) {
    RemovePiece(move.To());
    int value = piece_evaluations_[capture->GetPieceType()];
    if (capture->GetTeam() == RED_YELLOW) {
      piece_evaluation_ -= value;
    } else {
      piece_evaluation_ += value;
    }
  }

  const auto& promotion_piece_type = move.GetPromotionPieceType();
  if (promotion_piece_type.has_value()) { // Promotion
    SetPiece(
        move.To(),
        *kPieceSet[turn_.GetColor()][promotion_piece_type.value()]);
  } else { // Move
    if (piece == nullptr) {
      std::cout
        << "move"
        << " from: " << move.From()
        << " to: " << move.To()
        << " turn: " << turn_
        << std::endl;
      std::cout << *this << std::endl;
    }
    assert(piece != nullptr);
    SetPiece(move.To(), *piece);
  }
  RemovePiece(move.From());

  // En-passant
  const auto& enpassant_location = move.GetEnpassantLocation();
  if (enpassant_location.has_value()) {
    RemovePiece(enpassant_location.value());
  } else {
    // Castling
    const auto& rook_move = move.GetRookMove();
    if (rook_move.has_value()) {
      SetPiece(rook_move->To(), *GetPiece(rook_move->From()));
      RemovePiece(rook_move->From());
    }

    // Castling: rights update
    const auto& castling_rights = move.GetCastlingRights();
    if (castling_rights.has_value()) {
      castling_rights_[turn_] = castling_rights.value();
    }
  }

  int t = static_cast<int>(turn_.GetColor());
  UpdateTurnHash(t);
  UpdateTurnHash((t+1)%4);

  turn_ = GetNextPlayer(turn_);
  moves_.push_back(move);
}

void Board::UndoMove() {
  // Cases:
  // 1. Move
  // 2. Capture
  // 3. En-passant
  // 4. Promotion
  // 5. Castling (rights, rook move)

  assert(!moves_.empty());
  const Move& move = moves_.back();
  Player turn_before = GetPreviousPlayer(turn_);

  const BoardLocation& to = move.To();
  const BoardLocation& from = move.From();

  // Move the piece back.
  const auto& promotion_piece_type = move.GetPromotionPieceType();
  if (promotion_piece_type.has_value()) {
    // Handle promotions
    SetPiece(from, *kPieceSet[turn_before.GetColor()][PAWN]);
  } else {
    SetPiece(from, *GetPiece(to));
  }
  RemovePiece(to);

  // Place back captured pieces
  const auto* standard_capture = move.GetStandardCapture();
  if (standard_capture != nullptr) {
    SetPiece(to, *standard_capture);
    int value = piece_evaluations_[standard_capture->GetPieceType()];
    if (standard_capture->GetTeam() == RED_YELLOW) {
      piece_evaluation_ += value;
    } else {
      piece_evaluation_ -= value;
    }
  }

  // Place back en-passant pawns
  const auto& enpassant_location = move.GetEnpassantLocation();
  if (enpassant_location.has_value()) {
    SetPiece(enpassant_location.value(),
             *move.GetEnpassantCapture());
  } else {
    // Castling: rook move
    const auto& rook_move = move.GetRookMove();
    if (rook_move.has_value()) {
      SetPiece(rook_move->From(),
               *kPieceSet[turn_before.GetColor()][ROOK]);
      RemovePiece(rook_move->To());
    }

    // Castling: rights update
    const auto& initial_castling_rights = move.GetInitialCastlingRights();
    if (initial_castling_rights.has_value()) {
      castling_rights_[turn_before] = initial_castling_rights.value();
    }
  }

  turn_ = turn_before;
  moves_.pop_back();

  int t = static_cast<int>(turn_.GetColor());
  UpdateTurnHash(t);
  UpdateTurnHash((t+1)%4);
}

std::optional<BoardLocation> Board::GetKingLocation(const Player& turn) const {
  // NOTE: This can be faster by keeping track of the king location
  for (const auto& x : piece_list_[turn.GetColor()]) {
    if (x.GetPiece()->GetColor() == turn.GetColor()
        && x.GetPiece()->GetPieceType() == KING) {
      return x.GetLocation();
    }
  }

  return std::nullopt;
}

Team Board::TeamToPlay() const {
  return GetTeam(GetTurn().GetColor());
}

int Board::GetMaxRow() const {
  return 13;
}

int Board::GetMaxCol() const {
  return 13;
}

bool Board::IsLegalLocation(int row, int col) const {
  if (row < 0
      || row > GetMaxRow()
      || col < 0
      || col > GetMaxCol()
      || (row < 3 && (col < 3 || col > 10))
      || (row > 10 && (col < 3 || col > 10))) {
    return false;
  }
  return true;
}

bool Board::IsLegalLocation(const BoardLocation& location) const {
  return IsLegalLocation(location.GetRow(), location.GetCol());
}

int Board::PieceEvaluation() const {
  return piece_evaluation_;
}

float Board::MobilityEvaluation(const Player& player) {
  Player turn = turn_;
  turn_ = player;
  float mobility = 0;
  auto moves = GetPseudoLegalMoves();
  float player_mobility = (float) moves.size();

  if (turn_.GetTeam() == RED_YELLOW) {
    mobility += player_mobility;
  } else {
    mobility -= player_mobility;
  }

  mobility *= 0.05;

  turn_ = turn;
  return mobility;
}

float Board::MobilityEvaluation() {
  Player turn = turn_;

  float mobility = 0;
  for (int player_color = 0; player_color < 4; ++player_color) {
    turn_ = Player(static_cast<PlayerColor>(player_color));
    auto moves = GetPseudoLegalMoves();
    float player_mobility = (float) moves.size();

    if (turn_.GetTeam() == RED_YELLOW) {
      mobility += player_mobility;
    } else {
      mobility -= player_mobility;
    }
  }

  mobility *= 0.05;

  turn_ = turn;
  return mobility;
}

Board::Board(
    Player turn,
    std::unordered_map<BoardLocation, Piece> location_to_piece,
    std::optional<std::unordered_map<Player, CastlingRights>> castling_rights)
  : turn_(std::move(turn))
    {

  piece_evaluations_[PAWN] = 1;
  piece_evaluations_[KNIGHT] = 3;
  piece_evaluations_[BISHOP] = 4;
  piece_evaluations_[ROOK] = 5;
  piece_evaluations_[QUEEN] = 10;
  piece_evaluations_[KING] = 0;

  if (castling_rights.has_value()) {
    castling_rights_ = std::move(castling_rights.value());
  }
  move_buffer_.reserve(1000);

  for (int i = 0; i < 14; ++i) {
    for (int j = 0; j < 14; ++j) {
      locations_[i][j] = BoardLocation(i, j);
      location_to_piece_[i][j] = nullptr;
    }
  }

  for (int i = 0; i < 4; i++) {
    piece_list_.push_back(std::vector<PlacedPiece>());
    piece_list_[i].reserve(16);
  }

  for (const auto& it : location_to_piece) {
    const auto& location = it.first;
    const auto& piece_ref = it.second;
    const auto* piece = kPieceSet[piece_ref.GetColor()][piece_ref.GetPieceType()];
    location_to_piece_[location.GetRow()][location.GetCol()] = piece;
    piece_list_[piece->GetColor()].push_back(PlacedPiece(
          locations_[location.GetRow()][location.GetCol()],
          piece));
    PieceType piece_type = piece->GetPieceType();
    if (piece->GetTeam() == RED_YELLOW) {
      piece_evaluation_ += piece_evaluations_[static_cast<int>(piece_type)];
    } else {
      piece_evaluation_ -= piece_evaluations_[static_cast<int>(piece_type)];
    }
  }

  struct {
    bool operator()(const PlacedPiece& a, const PlacedPiece& b) {
      // this doesn't need to be fast.
      int piece_move_order_scores[6];
      piece_move_order_scores[PAWN] = 0.1;
      piece_move_order_scores[KNIGHT] = 0.2;
      piece_move_order_scores[BISHOP] = 0.3;
      piece_move_order_scores[ROOK] = 0.4;
      piece_move_order_scores[QUEEN] = 0.5;
      piece_move_order_scores[KING] = 0.0;

      int order_a = piece_move_order_scores[a.GetPiece()->GetPieceType()];
      int order_b = piece_move_order_scores[b.GetPiece()->GetPieceType()];
      return order_a < order_b;
    }
  } customLess;

  for (auto& placed_pieces : piece_list_) {
    std::sort(placed_pieces.begin(), placed_pieces.end(), customLess);
  }

  // Initialize hashes for each piece at each location, and each turn
  std::srand(958829);
  for (int color = 0; color < 4; color++) {
    turn_hashes_[color] = rand64();
    for (int row = 0; row < 14; row++) {
      for (int col = 0; col < 14; col++) {
        piece_hashes_[color][row][col] = rand64();
      }
    }
  }

  InitializeHash();
}

inline Team GetTeam(PlayerColor color) {
  return (color == RED || color == YELLOW) ? RED_YELLOW : BLUE_GREEN;
}

Player GetNextPlayer(const Player& player) {
  switch (player.GetColor()) {
  case RED:
    return kBluePlayer;
  case BLUE:
    return kYellowPlayer;
  case YELLOW:
    return kGreenPlayer;
  case GREEN:
  default:
    return kRedPlayer;
  }
}

inline Player GetPreviousPlayer(const Player& player) {
  switch (player.GetColor()) {
  case RED:
    return kGreenPlayer;
  case BLUE:
    return kRedPlayer;
  case YELLOW:
    return kBluePlayer;
  case GREEN:
  default:
    return kYellowPlayer;
  }
}

Board Board::CreateStandardSetup() {
  std::unordered_map<BoardLocation, Piece> location_to_piece;
  std::unordered_map<Player, CastlingRights> castling_rights;

  std::vector<PieceType> piece_types = {
    ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK,
  };
  std::vector<PlayerColor> player_colors = {RED, BLUE, YELLOW, GREEN};

  for (const PlayerColor& color : player_colors) {
    Player player(color);
    castling_rights[player] = CastlingRights(true, true);

    BoardLocation piece_location;
    int delta_row = 0;
    int delta_col = 0;
    int pawn_offset_row = 0;
    int pawn_offset_col = 0;

    switch (color) {
    case RED:
      piece_location = BoardLocation(13, 3);
      delta_col = 1;
      pawn_offset_row = -1;
      break;
    case BLUE:
      piece_location = BoardLocation(3, 0);
      delta_row = 1;
      pawn_offset_col = 1;
      break;
    case YELLOW:
      piece_location = BoardLocation(0, 10);
      delta_col = -1;
      pawn_offset_row = 1;
      break;
    case GREEN:
      piece_location = BoardLocation(10, 13);
      delta_row = -1;
      pawn_offset_col = -1;
      break;
    default:
      assert(false);
      break;
    }

    for (const PieceType piece_type : piece_types) {
      BoardLocation pawn_location = piece_location.Relative(
          pawn_offset_row, pawn_offset_col);
      location_to_piece[piece_location] = *kPieceSet[player.GetColor()][piece_type];
      location_to_piece[pawn_location] = *kPieceSet[player.GetColor()][PAWN];
      piece_location = piece_location.Relative(delta_row, delta_col);
    }
  }

  return Board(Player(RED), std::move(location_to_piece),
      std::move(castling_rights));
}

int Move::ManhattanDistance() const {
  return std::abs(from_.GetRow() - to_.GetRow())
       + std::abs(from_.GetCol() - to_.GetCol());
}

namespace {

std::string ToStr(PlayerColor color) {
  switch (color) {
  case RED:
    return "RED";
  case BLUE:
    return "BLUE";
  case YELLOW:
    return "YELLOW";
  case GREEN:
    return "GREEN";
  default:
    return "UNINITIALIZED_PLAYER";
  }
}

std::string ToStr(PieceType piece_type) {
  switch (piece_type) {
  case PAWN:
    return "P";
  case ROOK:
    return "R";
  case KNIGHT:
    return "N";
  case BISHOP:
    return "B";
  case KING:
    return "K";
  case QUEEN:
    return "Q";
  default:
    return "U";
  }
}

//std::string ToStr(const std::vector<PlacedPiece>& placed_pieces) {
//  std::stringstream ss;
//  for (const auto& placed_piece : placed_pieces) {
//    ss << placed_piece << " ";
//  }
//  return ss.str();
//}

}  // namespace

std::ostream& operator<<(
    std::ostream& os, const Piece& piece) {
  os << ToStr(piece.GetColor()) << "(" << ToStr(piece.GetPieceType()) << ")";
  return os;
}

std::ostream& operator<<(
    std::ostream& os, const PlacedPiece& placed_piece) {
  os << *placed_piece.GetPiece() << "@" << placed_piece.GetLocation();
  return os;
}

std::ostream& operator<<(
    std::ostream& os, const Player& player) {
  os << "Player(" << ToStr(player.GetColor()) << ")";
  return os;
}

std::ostream& operator<<(
    std::ostream& os, const BoardLocation& location) {
  os << "Loc(" << location.GetRow() << ", " << location.GetCol() << ")";
  return os;
}

std::ostream& operator<<(std::ostream& os, const Move& move) {
  os << "Move(" << move.From() << " -> " << move.To() << ")";
  return os;
}

std::ostream& operator<<(
    std::ostream& os, const Board& board) {
  for (int i = 0; i < 14; i++) {
    for (int j = 0; j < 14; j++) {
      if (board.IsLegalLocation(BoardLocation(i, j))) {
        const auto* piece = board.location_to_piece_[i][j];
        if (piece == nullptr) {
          os << ".";
        } else {
          os << ToStr(piece->GetPieceType());
        }
      } else {
        os << " ";
      }
    }
    os << std::endl;
  }

//  for (int player_color = 0; player_color < 4; ++player_color) {
//    os << ToStr(static_cast<PlayerColor>(player_color)) << " pieces: "
//        << ToStr(board.piece_list_[player_color])
//        << std::endl;
//  }

  os << "Turn: " << board.turn_ << std::endl;

  os << "All moves: " << std::endl;
  for (const auto& move : board.moves_) {
    os << move << std::endl;
  }
  return os;
}

const CastlingRights& Board::GetCastlingRights(const Player& player) const {
  return castling_rights_.at(player);
}

std::optional<CastlingType> Board::GetRookLocationType(
    const Player& player, const BoardLocation& location) const {
  switch (player.GetColor()) {
  case RED:
    if (location == kRedInitialRookLocationKingside) {
      return KINGSIDE;
    } else if (location == kRedInitialRookLocationQueenside) {
      return QUEENSIDE;
    }
    break;
  case BLUE:
    if (location == kBlueInitialRookLocationKingside) {
      return KINGSIDE;
    } else if (location == kBlueInitialRookLocationQueenside) {
      return QUEENSIDE;
    }
    break;
  case YELLOW:
    if (location == kYellowInitialRookLocationKingside) {
      return KINGSIDE;
    } else if (location == kYellowInitialRookLocationQueenside) {
      return QUEENSIDE;
    }
    break;
  case GREEN:
    if (location == kGreenInitialRookLocationKingside) {
      return KINGSIDE;
    } else if (location == kGreenInitialRookLocationQueenside) {
      return QUEENSIDE;
    }
    break;
  default:
    assert(false);
    break;
  }
  return std::nullopt;
}

Team OtherTeam(Team team) {
  return team == RED_YELLOW ? BLUE_GREEN : RED_YELLOW;
}

}  // namespace chess

