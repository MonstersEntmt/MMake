#pragma once

#include "Source.h"

#include <string>

namespace CommonLexer
{
	enum class EMessageSeverity
	{
		Warning,
		Error
	};

	struct Message
	{
	public:
		Message(const std::string& message, SourcePoint point, SourceSpan span, EMessageSeverity severity = EMessageSeverity::Error);
		Message(std::string&& message, SourcePoint point, SourceSpan span, EMessageSeverity severity = EMessageSeverity::Error);

		[[nodiscard]] auto& getMessage() const { return m_Message; }
		[[nodiscard]] auto  getPoint() const { return m_Point; }
		[[nodiscard]] auto  getSpan() const { return m_Span; }
		[[nodiscard]] auto  getSeverity() const { return m_Severity; }

	private:
		std::string      m_Message;
		SourcePoint      m_Point;
		SourceSpan       m_Span;
		EMessageSeverity m_Severity;
	};
} // namespace CommonLexer