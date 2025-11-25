#pragma once

#include <format>
#include <array>

#if not defined(constant)
#define constant static constexpr auto
#endif

enum struct TokenType 
{
  SYM_PARAN_OPEN, // [X]
  SYM_PARAN_CLOSE,// [X]
  SYM_BRACE_OPEN, // [X]
  SYM_BRACE_CLOSE,// [X]
  SYM_ASSIGN,     // [X]
  SYM_PLUS,       // [X]
  SYM_MINUS,      // [X]
  SYM_MUL,        // [X]
  SYM_DIV,        // [X]
  SYM_EQ,         // [X]
  SYM_UNEQ,       // [X]
  SYM_COLON,      // [X]
  SYM_GEQ,        // [X]
  SYM_LEQ,        // [X]
  SYM_GR,         // [X]
  SYM_LE,         // [X]

  KW_IF,          // [X]
  KW_ELSE,        // [X]
  KW_ELIF,        // [X]
  KW_FOR,         // [X]
  KW_PROC,        // [X]
  KW_VAR,         // [X]
  KW_RUN,         // [X]
  KW_RETURN,      // [X]
  KW_INT,         // [X]
  KW_FLOAT,       // [X]
  KW_BOOL,        // [X]
  KW_TRUE,        // [X]
  KW_FALSE,       // [X]
  KW_NUM,         // [X]
  KW_FRAC_NUM,    // [X]

  ID        // [X]
};

constant table = std::array
{
  "SYM_PARAN_OPEN_TK",
  "SYM_PARAN_CLOSE_TK",
  "SYM_BRACE_OPEN_TK",
  "SYM_BRACE_CLOSE_TK",
  "SYM_ASSIGN_TK",
  "SYM_PLUS_TK",
  "SYM_MINUS_TK",
  "SYM_MUL_TK",
  "SYM_DIV_TK",
  "SYM_EQ_TK",
  "SYM_UNEQ_TK",
  "SYM_COLON_TK",
  "SYM_GEQ_TK",
  "SYM_LEQ_TK",
  "SYM_GR_TK",        // [X]
  "SYM_LE_TK",        // [X]

  "KW_IF_TK",               // [X]
  "KW_ELSE_TK",             // [X]
  "KW_ELIF_TK",             // [X]
  "KW_FOR_TK",
  "KW_PROC_TK",             // [X]
  "KW_VAR_TK",              // [X]
  "KW_RUN_TK",              // [X]
  "KW_RETURN_TK",           // [X]
  "KW_INT_TK",              // [X]
  "KW_FLOAT_TK",            // [X]
  "KW_BOOL_TK",             // [X]
  "KW_TRUE_TK",             // [X]
  "KW_FALSE_TK",            // [X]
  "KW_NUM_TK",              // [X]
  "KW_FRAC_NUM_TK",         // [X]

  "ID_TK"
};


template<>
struct std::formatter<TokenType> 
{
  constexpr auto parse(std::format_parse_context& ctx)
  {
    return ctx.begin();
  }

  auto format(TokenType t, auto& ctx) const
  {
    return std::format_to(ctx.out(), "{}", table[static_cast<std::size_t>(t)]);
  }

};
