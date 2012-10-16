#ifndef BRAINCUNT_APP_HPP
#define BRAINCUNT_APP_HPP

#include <sprig/config/config.hpp>

#ifdef SPRIG_USING_PRAGMA_ONCE
#	pragma once
#endif	// #ifdef SPRIG_USING_PRAGMA_ONCE

#include <cstddef>
#include <clocale>
#include <string>
#include <vector>
#include <iterator>
#include <iostream>
#include <fstream>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/utility/swap.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/program_options/parsers.hpp>
#include <sprig/braincunt.hpp>

#include "app_version.h"

namespace braincunt {
	namespace app {
		//
		// startup_app
		//
		SPRIG_INLINE void startup_app() {
			std::setlocale(LC_ALL, "");
			std::cout
				<< std::endl
				<< "> braincunt: " BRAINCUNT_MESSAGE << std::endl
				<< "> version: " BRAINCUNT_VERSION_STRING << std::endl
				<< "> build: " __DATE__ " " __TIME__ << std::endl
				<< std::endl
				;
		}
		//
		// add_options
		//
		SPRIG_INLINE void add_options(
			boost::program_options::options_description& desc
			)
		{
			desc.add_options()
				(
					"input,i",
					boost::program_options::value<std::vector<std::string> >(),
					"Input file. (source code)"
					)
				(
					"break,b",
					"Break mode. (execution control)"
					)
				(
					"pure,p",
					"Pure Brainfuck operation. (command restriction)"
					)
				(
					"ext-bitwise",
					"Bitwise operation. (extended command)"
					)
				(
					"ext-arithmetic",
					"Arithmetic operation. (extended command)"
					)
				(
					"ext-copyswap",
					"Copy-swap operation. (extended command)"
					)
				(
					"ext-axis",
					"Axis operation. (extended command)"
					)
				(
					"ext-comment",
					"Comment process. (remove from code)"
					)
				(
					"load-xml",
					boost::program_options::value<std::string>(),
					"Load serialized xml file. (braincunt engine)"
					)
				(
					"load-text",
					boost::program_options::value<std::string>(),
					"Load serialized text file. (braincunt engine)"
					)
				(
					"load-binary",
					boost::program_options::value<std::string>(),
					"Load serialized binary file. (braincunt engine)"
					)
				;
		}
		//
		// analy_command_line
		//
		SPRIG_INLINE void analy_command_line(
			boost::program_options::variables_map& vmap,
			boost::program_options::options_description const& desc,
			int argc,
			char* argv[]
			)
		{
			boost::program_options::positional_options_description positional_desc;
			positional_desc.add("input", -1);
			boost::program_options::options_description command_line_desc;
			command_line_desc.add_options()
				("help,h", "Print out program usage. (this message)")
				;
			command_line_desc.add(desc);
			boost::program_options::store(
				boost::program_options::command_line_parser(argc, argv)
					.options(command_line_desc)
					.positional(positional_desc)
					.run(),
				vmap
				);
			boost::program_options::notify(vmap);
			if (vmap.count("help")) {
				std::cout
					<< "> help:" << std::endl
					<< command_line_desc
					<< std::endl
					;
			}
		}
		//
		// get_break_mode
		//
		SPRIG_INLINE bool get_break_mode(
			boost::program_options::variables_map const& vmap
			)
		{
			bool result = false;
			if (vmap.count("break")) {
				result = true;
				std::cout
					<< "> break: " << result << std::endl
					;
			}
			return result;
		}
		//
		// setup_load
		//
		SPRIG_INLINE void setup_load(
			sprig::braincunt::engine& engine,
			boost::program_options::variables_map const& vmap
			)
		{
			if (vmap.count("load-xml")) {
				std::string const& file = vmap["load-xml"].as<std::string>();
				std::cout
					<< "> load: " << file << std::endl
					;
				std::ifstream fis(file.c_str());
				if (fis) {
					fis >> std::noskipws;
					boost::archive::xml_iarchive ia(fis);
					ia >> boost::serialization::make_nvp("braincunt", engine);
				} else {
					std::cout
						<< "> load error: Invalid load file." << std::endl
						;
				}
			} else if (vmap.count("load-text")) {
				std::string const& file = vmap["load-text"].as<std::string>();
				std::cout
					<< "> load: " << file << std::endl
					;
				std::ifstream fis(file.c_str());
				if (fis) {
					fis >> std::noskipws;
					boost::archive::text_iarchive ia(fis);
					ia >> boost::serialization::make_nvp("braincunt", engine);
				} else {
					std::cout
						<< "> load error: Invalid load file." << std::endl
						;
				}
			} else if (vmap.count("load-binary")) {
				std::string const& file = vmap["load-binary"].as<std::string>();
				std::cout
					<< "> load: " << file << std::endl
					;
				std::ifstream fis(file.c_str());
				if (fis) {
					fis >> std::noskipws;
					boost::archive::binary_iarchive ia(fis);
					ia >> boost::serialization::make_nvp("braincunt", engine);
				} else {
					std::cout
						<< "> load error: Invalid load file." << std::endl
						;
				}
			}
		}
		//
		// setup_ext
		//
		SPRIG_INLINE void setup_ext(
			sprig::braincunt::engine& engine,
			boost::program_options::variables_map const& vmap
			)
		{
			if (vmap.count("pure")) {
				std::cout
					<< "> option: pure" << std::endl
					;
				engine.pure() = true;
			} else {
				if (vmap.count("ext-bitwise")) {
					std::cout
						<< "> option: ext-bitwise" << std::endl
						;
					engine.ext_bitwise() = true;
				}
				if (vmap.count("ext-arithmetic")) {
					std::cout
						<< "> option: ext-arithmetic" << std::endl
						;
					engine.ext_arithmetic() = true;
				}
				if (vmap.count("ext-copyswap")) {
					std::cout
						<< "> option: ext-copyswap" << std::endl
						;
					engine.ext_copyswap() = true;
				}
				if (vmap.count("ext-axis")) {
					std::cout
						<< "> option: ext-axis" << std::endl
						;
					engine.ext_axis() = true;
				}
				if (vmap.count("ext-comment")) {
					std::cout
						<< "> option: ext-comment" << std::endl
						;
					engine.ext_comment() = true;
				}
			}
		}
		//
		// setup_input_and_folder
		//
		SPRIG_INLINE void setup_input_and_folder(
			sprig::braincunt::engine& engine,
			boost::program_options::variables_map const& vmap,
			int argc,
			char* argv[]
			)
		{
			boost::filesystem::path working_folder;
			if (vmap.count("input")) {
				BOOST_FOREACH(std::string const& file, vmap["input"].as<std::vector<std::string> >()) {
					std::cout
						<< "> input: " << file << std::endl
						;
					std::ifstream fis(file.c_str());
					if (fis) {
						fis >> std::noskipws;
						engine.push_code(fis);
						if (working_folder.empty()) {
							working_folder = boost::filesystem::path(file).parent_path();
						}
					} else {
						std::cout
							<< "> input error: Invalid input file." << std::endl
							;
					}
				}
			} else {
				if (argc >= 1) {
					working_folder = boost::filesystem::path(argv[0]).parent_path();
				}
			}
			if (working_folder.empty()) {
				working_folder = boost::filesystem::initial_path();
			}
			std::cout
				<< "> working folder: " << working_folder << std::endl
				;
			boost::filesystem::current_path(working_folder);
		}
		//
		// print_info
		//
		SPRIG_INLINE void print_info(
			sprig::braincunt::engine& engine
			)
		{
			std::cout
				<< "> dbuffer:" << std::endl
				;
			{
				sprig::braincunt::engine::dxyzbuffer_type const& dxyzbuffer = engine.dbuffer();
				std::cout
					<< boost::format("> z: {%1%-%2%}")
						% dxyzbuffer.first.size()
						% dxyzbuffer.second
					;
				std::cout
					<< std::endl
					;
				sprig::braincunt::engine::dxyz_type::size_type z = 0;
				BOOST_FOREACH(sprig::braincunt::engine::dxybuffer_type const& dxybuffer, engine.dbuffer().first) {
					std::cout
						<< boost::format("> [%3%] y: {%1%-%2%}")
							% dxybuffer.first.size()
							% dxybuffer.second
							% static_cast<std::ptrdiff_t>(z - dxyzbuffer.second)
						;
					std::cout
						<< std::endl
						;
					sprig::braincunt::engine::dxy_type::size_type y = 0;
					BOOST_FOREACH(sprig::braincunt::engine::dxbuffer_type const& dxbuffer, dxybuffer.first) {
						std::cout
							<< boost::format("> [%3%][%4%] x: {%1%-%2%} |")
								% dxbuffer.first.size()
								% dxbuffer.second
								% static_cast<std::ptrdiff_t>(z - dxyzbuffer.second)
								% static_cast<std::ptrdiff_t>(y - dxybuffer.second)
							;
						sprig::braincunt::engine::dxy_type::size_type x = 0;
						BOOST_FOREACH(sprig::braincunt::engine::byte_type const& d, dxbuffer.first) {
							std::cout
								<< (x == dxbuffer.second
									? '*'
									: ' '
									)
								<< boost::format("%1$02X")
									% static_cast<std::size_t>(d)
								;
							++x;
						}
						if (dxbuffer.first.size() == dxbuffer.second) {
							std::cout
								<< '*'
								;
						}
						std::cout
							<< std::endl
							;
						++y;
					}
					++z;
				}
			}
			std::cout
				<< "> dpointer: ["
				<< engine.dpointer().get<sprig::braincunt::engine::x_axis>() << ", "
				<< engine.dpointer().get<sprig::braincunt::engine::y_axis>() << ", "
				<< engine.dpointer().get<sprig::braincunt::engine::z_axis>()
				<< "]" << std::endl
				;
			std::cout
				<< "> front: "
				<< (engine.front().second
					? '-'
					: '+'
					)
				<< (engine.front().first == sprig::braincunt::engine::x_axis_direction
					? 'x'
					: engine.front().first == sprig::braincunt::engine::y_axis_direction
					? 'y'
					: engine.front().first == sprig::braincunt::engine::z_axis_direction
					? 'z'
					: '!'
					)
				<< std::endl
				;
			std::cout
				<< "> head: "
				<< (engine.head().second
					? '-'
					: '+'
					)
				<< (engine.head().first == sprig::braincunt::engine::x_axis_direction
					? 'x'
					: engine.head().first == sprig::braincunt::engine::y_axis_direction
					? 'y'
					: engine.head().first == sprig::braincunt::engine::z_axis_direction
					? 'z'
					: '!'
					)
				<< std::endl
				;
		}
		//
		// process_command
		//
		SPRIG_INLINE bool process_command(
			sprig::braincunt::engine& engine,
			std::string const& command,
			bool& is_quit
			)
		{
			if (command == "i") {
				std::cout
					<< "> input code: "
					;
				std::string code;
				do {
					std::getline(std::cin, code);
				} while (code.empty());
				engine.push_code(code);
			} else if (command == "p") {
				std::cout
					<< "> print code: "
					;
					std::copy(
						engine.pbuffer().begin(),
						engine.pbuffer().end(),
						std::ostream_iterator<char>(std::cout)
					);
				std::cout
					<< std::endl
					;
			} else if (command == "s") {
				std::cout
					<< "> save code: "
					;
				std::string file;
				do {
					std::getline(std::cin, file);
				} while (file.empty());
				std::ofstream fos(file.c_str());
				if (fos) {
					fos << engine.pbuffer();
				} else {
					std::cout
						<< "> save error: Invalid save file." << std::endl
						;
				}
			} else if (command == "q") {
				std::cout
					<< "> quit:" << std::endl
					;
				is_quit = true;
			} else if (command == "r") {
				std::cout
					<< "> reset:" << std::endl
					;
				sprig::braincunt::engine new_engine;
				new_engine.pure() = engine.pure();
				new_engine.ext_bitwise() = engine.ext_bitwise();
				new_engine.ext_arithmetic() = engine.ext_arithmetic();
				new_engine.ext_copyswap() = engine.ext_copyswap();
				new_engine.ext_axis() = engine.ext_axis();
				new_engine.ext_comment() = engine.ext_comment();
				boost::swap(engine, new_engine);
			} else if (command == "sxml") {
				std::cout
					<< "> save xml: "
					;
				std::string file;
				do {
					std::getline(std::cin, file);
				} while (file.empty());
				std::ofstream fos(file.c_str());
				if (fos) {
					boost::archive::xml_oarchive oa(fos);
					oa << boost::serialization::make_nvp("braincunt", engine);
				} else {
					std::cout
						<< "> save error: Invalid save file." << std::endl
						;
				}
			} else if (command == "stxt") {
				std::cout
					<< "> save text: "
					;
				std::string file;
				do {
					std::getline(std::cin, file);
				} while (file.empty());
				std::ofstream fos(file.c_str());
				if (fos) {
					boost::archive::text_oarchive oa(fos);
					oa << boost::serialization::make_nvp("braincunt", engine);
				} else {
					std::cout
						<< "> save error: Invalid save file." << std::endl
						;
				}
			} else if (command == "sbin") {
				std::cout
					<< "> save binary: "
					;
				std::string file;
				do {
					std::getline(std::cin, file);
				} while (file.empty());
				std::ofstream fos(file.c_str());
				if (fos) {
					boost::archive::binary_oarchive oa(fos);
					oa << boost::serialization::make_nvp("braincunt", engine);
				} else {
					std::cout
						<< "> save error: Invalid save file." << std::endl
						;
				}
			} else {
				return false;
			}
			return true;
		}
		//
		// shutdown_app
		//
		SPRIG_INLINE void shutdown_app() {
			std::cout
				<< std::endl
				<< "> end braincunt: " BRAINCUNT_EXIT_MESSAGE << std::endl
				<< std::endl
				;
		}
	}	// namespace app
}	// namespace braincunt

#endif	// #ifndef BRAINCUNT_APP_HPP
