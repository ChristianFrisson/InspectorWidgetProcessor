/**
 * @file InspectorWidgetProcessorCommandParser.h
 * @brief Classes for parsing the command line syntax
 * @author Christian Frisson
 */

#include <map>
#include <vector>
#include <memory>
#include <cassert>
#include <iostream>
#include <algorithm>

#include <pegtl.hh>

// Include the analyze function that checks
// a grammar for possible infinite cycles.

#include <pegtl/analyze.hh>

namespace InspectorWidgetProcessorCommandParser
{
  enum class order : int {};

  enum class assoc : bool
  {
    LEFT = true,
    POSTFIX = true,
    RIGHT = false,
    PREFIX = false
  };

  enum class optype : bool
  {
    UNARY = true,
    BINARY = false
  };

  using namespace pegtl;

  struct op
  {
    order p;
    assoc a;
    optype t;
    std::function< std::string ( std::vector<std::string> ) > f;
  };

  struct value_base
  {
    virtual ~value_base()
    { }

    value_base( const value_base & ) = delete;
    void operator= ( const value_base & ) = delete;

    void write( std::ostream & o ) const
    {
      v_write( o );
    }

    virtual void s_write( std::stringstream & ) const = 0;

  protected:
    value_base()
    { }

    virtual void v_write( std::ostream & ) const = 0;

  };

  struct number_value
      : public value_base
  {
    explicit
    number_value( const long s )
      : m_s ( s )
    { }

    long get() const
    {
      return m_s;
    }

    void s_write( std::stringstream & s) const
    {
      s << m_s;
    }

  private:
    const long m_s;

    void v_write( std::ostream & o ) const
    {
      o << m_s;
    }
  };

  struct string_value
      : public value_base
  {
    explicit
    string_value( const std::string & s )
      : m_s( s )
    { }

    const std::string & get() const
    {
      return m_s;
    }

    void s_write( std::stringstream & s) const
    {
      s << m_s;
    }

  private:
    const std::string m_s;

    void v_write( std::ostream & o ) const
    {
      o << m_s;
    }
  };

  struct unary_value
      : public value_base
  {
    //unary_value( const std::string & f, const std::shared_ptr< value_base > & a )
    unary_value( const std::function< std::string ( std::vector<std::string> ) > & f, const std::shared_ptr< value_base > & a )
      : m_f( f ),
        m_a( a )
    {
      assert( m_a );
    }

    void s_write( std::stringstream & s) const
    {
      std::stringstream ss;
      m_a->s_write(ss);
      std::vector<std::string> vs;
      vs.push_back(ss.str());
      s << m_f(vs);
    }

  private:
    //const std::string m_f;
    std::function< std::string ( std::vector<std::string> ) > m_f;
    const std::shared_ptr< value_base > m_a;

    void v_write( std::ostream & o ) const
    {
      /*o << m_f << "( ";
         m_a->write( o );
         o << " )";*/
      std::stringstream ss;
      m_a->s_write(ss);
      std::vector<std::string> vs;
      vs.push_back(ss.str());
      o << m_f(vs);
    }
  };

  struct binary_value
      : public value_base
  {
    //binary_value( const std::string & f, const std::shared_ptr< value_base > & l, const std::shared_ptr< value_base > & r )
    binary_value( const std::function< std::string ( std::vector<std::string> ) > & f, const std::shared_ptr< value_base > & l, const std::shared_ptr< value_base > & r )
      : m_f( f ),
        m_l( l ),
        m_r( r )
    {
      assert( m_l );
      assert( m_r );
    }


    void s_write( std::stringstream & s) const
    {
      std::stringstream sl,sr;
      m_l->s_write(sl);
      m_r->s_write(sr);
      std::vector<std::string> vs;
      vs.push_back(sl.str());
      vs.push_back(sr.str());
      s << m_f(vs);
    }

  private:
    //const std::string m_f;
    const std::function< std::string ( std::vector<std::string> ) > m_f;
    const std::shared_ptr< value_base > m_l;
    const std::shared_ptr< value_base > m_r;

    void v_write( std::ostream & o ) const
    {
      /*o << m_f << "( ";
        m_l->write( o );
        o << ", ";
        m_r->write( o );
        o << " )";*/
      std::stringstream sl,sr;
      m_l->s_write(sl);
      m_r->s_write(sr);
      std::vector<std::string> vs;
      vs.push_back(sl.str());
      vs.push_back(sr.str());
      o << m_f(vs);
    }
  };

  class operators
  {
  public:
    operators( std::function< bool( std::string ) >  _s2b , std::function< std::string ( bool ) >  _b2s )
    {
      this->s2b = _s2b;
      this->b2s = _b2s;

      /*postfix( "++", "post-inc" );
         postfix( "--", "post-dec" );*/

      prefix( "!", [&]( std::vector<std::string> s ){ assert(s.size()==1); bool b(s2b(s[0])); return b2s(!b); } );
      // bash needs space '! '
      prefix( "!=", [&]( std::vector<std::string> s ){ assert(s.size()==1); bool b(s2b(s[0])); return b2s(!b); } );
      /*prefix( "+", "plus" );
         prefix( "-", "minus" );
         prefix( "++", "pre-inc" );
         prefix( "--", "pre-dec" );
         prefix( "!", "logical-not" );
         prefix( "~", "bitwise-not" );
         //prefix( "&", "address-of" );
         prefix( "*", "dereference" );*/

      //         infix( ".", order( 2 ), assoc::LEFT, "dot" );
      //         infix( "->", order( 2 ), assoc::LEFT, "arrow" );
      //         infix( "*", order( 5 ), assoc::LEFT, "mul" );
      //         infix( "/", order( 5 ), assoc::LEFT, "div" );
      //         infix( "%", order( 5 ), assoc::LEFT, "mod" );
      //         //infix( "+", order( 6 ), assoc::LEFT, "add" );
      //infix( "+", order( 6 ), assoc::LEFT, [&]( std::vector<std::string> s ){ assert(s.size()==2); return s[0] + s[1]; } );
      //         infix( "-", order( 6 ), assoc::LEFT, "sub" );
      //         infix( "<<", order( 7 ), assoc::LEFT, "shl" );
      //         infix( ">>", order( 7 ), assoc::LEFT, "shr" );
      //         infix( "==", order( 9 ), assoc::LEFT, "equ" );
      //         infix( "!=", order( 9 ), assoc::LEFT, "neq" );
      //         infix( "|", order( 12 ), assoc::LEFT, "bw-or" );
      //         infix( "&&", order( 9 ), assoc::LEFT, "and" );
      infix( "&", order( 13 ), assoc::LEFT, [&]( std::vector<std::string> s ){ assert(s.size()==2); bool l(s2b(s[0])),r(s2b(s[1])); return b2s(l&r); } );
      infix( "&&", order( 13 ), assoc::LEFT, [&]( std::vector<std::string> s ){ assert(s.size()==2); bool l(s2b(s[0])),r(s2b(s[1])); return b2s(l&&r); } );
      infix( "||", order( 14 ), assoc::LEFT, [&]( std::vector<std::string> s ){ assert(s.size()==2); bool l(s2b(s[0])),r(s2b(s[1])); return b2s(l||r); } );
      //         infix( "||", order( 14 ), assoc::LEFT, "or" );
      //         infix( "=", order( 16 ), assoc::RIGHT, "let" );
      //         infix( "+=", order( 16 ), assoc::RIGHT, "add_let" );
      //         infix( "&", order( 10 ), assoc::LEFT, "bw-and" );

    }

    void infix( const std::string & name, const order p, const assoc a, const std::function< std::string ( std::vector<std::string> ) > &f/*const std::string & f*/ )
    {
      // It would be ambiguous to have a postfix and an infix operator with the same name.
      assert( m_postfix.find( name ) == m_postfix.end() );
      const auto i = m_pas.insert( { p, a } );
      assert( i.first->second == a );
      m_infix.insert( { name, { p, a, optype::BINARY, f } } );
    }

    void prefix( const std::string & name, const std::function< std::string ( std::vector<std::string> ) > &f/*const std::string & f*/ )
    {
      m_prefix.insert( { name, { order( 0 ), assoc::RIGHT, optype::UNARY, f } } );
    }

    void postfix( const std::string & name, const std::function< std::string ( std::vector<std::string> ) > &f/*const std::string & f*/ )
    {
      // It would be ambiguous to have a postfix and an infix operator with the same name.
      assert( m_infix.find( name ) == m_infix.end() );
      m_postfix.insert( { name, { order( 0 ), assoc::LEFT, optype::UNARY, f } } );
    }

    const std::map< order, assoc > & get() const
    {
      return m_pas;
    }

    const std::map< std::string, op > & get( const assoc a, const optype t ) const
    {
      switch ( t ) {
        case optype::BINARY:
          return m_infix;
        case optype::UNARY:
          switch ( a ) {
            case assoc::PREFIX:
              return m_prefix;
            case assoc::POSTFIX:
              return m_postfix;
            }
        }
      assert( 0 );
    }

  private:
    std::map< order, assoc > m_pas;
    std::function< bool( std::string ) > s2b;
    std::function< std::string ( bool ) > b2s;

    std::map< std::string, op > m_infix;
    std::map< std::string, op > m_prefix;
    std::map< std::string, op > m_postfix;
  };

  struct sh
  {
    std::vector< op > prefix;  // Initially outside-to-inside, then inside-to-outside
    std::shared_ptr< value_base > value;
  };

  struct stack
  {
    stack()
    {
      m_s.push_back( sh() );
    }

    void push( const op & b )
    {
      assert( ! m_s.empty() );

      switch ( b.t ) {
        case optype::BINARY:
          m_o.push_back( b );
          m_s.push_back( sh() );
          return;
        case optype::UNARY:
          switch( b.a ) {
            case assoc::PREFIX:
              m_s.back().prefix.push_back( b );
              return;
            case assoc::POSTFIX:
              m_s.back().value = std::make_shared< unary_value >( b.f, m_s.back().value );
              return;
            }
        }
    }

    void push( const long l )
    {
      push( std::make_shared< number_value >( l ) );
    }

    void push( const std::string & s )
    {
      push( std::make_shared< string_value >( s ) );
    }

    void push( const std::shared_ptr< value_base > & v )
    {
      m_s.back().value = v;
    }

    std::shared_ptr< value_base > finish( const operators & ops )
    {
      reduce_unary();

      for ( const auto & o : ops.get() ) {
          switch ( o.second ) {
            case assoc::LEFT:
              reduce_binary( o.first, false );
              break;
            case assoc::RIGHT:
              reverse();
              reduce_binary( o.first, true );
              reverse();
              break;
            }
        }
      assert( m_s.size() == 1 );
      const auto r = m_s.back().value;
      m_s.clear();
      return r;
    }

    std::string get( const operators & ops ){
      std::shared_ptr< value_base > val = this->finish(ops);
      std::stringstream ss;
      val->s_write(ss);
      return ss.str();
    }

  private:
    std::vector< op > m_o;
    std::vector< sh > m_s;

    void reverse()
    {
      std::reverse( m_o.begin(), m_o.end() );
      std::reverse( m_s.begin(), m_s.end() );
    }

    void reduce_unary()
    {
      for ( auto & s : m_s ) {
          std::reverse( s.prefix.begin(), s.prefix.end() );

          for ( const auto & i : s.prefix ) {
              s.value = std::make_shared< unary_value >( i.f, s.value );
            }
          s.prefix.clear();
        }
    }

    void reduce_binary( const order p, const bool r )
    {
      auto oi = m_o.begin();
      auto si = m_s.begin();

      while ( oi < m_o.end() ) {
          assert( si != m_s.end() );
          assert( ( si + 1 ) != m_s.end() );
          if ( oi->p == p ) {
              auto t = std::make_shared< binary_value >( oi->f, ( si + r )->value, ( si + ! r )->value );
              si = m_s.erase( si );
              oi = m_o.erase( oi );
              si->value = t;
            }
          else {
              ++oi;
              ++si;
            }
          assert( si != m_s.end() );
        }
    }
  };

  struct stacks
  {
    stacks()
    {
      open();
    }

    void open()
    {
      m_v.push_back( stack() );
    }

    template< typename T >
    void push( const T & t )
    {
      assert( ! m_v.empty() );
      m_v.back().push( t );
    }

    void close( const operators & ops )
    {
      assert( m_v.size() > 1 );
      const auto r = m_v.back().finish( ops );
      m_v.pop_back();
      m_v.back().push( r );
    }


    std::shared_ptr< value_base > finish( const operators & ops )
    {
      assert( m_v.size() == 1 );
      return m_v.back().finish( ops );
    }

    std::string get( const operators & ops ){
      std::shared_ptr< value_base > val = this->finish(ops);
      std::stringstream ss;
      val->s_write(ss);
      return ss.str();
    }

  private:
    std::vector< stack > m_v;
  };

  struct comment
      : if_must< one< '#' >, until< eolf > > {};

  struct nothing
      : sor< space, comment > {};

  template< assoc A, optype T >
  struct anyfix
  {
    template< typename Input >
    static bool match( Input & in, const operators & b, stacks & s, std::string t )
    {
      if ( in.size() > t.size() ) {
          t += in.peek_char( t.size() );
          const auto & m = b.get( A, T );
          const auto i = m.lower_bound( t );
          if ( i != m.end() ) {
              if ( match( in, b, s, t ) ) {
                  return true;
                }
              else if ( i->first == t ) {
                  s.push( i->second );
                  in.bump( t.size() );
                  return true;
                }
            }
        }
      return false;
    }

    /*CF //old
      template< error_mode E, template< typename ... > class Action, template< typename ... > class Control, typename Input >
      static bool match( Input & in, const operators & b, stacks & s )
      {
         return match( in, b, s, std::string() );
      }*/

    using analyze_t = analysis::generic< analysis::rule_type::ANY >;

    template< apply_mode E, template< typename ... > class Action, template< typename ... > class Control, typename Input >
    static bool match( Input & in, const operators & b, stacks & s )
    {
      // Look for the longest match of the input against the operators in the operator map.

      return match( in, b, s, std::string() );
    }
  };

  struct infix
      : anyfix< assoc::LEFT, optype::BINARY > {};  // The first template argument is a dummy here.

  struct prefix
      : star< anyfix< assoc::RIGHT, optype::UNARY >, star< nothing > > {};

  struct postfix
      : star< star< nothing >, anyfix< assoc::LEFT, optype::UNARY > > {};

  template< typename Rule >
  struct action
      : pegtl::nothing< Rule > {};

  struct number
      : plus< digit > {};

  struct expression;

  struct bracket
      : if_must< one< '(' >, expression, one< ')' > > {};

  struct atomic
      : sor< number, bracket, identifier > {};

  struct singular
      : seq< prefix, atomic, postfix > {};

  struct expression
      //: pad< list< singular, infix, nothing >, nothing > {};
      : list< singular, infix, nothing > {};

  struct grammar
      : must< expression, eof > {};
  //: if_must< expression, until< eof, nothing > > {};

  template<> struct action< number >
  {
    static bool apply( const input & in, const operators &, stacks & s )
    {
      s.push( std::stol( in.string() ) );
      return true;
    }
  };

  template<> struct action< identifier >
  {
    static bool apply( const input & in, const operators &, stacks & s )
    {
      s.push( in.string() );
      return true;
    }
  };

  template<> struct action< one< '(' > >
  {
    static bool apply( const input &, const operators &, stacks & s )
    {
      s.open();
      return true;
    }
  };

  template<> struct action< one< ')' > >
  {
    static bool apply( const input &, const operators & ops, stacks & s )
    {
      s.close( ops );
      return true;
    }
  };

} // expression
