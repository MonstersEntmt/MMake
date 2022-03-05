#include "CommonLexer/Source.h"

#include <string>
#include <utility>
#include <vector>

namespace CommonLexer
{
	std::size_t SourcePoint::getLine(ISource* source) const
	{
		return source->getLineNumberFromIndex(m_Index);
	}

	std::size_t SourcePoint::getColumn(ISource* source) const
	{
		return source->getColumnNumberFromIndex(m_Index);
	}

	SourceIterator::SourceIterator()
	    : m_Source(nullptr), m_CachedOffset(128)
	{
		movePoint(0);
	}

	SourceIterator::SourceIterator(ISource* source, SourcePoint point)
	    : m_Source(source), m_Point(point), m_CachedOffset(~0ULL)
	{
		movePoint(0);
	}

	char SourceIterator::operator*() const
	{
		if (m_Cached.empty() || m_CachedOffset >= m_Cached.size())
			return '\0';
		return m_Cached[m_CachedOffset];
	}

	SourceIterator& SourceIterator::operator++()
	{
		movePoint(1);
		return *this;
	}

	SourceIterator SourceIterator::operator++(int)
	{
		auto c = *this;
		++(*this);
		return c;
	}

	SourceIterator& SourceIterator::operator--()
	{
		movePoint(1, true);
		return *this;
	}

	SourceIterator SourceIterator::operator--(int)
	{
		auto c = *this;
		--(*this);
		return c;
	}

	SourceIterator& SourceIterator::operator+=(std::size_t count)
	{
		movePoint(count);
		return *this;
	}

	SourceIterator& SourceIterator::operator-=(std::size_t count)
	{
		movePoint(count, true);
		return *this;
	}

	bool SourceIterator::operator==(const SourceIterator& other) const
	{
		return m_Point.m_Index == other.m_Point.m_Index;
	}

	bool SourceIterator::operator!=(const SourceIterator& other) const
	{
		return m_Point.m_Index != other.m_Point.m_Index;
	}

	bool SourceIterator::operator<(const SourceIterator& other) const
	{
		return m_Point.m_Index < other.m_Point.m_Index;
	}

	bool SourceIterator::operator<=(const SourceIterator& other) const
	{
		return m_Point.m_Index <= other.m_Point.m_Index;
	}

	bool SourceIterator::operator>(const SourceIterator& other) const
	{
		return m_Point.m_Index > other.m_Point.m_Index;
	}

	bool SourceIterator::operator>=(const SourceIterator& other) const
	{
		return m_Point.m_Index >= other.m_Point.m_Index;
	}

	void SourceIterator::movePoint(std::size_t count, bool negative)
	{
		if (negative)
		{
			m_CachedOffset -= count;
			m_Point.m_Index -= count;
		}
		else
		{
			m_CachedOffset += count;
			m_Point.m_Index += count;
		}

		if (m_Source && m_CachedOffset >= m_Cached.size())
		{
			std::size_t index = m_Point.m_Index < 128 ? 0 : m_Point.m_Index - 128;
			m_Cached          = m_Source->getSpan(index, 256);
			m_CachedOffset    = m_Point.m_Index < 128 ? m_Point.m_Index : 128;
		}
	}

	SourceIterator SourceSpan::begin(ISource* source) const
	{
		return { source, m_Begin };
	}

	SourceIterator SourceSpan::end(ISource* source) const
	{
		return { source, m_End };
	}

	std::size_t SourceSpan::length() const
	{
		return m_End.m_Index - m_Begin.m_Index;
	}

	SourceSpan ISource::getCompleteSpan()
	{
		return { 0, getSize() };
	}

	StringSource::StringSource(const std::string& str)
	    : m_Str(str)
	{
		setupLineToIndex();
	}

	StringSource::StringSource(std::string&& str)
	    : m_Str(std::move(str))
	{
		setupLineToIndex();
	}

	std::size_t StringSource::getSize()
	{
		return m_Str.size();
	}

	std::size_t StringSource::getNumLines()
	{
		return m_LineToIndex.size();
	}

	std::size_t StringSource::getIndexFromLineNumber(std::size_t line)
	{
		return line < m_LineToIndex.size() ? m_LineToIndex[line - 1] : ~0ULL;
	}

	std::size_t StringSource::getLineNumberFromIndex(std::size_t index)
	{
		for (std::size_t i = m_LineToIndex.size(); i > 0; --i)
			if (m_LineToIndex[i - 1] <= index)
				return i;
		return ~0ULL;
	}

	std::size_t StringSource::getColumnNumberFromIndex(std::size_t index)
	{
		std::size_t line = getLineNumberFromIndex(index);
		if (line == ~0ULL)
			return 1;
		std::size_t lineStart = getIndexFromLineNumber(line);
		if (lineStart == ~0ULL)
			return 1;
		return index - lineStart + 1;
	}

	std::string StringSource::getSpan(std::size_t index, std::size_t length)
	{
		if (index >= m_Str.size())
			return {};

		if ((index + length) >= m_Str.size())
			length = m_Str.size() - index;

		return m_Str.substr(index, length);
	}

	std::string StringSource::getLine(std::size_t line)
	{
		std::size_t lineStart = getIndexFromLineNumber(line);
		if (lineStart == ~0ULL)
			return {};

		std::size_t lineEnd = getIndexFromLineNumber(line + 1);
		if (lineEnd == ~0ULL)
			return {};

		return m_Str.substr(lineStart, lineEnd - lineStart - 1);
	}

	std::vector<std::string> StringSource::getLines(std::size_t startLine, std::size_t lines)
	{
		std::vector<std::string> lns;
		for (std::size_t line = startLine, end = startLine + lines; line != end; ++line)
			lns.push_back(std::move(getLine(line)));
		return lns;
	}

	void StringSource::setupLineToIndex()
	{
		m_LineToIndex.emplace_back(0);
		for (std::size_t i = 0; i < m_Str.size(); ++i)
			if (m_Str[i] == '\n')
				m_LineToIndex.emplace_back(i + 1);
	}
} // namespace CommonLexer
