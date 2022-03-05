#include "CommonLexer/Message.h"

namespace CommonLexer
{
	Message::Message(const std::string& message, SourcePoint point, SourceSpan span, EMessageSeverity severity)
	    : m_Message(message), m_Point(point), m_Span(span), m_Severity(severity) {}

	Message::Message(std::string&& message, SourcePoint point, SourceSpan span, EMessageSeverity severity)
	    : m_Message(std::move(message)), m_Point(point), m_Span(span), m_Severity(severity) {}
} // namespace CommonLexer