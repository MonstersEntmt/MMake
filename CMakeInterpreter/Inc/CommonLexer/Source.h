#pragma once

#include <cstddef>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace CommonLexer {
	struct SourcePoint {
	public:
		SourcePoint();
		SourcePoint(std::size_t index, std::size_t line, std::size_t column);
		SourcePoint(const SourcePoint&) = default;
		SourcePoint(SourcePoint&&)      = default;
		SourcePoint& operator=(const SourcePoint&) = default;
		SourcePoint& operator=(SourcePoint&&) = default;

	public:
		std::size_t m_Index, m_Line, m_Column;
	};

	struct SourceSpan {
	public:
		SourceSpan();
		SourceSpan(const SourcePoint& begin, const SourcePoint& end);
		SourceSpan(SourcePoint&& begin, SourcePoint&& end);
		SourceSpan(const SourceSpan&) = default;
		SourceSpan(SourceSpan&&)      = default;
		SourceSpan& operator=(const SourceSpan&) = default;
		SourceSpan& operator=(SourceSpan&&) = default;

	public:
		SourcePoint m_Begin, m_End;
	};

	class ISource {
	public:
		virtual std::size_t getSize()     = 0;
		virtual std::size_t getNumLines() = 0;

		virtual std::size_t getIndexFromLineNumber(std::size_t line)    = 0;
		virtual std::size_t getLineNumberFromIndex(std::size_t index)   = 0;
		virtual std::size_t getColumnNumberFromIndex(std::size_t index) = 0;

		std::string getSpan(std::size_t index, std::size_t length) {
			std::size_t end = index + length;
			return getSpan({ { index, getLineNumberFromIndex(index), getColumnNumberFromIndex(index) }, { end, getLineNumberFromIndex(end), getColumnNumberFromIndex(end) } });
		}
		virtual std::string getSpan(const SourceSpan& span) = 0;

		std::string getLine(std::size_t line) {
			return getLine({ getIndexFromLineNumber(line), line, 1 });
		}
		virtual std::string getLine(const SourcePoint& point) = 0;

		virtual std::vector<std::string> getLines(std::size_t startLine, std::size_t lines) = 0;
	};

	class StringSource : public ISource {
	public:
		StringSource(const std::string& str);
		StringSource(std::string&& str);

		virtual std::size_t getSize() override;
		virtual std::size_t getNumLines() override;

		virtual std::size_t getIndexFromLineNumber(std::size_t line) override;
		virtual std::size_t getLineNumberFromIndex(std::size_t index) override;
		virtual std::size_t getColumnNumberFromIndex(std::size_t index) override;

		virtual std::string getSpan(const SourceSpan& span) override;

		virtual std::string getLine(const SourcePoint& point) override;

		virtual std::vector<std::string> getLines(std::size_t startLine, std::size_t lines) override;

	private:
		void setupLineToIndex();

	private:
		std::string m_Str;
		std::vector<std::size_t> m_LineToIndex;
	};

	class FileSource : public ISource {
	public:
		FileSource(const std::filesystem::path& filepath);
		~FileSource();

		virtual std::size_t getSize() override;
		virtual std::size_t getNumLines() override;

		virtual std::size_t getIndexFromLineNumber(std::size_t line) override;
		virtual std::size_t getLineNumberFromIndex(std::size_t index) override;
		virtual std::size_t getColumnNumberFromIndex(std::size_t index) override;

		virtual std::string getSpan(const SourceSpan& span) override;

		virtual std::string getLine(const SourcePoint& point) override;

		virtual std::vector<std::string> getLines(std::size_t startLine, std::size_t lines) override;

	private:
		std::filesystem::path m_Filepath;
		std::ifstream m_Stream;
		std::size_t m_Size;
		std::vector<std::size_t> m_LineToIndex;
	};
} // namespace CommonLexer