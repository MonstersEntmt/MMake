#include "CommonLexer/Source.h"

#include <string>
#include <utility>
#include <vector>

namespace CommonLexer
{
	SourcePoint::SourcePoint()
	    : m_Index(0), m_Line(0), m_Column(0) {}

	SourcePoint::SourcePoint(std::size_t index, std::size_t line, std::size_t column)
	    : m_Index(index), m_Line(line), m_Column(column) {}

	SourceIterator::SourceIterator()
	    : m_Source(nullptr), m_Point({}), m_CachedOffset(128)
	{
		movePoint(0);
	}

	SourceIterator::SourceIterator(ISource* source, const SourcePoint& point)
	    : m_Source(source), m_Point(point), m_CachedOffset(~0ULL)
	{
		movePoint(0);
	}

	SourceIterator::SourceIterator(ISource* source, SourcePoint&& point)
	    : m_Source(source), m_Point(std::move(point)), m_CachedOffset(~0ULL)
	{
		movePoint(0);
	}

	char SourceIterator::operator*() const
	{
		if (m_Cached.empty())
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

		if (m_Source)
		{
			m_Point.m_Column = m_Source->getColumnNumberFromIndex(m_Point.m_Index);
			m_Point.m_Line   = m_Source->getLineNumberFromIndex(m_Point.m_Index);

			if (m_CachedOffset >= m_Cached.size())
			{
				std::size_t index = m_Point.m_Index < 128 ? 0 : m_Point.m_Index - 128;
				m_Cached          = m_Source->getSpan(index, 256);
				m_CachedOffset    = m_Point.m_Index < 128 ? m_Point.m_Index : 128;
			}
		}
	}

	SourceSpan::SourceSpan()
	    : m_Begin({}), m_End({}) {}

	SourceSpan::SourceSpan(const SourcePoint& begin, const SourcePoint& end)
	    : m_Begin(begin), m_End(end) {}

	SourceSpan::SourceSpan(SourcePoint&& begin, SourcePoint&& end)
	    : m_Begin(std::move(begin)), m_End(std::move(end)) {}

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
		return SourceSpan { SourcePoint { 0, 1, 1 },
			                SourcePoint { getSize(), getNumLines(), getColumnNumberFromIndex(getSize()) } };
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
		return line < m_LineToIndex.size() ? m_LineToIndex[line] : ~0ULL;
	}

	std::size_t StringSource::getLineNumberFromIndex(std::size_t index)
	{
		for (std::size_t i = m_LineToIndex.size(); i > 0; --i)
			if (m_LineToIndex[i - 1] <= index)
				return i - 1;
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

	std::string StringSource::getSpan(const SourceSpan& span)
	{
		if (span.m_Begin.m_Index >= m_Str.size())
			return {};

		return m_Str.substr(span.m_Begin.m_Index, span.m_End.m_Index - span.m_Begin.m_Index);
	}

	std::string StringSource::getLine(const SourcePoint& point)
	{
		std::size_t lineStart = getIndexFromLineNumber(point.m_Line);
		if (lineStart == ~0ULL)
			return {};

		std::size_t lineEnd = getIndexFromLineNumber(point.m_Line + 1);
		if (lineEnd == ~0ULL)
			return {};

		return m_Str.substr(lineStart, lineEnd - lineStart);
	}

	std::vector<std::string> StringSource::getLines(std::size_t startLine, std::size_t lines)
	{
		std::vector<std::string> lns;
		for (std::size_t line = startLine, end = startLine + lines; line != end; ++line)
			lns.push_back(std::move(ISource::getLine(line)));
		return lns;
	}

	void StringSource::setupLineToIndex()
	{
		m_LineToIndex.emplace_back(0);
		for (std::size_t i = 0; i < m_Str.size(); ++i)
			if (m_Str[i] == '\n')
				m_LineToIndex.emplace_back(i + 1);
	}

	FileSource::FileSource(const std::filesystem::path& filepath)
	    : m_Filepath(filepath), m_Stream(filepath, std::ios::ate)
	{
		m_Size = m_Stream.tellg();
		m_Stream.seekg(0);

		setupLineToIndex();
	}

	FileSource::~FileSource()
	{
		m_Stream.close();
	}

	std::size_t FileSource::getSize()
	{
		return m_Size;
	}

	std::size_t FileSource::getNumLines()
	{
		return m_LineToIndex.size();
	}

	std::size_t FileSource::getIndexFromLineNumber(std::size_t line)
	{
		return line < m_LineToIndex.size() ? m_LineToIndex[line] : ~0ULL;
	}

	std::size_t FileSource::getLineNumberFromIndex(std::size_t index)
	{
		for (std::size_t i = 0; i < m_LineToIndex.size(); ++i)
			if (m_LineToIndex[i] > index)
				return i;
		return ~0ULL;
	}

	std::size_t FileSource::getColumnNumberFromIndex(std::size_t index)
	{
		std::size_t line = getLineNumberFromIndex(index);
		if (line == ~0ULL)
			return 1;
		std::size_t lineStart = getIndexFromLineNumber(line);
		if (lineStart == ~0ULL)
			return 1;
		return index - lineStart;
	}

	std::string FileSource::getSpan(const SourceSpan& span)
	{
		if (span.m_Begin.m_Index >= m_Size)
			return {};

		std::size_t length;
		if (span.m_End.m_Index >= m_Size)
			length = m_Size - span.m_Begin.m_Index;
		else
			length = span.m_End.m_Index - span.m_Begin.m_Index;

		std::string str(length, '\0');
		m_Stream.seekg(span.m_Begin.m_Index);
		m_Stream.read(str.data(), str.size());
		return str;
	}

	std::string FileSource::getLine(const SourcePoint& point)
	{
		std::size_t lineStart = getIndexFromLineNumber(point.m_Line);
		if (lineStart == ~0ULL)
			return {};

		std::size_t lineEnd = getIndexFromLineNumber(point.m_Line + 1);
		if (lineEnd == ~0ULL)
			return {};

		std::string str(lineEnd - lineStart, '\0');
		m_Stream.seekg(lineStart);
		m_Stream.read(str.data(), str.size());
		return str;
	}

	std::vector<std::string> FileSource::getLines(std::size_t startLine, std::size_t lines)
	{
		std::vector<std::string> lns;
		for (std::size_t line = startLine, end = startLine + lines; line != end; ++line)
			lns.push_back(std::move(ISource::getLine(line)));
		return lns;
	}

	void FileSource::setupLineToIndex()
	{
		m_LineToIndex.emplace_back(0);
		char* buf = new char[32767];

		std::size_t offset = 0;
		while (offset < m_Size)
		{
			auto readCount = m_Stream.readsome(buf, sizeof(buf));
			for (std::size_t i = 0; i < sizeof(buf); ++i)
				if (buf[i] == '\n')
					m_LineToIndex.emplace_back(offset + i + 1);
			offset += readCount;
		}
	}
} // namespace CommonLexer
