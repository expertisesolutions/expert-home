#ifndef EXPERT_HOME_VARIANT_HPP
#define EXPERT_HOME_VARIANT_HPP

#include <boost/variant.hpp>

namespace eh {

struct argument_variant : //boost::spirit::extended_variant<int, std::string>
    boost::variant<int, std::string>
{
  typedef boost::variant<int, std::string> base_type;
  argument_variant() {}
  argument_variant(int i) : base_type(i) {}
  argument_variant(std::string i) : base_type(i) {}
};

}

#endif
