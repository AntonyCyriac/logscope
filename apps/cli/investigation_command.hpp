/**
 * @file investigation_command.hpp
 * @brief Investigation container CLI subcommands (v2.3.0).
 */

#pragma once

#include <iosfwd>

#include "cli_parser.hpp"

namespace scope::cli
{

void printInvestigationCreateUsage(std::ostream& output);

void printInvestigationAddUsage(std::ostream& output);

void printInvestigationAddNoteUsage(std::ostream& output);

void printInvestigationListUsage(std::ostream& output);

void printInvestigationShowUsage(std::ostream& output);

void printInvestigationOpenUsage(std::ostream& output);

void printInvestigationTimelineUsage(std::ostream& output);

void printInvestigationCrashUsage(std::ostream& output);

void printInvestigationLinksUsage(std::ostream& output);

[[nodiscard]] int runInvestigationCreateCommand(const InvestigationCreateOptions& options,
                                                std::ostream& output,
                                                std::ostream& errorOutput);

[[nodiscard]] int runInvestigationAddCommand(const InvestigationAddOptions& options,
                                             std::ostream& output,
                                             std::ostream& errorOutput);

[[nodiscard]] int runInvestigationAddNoteCommand(const InvestigationAddNoteOptions& options,
                                                 std::ostream& output,
                                                 std::ostream& errorOutput);

[[nodiscard]] int runInvestigationListCommand(const InvestigationListOptions& options,
                                              std::ostream& output,
                                              std::ostream& errorOutput);

[[nodiscard]] int runInvestigationShowCommand(const InvestigationShowOptions& options,
                                              std::ostream& output,
                                              std::ostream& errorOutput);

[[nodiscard]] int runInvestigationOpenCommand(const InvestigationOpenOptions& options,
                                              std::ostream& output,
                                              std::ostream& errorOutput);

[[nodiscard]] int runInvestigationTimelineCommand(const InvestigationTimelineOptions& options,
                                                  std::ostream& output,
                                                  std::ostream& errorOutput);

[[nodiscard]] int runInvestigationCrashCommand(const InvestigationCrashOptions& options,
                                               std::ostream& output,
                                               std::ostream& errorOutput);

[[nodiscard]] int runInvestigationLinksCommand(const InvestigationLinksOptions& options,
                                               std::ostream& output,
                                               std::ostream& errorOutput);

} // namespace scope::cli
