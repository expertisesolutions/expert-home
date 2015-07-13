#ifndef EXPERT_HOME_VARIANT_HPP
#define EXPERT_HOME_VARIANT_HPP

#include <boost/variant.hpp>
#include <luabind/luabind.hpp>

#include <iosfwd>

namespace eh {

struct argument_variant : //boost::spirit::extended_variant<int, std::string>
    boost::variant<int, std::string>
{
  typedef boost::variant<int, std::string> base_type;
  argument_variant() {}
  argument_variant(int i) : base_type(i) {}
  argument_variant(std::string i) : base_type(i) {}
};

struct ostream_visitor
{
  std::ostream* os;
  void operator()(std::string const& s) const
  {
    (*os) << "[variant string: " << s << ']';
  }
  void operator()(int i) const
  {
    (*os) << "[variant int: " << i << ']';
  }
};
  
std::ostream& operator<<(std::ostream& os, argument_variant const& v)
{
  boost::apply_visitor(ostream_visitor{&os}, v);
  return os;
}
  
}

namespace boost { namespace spirit { namespace x3 { namespace traits {

template <>
struct is_variant<eh::argument_variant>
  : mpl::true_
{};

template <>
struct is_variant<eh::argument_variant const>
  : mpl::true_
{};

template <>
struct is_variant<eh::argument_variant const&>
  : mpl::true_
{};

template <>
struct is_variant<eh::argument_variant&>
  : mpl::true_
{};
        
} } } }

namespace luabind
{
    template <>
    struct default_converter<eh::argument_variant>
      : native_converter_base<eh::argument_variant>
    {
        static int compute_score(lua_State* L, int index)
        {
          std::cout << "compute_score " << lua_type(L, index) << std::endl;
            return lua_type(L, index) == LUA_TNUMBER
              || lua_type(L, index) == LUA_TSTRING
              ? 0 : -1;
        }

        eh::argument_variant from(lua_State* L, int index)
        {
          return lua_type(L, index) == LUA_TNUMBER ?
            eh::argument_variant(lua_tonumber(L, index))
            :eh::argument_variant(lua_tostring(L, index));
        }

        struct push_visitor
        {
          lua_State* L;
          
          void operator()(int x) const
          {
            lua_pushnumber(L, x);            
          }
          void operator()(std::string const& x) const
          {
            lua_pushstring(L, x.c_str());
          }
        };
      
        void to(lua_State* L, eh::argument_variant const& x)
        {
          boost::apply_visitor(push_visitor{L}, x);
        }
    };

    template <>
    struct default_converter<eh::argument_variant const&>
      : default_converter<eh::argument_variant>
    {};

    template <>
    struct default_converter<std::vector<eh::argument_variant>>
      : native_converter_base<std::vector<eh::argument_variant>>
    {
        static int compute_score(lua_State* L, int index)
        {
            return lua_type(L, index) == LUA_TTABLE ? 0 : -1;
        }

        std::vector<eh::argument_variant> from(lua_State* L, int index)
        {
          assert(lua_type(L, index) == LUA_TTABLE);

          std::cout << "from " << std::endl;
          luabind::object table (luabind::from_stack(L, index));

          
          std::vector<eh::argument_variant> v;

          int i = 1;
          luabind::object object = table[i++];
          while(object.is_valid() && !luabind::type(object) == LUA_TNIL)
          {
            std::cout << "object is valid " << luabind::type(object) << std::endl;
            if(luabind::type(object) == LUA_TSTRING)
              v.push_back(luabind::object_cast<std::string>(object));
            else if(luabind::type(object) == LUA_TNUMBER)
              v.push_back(luabind::object_cast<int>(object));
            object = table[i++];
          }
          
          return v;
        }

        void to(lua_State* L, std::vector<eh::argument_variant> const& x)
        {
          std::cout << "to" << std::endl;
          luabind::object table(luabind::newtable(L));
          int index = 1;
          for(auto const& variant : x)
          {
            table[index++] = variant;
          }
          table.push(L);
        }
    };

    template <>
    struct default_converter<std::vector<eh::argument_variant> const&>
      : default_converter<std::vector<eh::argument_variant>>
    {};
}

#endif
