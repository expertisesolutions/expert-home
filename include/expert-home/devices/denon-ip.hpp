// Copyright Felipe Magno de Almeida 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef EXPERT_DEVICES_DENON_IP_HPP
#define EXPERT_DEVICES_DENON_IP_HPP

#include <boost/asio.hpp>

#include <boost/signals2/signal.hpp>

#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/vector.hpp>
//#include <boost/spirit/home/support/extended_variant.hpp>
#include <expert-home/argument_variant.hpp>

namespace eh { namespace device {

namespace denon_detail {
  // boost::spirit::x3::rule<class identifier, std::string> const identifier("identifier");
  // BOOST_SPIRIT_DEFINE(identifier = +boost::spirit::x3::alpha);

namespace fusion = boost::fusion;
  
namespace x3 = boost::spirit::x3;
using x3::string; using x3::int_;
using x3::omit; using x3::eps; using x3::char_;
  
x3::rule<class switch_input, fusion::vector2<std::string, std::vector<std::string>>> const switch_input("switch_input");
auto switch_input_def =
 (string("SI")
  >> (string("PHONO") | string("CD") | string("TUNER") | string("DVD")
      | string("BD") | string("TV") | string("SAT/CBL") | string("MPLAY")
      | string("GAME") | string("HDRADIO") | string("NET") | string("PANDORA")
      | string("SIRIUSXM") | string("SPOTIFY") | string("LASTFM") | string("FLICKR")
      | string("IRADIO") | string("SERVER") | string("FAVORITES") | string("BT")
      | string("USB/IPOD") | string("USB") | string("IPD") | string("IRP")
      | string("FVP")
      // | (string("AUX") >> int_)
      )
  );
  //BOOST_SPIRIT_DEFINE(switch_input = switch_input_def);

}
    
struct denon_ip {

  denon_ip(boost::asio::io_service& service
           , const char* hostname)
    : socket(service), signal(nullptr), hostname(hostname)
  {
  }

  void change_input(std::string const& input)
  {
    std::cout << "Should change input to " << input << std::endl;

    namespace x3 = boost::spirit::x3;
    namespace fusion = boost::fusion;

    std::string output;
    if(x3::generate(std::back_inserter(output)
                    , denon_detail::switch_input_def >> x3::omit['\r']
                    , fusion::vector2<std::string, std::string>("SI", input)))
      {
        std::cout << "generated! yay (" << output.size() << ") |" << output << '|' << std::endl;
        boost::asio::write(socket, boost::asio::const_buffers_1(&output[0], output.size())
                           // , [this] (boost::system::error_code const& ec, std::size_t size)
                           // {
                           //   std::cout << "sent I think" << std::endl;
                           // }
                           );
      }
    else
      {
        std::cout << "failed generation" << std::endl;
      }
  }
  
  void handler(boost::system::error_code const& ec, std::size_t size)
  {
    if(!ec)
    {
      auto first = buffer.begin(), end = first + size
        , last = std::find(first, end, '\r');
      while(last != end)
        {
          std::cout << "command ";
          std::copy(first, last, std::ostream_iterator<char>(std::cout));
          std::endl(std::cout);

          boost::fusion::vector2<std::string, std::vector<argument_variant>>
            value;
            
          {
            namespace x3 = boost::spirit::x3;
            using x3::string; using x3::int_;
            using x3::omit; using x3::eps; using x3::char_;
            auto no_args = x3::attr(std::vector<argument_variant>());
            auto up_down_or_number = (string("UP") | string("DOWN") | int_);
            //auto up_down = (string("UP") | string("DOWN"));
            auto on_off = (string("ON") | string("OFF"));
            //auto on_off_up_down_or_number = string("ON") | string("OFF") | string("UP") | string("DOWN") | int_;
            
            bool b = boost::spirit::x3::parse
              (first, last
               ,
               ( (string("PW") >> (string("ON") | string("STANDBY")))
                 | (string("MV") >> up_down_or_number)
                 // CV command
                 | (string("CV")
                    >> ((string("FL") >> omit[' '] >>     up_down_or_number)
                        |  (string("FR") >> omit[' '] >>  up_down_or_number)
                        |  (string("FR") >> omit[' '] >>  up_down_or_number)
                        |  (string("C") >> omit[' '] >>   up_down_or_number)
                        |  (string("SW") >> omit[' '] >>  up_down_or_number)
                        |  (string("SW2") >> omit[' '] >> up_down_or_number)
                        |  (string("SL") >> omit[' '] >>  up_down_or_number)
                        |  (string("SR") >> omit[' '] >>  up_down_or_number)
                        |  (string("SBL") >> omit[' '] >> up_down_or_number)
                        |  (string("SBR") >> omit[' '] >> up_down_or_number)
                        |  (string("SB") >> omit[' '] >>  up_down_or_number)
                        |  (string("FHL") >> omit[' '] >> up_down_or_number)
                        |  (string("FHR") >> omit[' '] >> up_down_or_number)
                        |  (string("FWR") >> omit[' '] >> up_down_or_number)
                        
                        |  (string("TFL") >> omit[' '] >> up_down_or_number)
                        |  (string("TFR") >> omit[' '] >> up_down_or_number)
                        |  (string("TML") >> omit[' '] >> up_down_or_number)
                        |  (string("TMR") >> omit[' '] >> up_down_or_number)
                        |  (string("TRL") >> omit[' '] >> up_down_or_number)
                        |  (string("TRR") >> omit[' '] >> up_down_or_number)
                        |  (string("RHL") >> omit[' '] >> up_down_or_number)
                        |  (string("RHR") >> omit[' '] >> up_down_or_number)
                        |  (string("FDL") >> omit[' '] >> up_down_or_number)
                        |  (string("FDR") >> omit[' '] >> up_down_or_number)
                        |  (string("SDL") >> omit[' '] >> up_down_or_number)
                        |  (string("SDR") >> omit[' '] >> up_down_or_number)
                        |  (string("BDL") >> omit[' '] >> up_down_or_number)
                        |  (string("BDR") >> omit[' '] >> up_down_or_number)
                        |  (string("ZRL") >> omit[' '] >> up_down_or_number))
                   )
                 | (string("MU") >> on_off)
                 | denon_detail::switch_input_def
                 | (string("ZM")
                    >> (string("ON") | string("OFF")
                        | (string("FAVORITE") >> int_)
                        | (string("FAVORITE") >> int_ >> omit[' '] >> string("MEMORY"))
                       )
                   )
                 | (string("SD")
                    >> (string("AUTO") | string("HDMI") | string("DIGITAL") | string("ANALOG")
                        | string("EXT.IN") | string("7.1IN") | string("NO"))
                   )
                 | (string("DC")
                    >> (string("AUTO") | string("PCM") | string("DTS"))
                   )
                 | (string("SV")
                    >> (string("CD") | string("DVD")
                        | string("BD") | string("TV") | string("SAT/CBL") | string("MPLAY")
                        | string("GAME") | string("SOURCE") | string("ON") | string("OFF")
                        | (string("AUX") >> int_)
                        ))
                 | (string("SLP") >> (string("OFF") | int_))
                 | (string("STBY") >> (string("15M") | string("30M") | string("60M") | string("OFF")))
                 | (string("ECO") >> (string("ON") | string("AUTO") | string("OFF")))
                 | (string("MS")
                    >> (string("MOVIE") | string("MUSIC") | string("GAME") | string("DIRECT")
                        | string("PURE DIRECT") | string("STEREO") | string("AUTO") | string("DOLBY DIGITAL")
                        | string("DTS SURROUND") | string("MCH STEREO") | string("WIDE SCREEN") | string("SUPER STADIUM")
                        | string("ROCK ARENA") | string("JAZZ CLUB") | string("CLASSIC CONCERT") | string("MONO MOVIE")
                        | string("MATRIX") | string("VIDEO GAME") | string("VIRTUAL") | string("LEFT")
                        | string("RIGHT")
                        | (string("QUICK") >> int_)
                        | (string("QUICK") >> int_ >> string(" MEMORY"))
                        ))
                 | (string("VS")
                    >> (string("ASPNRM") | string("ASPFUL") | string("MONIAUTO") | string("MON1")
                        | string("MON2") | string("SC48P") | string("SC10I") | string("SC72P")
                        | string("SC10P") | string("SC10P24") | string("SC4K") | string("SC40KF")
                        | string("SCAUTO") | string("SCH48P") | string("SCH10I") | string("SCH72P")
                        | string("SCH10P") | string("SCH10P24") | string("SCH4K") | string("SCH4KF")
                        | string("SCHAUTO") | string("AUDIO AMP") | string("AUDIO TV") | string("VPMAUTO")
                        | string("VPMGAME") | string("VPMMOVI")
                        ))
                 // | (string("PS")
                 //    >> (
                 //        (string("TONE CTRL") >> omit[' '] >> on_off)
                 //        // | (string("BAS") >> omit[' '] >> up_down_or_number)
                 //        // | (string("TRE") >> omit[' '] >> up_down_or_number)
                 //        // | (string("DIL") >> omit[' '] >> on_off_up_down_or_number)
                 //        // | (string("SWL") >> omit[' '] >> on_off_up_down_or_number)
                 //        // | (string("SWL2") >> omit[' '] >> up_down_or_number)
                 //        // | (string("CINEMA EQ") >> omit['.'] >> on_off)
                 //        // | (string("MODE") >> omit[':'] >>
                 //        //    (string("MUSIC") | string("CINEMA") | string("GAME") | string("PRO LOGIC")))
                 //        // | (string("LOM") >> omit[' '] >> on_off)
                 //        // | (string("FH") >> omit[':'] >> on_off)
                 //        // // | (string("SP") >> omit[':']
                 //        // //    >> (string("FW") | string("FH") | string("SB") | string("HW")
                 //        // //        | string("BH") | string("BW") | string("FL") | string("HF")
                 //        // //        | string("FR"))
                 //        // //    )
                 //        // | (string("PHG") >> omit[' '] >> string("LOW") | string("MID") | string("HI"))
                 //        // | (string("MULTEQ") >> omit[':'] >>
                 //        //    (string("AUDYSSEY") | string("BYP.LR") | string("FLAT") | string("MANUAL") | string("OFF")))
                 //        // | (string("DYNEQ") >> omit[' '] >> on_off)
                 //        // | (string("REFLEV") >> omit[' '] >> int_)
                 //        // | (string("DYNVOL") >> omit[' '] >> (string("HEV") | string("MED") | string("LIT") | string("OFF")))
                 //        // | (string("LFC") >> omit[' '] >> on_off)
                 //        // | (string("CNTAMT") >> omit[' '] >> up_down_or_number)
                 //        // | (string("DSX") >> omit[' ']
                 //        //    >> (string("ONHW") | string("ONH") | string("ONW") | string("OFF")))
                 //        // | (string("STW") >> omit[' '] >> up_down_or_number)
                 //        // | (string("STH") >> omit[' '] >> up_down_or_number)
                 //        // | (string("GEQ") >> omit[' '] >> on_off)
                 //        // | (string("DRC") >> omit[' ']
                 //        //    >> (string("AUTO") | string("LOW") | string("MID") | string("HI") | string("OFF")))
                 //        // | (string("BSC") >> omit[' '] >> up_down_or_number)
                 //       // ))
               )
               >> x3::eoi
               , value
              );
            if(b)
            {
              std::cout << "command successfully parsed" << std::endl;
              (*signal)(boost::fusion::at_c<0>(value), boost::fusion::at_c<1>(value));
            }
            else
            {
              std::cout << "command failed parsing" << std::endl;
            }
          }

          first = last+1;
          last = std::find(first, end, '\r');
        }

      std::move(first, end, buffer.begin());

      boost::asio::async_read(socket, boost::asio::mutable_buffers_1(buffer.begin(), buffer.size())
                              , [this] (boost::system::error_code const& ec, std::size_t size)
                              {
                                auto last = buffer.begin() + size;
                                return ec || size == 1024 || std::find(buffer.begin(), last, '\r') != last;
                              }
                              , boost::bind(&denon_ip::handler, this, _1, _2)
                              );
    }
    else
    {
      std::cout << "error " << ec.message() << std::endl;
    }
  }
  
  void watch(boost::signals2::signal<void(std::string, std::vector<argument_variant>)>& signal)
  {
    boost::asio::ip::tcp::resolver resolver(socket.get_io_service());
    {
      boost::asio::ip::tcp::resolver::iterator
        iterator = resolver.resolve(boost::asio::ip::tcp::resolver::query{hostname, "telnet"});
      if(iterator != boost::asio::ip::tcp::resolver::iterator())
        socket.connect(*iterator);
    }
    
    boost::asio::async_read(socket, boost::asio::mutable_buffers_1(buffer.begin(), buffer.size())
                            , [this] (boost::system::error_code const& ec, std::size_t size)
                            {
                              auto last = buffer.begin() + size;
                              return ec || size == 1024 || std::find(buffer.begin(), last, '\r') != last;
                            }
                            , boost::bind(&denon_ip::handler, this, _1, _2)
                            );
    this->signal = &signal;
  }

  void command(const char*)
  {
  }

  std::array<char, 1024> buffer;
  boost::asio::ip::tcp::socket socket;
  boost::signals2::signal<void(std::string, std::vector<argument_variant>)>* signal;
  const char* hostname;
};
      
} }

#endif
