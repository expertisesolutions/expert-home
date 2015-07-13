// Copyright Felipe Magno de Almeida 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef EXPERT_DEVICES_DENON_IP_HPP
#define EXPERT_DEVICES_DENON_IP_HPP

#include <boost/asio.hpp>

#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/vector.hpp>
//#include <boost/spirit/home/support/extended_variant.hpp>
#include <expert-home/argument_variant.hpp>

#include <functional>

namespace eh { namespace device {

namespace denon_detail {
  // boost::spirit::x3::rule<class identifier, std::string> const identifier("identifier");
  // BOOST_SPIRIT_DEFINE(identifier = +boost::spirit::x3::alpha);

namespace fusion = boost::fusion;
  
namespace x3 = boost::spirit::x3;
using x3::string; using x3::int_;
using x3::omit; using x3::eps; using x3::char_;

auto up_down_or_number = (string("UP") | string("DOWN") | int_);
//auto up_down = (string("UP") | string("DOWN"));
auto on_off = (string("ON") | string("OFF"));
//auto on_off_up_down_or_number = string("ON") | string("OFF") | string("UP") | string("DOWN") | int_;
  
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
auto zone_master_def =
  (string("ZM")
   >> (string("ON")
       | string("OFF")
       //| (string("FAVORITE") >> int_)
       //| (string("FAVORITE") >> int_ >> omit[' '] >> string("MEMORY"))
       )
   );
auto zone_master_vol_def = string("MV") >> up_down_or_number;
  //BOOST_SPIRIT_DEFINE(switch_input = switch_input_def);

auto zone2_def =
  x3::string("Z2")
  >> (string("PHONO") | string("CD") | string("TUNER") | string("DVD")
      | string("BD") | string("TV") | string("SAT/CBL") | string("MPLAY")
      | string("GAME") | string("HDRADIO") | string("NET") | string("PANDORA")
      | string("SIRIUSXM") | string("SPOTIFY") | string("LASTFM") | string("FLICKR")
      | string("IRADIO") | string("SERVER") | string("FAVORITES") | string("BT")
      | string("USB/IPOD") | string("USB") | string("IPD") | string("IRP")
      | string("FVP")
      | string("SOURCE") // same as main zone
      | string("UP") | string("DOWN") | string("ON") | string("OFF")
      | x3::int_
      )
  ;
auto system_control_def =
  x3::string("MN")
  >> (x3::string("ZST ON") /*>> (x3::string("ON") | x3::string("OFF") | x3::string("?"))*/
      | x3::string("ZST OFF")
     )
  ;

auto system_power_def = string("PW") >> (string("ON") | string("STANDBY"));
auto mute_def = string("MU") >> on_off;
  
}
    
struct denon_ip {

  denon_ip(boost::asio::io_service& service
           , std::string hostname)
    : socket(service), hostname(hostname)
  {
    std::cout << "denon_ip constructor" << std::endl;
  }

  void send_command(std::string const& command, std::vector<argument_variant> const& args)
  {
    std::cout << "Denon Command " << command << " size " << args.size() << std::endl;

    namespace x3 = boost::spirit::x3;
    namespace fusion = boost::fusion;

    argument_variant arg1;
    if(!args.empty())
      arg1 = args[0];
      // if(std::string const* string = boost::get<std::string>(&args[0]))
      //   arg1 = *string;
    std::cout << "arg1 " << arg1 << std::endl;
    fusion::vector2<std::string, argument_variant> attr(command, arg1);

    std::string output;
    if(x3::generate(std::back_inserter(output)
                    ,
                    (
                     denon_detail::switch_input_def
                     | denon_detail::zone_master_def
                     | denon_detail::zone_master_vol_def
                     | denon_detail::zone2_def
                     | denon_detail::system_control_def
                     | denon_detail::system_power_def
                     | denon_detail::mute_def
                    )
                    >> x3::omit['\r']
                    , attr))
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
            using denon_detail::up_down_or_number;
            using denon_detail::on_off;
            
            bool b = boost::spirit::x3::parse
              (first, last
               ,
               ( denon_detail::system_power_def
                 | denon_detail::zone_master_vol_def
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
                 | denon_detail::mute_def
                 | denon_detail::switch_input_def
                 | denon_detail::zone_master_def
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
                 | (string("PS")
                    >> (
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
                        (string("MULTEQ") >> omit[':'] >>
                           (string("AUDYSSEY") | string("BYP.LR") | string("FLAT") | string("MANUAL") | string("OFF")))
                        | (string("DYNEQ") >> omit[' '] >> on_off)
                        | (string("REFLEV") >> omit[' '] >> int_)
                        | (string("DYNVOL") >> omit[' '] >> (string("HEV") | string("MED") | string("LIT") | string("OFF")))
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
                       ))
                 | denon_detail::zone2_def
                 | denon_detail::system_control_def
               )
               >> x3::eoi
               , value
              );
            if(b)
            {
              std::cout << "command successfully parsed" << std::endl;
              function(boost::fusion::at_c<0>(value), boost::fusion::at_c<1>(value));
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
  
  void watch(std::function<void(std::string, std::vector<argument_variant>)> function)
  {
    std::cout << "watch" << std::endl;
    boost::asio::ip::tcp::resolver resolver(socket.get_io_service());
    {
      boost::asio::ip::tcp::resolver::iterator
        iterator = resolver.resolve(boost::asio::ip::tcp::resolver::query{hostname, "telnet"});
      if(iterator != boost::asio::ip::tcp::resolver::iterator())
        socket.connect(*iterator);
    }
    std::cout << "watch" << std::endl;
    
    boost::asio::async_read(socket, boost::asio::mutable_buffers_1(buffer.begin(), buffer.size())
                            , [this] (boost::system::error_code const& ec, std::size_t size)
                            {
                              auto last = buffer.begin() + size;
                              return ec || size == buffer.size() || std::find(buffer.begin(), last, '\r') != last;
                            }
                            , boost::bind(&denon_ip::handler, this, _1, _2)
                            );

    const char pwstatus[] = "PW?\r";
    boost::asio::write(socket, boost::asio::const_buffers_1(&pwstatus[0], sizeof(pwstatus)-1));
    const char zmstatus[] = "ZM?\r";
    boost::asio::write(socket, boost::asio::const_buffers_1(&zmstatus[0], sizeof(zmstatus)-1));
    const char mvstatus[] = "MV?\r";
    boost::asio::write(socket, boost::asio::const_buffers_1(&mvstatus[0], sizeof(mvstatus)-1));
    const char z2status[] = "Z2?\r";
    boost::asio::write(socket, boost::asio::const_buffers_1(&z2status[0], sizeof(z2status)-1));
    const char zonestereostatus[] = "MNZST?\r";
    boost::asio::write(socket, boost::asio::const_buffers_1(&zonestereostatus[0], sizeof(zonestereostatus)-1));
    const char psmulteqstatus[] = "PSMULTEQ: ?\r";
    boost::asio::write(socket, boost::asio::const_buffers_1(&psmulteqstatus[0], sizeof(psmulteqstatus)-1));
    const char psdyneqstatus[] = "PSDYNEQ ?\r";
    boost::asio::write(socket, boost::asio::const_buffers_1(&psdyneqstatus[0], sizeof(psdyneqstatus)-1));
    const char psdynvolstatus[] = "PSDYNVOL ?\r";
    boost::asio::write(socket, boost::asio::const_buffers_1(&psdynvolstatus[0], sizeof(psdynvolstatus)-1));
    const char psreflevstatus[] = "PSREFLEV ?\r";
    boost::asio::write(socket, boost::asio::const_buffers_1(&psreflevstatus[0], sizeof(psreflevstatus)-1));
    
    std::cout << "watch" << std::endl;
    this->function = function;
  }

  std::array<char, 4096> buffer;
  boost::asio::ip::tcp::socket socket;
  std::string hostname;
  std::function<void(std::string, std::vector<argument_variant>)> function;
};
      
} }

#endif
