#pragma once

#include <cstdint>
#include <string>
#include <variant>

namespace tks
{

  struct Unknown 
  {
    
  };

  struct Id
  {
    std::size_t symbol;
  };

  struct If 
  {
    
  };

  struct Else 
  {

  };

  struct For 
  {

  };

  struct Elif 
  {

  };

  struct Proc 
  {

  };

  struct Var
  {

  };

  struct Run
  {

  };

  struct Return
  {

  };

  struct Int
  {

  };

  struct Float 
  {

  };

  struct True 
  {

  };

  struct False 
  {

  };

  struct Bool 
  {

  };

  struct IntNum 
  {
    std::uint64_t value;
  };

  struct FloatNum 
  {
    double value;
  };

}

using Token =
    std::variant<
        tks::Unknown,
        tks::Id,
        tks::If,
        tks::Else,
        tks::For,
        tks::Elif,
        tks::Proc,
        tks::Var,
        tks::Run,
        tks::Return,
        tks::Int,
        tks::Float,
        tks::True,
        tks::False,
        tks::Bool,
        tks::IntNum,
        tks::FloatNum
    >;

namespace tks
{

  template<typename ...Ts>
  struct overload : public Ts...
  {
    using Ts::operator()...;
  };

  [[nodiscard]]
  constexpr auto to_string(const Token token) -> std::string 
  {
    return token.visit
      (
        overload
        {
          [](tks::Unknown)  { return "<UNKNOWN_TK>";  },
          [](tks::Id)       { return "<ID_TK>";       },
          [](tks::If)       { return "<IF_TK>";       },
          [](tks::Else)     { return "<ELSE_TK>";     },
          [](tks::For)      { return "<FOR_TK>";      },
          [](tks::Elif)     { return "<ELIF_TK>";     },
          [](tks::Proc)     { return "<PROC_TK>";     },
          [](tks::Var)      { return "<VAR_TK>";      },
          [](tks::Run)      { return "<RUN_TK>";      },
          [](tks::Return)   { return "<RETURN_TK>";   },
          [](tks::Int)      { return "<INT_TK>";      },
          [](tks::Float)    { return "<FLOAT_TK>";    },
          [](tks::True)     { return "<TRUE_TK>";     },
          [](tks::False)    { return "<FALSE_TK>";    },
          [](tks::Bool)     { return "<BOOL_TK>";     },
          [](tks::IntNum)   { return "<INTNUM_TK>";   },
          [](tks::FloatNum) { return "<FLOATNUM_TK>"; }
        }
      );
  }

}
