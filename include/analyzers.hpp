#pragma once

#include "token_types.hpp"
#include "tokens.hpp"

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

namespace stdr = std::ranges;
namespace stdv = std::views;
using namespace std::literals;


#if not defined(constant)
#define constant static constexpr auto
#endif

constant range = stdv::iota;

constant known_tokens = 
  std::array
    {
      "+"sv,
      "-"sv,
      "*"sv,
      "/"sv,
      "=="sv,
      "!="sv,
      "<-"sv,
      ":"sv,
      ">="sv,
      "<="sv,
      "("sv,
      ")"sv,
      "{"sv,
      "}"sv,
      ">"sv,
      "<"sv,
      "if"sv,
      "else"sv,
      "for"sv,
      "elif"sv,
      "proc"sv,
      "var"sv,
      "run"sv,
      "return"sv,
      "int"sv,
      "float"sv,
      "True"sv,
      "False"sv,
      "bool"sv
    };

static std::int32_t levenshtein(std::string_view a, std::string_view b) {
    const auto m = a.size();
    const auto n = b.size();

    auto dp   = std::vector<int>(n + 1);
    auto prev = std::vector<int>(n + 1);

    for (auto j : range(0uz, n + 1))
    {
      prev.at(j) = j;
    }

    for (auto i : range(0uz, m))
    {
        dp.at(0) = i + 1;
        for (size_t j = 0; j < n; ++j) {
            int cost = (a.at(i) == b.at(j)) ? 0 : 1;
            dp.at(j + 1) = std::min({
                prev.at(j + 1) + 1,    // deletion
                dp.at(j) + 1,          // insertion
                prev.at(j) + cost      // substitution
            });
        }
        stdr::swap(dp, prev);
    }

    return  prev.at(n);
}

static std::string find_suggestion(std::string_view input) 
{
  auto scored = known_tokens
                | stdv::transform([&input](auto const& tok) { return std::pair{ levenshtein(input, tok), tok }; })
                | stdv::filter([](auto const& p) { return p.first <= 2; })
                | stdr::to<std::vector>();

  stdr::sort
  (
      scored, 
      [](const auto& a, const auto& b)
      {
        return a.first < b.first;
      }
  );

  auto result = scored
                | stdv::transform([](const auto& pair){ return pair.second; })
                | stdr::to<std::vector>();

  if(not result.empty())
  {
    return std::string{result.at(0)};
  }

  return {};
}

namespace analyzer
{

  namespace detail 
  {

    template<typename T>
    constexpr auto parse_number(std::string_view expr) -> T
      requires std::integral<T> or std::floating_point<T>
    {
      auto res = T{};

      auto [ptr, ec] = std::from_chars(
          expr.data(),
          expr.data() + expr.size(),
          res
      );

      switch (ec)
      {
        case std::errc::invalid_argument   : res = 0; break;
        case std::errc::result_out_of_range: res = std::numeric_limits<std::uint64_t>::max(); break;
        default: break;
      }

      return res;
    }

  }

  using Result = std::expected<Token, tks::Unknown>;
  constant Err = std::unexpected<tks::Unknown>{tks::Unknown{}};

  static auto identifier_table = std::unordered_map<std::string, std::size_t>{};
  static auto identifier_id    = std::size_t{0};

  constexpr Result symbol(std::string lexeme) 
  {
    constant symbols = 
      std::array<std::pair<std::string_view, Token>, 16>
      {
        std::pair{"+"sv, tks::Plus{}},       // [X]
        std::pair{"-"sv, tks::Minus{}},      // [X]
        std::pair{"*"sv, tks::Mul{}},        // [X]
        std::pair{"/"sv, tks::Devide{}},        // [X]
        std::pair{"=="sv,tks::Equal{}},         // [X]
        std::pair{"!="sv,tks::Unequal{}},       // [X]
        std::pair{"<-"sv,tks::Assign{}},     // [X]
        std::pair{":"sv, tks::Colon{}},      // [X]
        std::pair{">="sv,tks::GrEqual{}},        // [X]
        std::pair{"<="sv,tks::LeEqual{}},        // [X]
        std::pair{"("sv, tks::ParanOpen{}}, // [X]
        std::pair{")"sv, tks::ParanClose{}},// [X]
        std::pair{"{"sv, tks::BraceOpen{}}, // [X]
        std::pair{"}"sv, tks::BraceClose{}},// [X]
        std::pair{">"sv, tks::Greater{}},         // [X]
        std::pair{"<"sv, tks::Less{}},         // [X]
      };

    for (const auto& [sym_str, type] : symbols) 
    {
      if (sym_str == lexeme)
      {
        return type;
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
      if(alphabet.contains(ch))
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
              return tks::Id{identifier_table[lexeme]};
            }
            else
            {
              // If Identifier does not exist in Identifier table
              identifier_table[lexeme] = ++identifier_id;
              return tks::Id{identifier_table[lexeme]};
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
            return tks::If{};
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
            return tks::Else{};
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
            return tks::For{};
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
            return tks::Elif{};
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
            return tks::Proc{};
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
            return tks::Var{};
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
            return tks::Run{};
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
            return tks::Return{};
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
            return tks::Int{};
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
            return tks::Float{};
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
            return tks::True{};
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
            return tks::False{};
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
            return tks::Bool{};
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

  constexpr Result int_num_kw(std::string lexeme) 
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
            return tks::IntNum{.value = detail::parse_number<std::uint64_t>(lexeme)};
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
            return tks::IntNum{.value = detail::parse_number<std::uint64_t>(lexeme)};
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

  constexpr Result float_num_kw(std::string lexeme) 
  {
    [[maybe_unused]] constant alphabet = "0123456789."sv;
    enum struct State { A, B, C, D, E, F };

    auto is_in_alphabet = [](char ch) -> bool
    {
      if(alphabet.contains(ch))
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
          if (is_in_alphabet(lexeme[index]) and std::isdigit(lexeme[index]))
          {
            state = State::B;
            ++index;
          }
          else if (is_in_alphabet(lexeme[index]) and lexeme[index] == '.')
          {
            state = State::C;
            ++index;
          }
          else 
          {
            state = State::F;
          }
          break;

        case State::B:
          if (is_in_alphabet(lexeme[index]) and std::isdigit(lexeme[index]))
          {
            state = State::B;
            ++index;
          }
          else if (is_in_alphabet(lexeme[index]) and lexeme[index] == '.')
          {
            state = State::D;
            ++index;
          }
          else
          {
            state = State::F;
          }
          break;

        case State::C:
          if (is_in_alphabet(lexeme[index]) and std::isdigit(lexeme[index]))
          {
            state = State::E;
            ++index;
          }
          else if (is_in_alphabet(lexeme[index]) and lexeme[index] == '.')
          {
            state = State::F;
          }
          else
          {
            state = State::F;
          }
          break;

        case State::D:
          if(index == lexeme.size())
          {
            return tks::FloatNum{.value = detail::parse_number<double>(lexeme)};
          }
          else 
          {
            if (is_in_alphabet(lexeme[index]) and std::isdigit(lexeme[index]))
            {
              state = State::D;
              ++index;
            }
            else if (is_in_alphabet(lexeme[index]) and lexeme[index] == '.')
            {
              state = State::F;
            }
            else
            {
              state = State::F;
            }
          }
          break;

        case State::E:
          if(index == lexeme.size())
          {
            return tks::FloatNum{.value = detail::parse_number<double>(lexeme)};
          }
          else 
          {
            if (is_in_alphabet(lexeme[index]) and std::isdigit(lexeme[index]))
            {
              state = State::E;
              ++index;
            }
            else if (is_in_alphabet(lexeme[index]) and lexeme[index] == '.')
            {
              state = State::F;
            }
            else
            {
              state = State::F;
            }
          }
          break;

        case State::F: return Err;
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
      int_num_kw,
      float_num_kw,
      identifier
    };

    for (auto parser : parsers)
    {
      if (auto res = parser(lexeme))
      {
        return tks::to_string(*res);
      }
    }

    if (auto suggestion = find_suggestion(lexeme); !suggestion.empty()) 
    {
      std::print(std::cout,
          "[Error] <UNKNOWN_TOKEN \"{}\"> at line {}. Did you mean \"{}\"? ",
          lexeme,
          line,
          suggestion
      );

      auto new_token = std::string{};
      std::cin >> new_token;
      return parse_all(new_token, line);
    }

    return std::format("<ERROR_TOKEN \"{}\" at line {}>", lexeme, line);
  }

}
