/**
 * @file crash_analyzer.cpp
 */

#include "crash_analyzer.hpp"

#include "foundation/hash.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>

#ifdef _WIN32
#include <process.h>
#define crash_popen _popen
#define crash_pclose _pclose
#else
#define crash_popen popen
#define crash_pclose pclose
#endif

namespace scope::workspace
{

namespace
{

constexpr const char* kPstackAnalyzerVersion = "pstack-v2";
constexpr const char* kCoreAnalyzerVersion = "core-gdb-v1";

std::string trim(const std::string& value)
{
    std::size_t start = 0U;

    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start])) != 0)
    {
        ++start;
    }

    std::size_t end = value.size();

    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1U])) != 0)
    {
        --end;
    }

    return value.substr(start, end - start);
}

std::optional<std::string> extractSignal(const std::string& line)
{
    static const std::regex pattern(R"(Program received signal (SIG\w+))");
    std::smatch match;

    if (std::regex_search(line, match, pattern) && match.size() > 1U)
    {
        return match[1].str();
    }

    return std::nullopt;
}

std::optional<std::string> extractThreadHeader(const std::string& line, std::string& threadName)
{
    static const std::regex pattern(R"(^Thread\s+(\d+)(.*)$)");
    std::smatch match;

    if (!std::regex_match(line, match, pattern) || match.size() < 2U)
    {
        return std::nullopt;
    }

    threadName = "Thread " + match[1].str();

    if (match.size() > 2U)
    {
        const std::string suffix = trim(match[2].str());

        if (!suffix.empty() && suffix != ":")
        {
            threadName += ' ' + suffix;
        }
    }

    return match[1].str();
}

std::optional<std::size_t> extractFrameIndex(const std::string& line)
{
    if (line.empty() || line[0] != '#')
    {
        return std::nullopt;
    }

    std::size_t position = 1U;

    while (position < line.size() && std::isspace(static_cast<unsigned char>(line[position])) != 0)
    {
        ++position;
    }

    std::size_t end = position;

    while (end < line.size() && std::isdigit(static_cast<unsigned char>(line[end])) != 0)
    {
        ++end;
    }

    if (end == position)
    {
        return std::nullopt;
    }

    return static_cast<std::size_t>(std::stoul(line.substr(position, end - position)));
}

std::optional<std::string> extractHexAddress(const std::string& line, std::size_t& position)
{
    while (position < line.size() && std::isspace(static_cast<unsigned char>(line[position])) != 0)
    {
        ++position;
    }

    if (position + 2U >= line.size() || line[position] != '0' ||
        (line[position + 1U] != 'x' && line[position + 1U] != 'X'))
    {
        return std::nullopt;
    }

    std::size_t end = position + 2U;

    while (end < line.size() &&
           (std::isxdigit(static_cast<unsigned char>(line[end])) != 0 || line[end] == '`'))
    {
        ++end;
    }

    if (end == position + 2U)
    {
        return std::nullopt;
    }

    const std::string address = line.substr(position, end - position);
    position = end;

    return address;
}

std::optional<CrashFrame> parseFrameLine(const std::string& line)
{
    const auto frameIndex = extractFrameIndex(line);

    if (!frameIndex.has_value())
    {
        return std::nullopt;
    }

    std::size_t position = line.find('#') + 1U;

    while (position < line.size() && std::isspace(static_cast<unsigned char>(line[position])) != 0)
    {
        ++position;
    }

    while (position < line.size() && std::isdigit(static_cast<unsigned char>(line[position])) != 0)
    {
        ++position;
    }

    const auto address = extractHexAddress(line, position);

    if (!address.has_value())
    {
        return std::nullopt;
    }

    const std::size_t inPosition = line.find(" in ", position);

    if (inPosition == std::string::npos)
    {
        return std::nullopt;
    }

    CrashFrame frame;
    frame.index = *frameIndex;
    frame.address = *address;

    std::string remainder = trim(line.substr(inPosition + 4U));
    const std::size_t atPosition = remainder.find(" at ");

    if (atPosition != std::string::npos)
    {
        frame.location = trim(remainder.substr(atPosition + 4U));
        remainder = trim(remainder.substr(0U, atPosition));
    }

    const std::size_t fromPosition = remainder.find(" from ");

    if (fromPosition != std::string::npos)
    {
        frame.module = trim(remainder.substr(fromPosition + 6U));
        remainder = trim(remainder.substr(0U, fromPosition));
    }

    const std::size_t parenPosition = remainder.find('(');

    if (parenPosition != std::string::npos)
    {
        frame.symbol = trim(remainder.substr(0U, parenPosition));
    }
    else
    {
        frame.symbol = trim(remainder);
    }

    if (frame.symbol.empty())
    {
        frame.symbol = "<unknown>";
    }

    return frame;
}

std::optional<CrashFrame> parseSignalHandlerMarker(const std::string& line)
{
    if (line.find("<signal handler called>") == std::string::npos)
    {
        return std::nullopt;
    }

    const auto frameIndex = extractFrameIndex(line);

    if (!frameIndex.has_value())
    {
        return std::nullopt;
    }

    CrashFrame frame;
    frame.index = *frameIndex;
    frame.address = "0x0";
    frame.symbol = "<signal handler called>";

    return frame;
}

std::optional<std::string> extractSwitchingThreadId(const std::string& line)
{
    static const std::regex pattern(R"(\[Switching to thread (\d+))", std::regex::icase);
    std::smatch match;

    if (std::regex_search(line, match, pattern) && match.size() > 1U)
    {
        return match[1].str();
    }

    return std::nullopt;
}

std::string toLowerCopy(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char character) { return static_cast<char>(std::tolower(character)); });

    return value;
}

bool symbolContains(const std::string& symbol, const std::string& needle)
{
    return toLowerCopy(symbol).find(toLowerCopy(needle)) != std::string::npos;
}

bool isSignalHandlerBoundaryFrame(const CrashFrame& frame)
{
    if (frame.symbol == "<signal handler called>")
    {
        return true;
    }

    return symbolContains(frame.symbol, "__restore_rt") ||
           symbolContains(frame.symbol, "__kernel_rt_sigreturn");
}

bool isIdleWaitSymbol(const std::string& symbol)
{
    static constexpr std::array<const char*, 8> kIdleSymbols = {
        "epoll_wait",
        "poll",
        "pthread_cond_wait",
        "futex",
        "nanosleep",
        "select",
        "sched_yield",
        "__poll",
    };

    for (const char* idleSymbol : kIdleSymbols)
    {
        if (symbolContains(symbol, idleSymbol))
        {
            return true;
        }
    }

    const std::string lowered = toLowerCopy(symbol);

    return lowered.rfind("gpr_", 0) == 0U || lowered.rfind("grpc_", 0) == 0U ||
           symbolContains(symbol, "grpc_core::");
}

bool isSignalAbortSymbol(const std::string& symbol)
{
    static constexpr std::array<const char*, 8> kAbortSymbols = {
        "raise",
        "abort",
        "gsignal",
        "__verbose_terminate_handler",
        "__cxxabiv1::__terminate",
        "std::terminate",
        "__terminate",
        "sighdl",
    };

    for (const char* abortSymbol : kAbortSymbols)
    {
        if (symbolContains(symbol, abortSymbol))
        {
            return true;
        }
    }

    return false;
}

bool isSystemRuntimeModule(const std::optional<std::string>& module)
{
    if (!module.has_value())
    {
        return false;
    }

    const std::string lowered = toLowerCopy(*module);

    return lowered.find("libc") != std::string::npos ||
           lowered.find("libpthread") != std::string::npos ||
           lowered.find("libstdc++") != std::string::npos ||
           lowered.find("libgcc") != std::string::npos ||
           lowered.find("ld-linux") != std::string::npos;
}

const CrashFrame* findApplicationFrame(const CrashThread& thread)
{
    std::size_t startIndex = 0U;

    for (std::size_t index = 0U; index < thread.frames.size(); ++index)
    {
        if (isSignalHandlerBoundaryFrame(thread.frames[index]))
        {
            startIndex = index + 1U;
            break;
        }
    }

    for (std::size_t index = startIndex; index < thread.frames.size(); ++index)
    {
        const CrashFrame& frame = thread.frames[index];

        if (isSignalHandlerBoundaryFrame(frame) || isSignalAbortSymbol(frame.symbol))
        {
            continue;
        }

        if (!isSystemRuntimeModule(frame.module) && !isIdleWaitSymbol(frame.symbol))
        {
            return &frame;
        }
    }

    return nullptr;
}

int scoreThreadForFault(const CrashThread& thread)
{
    if (thread.frames.empty())
    {
        return -1000;
    }

    int score = 0;
    bool hasAbortChain = false;
    bool hasApplicationFrame = false;

    for (const CrashFrame& frame : thread.frames)
    {
        if (isSignalAbortSymbol(frame.symbol))
        {
            hasAbortChain = true;
        }

        if (!isSystemRuntimeModule(frame.module) && !isIdleWaitSymbol(frame.symbol))
        {
            hasApplicationFrame = true;
        }
    }

    if (hasAbortChain)
    {
        score += 100;
    }

    if (hasApplicationFrame)
    {
        score += 30;
    }

    if (isIdleWaitSymbol(thread.frames.front().symbol))
    {
        score -= 80;
    }

    return score;
}

std::optional<std::string> selectFaultThreadId(const std::vector<CrashThread>& threads,
                                               const std::optional<std::string>& switchingThreadId)
{
    if (switchingThreadId.has_value())
    {
        return switchingThreadId;
    }

    if (threads.empty())
    {
        return std::nullopt;
    }

    const CrashThread* best = &threads.front();
    int bestScore = scoreThreadForFault(*best);

    for (const CrashThread& thread : threads)
    {
        const int score = scoreThreadForFault(thread);

        if (score > bestScore)
        {
            bestScore = score;
            best = &thread;
        }
    }

    if (bestScore <= 0)
    {
        return std::nullopt;
    }

    return best->id;
}

const CrashFrame* prominentFaultFrame(const CrashThread& thread)
{
    if (const CrashFrame* applicationFrame = findApplicationFrame(thread); applicationFrame != nullptr)
    {
        return applicationFrame;
    }

    return thread.frames.empty() ? nullptr : &thread.frames.front();
}

std::optional<std::string> extractTidThreadHeader(const std::string& line, std::string& threadName)
{
    static const std::regex pattern(R"(^TID\s+(\d+):\s*$)");
    std::smatch match;

    if (!std::regex_match(line, match, pattern) || match.size() < 2U)
    {
        return std::nullopt;
    }

    threadName = "TID " + match[1].str();
    return match[1].str();
}

std::optional<CrashFrame> parseTidFrameLine(const std::string& line)
{
    const auto frameIndex = extractFrameIndex(line);

    if (!frameIndex.has_value())
    {
        return std::nullopt;
    }

    std::size_t position = line.find('#') + 1U;

    while (position < line.size() && std::isspace(static_cast<unsigned char>(line[position])) != 0)
    {
        ++position;
    }

    while (position < line.size() && std::isdigit(static_cast<unsigned char>(line[position])) != 0)
    {
        ++position;
    }

    const auto address = extractHexAddress(line, position);

    if (!address.has_value())
    {
        return std::nullopt;
    }

    std::string remainder = trim(line.substr(position));

    static const std::regex variantPrefix(R"(^-\s*\d+\s+)");
    remainder = std::regex_replace(remainder, variantPrefix, std::string{});
    remainder = trim(remainder);

    const std::size_t moduleSeparator = remainder.rfind(" - /");

    if (moduleSeparator == std::string::npos)
    {
        return std::nullopt;
    }

    CrashFrame frame;
    frame.index = *frameIndex;
    frame.address = *address;
    frame.symbol = trim(remainder.substr(0U, moduleSeparator));
    frame.module = trim(remainder.substr(moduleSeparator + 3U));

    if (frame.symbol.empty())
    {
        frame.symbol = "<unknown>";
    }

    return frame;
}

bool isTidSourceLocationLine(const std::string& line)
{
    if (line.empty() || (line.front() != ' ' && line.front() != '\t'))
    {
        return false;
    }

    const std::string trimmed = trim(line);

    return !trimmed.empty() && trimmed.front() == '/';
}

enum class PstackDialect
{
    Gdb,
    Tid,
};

PstackDialect detectPstackDialect(const std::string& content)
{
    static const std::regex tidHeader(R"(^TID\s+\d+:\s*$)");

    std::istringstream stream(content);
    std::string line;

    while (std::getline(stream, line))
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }

        const std::string trimmed = trim(line);

        if (trimmed.empty())
        {
            continue;
        }

        if (std::regex_match(trimmed, tidHeader))
        {
            return PstackDialect::Tid;
        }

        if (trimmed.rfind("Thread ", 0) == 0)
        {
            return PstackDialect::Gdb;
        }
    }

    return PstackDialect::Gdb;
}

void parseGdbPstackContent(const std::string& content, CrashReport& report,
                           std::optional<std::string>& switchingThreadId)
{
    CrashThread* currentThread = nullptr;

    std::istringstream stream(content);
    std::string line;

    while (std::getline(stream, line))
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }

        const std::string trimmed = trim(line);

        if (trimmed.empty())
        {
            continue;
        }

        if (const auto signal = extractSignal(trimmed); signal.has_value())
        {
            report.signal = signal;
            continue;
        }

        if (const auto threadId = extractSwitchingThreadId(trimmed); threadId.has_value())
        {
            switchingThreadId = threadId;
            continue;
        }

        std::string threadName;

        if (const auto threadId = extractThreadHeader(trimmed, threadName); threadId.has_value())
        {
            CrashThread thread;
            thread.id = *threadId;
            thread.name = threadName;
            report.threads.push_back(std::move(thread));
            currentThread = &report.threads.back();
            continue;
        }

        if (currentThread == nullptr)
        {
            continue;
        }

        if (const auto frame = parseSignalHandlerMarker(trimmed); frame.has_value())
        {
            currentThread->frames.push_back(*frame);
            continue;
        }

        if (const auto frame = parseFrameLine(trimmed); frame.has_value())
        {
            currentThread->frames.push_back(*frame);
        }
    }
}

void parseTidPstackContent(const std::string& content, CrashReport& report)
{
    CrashThread* currentThread = nullptr;

    std::istringstream stream(content);
    std::string line;

    while (std::getline(stream, line))
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }

        const std::string trimmed = trim(line);

        if (trimmed.empty())
        {
            continue;
        }

        std::string threadName;

        if (const auto threadId = extractTidThreadHeader(trimmed, threadName); threadId.has_value())
        {
            CrashThread thread;
            thread.id = *threadId;
            thread.name = threadName;
            report.threads.push_back(std::move(thread));
            currentThread = &report.threads.back();
            continue;
        }

        if (currentThread == nullptr)
        {
            continue;
        }

        if (isTidSourceLocationLine(line))
        {
            if (!currentThread->frames.empty())
            {
                currentThread->frames.back().location = trimmed;
            }

            continue;
        }

        if (const auto frame = parseTidFrameLine(trimmed); frame.has_value())
        {
            currentThread->frames.push_back(*frame);
        }
    }
}

void finalizeParsedPstackReport(CrashReport& report, const std::optional<std::string>& switchingThreadId)
{
    if (report.threads.empty())
    {
        report.status = CrashAnalysisStatus::Failed;
        report.summary = "No threads parsed from pstack";
        report.warnings.push_back("Pstack format not recognized or file is empty.");

        return;
    }

    report.faultThreadId = selectFaultThreadId(report.threads, switchingThreadId);

    for (CrashThread& thread : report.threads)
    {
        thread.isFaultThread =
            report.faultThreadId.has_value() && thread.id == *report.faultThreadId;
    }

    const CrashThread* faultThread = nullptr;

    for (const CrashThread& thread : report.threads)
    {
        if (thread.isFaultThread)
        {
            faultThread = &thread;
            break;
        }
    }

    if (faultThread != nullptr)
    {
        if (const CrashFrame* prominentFrame = prominentFaultFrame(*faultThread);
            prominentFrame != nullptr)
        {
            std::ostringstream observation;
            observation << "Fault in " << prominentFrame->symbol;

            if (prominentFrame->location.has_value())
            {
                observation << " at " << *prominentFrame->location;
            }

            report.observations.push_back(observation.str());
        }
    }
    else if (report.faultThreadId.has_value() == false)
    {
        report.observations.push_back(
            "No fault thread identified (no abort chain or signal handler found)");
    }

    if (report.signal.has_value())
    {
        report.observations.push_back("Signal " + *report.signal + " received");
    }

    if (faultThread != nullptr)
    {
        if (const CrashFrame* prominentFrame = prominentFaultFrame(*faultThread);
            prominentFrame != nullptr)
        {
            std::ostringstream summary;
            summary << faultThread->name;

            if (report.signal.has_value())
            {
                summary << " received " << *report.signal;
            }

            summary << " in " << prominentFrame->symbol;
            report.summary = summary.str();
        }
        else
        {
            report.summary = faultThread->name;
        }
    }
    else
    {
        report.summary = "Pstack parsed with " + std::to_string(report.threads.size()) + " thread(s)";
    }

    report.status = CrashAnalysisStatus::Ready;
    report.metadata["analyzer"] = kPstackAnalyzerVersion;
    report.metadata["threadCount"] = std::to_string(report.threads.size());
}

CrashReport makeBaseReport(const ArtifactRecord& artifact, const CrashAnalysisContext& context,
                           const std::string& analyzerVersion)
{
    CrashReport report;
    report.id = makeCrashReportId(context.investigationId, artifact.id, analyzerVersion);
    report.artifactId = artifact.id;
    report.artifactType = artifact.type;

    return report;
}

std::string runCommandCaptureOutput(const std::string& command)
{
    std::string output;
    FILE* pipe = crash_popen(command.c_str(), "r");

    if (pipe == nullptr)
    {
        return output;
    }

    std::array<char, 4096> buffer{};

    while (std::fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr)
    {
        output.append(buffer.data());
    }

    (void)crash_pclose(pipe);

    return output;
}

bool isGdbAvailable()
{
    const std::string output = runCommandCaptureOutput("gdb --version 2>&1");

    return output.find("GNU gdb") != std::string::npos || output.find("gdb") != std::string::npos;
}

std::string quoteShellPath(const std::string& path)
{
#ifdef _WIN32
    return '"' + path + '"';
#else
    std::string quoted = "'";
    quoted.reserve(path.size() + 2U);

    for (const char character : path)
    {
        if (character == '\'')
        {
            quoted += "'\\''";
        }
        else
        {
            quoted.push_back(character);
        }
    }

    quoted.push_back('\'');
    return quoted;
#endif
}

void parseGdbBacktrace(const std::string& gdbOutput, CrashReport& report)
{
    CrashThread* currentThread = nullptr;
    std::size_t threadSequence = 0U;

    std::istringstream stream(gdbOutput);
    std::string line;

    while (std::getline(stream, line))
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }

        const std::string trimmed = trim(line);

        if (trimmed.empty())
        {
            continue;
        }

        if (trimmed.rfind("Thread ", 0) == 0)
        {
            CrashThread thread;
            thread.id = std::to_string(++threadSequence);
            thread.name = trimmed;
            report.threads.push_back(std::move(thread));
            currentThread = &report.threads.back();
            continue;
        }

        if (currentThread == nullptr)
        {
            continue;
        }

        if (const auto frame = parseFrameLine(trimmed); frame.has_value())
        {
            currentThread->frames.push_back(*frame);
        }
    }
}

class PstackCrashAnalyzer final : public IArtifactCrashAnalyzer
{
  public:
    [[nodiscard]] std::string_view artifactType() const noexcept override
    {
        return "pstack";
    }

    [[nodiscard]] bool supports(const ArtifactRecord& artifact) const override
    {
        return artifact.type == "pstack";
    }

    [[nodiscard]] bool canAnalyze(const ArtifactRecord& artifact, const foundation::Path& dataPath) const override
    {
        (void)artifact;

        std::error_code errorCode;

        return std::filesystem::is_regular_file(dataPath.string(), errorCode) && !errorCode;
    }

    [[nodiscard]] foundation::Result<CrashReport> analyze(const ArtifactRecord& artifact,
                                                            const foundation::Path& dataPath,
                                                            const CrashAnalysisContext& context) const override
    {
        CrashReport report = makeBaseReport(artifact, context, kPstackAnalyzerVersion);

        std::ifstream stream(dataPath.string());

        if (!stream)
        {
            report.status = CrashAnalysisStatus::Failed;
            report.summary = "Pstack file could not be read";
            report.warnings.push_back("Failed to open pstack artifact data file.");

            return foundation::Result<CrashReport>(std::move(report));
        }

        std::ostringstream content;
        content << stream.rdbuf();
        const std::string fileContent = content.str();

        std::optional<std::string> switchingThreadId;

        if (detectPstackDialect(fileContent) == PstackDialect::Tid)
        {
            parseTidPstackContent(fileContent, report);
            report.metadata["pstackDialect"] = "tid";
        }
        else
        {
            parseGdbPstackContent(fileContent, report, switchingThreadId);
            report.metadata["pstackDialect"] = "gdb";
        }

        finalizeParsedPstackReport(report, switchingThreadId);

        return foundation::Result<CrashReport>(std::move(report));
    }
};

class CoreCrashAnalyzer final : public IArtifactCrashAnalyzer
{
  public:
    [[nodiscard]] std::string_view artifactType() const noexcept override
    {
        return "core";
    }

    [[nodiscard]] bool supports(const ArtifactRecord& artifact) const override
    {
        return artifact.type == "core";
    }

    [[nodiscard]] bool canAnalyze(const ArtifactRecord& artifact, const foundation::Path& dataPath) const override
    {
        (void)artifact;

        std::error_code errorCode;

        return std::filesystem::is_regular_file(dataPath.string(), errorCode) && !errorCode;
    }

    [[nodiscard]] foundation::Result<CrashReport> analyze(const ArtifactRecord& artifact,
                                                            const foundation::Path& dataPath,
                                                            const CrashAnalysisContext& context) const override
    {
        CrashReport report = makeBaseReport(artifact, context, kCoreAnalyzerVersion);

        if (!isGdbAvailable())
        {
            report.status = CrashAnalysisStatus::Unavailable;
            report.summary = "Core dump analysis unavailable";
            report.warnings.push_back("GDB not installed");
            report.metadata["analyzer"] = kCoreAnalyzerVersion;

            return foundation::Result<CrashReport>(std::move(report));
        }

        const std::string corePath = quoteShellPath(dataPath.string());
        const std::string command = "gdb --batch -ex \"thread apply all bt\" -ex quit -c " + corePath + " 2>&1";
        const std::string gdbOutput = runCommandCaptureOutput(command);

        if (gdbOutput.empty())
        {
            report.status = CrashAnalysisStatus::Failed;
            report.summary = "GDB produced no output";
            report.warnings.push_back("GDB command failed or core file is unreadable.");

            return foundation::Result<CrashReport>(std::move(report));
        }

        parseGdbBacktrace(gdbOutput, report);

        if (report.threads.empty())
        {
            report.status = CrashAnalysisStatus::Failed;
            report.summary = "No stack frames parsed from core dump";
            report.warnings.push_back("Debug symbols unavailable or core format not recognized.");
            report.observations.push_back("GDB ran but no thread backtraces were parsed.");

            return foundation::Result<CrashReport>(std::move(report));
        }

        if (!report.threads.empty())
        {
            report.threads.front().isFaultThread = true;
            report.faultThreadId = report.threads.front().id;
        }

        const CrashThread& faultThread = report.threads.front();

        if (!faultThread.frames.empty())
        {
            report.summary = faultThread.name + " backtrace from core dump";

            if (faultThread.frames.front().symbol == "<unknown>")
            {
                report.observations.push_back("Debug symbols unavailable");
            }
            else
            {
                report.observations.push_back("Top frame: " + faultThread.frames.front().symbol);
            }
        }
        else
        {
            report.summary = "Core dump analyzed with limited frame data";
        }

        const bool hasUnknownSymbols =
            std::any_of(report.threads.begin(), report.threads.end(), [](const CrashThread& thread) {
                return std::any_of(thread.frames.begin(), thread.frames.end(),
                                   [](const CrashFrame& frame) { return frame.symbol == "<unknown>"; });
            });

        report.status = CrashAnalysisStatus::Ready;

        if (hasUnknownSymbols)
        {
            report.warnings.push_back("Some frames lack symbol information.");
        }

        report.metadata["analyzer"] = kCoreAnalyzerVersion;
        report.metadata["threadCount"] = std::to_string(report.threads.size());

        return foundation::Result<CrashReport>(std::move(report));
    }
};

const PstackCrashAnalyzer kPstackAnalyzer;
const CoreCrashAnalyzer kCoreAnalyzer;

} // namespace

const IArtifactCrashAnalyzer* findCrashAnalyzer(const std::string_view type) noexcept
{
    if (type == "pstack")
    {
        return &kPstackAnalyzer;
    }

    if (type == "core")
    {
        return &kCoreAnalyzer;
    }

    return nullptr;
}

bool isCrashAnalyzableArtifactType(const std::string_view type) noexcept
{
    return findCrashAnalyzer(type) != nullptr;
}

} // namespace scope::workspace
