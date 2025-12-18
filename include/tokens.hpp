#pragma once

#include <cstdint>
#include <format>
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

  struct ParanOpen 
  {

  };

  struct ParanClose 
  {

  };

  struct BraceOpen 
  {

  }; 

  struct BraceClose
  {

  }; 

  struct Assign 
  {

  };

  struct Plus 
  {

  };

  struct Minus 
  {

  };

  struct Mul 
  {

  };

  struct Devide 
  {

  };

  struct Equal 
  {

  };

  struct Unequal 
  {

  };

  struct Colon 
  {

  };

  struct GrEqual 
  {

  };

  struct LeEqual 
  {

  };

  struct Greater 
  {

  };

  struct Less 
  {

  };

}

using Token = std::variant<
    tks::Unknown,
    tks::Id,
    tks::If, tks::Else, tks::For, tks::Elif, tks::Proc, tks::Var, tks::Run, tks::Return,
    tks::Int, tks::Float, tks::Bool, tks::True, tks::False,
    tks::IntNum, tks::FloatNum,
    tks::ParanOpen, tks::ParanClose,
    tks::BraceOpen, tks::BraceClose,
    tks::Assign,
    tks::Plus, tks::Minus, tks::Mul, tks::Devide,
    tks::Equal, tks::Unequal,
    tks::Colon,
    tks::GrEqual, tks::LeEqual, tks::Greater, tks::Less
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
        [](tks::Unknown)    -> std::string { return "<UNKNOWN_TK>";       },
        [](tks::Id t)       -> std::string { return std::format("<ID_TK: {}>", t.symbol); },
  
        [](tks::If)         -> std::string { return "<IF_TK>";            },
        [](tks::Else)       -> std::string { return "<ELSE_TK>";          },
        [](tks::For)        -> std::string { return "<FOR_TK>";           },
        [](tks::Elif)       -> std::string { return "<ELIF_TK>";          },
        [](tks::Proc)       -> std::string { return "<PROC_TK>";          },
        [](tks::Var)        -> std::string { return "<VAR_TK>";           },
        [](tks::Run)        -> std::string { return "<RUN_TK>";           },
        [](tks::Return)     -> std::string { return "<RETURN_TK>";        },
  
        [](tks::Int)        -> std::string { return "<INT_TK>";           },
        [](tks::Float)      -> std::string { return "<FLOAT_TK>";         },
        [](tks::Bool)       -> std::string { return "<BOOL_TK>";          },
        [](tks::True)       -> std::string { return "<TRUE_TK>";          },
        [](tks::False)      -> std::string { return "<FALSE_TK>";         },
  
        [](tks::IntNum t)     -> std::string { return std::format("<INTNUM_TK: {}>", t.value); },
        [](tks::FloatNum t)   -> std::string { return std::format("<FLOATNUM_TK: {}>", t.value); },
  
        [](tks::ParanOpen)  -> std::string { return "<PARAN_OPEN_TK>";    },
        [](tks::ParanClose) -> std::string { return "<PARAN_CLOSE_TK>";   },
  
        [](tks::BraceOpen)  -> std::string { return "<BRACE_OPEN_TK>";    },
        [](tks::BraceClose) -> std::string { return "<BRACE_CLOSE_TK>";   },
  
        [](tks::Assign)     -> std::string { return "<ASSIGN_TK>";        },
  
        [](tks::Plus)       -> std::string { return "<PLUS_TK>";          },
        [](tks::Minus)      -> std::string { return "<MINUS_TK>";         },
        [](tks::Mul)        -> std::string { return "<MUL_TK>";           },
        [](tks::Devide)     -> std::string { return "<DIVIDE_TK>";        },
  
        [](tks::Equal)      -> std::string { return "<EQUAL_TK>";         },
        [](tks::Unequal)    -> std::string { return "<UNEQUAL_TK>";       },
  
        [](tks::Colon)      -> std::string { return "<COLON_TK>";         },
  
        [](tks::GrEqual)    -> std::string { return "<GREATER_EQUAL_TK>"; },
        [](tks::LeEqual)    -> std::string { return "<LESS_EQUAL_TK>";    },
        [](tks::Greater)    -> std::string { return "<GREATER_TK>";       },
        [](tks::Less)       -> std::string { return "<LESS_TK>";          }
      }
    );
  }

}
