#pragma once

#include "token_types.hpp"

#include <expected>
#include <string>
#include <string_view>
#include <array>
#include <ranges>
#include <algorithm>
#include <format>
#include <unordered_map>
#include <iostream>
#include <utility>
#include <vector>

#if not defined(constant)
#define constant static constexpr auto
#endif

constant range = std::views::iota;

constant known_tokens = 
  std::array
    {
      "+",
      "-",
      "*",
      "/",
      "==",
      "!=",
      "<-",
      ":",
      ">=",
      "<=",
      "(",
      ")",
      "{",
      "}",
      ">",
      "<",
      "if",
      "else",
      "for",
      "elif",
      "proc",
      "var",
      "run",
      "return",
      "int",
      "float",
      "True",
      "False",
      "bool"
    };

static int levenshtein(std::string_view a, std::string_view b) {
    const auto m = a.size();
    const auto n = b.size();

    auto dp   = std::vector<int>(n + 1);
    auto prev = std::vector<int>(n + 1);

    for (size_t j = 0; j < n + 1; ++j)
    {
      prev[j] = j;
    }

    for (size_t i = 0; i < m; ++i)
    {
        dp[0] = i + 1;
        for (size_t j = 0; j < n; ++j) {
            int cost = (a[i] == b[j]) ? 0 : 1;
            dp[j + 1] = std::min({
                prev[j + 1] + 1,    // deletion
                dp[j] + 1,          // insertion
                prev[j] + cost      // substitution
            });
        }
        std::swap(dp, prev);
    }

    return prev[n];
}

static std::string find_suggestion(std::string_view input) 
{
  auto scored = known_tokens
                | std::views::transform([&input](auto const& tok) { return std::pair{ levenshtein(input, tok), tok }; })
                | std::views::filter([](auto const& p) { return p.first <= 2; })
                | std::ranges::to<std::vector>();

  std::ranges::sort
  (
      scored, 
      [](auto& a, auto& b)
      {
        return a.first < b.first;
      }
  );

  auto result = scored
              | std::views::transform([](const auto& pair){ return pair.second; })
              | std::ranges::to<std::vector>();

  if(not result.empty())
  {
    return result.at(0);
  }

  return {};
}

namespace analyzer
{
  using namespace std::literals;

  using Result = std::expected<std::string, std::string>;
  constant Err = std::unexpected<std::string>{"<UNKNOWN_TOKEN>"};

  static auto identifier_table = std::unordered_map<std::string, std::size_t>{};
  static auto identifier_id    = std::size_t{0};

  constexpr Result symbol(std::string lexeme) 
  {
    constant symbols = 
      std::array
      {
        std::pair{"+"sv, TokenType::SYM_PLUS},       // [X]
        std::pair{"-"sv, TokenType::SYM_MINUS},      // [X]
        std::pair{"*"sv, TokenType::SYM_MUL},        // [X]
        std::pair{"/"sv, TokenType::SYM_DIV},        // [X]
        std::pair{"=="sv,TokenType::SYM_EQ},         // [X]
        std::pair{"!="sv,TokenType::SYM_UNEQ},       // [X]
        std::pair{"<-"sv,TokenType::SYM_ASSIGN},     // [X]
        std::pair{":"sv, TokenType::SYM_COLON},      // [X]
        std::pair{">="sv,TokenType::SYM_GEQ},        // [X]
        std::pair{"<="sv,TokenType::SYM_LEQ},        // [X]
        std::pair{"("sv, TokenType::SYM_PARAN_OPEN}, // [X]
        std::pair{")"sv, TokenType::SYM_PARAN_CLOSE},// [X]
        std::pair{"{"sv, TokenType::SYM_BRACE_OPEN}, // [X]
        std::pair{"}"sv, TokenType::SYM_BRACE_CLOSE},// [X]
        std::pair{">"sv, TokenType::SYM_GR},         // [X]
        std::pair{"<"sv, TokenType::SYM_LE},         // [X]
      };

    for (const auto& [sym_str, type] : symbols) 
    {
      if (sym_str == lexeme)
      {
        return std::format("<{}: \"{}\">", type, lexeme);
      }
    }

    return Err;
  }

  constexpr Result identifier(std::string lexeme) 
  {
    constant alphabet = "_abcdefghijklmnopqrstuvwxyz"sv;

    enum struct State { A, B, C };

    auto is_in_alphabet = [](char ch) -> bool
    {
      if(alphabet.find(ch) != std::string::npos)
      {
        return true;
      }
      return false;
    };

    auto state = State::A;
    auto index = std::size_t{0};
    while(true) 
    {
      switch(state)
      {
        case State::A: 
          if (lexeme[index] == '_')
          {
            state = State::B;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;

        case State::B: 
          if(index >= lexeme.size())
          {
            if(identifier_table.contains(lexeme))
            {
              // If Identifier exists in Identifier table
              return std::format("<{}: {}>", TokenType::ID, identifier_table[lexeme]);
            }
            else
            {
              // If Identifier does not exist in Identifier table
              identifier_table[lexeme] = ++identifier_id;
              return std::format("<{}: {}>", TokenType::ID, identifier_table[lexeme]);
            }
          }
          else 
          {
            if(is_in_alphabet(lexeme[index]))
            {
              state = State::B;
              ++index;
            }
            else 
            {
              state = State::C;
            }
          }
          break;

        case State::C: return Err;
      }
    }

    std::unreachable();
  }

  constexpr Result if_kw(std::string lexeme) 
  {
    [[maybe_unused]] constant alphabet = "if"sv;
    enum struct State { A, B, C, D };

    auto state = State::A;
    auto index = std::size_t{0};
    while(true)
    {
      switch(state)
      {
        case State::A:
          if (lexeme[index] == 'i')
          {
            state = State::B;
            ++index;
          }
          else 
          {
            state = State::C;
          }
          break;

        case State::B:
          if (lexeme[index] == 'f')
          {
            state = State::D;
            ++index;
          }
          else 
          {
            state = State::C;
          }
          break;

        case State::C: return Err;
          
        case State::D:
          if(index == lexeme.size())
          {
            return std::format("<{}: \"{}\">", TokenType::KW_IF, lexeme);
          }
          else 
          {
            state = State::C;
          }
          break;
      }
    }
    std::unreachable();
  }

  constexpr Result else_kw(std::string lexeme) 
  {
    [[maybe_unused]] constant alphabet = "else"sv;
    enum struct State { A, B, C, D, E, F };

    auto state = State::A;
    auto index = std::size_t{0};
    while(true)
    {
      switch(state)
      {
        case State::A:
          if(lexeme[index] == 'e')
          {
            state = State::B;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;

        case State::B:
          if(lexeme[index] == 'l')
          {
            state = State::D;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;

        case State::C: return Err;

        case State::D:
          if(lexeme[index] == 's')
          {
            state = State::E;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;

        case State::E:
          if(lexeme[index] == 'e')
          {
            state = State::F;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;

        case State::F:
          if(index == lexeme.size())
          {
            return std::format("<{}: \"{}\">", TokenType::KW_ELSE, lexeme);
          }
          else
          {
            state = State::C;
          }
          break;
      }
    }
    std::unreachable();
  }


  constexpr Result for_kw(std::string lexeme) 
  {
    [[maybe_unused]] constant alphabet = "for"sv;
    enum struct State { A, B, C, D, E };

    auto state = State::A;
    auto index = std::size_t{0};

    while(true)
    {
      switch(state) 
      {
        case State::A:
          if (lexeme[index] == 'f')
          {
            state = State::B;
            ++index;
          }
          else 
          {
            state = State::C;
          }
          break;

        case State::B:
          if(lexeme[index] == 'o')
          {
            state = State::D;
            ++index;
          }
          else 
          {
            state = State::C;
          }
          break;

        case State::C: return Err;

        case State::D:
          if(lexeme[index] == 'r')
          {
            state = State::E;
            ++index;
          }
          else 
          {
            state = State::C;
          }
          break;

        case State::E:
          if(index == lexeme.size())
          {
            return std::format("<{}: \"{}\">", TokenType::KW_FOR, lexeme);
          }
          else 
          {
            state = State::C;
          }
          break;
      }
    }
    std::unreachable();
  }

  constexpr Result elif_kw(std::string lexeme) 
  {
    [[maybe_unused]] constant alphabet = "elif"sv;
    enum struct State { A, B, C, D, E, F };

    auto state = State::A;
    auto index = std::size_t{0};

    while(true)
    {
      switch(state) 
      {
        case State::A:
          if(lexeme[index] == 'e')
          {
            state = State::B;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;

        case State::B:
          if(lexeme[index] == 'l')
          {
            state = State::D;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;
        case State::C: return Err;

        case State::D:
          if(lexeme[index] == 'i')
          {
            state = State::E;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;

        case State::E:
          if(lexeme[index] == 'f')
          {
            state = State::F;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;

        case State::F:
          if(index == lexeme.size())
          {
            return std::format("<{}: \"{}\">", TokenType::KW_ELIF, lexeme);
          }
          else 
          {
            state = State::C;
          }
          break;
      }
    }
    std::unreachable();
  }

  constexpr Result proc_kw(std::string lexeme) 
  {

    [[maybe_unused]] constant alphabet = "proc"sv;
    enum struct State { A, B, C, D, E, F };

    auto state = State::A;
    auto index = std::size_t{0};

    while(true)
    {
      switch(state) 
      {
        case State::A:
          if(lexeme[index] == 'p')
          {
            state = State::B;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;
        case State::B:
          if(lexeme[index] == 'r')
          {
            state = State::D;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;

        case State::C: return Err;

        case State::D:
          if(lexeme[index] == 'o')
          {
            state = State::E;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;

        case State::E:
          if(lexeme[index] == 'c')
          {
            state = State::F;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;

        case State::F:
          if(index == lexeme.size())
          {
            return std::format("<{}: \"{}\">", TokenType::KW_PROC, lexeme);
          }
          else
          {
            state = State::C;
          }
          break;
      }
    }
    std::unreachable();
  }

  constexpr Result var_kw(std::string lexeme) 
  {

    [[maybe_unused]] constant alphabet = "proc"sv;
    enum struct State { A, B, C, D, E };

    auto state = State::A;
    auto index = std::size_t{0};

    while(true)
    {
      switch(state) 
      {
        case State::A:
          if(lexeme[index] == 'v')
          {
            state = State::B;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;
        case State::B:
          if(lexeme[index] == 'a')
          {
            state = State::D;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;

        case State::C: return Err;

        case State::D:
          if(lexeme[index] == 'r')
          {
            state = State::E;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;

        case State::E:
          if(index == lexeme.size())
          {
            return std::format("<{}: \"{}\">", TokenType::KW_VAR, lexeme);
          }
          else
          {
            state = State::C;
          }
          break;
      }
    }
    std::unreachable();
  }

  constexpr Result run_kw(std::string lexeme) 
  {

    [[maybe_unused]] constant alphabet = "proc"sv;
    enum struct State { A, B, C, D, E };

    auto state = State::A;
    auto index = std::size_t{0};

    while(true)
    {
      switch(state) 
      {
        case State::A:
          if(lexeme[index] == 'r')
          {
            state = State::B;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;
        case State::B:
          if(lexeme[index] == 'u')
          {
            state = State::D;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;

        case State::C: return Err;

        case State::D:
          if(lexeme[index] == 'n')
          {
            state = State::E;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;

        case State::E:
          if(index == lexeme.size())
          {
            return std::format("<{}: \"{}\">", TokenType::KW_RUN, lexeme);
          }
          else
          {
            state = State::C;
          }
          break;
      }
    }
    std::unreachable();
  }

  constexpr Result return_kw(std::string lexeme) 
  {

    [[maybe_unused]] constant alphabet = "proc"sv;
    enum struct State { A, B, C, D, E, F, G, H };

    auto state = State::A;
    auto index = std::size_t{0};

    while(true)
    {
      switch(state) 
      {
        case State::A:
          if(lexeme[index] == 'r')
          {
            state = State::B;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;
        case State::B:
          if(lexeme[index] == 'e')
          {
            state = State::D;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;

        case State::C: return Err;

        case State::D:
          if(lexeme[index] == 't')
          {
            state = State::E;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;

        case State::E:
          if(lexeme[index] == 'u')
          {
            state = State::F;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;
        case State::F:
          if(lexeme[index] == 'r')
          {
            state = State::G;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;
        case State::G:
          if(lexeme[index] == 'n')
          {
            state = State::H;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;
        case State::H:
          if(index == lexeme.size())
          {
            return std::format("<{}: \"{}\">", TokenType::KW_RETURN, lexeme);
          }
          else
          {
            state = State::C;
          }
          break;
        }
    }
    std::unreachable();
  }

  constexpr Result int_kw(std::string lexeme) 
  {
    [[maybe_unused]] constant alphabet = "int"sv;
    enum struct State { A, B, C, D, E };

    auto state = State::A;
    auto index = std::size_t{0};

    while(true)
    {
      switch(state) 
      {
        case State::A:
          if(lexeme[index] == 'i')
          {
            state = State::B;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;
        case State::B:
          if(lexeme[index] == 'n')
          {
            state = State::D;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;

        case State::C: return Err;

        case State::D:
          if(lexeme[index] == 't')
          {
            state = State::E;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;

        case State::E:
          if(index == lexeme.size())
          {
            return std::format("<{}: \"{}\">", TokenType::KW_INT, lexeme);
          }
          else
          {
            state = State::C;
          }
          break;
      }
    }
    std::unreachable();
  }

  constexpr Result float_kw(std::string lexeme) 
  {

    [[maybe_unused]] constant alphabet = "float"sv;
    enum struct State { A, B, C, D, E, F, G };

    auto state = State::A;
    auto index = std::size_t{0};

    while(true)
    {
      switch(state) 
      {
        case State::A:
          if(lexeme[index] == 'f')
          {
            state = State::B;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;
        case State::B:
          if(lexeme[index] == 'l')
          {
            state = State::D;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;

        case State::C: return Err;

        case State::D:
          if(lexeme[index] == 'o')
          {
            state = State::E;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;

        case State::E:
          if(lexeme[index] == 'a')
          {
            state = State::F;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;

        case State::F:
          if(lexeme[index] == 't')
          {
            state = State::G;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;
        case State::G:
          if(index == lexeme.size())
          {
            return std::format("<{}: \"{}\">", TokenType::KW_FLOAT, lexeme);
          }
          else
          {
            state = State::C;
          }
          break;
      }
    }
    std::unreachable();
  }

  constexpr Result True_kw(std::string lexeme) 
  {

    [[maybe_unused]] constant alphabet = "True"sv;
    enum struct State { A, B, C, D, E, F };

    auto state = State::A;
    auto index = std::size_t{0};

    while(true)
    {
      switch(state) 
      {
        case State::A:
          if(lexeme[index] == 'T')
          {
            state = State::B;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;
        case State::B:
          if(lexeme[index] == 'r')
          {
            state = State::D;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;

        case State::C: return Err;

        case State::D:
          if(lexeme[index] == 'u')
          {
            state = State::E;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;

        case State::E:
          if(lexeme[index] == 'e')
          {
            state = State::F;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;

        case State::F:
          if(index == lexeme.size())
          {
            return std::format("<{}: \"{}\">", TokenType::KW_TRUE, lexeme);
          }
          else
          {
            state = State::C;
          }
          break;
      }
    }
    std::unreachable();
  }

  constexpr Result False_kw(std::string lexeme) 
  {

    [[maybe_unused]] constant alphabet = "False"sv;
    enum struct State { A, B, C, D, E, F, G };

    auto state = State::A;
    auto index = std::size_t{0};

    while(true)
    {
      switch(state) 
      {
        case State::A:
          if(lexeme[index] == 'F')
          {
            state = State::B;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;
        case State::B:
          if(lexeme[index] == 'a')
          {
            state = State::D;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;

        case State::C: return Err;

        case State::D:
          if(lexeme[index] == 'l')
          {
            state = State::E;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;

        case State::E:
          if(lexeme[index] == 's')
          {
            state = State::F;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;

        case State::F:
          if(lexeme[index] == 'e')
          {
            state = State::G;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;
        case State::G:
          if(index == lexeme.size())
          {
            return std::format("<{}: \"{}\">", TokenType::KW_FALSE, lexeme);
          }
          else
          {
            state = State::C;
          }
          break;
      }
    }
    std::unreachable();
  }

  constexpr Result bool_kw(std::string lexeme) 
  {

    [[maybe_unused]] constant alphabet = "bool"sv;
    enum struct State { A, B, C, D, E, F };

    auto state = State::A;
    auto index = std::size_t{0};

    while(true)
    {
      switch(state) 
      {
        case State::A:
          if(lexeme[index] == 'b')
          {
            state = State::B;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;
        case State::B:
          if(lexeme[index] == 'o')
          {
            state = State::D;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;

        case State::C: return Err;

        case State::D:
          if(lexeme[index] == 'o')
          {
            state = State::E;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;

        case State::E:
          if(lexeme[index] == 'l')
          {
            state = State::F;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;

        case State::F:
          if(index == lexeme.size())
          {
            return std::format("<{}: \"{}\">", TokenType::KW_BOOL, lexeme);
          }
          else
          {
            state = State::C;
          }
          break;
      }
    }
    std::unreachable();
  }

  constexpr Result num_kw(std::string lexeme) 
  {
    [[maybe_unused]] constant alphabet = "0123456789"sv;
    enum struct State { A, B, C, D };

    auto state = State::A;
    auto index = std::size_t{0};

    while(true)
    {
      switch(state) 
      {
        case State::A:
          if(alphabet.contains(lexeme[index]))
          {
            state = State::B;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;

        case State::B:
          if(index == lexeme.size())
          {
            return std::format("<{}: \"{}\">", TokenType::KW_NUM, lexeme);
          }
          else if(alphabet.contains(lexeme[index]))
          {
            state = State::D;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;

        case State::C: return Err;

        case State::D:
          if(index == lexeme.size())
          {
            return std::format("<{}: \"{}\">", TokenType::KW_NUM, lexeme);
          }
          else
          {
            if(alphabet.contains(lexeme[index]))
            {
              state = State::D;
              ++index;
            }
            else
            {
              state = State::C;
            }
          }
          break;


      }
    }
    std::unreachable();
  }

  constexpr Result num_frac_kw(std::string lexeme) 
  {

    [[maybe_unused]] constant alphabet = "0123456789."sv;
    enum struct State { A, B, C, D };

    auto is_in_alphabet = [](char ch) -> bool
    {
      if(alphabet.find(ch) != std::string_view::npos)
      {
        return true;
      }
      return false;
    };

    auto state = State::A;
    auto index = std::size_t{0};

    while(true)
    {
      switch(state) 
      {
        case State::A:
          if(is_in_alphabet(lexeme[index]))
          {
            state = State::B;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;

        case State::B:
          if(is_in_alphabet(lexeme[index]) and std::isdigit(lexeme[index]))
          {
            state = State::B;
            ++index;
          }
          else if(is_in_alphabet(lexeme[index]) and lexeme[index] == '.')
          {
            state = State::D;
            ++index;
          }
          else
          {
            state = State::C;
          }
          break;

        case State::C: return Err;

        case State::D:
          if(index == lexeme.size())
          {
            return std::format("<{}: \"{}\">", TokenType::KW_FRAC_NUM, lexeme);
          }
          else
          {
            if(is_in_alphabet(lexeme[index]) and std::isdigit(lexeme[index]))
            {
              state = State::D;
              ++index;
            }
            else
            {
              state = State::C;
            }
          }
          break;


      }
    }
    std::unreachable();
  }

  constexpr std::string parse_all(std::string lexeme, std::size_t line)
  {
    constant parsers =
      std::array
    {
      symbol, 
      if_kw,
      else_kw,
      for_kw,
      elif_kw,
      proc_kw,
      var_kw,
      run_kw,
      return_kw,
      int_kw,
      float_kw,
      True_kw,
      False_kw,
      bool_kw,
      num_kw,
      num_frac_kw,
      identifier
    };

    for (auto parser : parsers)
    {
      if (auto res = parser(lexeme))
      {
        return *res;
      }
    }

    if (auto suggestion = find_suggestion(lexeme); !suggestion.empty()) 
    {
      std::print(std::cout, "[Error] <UNKNOWN_TOKEN \"{}\"> at line {}. Did you mean \"{}\"? ",
            lexeme, line, suggestion
      );

      auto new_token = std::string{};
      std::cin >> new_token;
      return parse_all(new_token, line);
    }

    return std::format("<ERROR_TOKEN \"{}\" at line {}>", lexeme, line);
  }

}
