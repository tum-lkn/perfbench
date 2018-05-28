#ifndef REPORTER_H
#define REPORTER_H 1

#include <string>
#include <boost/thread.hpp>

#include "signals.h"

class Counter;
/**
 * Reporter Class for live reporting (console) and file reporting
 * This report is printed/written second-wise (short report)
 */
class Reporter {
public:
	static int report_interval; /**< Interval for both live reporting and file reporting */
	static bool live_report_active; /**< Live reporting (console) active? */
	static bool file_report_active; /**< File reporting active? */
	static bool finished; /**< Reporting process ended? */
	static std::string filepath; /**< Path to file for reporting */
	static std::ofstream filestream;
	static boost::thread report_thread; /**< Thread where the report loop runs */

	/**
	 * Initialize the reporter
	 * @param no_live_report Set this to true to deactivate live/console reporting
	 * @param interval_report_file path to file
	 */
	static void init(bool no_live_report, std::string interval_report_file);
	static void start(); /**< Start the reporter */
	static void finish(); /**< Finish reporting */
	static void report_loop(); /**< Report loop */
	static void live_report(); /**< Do the live (console) reporting */
	static void file_report(); /**< Do the file reporting */
};

#endif
#pragma once