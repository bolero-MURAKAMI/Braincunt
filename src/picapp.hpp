#ifndef BRAINCUNT_PICAPP_HPP
#define BRAINCUNT_PICAPP_HPP

#include <sprig/config/config.hpp>

#ifdef SPRIG_USING_PRAGMA_ONCE
#	pragma once
#endif	// #ifdef SPRIG_USING_PRAGMA_ONCE

#include <cctype>
#include <cstdio>
#include <string>
#include <iterator>
#include <algorithm>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/next_prior.hpp>
#include <boost/cstdint.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <sprig/split_join.hpp>
#include <sprig/exception.hpp>
#include <sprig/braincunt.hpp>
#include <sprig/libpng/basic.hpp>
#include <sprig/libpng/png_reader.hpp>
#include <sprig/libpng/png_writer.hpp>

#include "app_version.h"

namespace braincunt {
	namespace picapp {
		//
		// add_pic_options
		//
		SPRIG_INLINE void add_pic_options(
			boost::program_options::options_description& desc
			)
		{
			desc.add_options()
				(
					"pic-input",
					boost::program_options::value<std::string>(),
					"Input file. (picture)"
					)
				(
					"pic-color",
					boost::program_options::value<std::string>()->default_value("rgba"),
					"Pixel order. (picture)"
					)
				(
					"pic-output",
					boost::program_options::value<std::string>(),
					"Output file. (picture)"
					)
				;
		}
		//
		// setup_pic
		//
		SPRIG_INLINE void setup_pic(
			sprig::braincunt::engine& engine,
			boost::program_options::variables_map const& vmap
			)
		{
			if (vmap.count("pic-input")) {
				std::string const& file = vmap["pic-input"].as<std::string>();
				std::cout
					<< "> pic-input: " << file << std::endl
					;
				std::string file_ext = boost::filesystem::path(file).extension().string();
				std::transform(
					file_ext.begin(),
					file_ext.end(),
					file_ext.begin(),
					&::tolower
					);
				std::cout
					<< "> extension: " << file_ext << std::endl
					;
				if (file_ext == ".png") {
					std::vector<sprig::png::byte_type> data;
					{
						std::ifstream ifs(file.c_str(), std::ios_base::in | std::ios_base::binary);
						if (ifs) {
							ifs >> std::noskipws;
							data.insert(
								data.end(),
								std::istream_iterator<sprig::png::byte_type>(ifs),
								std::istream_iterator<sprig::png::byte_type>()
								);
						} else {
							std::cout
								<< "> input error: Invalid input file." << std::endl
								;
						}
					}
					if (!data.empty()) {
						if (!sprig::png::png_sig_check(&data[0], data.size())) {
							std::cout
								<< "> invalid format: Illegal PNG signature." << std::endl
								;
						} else {
							std::string color = vmap["pic-color"].as<std::string>();
							std::transform(
								color.begin(),
								color.end(),
								color.begin(),
								&::tolower
								);
							std::cout
								<< "> pic-color: " << color << std::endl
								;
							if (color != "rgba" && color != "rgb") {
								std::cout
									<< "> unsupported color: " << std::endl
									;
							}
							sprig::png::png_pixels_order order = color == "rgba"
								? sprig::png::png_pixels_order_rgba
								: color == "rgb"
								? sprig::png::png_pixels_order_rgb
								: sprig::png::png_pixels_order_rgba
								;
							std::vector<sprig::png::byte_type> image;
							sprig::png::IHDR ihdr;
							sprig::png::png_reader reader;
							if (SPRIG_PNG_SETJMP(reader.png())) {
								std::cout
									<< "> internal error: libpng" << std::endl
									;
								SPRIG_THROW_EXCEPTION(sprig::braincunt::base_exception("internal error: libpng"));
							}
							reader.read_all(&data[0], ihdr, image, order);
							std::cout
								<< "> IHDR-width: " << ihdr.width << std::endl
								<< "> IHDR-height: " << ihdr.height << std::endl
								<< "> IHDR-bit_depth: " << ihdr.bit_depth << std::endl
								<< "> IHDR-color_type: " << ihdr.color_type << std::endl
								<< "> IHDR-interlace_method: " << ihdr.interlace_method << std::endl
								<< "> IHDR-compression_method: " << ihdr.compression_method << std::endl
								<< "> IHDR-filter_method: " << ihdr.filter_method << std::endl
								;
							ihdr.bit_depth = 8;
							ihdr.color_type = order == sprig::png::png_pixels_order_rgba
								? PNG_COLOR_TYPE_RGB_ALPHA
								: order == sprig::png::png_pixels_order_rgb
								? PNG_COLOR_TYPE_RGB
								: PNG_COLOR_TYPE_RGB_ALPHA
								;
							png_uint_32 image_pitch = image.size() / ihdr.height;
							{
								sprig::braincunt::engine::dpointer_type head(0, 0, 1);
								sprig::braincunt::engine::dpointer_type tail(image_pitch - 1, 0, 1);
								engine.expand_dbuffer(tail);
								sprig::braincunt::engine::dxyz_type::value_type& dxybuffer = engine.dbuffer().first.at(
									engine.dbuffer().second + head.get<sprig::braincunt::engine::z_axis>()
									);
								for (png_uint_32 i = 0, last = ihdr.height; i != last; ++i) {
									head.get<sprig::braincunt::engine::y_axis>() = i;
									tail.get<sprig::braincunt::engine::y_axis>() = i;
									engine.expand_dbuffer(tail);
									sprig::braincunt::engine::dxy_type::value_type& dxbuffer = dxybuffer.first.at(
										dxybuffer.second + i
										);
									std::copy(
										boost::next(image.begin(), image_pitch * i),
										boost::next(image.begin(), image_pitch * (i + 1)),
										boost::next(dxbuffer.first.begin(), dxbuffer.second)
										);
								}
							}
							{
								sprig::braincunt::engine::dpointer_type head(0, -1, -1);
								sprig::braincunt::engine::dpointer_type tail(0, -1, -1);
								engine.expand_dbuffer(tail);
								sprig::braincunt::engine::dxyz_type::value_type& dxybuffer = engine.dbuffer().first.at(
									engine.dbuffer().second + head.get<sprig::braincunt::engine::z_axis>()
									);
								{
									head.get<sprig::braincunt::engine::y_axis>() = 0;
									tail.get<sprig::braincunt::engine::y_axis>() = 0;
									head.get<sprig::braincunt::engine::x_axis>() = 0;
									tail.get<sprig::braincunt::engine::x_axis>() = 4 * 4 - 1;
									engine.expand_dbuffer(tail);
									sprig::braincunt::engine::dxy_type::value_type& dxbuffer = dxybuffer.first.at(
										dxybuffer.second + head.get<sprig::braincunt::engine::y_axis>()
										);
									sprig::integer_split<boost::uint32_t>(
										ihdr.width,
										boost::next(dxbuffer.first.begin(), dxbuffer.second)
										);
									sprig::integer_split<boost::uint32_t>(
										ihdr.height,
										boost::next(dxbuffer.first.begin(), dxbuffer.second + 4)
										);
									sprig::integer_split<boost::int32_t>(
										ihdr.bit_depth,
										boost::next(dxbuffer.first.begin(), dxbuffer.second + 4 * 2)
										);
									sprig::integer_split<boost::int32_t>(
										ihdr.color_type,
										boost::next(dxbuffer.first.begin(), dxbuffer.second + 4 * 3)
										);
								}
							}
						}
					} else {
						std::cout
							<< "> input error: Invalid input file." << std::endl
							;
					}
				} else {
					std::cout
						<< "> unsupported file:" << std::endl
						;
				}
			}
		}
		//
		// pic_output
		//
		SPRIG_INLINE void pic_output(
			sprig::braincunt::engine& engine,
			std::string const& file
			)
		{
			std::cout
				<< "> pic-output: " << file << std::endl
				;
			std::string file_ext = boost::filesystem::path(file).extension().string();
			std::transform(
				file_ext.begin(),
				file_ext.end(),
				file_ext.begin(),
				&::tolower
				);
			std::cout
				<< "> extension: " << file_ext << std::endl
				;
			if (file_ext == ".png") {
				boost::shared_ptr<FILE> fp(
					std::fopen(file.c_str(), "wb"),
					&std::fclose
					);
				if (fp) {
					sprig::png::IHDR ihdr;
					{
						sprig::braincunt::engine::dpointer_type head(0, 0, -1);
						sprig::braincunt::engine::dpointer_type tail(0, 0, -1);
						engine.expand_dbuffer(tail);
						sprig::braincunt::engine::dxyz_type::value_type& dxybuffer = engine.dbuffer().first.at(
							engine.dbuffer().second + head.get<sprig::braincunt::engine::z_axis>()
							);
						{
							head.get<sprig::braincunt::engine::y_axis>() = 0;
							tail.get<sprig::braincunt::engine::y_axis>() = 0;
							head.get<sprig::braincunt::engine::x_axis>() = 0;
							tail.get<sprig::braincunt::engine::x_axis>() = 4 * 4 - 1;
							if (!engine.is_out_of_bounds_dbuffer(tail)) {
								sprig::braincunt::engine::dxy_type::value_type& dxbuffer = dxybuffer.first.at(
									dxybuffer.second + head.get<sprig::braincunt::engine::y_axis>()
									);
								ihdr.width = sprig::integer_join<boost::uint32_t>(
									boost::next(dxbuffer.first.begin(), dxbuffer.second)
									);
								ihdr.height = sprig::integer_join<boost::uint32_t>(
									boost::next(dxbuffer.first.begin(), dxbuffer.second + 4)
									);
								ihdr.bit_depth = sprig::integer_join<boost::int32_t>(
									boost::next(dxbuffer.first.begin(), dxbuffer.second + 4 * 2)
									);
								ihdr.color_type = sprig::integer_join<boost::int32_t>(
									boost::next(dxbuffer.first.begin(), dxbuffer.second + 4 * 3)
									);
							}
						}
						std::cout
							<< "> IHDR-width: " << ihdr.width << std::endl
							<< "> IHDR-height: " << ihdr.height << std::endl
							<< "> IHDR-bit_depth: " << ihdr.bit_depth << std::endl
							<< "> IHDR-color_type: " << ihdr.color_type << std::endl
							<< "> IHDR-interlace_method: " << ihdr.interlace_method << std::endl
							<< "> IHDR-compression_method: " << ihdr.compression_method << std::endl
							<< "> IHDR-filter_method: " << ihdr.filter_method << std::endl
							;
						if (!(ihdr.width <= 0
							|| ihdr.height <= 0
							|| ihdr.bit_depth != 8
							|| ihdr.color_type != PNG_COLOR_TYPE_RGB_ALPHA && ihdr.color_type != PNG_COLOR_TYPE_RGB
							))
						{
							sprig::png::png_pixels_order order = ihdr.color_type == PNG_COLOR_TYPE_RGB_ALPHA
								? sprig::png::png_pixels_order_rgba
								: ihdr.color_type == PNG_COLOR_TYPE_RGB
								? sprig::png::png_pixels_order_rgb
								: sprig::png::png_pixels_order_rgba
								;
							png_uint_32 image_pitch = ihdr.color_type == PNG_COLOR_TYPE_RGB_ALPHA
								? ihdr.width * 4
								: ihdr.color_type == PNG_COLOR_TYPE_RGB
								? ihdr.width * 3
								: ihdr.width * 4
								;
							if (image_pitch % 4) {
								image_pitch += 4 - image_pitch % 4;
							}
							std::vector<sprig::png::byte_type> image(image_pitch * ihdr.height);
							{
								sprig::braincunt::engine::dpointer_type head(0, 0, 1);
								sprig::braincunt::engine::dpointer_type tail(image_pitch - 1, 0, 1);
								engine.expand_dbuffer(tail);
								sprig::braincunt::engine::dxyz_type::value_type& dxybuffer = engine.dbuffer().first.at(
									engine.dbuffer().second + head.get<sprig::braincunt::engine::z_axis>()
									);
								for (png_uint_32 i = 0, last = ihdr.height; i != last; ++i) {
									head.get<sprig::braincunt::engine::y_axis>() = i;
									tail.get<sprig::braincunt::engine::y_axis>() = i;
									engine.expand_dbuffer(tail);
									sprig::braincunt::engine::dxy_type::value_type& dxbuffer = dxybuffer.first.at(
										dxybuffer.second + i
										);
									std::copy(
										boost::next(dxbuffer.first.begin(), dxbuffer.second),
										boost::next(dxbuffer.first.begin(), dxbuffer.second + image_pitch),
										boost::next(image.begin(), image_pitch * i)
										);
								}
							}
							sprig::png::png_writer writer;
							if (SPRIG_PNG_SETJMP(writer.png())) {
								std::cout
									<< "> internal error: libpng" << std::endl
									;
								SPRIG_THROW_EXCEPTION(sprig::braincunt::base_exception("internal error: libpng"));
							}
							writer.write_all(fp.get(), ihdr, image, order);
						} else {
							std::cout
								<< "> invalid format: Illegal IHDR." << std::endl
								;
						}
					}
				} else {
					std::cout
						<< "> output error: Invalid output file." << std::endl
						;
				}
			} else {
				std::cout
					<< "> unsupported file:" << std::endl
					;
			}
		}
		//
		// process_pic_command
		//
		SPRIG_INLINE bool process_pic_command(
			sprig::braincunt::engine& engine,
			std::string const& command
			)
		{
			if (command == "pout") {
				std::cout
					<< "> pic output: "
					;
				std::string file;
				do {
					std::getline(std::cin, file);
				} while (file.empty());
				pic_output(engine, file);
			} else {
				return false;
			}
			return true;
		}
		//
		// shutdown_pic
		//
		SPRIG_INLINE void shutdown_pic(
			sprig::braincunt::engine& engine,
			boost::program_options::variables_map const& vmap
			)
		{
			if (vmap.count("pic-output")) {
				std::string const& file = vmap["pic-output"].as<std::string>();
				pic_output(engine, file);
			}
		}
	}	// namespace picapp
}	// namespace braincunt

#endif	// #ifndef BRAINCUNT_PICAPP_HPP
