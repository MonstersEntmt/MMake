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
		std::size_t getLine(ISource* source) const;
		std::size_t getColumn(ISource* source) const;

	public:
		std::size_t m_Index = 0;
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
		SourceIterator(ISource* source, SourcePoint point);

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

		[[nodiscard]] virtual std::string getSpan(std::size_t index, std::size_t length) = 0;
		[[nodiscard]] std::string         getSpan(SourceSpan span)
		{
			return getSpan(span.m_Begin.m_Index, span.m_End.m_Index - span.m_Begin.m_Index);
		}

		[[nodiscard]] virtual std::string getLine(std::size_t line) = 0;
		[[nodiscard]] virtual std::string getLine(SourcePoint point)
		{
			return getLine(getLineNumberFromIndex(point.m_Index));
		}
		[[nodiscard]] virtual std::vector<std::string> getLines(std::size_t startLine, std::size_t lines) = 0;
		[[nodiscard]] std::vector<std::string>         getLines(SourcePoint start, SourcePoint end)
		{
			auto startLine = getLineNumberFromIndex(start.m_Index);
			auto endLine   = getLineNumberFromIndex(end.m_Index);
			return getLines(startLine, (endLine + 1) - startLine);
		}
		[[nodiscard]] std::vector<std::string> getLines(SourceSpan span)
		{
			auto startLine = getLineNumberFromIndex(span.m_Begin.m_Index);
			auto endLine   = getLineNumberFromIndex(span.m_End.m_Index);
			return getLines(startLine, (endLine + 1) - startLine);
		}
	};

	class StringSource : public ISource
	{
	public:
		explicit StringSource(const std::string& str);
		explicit StringSource(std::string&& str);

		virtual std::size_t              getSize() override;
		virtual std::size_t              getNumLines() override;
		virtual std::size_t              getIndexFromLineNumber(std::size_t line) override;
		virtual std::size_t              getLineNumberFromIndex(std::size_t index) override;
		virtual std::size_t              getColumnNumberFromIndex(std::size_t index) override;
		virtual std::string              getSpan(std::size_t index, std::size_t length) override;
		virtual std::string              getLine(std::size_t line) override;
		virtual std::vector<std::string> getLines(std::size_t startLine, std::size_t lines) override;

	private:
		void setupLineToIndex();

	private:
		std::string              m_Str;
		std::vector<std::size_t> m_LineToIndex;
	};
} // namespace CommonLexer