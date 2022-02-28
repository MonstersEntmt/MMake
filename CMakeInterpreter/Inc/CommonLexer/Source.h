#pragma once

#include <cstddef>

#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

namespace CommonLexer
{
	class ISource;

	struct SourcePoint
	{
	public:
		SourcePoint();
		SourcePoint(std::size_t index, std::size_t line, std::size_t column);
		SourcePoint(const SourcePoint&)     = default;
		SourcePoint(SourcePoint&&) noexcept = default;

		SourcePoint& operator=(const SourcePoint&) = default;
		SourcePoint& operator=(SourcePoint&&) noexcept = default;

	public:
		std::size_t m_Index, m_Line, m_Column;
	};

	struct SourceIterator
	{
	public:
		using value_type        = char;
		using difference_type   = std::size_t;
		using reference         = char&;
		using pointer           = char*;
		using iterator_category = std::bidirectional_iterator_tag;

		SourceIterator();
		SourceIterator(ISource* source, const SourcePoint& point);
		SourceIterator(ISource* source, SourcePoint&& point);
		SourceIterator(const SourceIterator& copy)     = default;
		SourceIterator(SourceIterator&& move) noexcept = default;

		SourceIterator& operator=(const SourceIterator& copy) = default;
		SourceIterator& operator=(SourceIterator&& move) noexcept = default;

		[[nodiscard]] operator SourcePoint() const { return m_Point; }

		char operator*() const;

		SourceIterator& operator++();
		SourceIterator  operator++(int);
		SourceIterator& operator--();
		SourceIterator  operator--(int);

		SourceIterator& operator+=(std::size_t count);
		SourceIterator& operator-=(std::size_t count);

		bool operator==(const SourceIterator& other) const;
		bool operator!=(const SourceIterator& other) const;
		bool operator<(const SourceIterator& other) const;
		bool operator<=(const SourceIterator& other) const;
		bool operator>(const SourceIterator& other) const;
		bool operator>=(const SourceIterator& other) const;

	private:
		void movePoint(std::size_t count, bool negative = false);

	private:
		ISource*    m_Source;
		SourcePoint m_Point;
		std::size_t m_CachedOffset;
		std::string m_Cached;
	};

	struct SourceSpan
	{
	public:
		SourceSpan();
		SourceSpan(const SourcePoint& begin, const SourcePoint& end);
		SourceSpan(SourcePoint&& begin, SourcePoint&& end);
		SourceSpan(const SourceSpan&)     = default;
		SourceSpan(SourceSpan&&) noexcept = default;

		SourceSpan& operator=(const SourceSpan&) = default;
		SourceSpan& operator=(SourceSpan&&) noexcept = default;

		SourceIterator begin(ISource* source) const;
		SourceIterator end(ISource* source) const;

		std::size_t length() const;

	public:
		SourcePoint m_Begin, m_End;
	};

	class ISource
	{
	public:
		[[nodiscard]] virtual std::size_t getSize()                                   = 0;
		[[nodiscard]] virtual std::size_t getNumLines()                               = 0;
		[[nodiscard]] virtual std::size_t getIndexFromLineNumber(std::size_t line)    = 0;
		[[nodiscard]] virtual std::size_t getLineNumberFromIndex(std::size_t index)   = 0;
		[[nodiscard]] virtual std::size_t getColumnNumberFromIndex(std::size_t index) = 0;

		[[nodiscard]] SourceSpan getCompleteSpan();

		[[nodiscard]] std::string getSpan(std::size_t index, std::size_t length)
		{
			std::size_t end = index + length;
			return getSpan({ { index, getLineNumberFromIndex(index), getColumnNumberFromIndex(index) },
			                 { end, getLineNumberFromIndex(end), getColumnNumberFromIndex(end) } });
		}
		[[nodiscard]] virtual std::string getSpan(const SourceSpan& span) = 0;

		[[nodiscard]] std::string getLine(std::size_t line)
		{
			return getLine({ getIndexFromLineNumber(line), line, 1 });
		}
		[[nodiscard]] virtual std::string              getLine(const SourcePoint& point)                  = 0;
		[[nodiscard]] virtual std::vector<std::string> getLines(std::size_t startLine, std::size_t lines) = 0;
	};

	class StringSource : public ISource
	{
	public:
		explicit StringSource(const std::string& str);
		explicit StringSource(std::string&& str);

		std::size_t              getSize() override;
		std::size_t              getNumLines() override;
		std::size_t              getIndexFromLineNumber(std::size_t line) override;
		std::size_t              getLineNumberFromIndex(std::size_t index) override;
		std::size_t              getColumnNumberFromIndex(std::size_t index) override;
		std::string              getSpan(const SourceSpan& span) override;
		std::string              getLine(const SourcePoint& point) override;
		std::vector<std::string> getLines(std::size_t startLine, std::size_t lines) override;

	private:
		void setupLineToIndex();

	private:
		std::string              m_Str;
		std::vector<std::size_t> m_LineToIndex;
	};

	class FileSource : public ISource
	{
	public:
		FileSource(const std::filesystem::path& filepath);
		~FileSource();

		[[nodiscard]] auto& getFilePath() const { return m_Filepath; }
		[[nodiscard]] auto& getStream() const { return m_Stream; }

		std::size_t              getSize() override;
		std::size_t              getNumLines() override;
		std::size_t              getIndexFromLineNumber(std::size_t line) override;
		std::size_t              getLineNumberFromIndex(std::size_t index) override;
		std::size_t              getColumnNumberFromIndex(std::size_t index) override;
		std::string              getSpan(const SourceSpan& span) override;
		std::string              getLine(const SourcePoint& point) override;
		std::vector<std::string> getLines(std::size_t startLine, std::size_t lines) override;

	private:
		void setupLineToIndex();

	private:
		std::filesystem::path    m_Filepath;
		std::ifstream            m_Stream;
		std::size_t              m_Size;
		std::vector<std::size_t> m_LineToIndex;
	};
} // namespace CommonLexer