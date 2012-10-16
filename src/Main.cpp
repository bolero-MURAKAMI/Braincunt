
#include "Config.hpp"

#include <cstddef>
#include <string>
#include <iostream>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/options_description.hpp>
#include <sprig/braincunt.hpp>

#include "app_version.h"
#include "app.hpp"
#include "picapp.hpp"

#pragma message("> braincunt: " BRAINCUNT_MESSAGE)
#pragma message("> version: " BRAINCUNT_VERSION_STRING)
#pragma message("> build: " __DATE__ " " __TIME__)

namespace app {
	//
	// exec
	//
	void exec(int argc, char* argv[]) {
		//
		//	COMMENT: スタートアップ
		//
		braincunt::app::startup_app();
		//
		//	COMMENT: オプション解析
		//
		boost::program_options::variables_map vmap;
		{
			boost::program_options::options_description desc;
			braincunt::app::add_options(desc);
			braincunt::picapp::add_pic_options(desc);
			braincunt::app::analy_command_line(vmap, desc, argc, argv);
		}
		//
		//	COMMENT: セットアップ
		//
		sprig::braincunt::engine engine;
		bool break_mode = braincunt::app::get_break_mode(vmap);
		braincunt::app::setup_load(engine, vmap);
		braincunt::app::setup_ext(engine, vmap);
		braincunt::app::setup_input_and_folder(engine, vmap, argc, argv);
		braincunt::picapp::setup_pic(engine, vmap);
		//
		//	COMMENT: メインループ
		//
		std::size_t total_step = 0;
		for (; ; ) {
			std::size_t step = 0;
			if (!engine.is_code_end()) {
				//
				//	COMMENT: 実行ループ
				//
				std::cout
					<< "> execute:" << std::endl
					;
				while (!engine.is_code_end()) {
					//
					//	COMMENT: 実行ブレーク
					//
					if (break_mode && engine.at(engine.ppointer()) == '\n') {
						//
						//	COMMENT: 情報表示
						//
						std::cout
							<< std::endl
							<< "> break execute:" << std::endl
							<< "> step: " << step << std::endl
							;
						braincunt::app::print_info(engine);
						//
						//	COMMENT: ブレークコマンド入力
						//
						std::cout
							<< std::endl
							<< "> input break command { none }: "
							;
						for (; ; ) {
							std::string break_command;
							do {
								std::getline(std::cin, break_command);
							} while (break_command.empty());
							break;
						}
					}
					//
					//	COMMENT: 実行
					//
					try {
						engine.execute();
						++step;
					} catch (sprig::braincunt::base_exception& e) {
						std::cout
							<< std::endl
							<< "> " << e.what() << std::endl
							;
						break;
					}
				}
				//
				//	COMMENT: 実行完了後処理
				//
				total_step += step;
				std::cout
					<< std::endl
					<< "> end execute:" << std::endl
					;
				std::cout
					<< "> step: " << step << std::endl
					;
			}
			//
			//	COMMENT: 情報表示
			//
			std::cout
				<< "> total step: " << total_step << std::endl
				;
			braincunt::app::print_info(engine);
			//
			//	COMMENT: コマンド入力
			//
			std::cout
				<< std::endl
				<< "> input command { i=[input code] / p=[print code] / s=[save code]\n"
				<< "                  q=[quit] / r=[reset]\n"
				<< "                  sxml=[save xml] / stxt=[save text] / sbin=[save binary]\n"
				<< "                  pout=[pic output]\n"
				<< "                }: "
				;
			for (; ; ) {
				bool is_quit = false;
				std::string command;
				do {
					std::getline(std::cin, command);
				} while (command.empty());
				//
				//	COMMENT: コマンド処理
				//
				if (braincunt::app::process_command(engine, command, is_quit)) {
					if (is_quit) {
						//
						//	COMMENT: 終了処理
						//
						braincunt::picapp::shutdown_pic(engine, vmap);
						braincunt::app::shutdown_app();
						return;
					}
					if (engine.pbuffer().empty()) {
						//
						//	COMMENT: クリーンアップ処理
						//
						total_step = 0;
					}
					break;
				} else if (braincunt::picapp::process_pic_command(engine, command)) {
					break;
				}
			}
		}
	}
}	// namespace app

//
// main
//
int main(int argc, char* argv[]) {
	try {
		app::exec(argc, argv);
	} catch (std::exception& e) {
		std::cout
			<< std::endl
			<< "> exception handled:" << std::endl
			<< boost::diagnostic_information(e)
			;
	}
}
