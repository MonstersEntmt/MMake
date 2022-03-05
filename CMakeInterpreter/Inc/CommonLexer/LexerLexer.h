#pragma once

#include "Lexer.h"
#include "Message.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace CommonLexer
{
	struct LexerLexerResult
	{
	public:
		std::vector<Message> m_Messages;

		Lexer m_Lexer;
	};

	struct LexerCPPResult
	{
	public:
		std::vector<Message> m_Messages;

		std::unordered_map<std::string, std::string> m_Rules;

		std::string m_MainRule;
	};

	class LexerLexer : public Lexer
	{
	public:
		LexerLexer();

		LexerLexerResult createLexer(const Lex& lex, std::unordered_map<std::string, CallbackRule::Callback> callbacks = {});

		LexerCPPResult createCPPLexer(const Lex& lex);
		std::string    compileLexer(LexerCPPResult& result, std::string_view namespaceName);
	};
} // namespace CommonLexer