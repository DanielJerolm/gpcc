/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2019 Daniel Jerolm
*/

#ifndef CLICOLORS_HPP_201803282204
#define CLICOLORS_HPP_201803282204
/**
 * @ingroup GPCC_CLI
 * @defgroup GPC_CLI_FCS Font and color styles
 * \brief Definitions for controling font and color style in CLI output.
 *
 * # Overview
 * This group contains definitions that can be used to embed font and color style control information for the terminal
 * in CLI output. The following figure provides an example of the application of the definitions in this group:
 *
 * \htmlonly <style>div.image img[src="cli/font_styles_and_colors.jpg"]{width:35%;}</style> \endhtmlonly
 * \image html "cli/font_styles_and_colors.jpg" "Font styles applied to demo output"
 *
 * # Usage
 * The string fragments defined in this group are intended to be integrated into your CLI output:
 * ~~~{.cpp}
 * #include "gpcc/src/cli/CLIColors.hpp"
 *
 * // [...]
 *
 * myCLI.WriteLine("Red text: " CLI_RED "RED" CLI_STD "!");
 * ~~~
 *
 * __CLI_STD__ is special: It resets the terminal back to standard/default font, color, and style.
 *
 * When printing text to the terminal via [CLI::WriteLine(...)](@ref gpcc::cli::CLI::WriteLine), then class
 * [CLI](@ref gpcc::cli::CLI) guarantees, that a `CLI_STD` has been send to the terminal before the text is printed
 * to the terminal. Any text printed to the terminal that does not contain any font/color/style control information
 * will therefore be printed with terminal's default settings, regardless the settings that might have been applied
 * by any previous text written to the terminal.
 *
 * CLI_STD should also be used when switching the color multiple times within one chunk of text send to
 * [CLI::WriteLine(...)](@ref gpcc::cli::CLI::WriteLine):
 *
 * ~~~{.cpp}
 * myCLI.WriteLine("Red and green text: " CLI_RED "RED " CLI_STD CLI_GREEN "GREEN");
 * ~~~
 *
 * Otherwise some terminals may produce undefined/unexpected colors.
 *
 * # Unit tests
 * Font, color, and style control information should be disabled when building a unit test executable. Otherwise test
 * cases using a [CLI](@ref gpcc::cli::CLI) and a [FakeTerminal](@ref gpcc_tests::cli::FakeTerminal) could experience
 * difficulties when they have to deal with the control information instead of plain text only. The section below
 * describes how to disable font, color, and style control information.
 *
 * # Disabling font, color and style commands
 * The definitions in this group can be disabled by compiling GPCC with `-DGPCC_CLI_NO_FONT_STYLES`.\n
 * If compiled with this define, than the defines in this group will have no effect.
 *
 * It is recommended to compile GPCC with `-DGPCC_CLI_NO_FONT_STYLES` if either your terminal does not support
 * the font, color, and style control information, or if you are building a unit test executable.
 *
 * # Compatible terminals
 * The following terminals seem to be compatible with the defines in this group:
 * - KDE's "Konsole"
 * - PUTTY (configured fo VT100) via serial connection
 * @{
 */

#ifndef GPCC_CLI_NO_FONT_STYLES

  #define CLI_STD                "\033[0m"      ///<CLI Font Style: Terminal's standard/default color
  #define CLI_BOLD_STD           "\033[1;39m"   ///<CLI Font Style: Terminal's standard/default color, bold letters
  #define CLI_BLACK              "\033[30m"     ///<CLI Font Style: Black
  #define CLI_RED                "\033[31m"     ///<CLI Font Style: Red
  #define CLI_GREEN              "\033[32m"     ///<CLI Font Style: Green
  #define CLI_BROWN              "\033[33m"     ///<CLI Font Style: Brown
  #define CLI_BLUE               "\033[34m"     ///<CLI Font Style: Blue
  #define CLI_MAGENTA            "\033[35m"     ///<CLI Font Style: Magenta
  #define CLI_CYAN               "\033[36m"     ///<CLI Font Style: Cyan
  #define CLI_GREY               "\033[37m"     ///<CLI Font Style: Grey/White
  #define CLI_BOLD_DARK_GREY     "\033[1;30m"   ///<CLI Font Style: Bold + dark grey
  #define CLI_BOLD_LIGHT_RED     "\033[1;31m"   ///<CLI Font Style: Bold + light red
  #define CLI_BOLD_LIGHT_GREEN   "\033[1;32m"   ///<CLI Font Style: Bold + light green
  #define CLI_BOLD_YELLOW        "\033[1;33m"   ///<CLI Font Style: Bold + yellow
  #define CLI_BOLD_LIGHT_BLUE    "\033[1;34m"   ///<CLI Font Style: Bold + light blue
  #define CLI_BOLD_LIGHT_MAGENTA "\033[1;35m"   ///<CLI Font Style: Bold + light magenta
  #define CLI_BOLD_LIGHT_CYAN    "\033[1;36m"   ///<CLI Font Style: Bold + light cyan
  #define CLI_BOLD_WHITE         "\033[1;37m"   ///<CLI Font Style: Bold + white

#else

  #define CLI_STD                ""             ///<CLI Font Style: Terminal's standard/default color
  #define CLI_BOLD_STD           ""             ///<CLI Font Style: Terminal's standard/default color, bold letters
  #define CLI_BLACK              ""             ///<CLI Font Style: Black
  #define CLI_RED                ""             ///<CLI Font Style: Red
  #define CLI_GREEN              ""             ///<CLI Font Style: Green
  #define CLI_BROWN              ""             ///<CLI Font Style: Brown
  #define CLI_BLUE               ""             ///<CLI Font Style: Blue
  #define CLI_MAGENTA            ""             ///<CLI Font Style: Magenta
  #define CLI_CYAN               ""             ///<CLI Font Style: Cyan
  #define CLI_GREY               ""             ///<CLI Font Style: Grey/White
  #define CLI_BOLD_DARK_GREY     ""             ///<CLI Font Style: Bold + dark grey
  #define CLI_BOLD_LIGHT_RED     ""             ///<CLI Font Style: Bold + light red
  #define CLI_BOLD_LIGHT_GREEN   ""             ///<CLI Font Style: Bold + light green
  #define CLI_BOLD_YELLOW        ""             ///<CLI Font Style: Bold + yellow
  #define CLI_BOLD_LIGHT_BLUE    ""             ///<CLI Font Style: Bold + light blue
  #define CLI_BOLD_LIGHT_MAGENTA ""             ///<CLI Font Style: Bold + light magenta
  #define CLI_BOLD_LIGHT_CYAN    ""             ///<CLI Font Style: Bold + light cyan
  #define CLI_BOLD_WHITE         ""             ///<CLI Font Style: Bold + white

#endif

/** @} */

#endif // CLICOLORS_HPP_201803282204
