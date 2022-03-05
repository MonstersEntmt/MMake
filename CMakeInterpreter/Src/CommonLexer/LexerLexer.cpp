#include "CommonLexer/LexerLexer.h"
#include "CommonLexer/Matchers.h"
#include "CommonLexer/Rule.h"

namespace CommonLexer
{
	std::uint64_t ReadUInt(std::string_view text)
	{
		std::uint64_t value = 0;

		auto itr = text.begin();
		auto end = text.end();

		while (itr != end)
		{
			char c = *itr;
			value  = (value * 10) + static_cast<std::uint64_t>(c - '0');
			++itr;
		}

		return value;
	}

	std::string& EscapeText(std::string& text)
	{
		for (std::size_t i = text.size(); i > 0; --i)
		{
			char c = text[i - 1];
			switch (c)
			{
			case '\a':
				text.replace(i - 1, 1, "\\a");
				break;
			case '\b':
				text.replace(i - 1, 1, "\\b");
				break;
			case '\f':
				text.replace(i - 1, 1, "\\f");
				break;
			case '\n':
				text.replace(i - 1, 1, "\\n");
				break;
			case '\r':
				text.replace(i - 1, 1, "\\r");
				break;
			case '\t':
				text.replace(i - 1, 1, "\\t");
				break;
			case '\v':
				text.replace(i - 1, 1, "\\v");
				break;
			case '\\':
				text.replace(i - 1, 1, "\\\\");
				break;
			case '\'':
				text.replace(i - 1, 1, "\\'");
				break;
			case '"':
				text.replace(i - 1, 1, "\\\"");
				break;
			}
		}
		return text;
	}

	std::string EscapeText(std::string&& text)
	{
		std::string copy = std::move(text);
		EscapeText(copy);
		return copy;
	}

	std::string& UnescapeText(std::string& text)
	{
		std::size_t offset = 1;
		for (std::size_t i = 1; i < text.size() - 1; ++i)
		{
			char c = text[i];

			bool doOffset = true;
			if (c == '\\')
			{
				if (++i < text.size())
				{
					c = text[i];
					switch (c)
					{
					case 'a':
						doOffset = false;
						++offset;
						text[i - offset] = '\a';
						break;
					case 'b':
						doOffset = false;
						++offset;
						text[i - offset] = '\b';
						break;
					case 'f':
						doOffset = false;
						++offset;
						text[i - offset] = '\f';
						break;
					case 'n':
						doOffset = false;
						++offset;
						text[i - offset] = '\n';
						break;
					case 'r':
						doOffset = false;
						++offset;
						text[i - offset] = '\r';
						break;
					case 't':
						doOffset = false;
						++offset;
						text[i - offset] = '\t';
						break;
					case 'v':
						doOffset = false;
						++offset;
						text[i - offset] = '\v';
						break;
					default:
						++offset;
						break;
					}
				}
			}

			if (offset > 0 && doOffset)
				text[i - offset] = text[i];
		}
		text.resize(text.size() - offset - 1);
		return text;
	}

	std::string UnescapeText(std::string&& text)
	{
		std::string copy = std::move(text);
		UnescapeText(copy);
		return copy;
	}

	LexerLexer::LexerLexer()
	{
		setMainRule("File");

		registerRule(MatcherRule {
		    "File",
		    RangeMatcher(
		        SpaceMatcher(
		            ReferenceMatcher("Declaration"),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both)),
		    false });
		registerRule(MatcherRule {
		    "Declaration",
		    OrMatcher(Tuple {
		        ReferenceMatcher("EmptyDeclaration"),
		        ReferenceMatcher("BlockDeclaration"),
		        ReferenceMatcher("OptionDeclaration"),
		        ReferenceMatcher("RuleDeclaration"),
		        ReferenceMatcher("NodelessRuleDeclaration"),
		        ReferenceMatcher("CallbackRuleDeclaration"),
		        ReferenceMatcher("Comment") }),
		    false });
		registerRule(MatcherRule { "EmptyDeclaration", TextMatcher(";"), false });
		registerRule(MatcherRule {
		    "BlockDeclaration",
		    CombinationMatcher(Tuple {
		        SpaceMatcher(
		            TextMatcher("{"),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        RangeMatcher(
		            SpaceMatcher(
		                ReferenceMatcher("Declaration"),
		                false,
		                ESpaceMethod::Whitespace,
		                ESpaceDirection::Both)),
		        TextMatcher("}") }) });
		registerRule(MatcherRule {
		    "OptionDeclaration",
		    CombinationMatcher(Tuple {
		        SpaceMatcher(
		            TextMatcher("!"),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        SpaceMatcher(
		            ReferenceMatcher("Identifier"),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        SpaceMatcher(
		            TextMatcher("="),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        SpaceMatcher(
		            ReferenceMatcher("Value"),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        TextMatcher(";") }) });
		registerRule(MatcherRule {
		    "RuleDeclaration",
		    CombinationMatcher(Tuple {
		        SpaceMatcher(
		            ReferenceMatcher("Identifier"),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        SpaceMatcher(
		            TextMatcher(":"),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        SpaceMatcher(
		            ReferenceMatcher("Matcher"),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        TextMatcher(";") }) });
		registerRule(MatcherRule {
		    "NodelessRuleDeclaration",
		    CombinationMatcher(Tuple {
		        SpaceMatcher(
		            ReferenceMatcher("Identifier"),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        SpaceMatcher(
		            TextMatcher("?"),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        SpaceMatcher(
		            TextMatcher(":"),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        SpaceMatcher(
		            ReferenceMatcher("Matcher"),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        TextMatcher(";") }) });
		registerRule(MatcherRule {
		    "CallbackRuleDeclaration",
		    CombinationMatcher(Tuple {
		        SpaceMatcher(
		            ReferenceMatcher("Identifier"),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        SpaceMatcher(
		            TextMatcher("!"),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        SpaceMatcher(
		            TextMatcher(";"),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both) }) });
		registerRule(MatcherRule { "Comment", RegexMatcher("#.*"), false });

		registerRule(MatcherRule { "Identifier", RegexMatcher("[A-Za-z_][A-Za-z0-9_]*") });

		registerRule(MatcherRule {
		    "Value",
		    OrMatcher(Tuple {
		        ReferenceMatcher("Identifier"),
		        ReferenceMatcher("Boolean"),
		        ReferenceMatcher("Integer"),
		        ReferenceMatcher("TextMatcher") }),
		    false });
		registerRule(MatcherRule {
		    "Boolean",
		    OrMatcher(Tuple {
		        TextMatcher("true"),
		        TextMatcher("false") }),
		    false });
		registerRule(MatcherRule { "Integer", RegexMatcher("[0-9]+") });

		registerRule(MatcherRule { "Matcher", ReferenceMatcher("Depth1Matcher"), false });
		registerRule(MatcherRule {
		    "Depth1Matcher",
		    OrMatcher(Tuple {
		        ReferenceMatcher("Branch"),
		        ReferenceMatcher("Depth2Matcher") }),
		    false });
		registerRule(MatcherRule {
		    "Depth2Matcher",
		    OrMatcher(Tuple {
		        ReferenceMatcher("CombinationMatcher"),
		        ReferenceMatcher("OrMatcher"),
		        ReferenceMatcher("Depth3Matcher") }),
		    false });
		registerRule(MatcherRule {
		    "Depth3Matcher",
		    OrMatcher(Tuple {
		        ReferenceMatcher("ZeroOrMore"),
		        ReferenceMatcher("OneOrMore"),
		        ReferenceMatcher("ExactAmount"),
		        ReferenceMatcher("RangeMatcher"),
		        ReferenceMatcher("OptionalMatcher"),
		        ReferenceMatcher("NegativeMatcher"),
		        ReferenceMatcher("Depth4Matcher") }),
		    false });
		registerRule(MatcherRule {
		    "Depth4Matcher",
		    OrMatcher(Tuple {
		        ReferenceMatcher("LenientSpaceMatcher"),
		        ReferenceMatcher("ForcedSpaceMatcher"),
		        ReferenceMatcher("Depth5Matcher") }),
		    false });
		registerRule(MatcherRule {
		    "Depth5Matcher",
		    OrMatcher(Tuple {
		        ReferenceMatcher("Group"),
		        ReferenceMatcher("NamedGroupMatcher"),
		        ReferenceMatcher("NamedGroupReferenceMatcher"),
		        ReferenceMatcher("ReferenceMatcher"),
		        ReferenceMatcher("TextMatcher"),
		        ReferenceMatcher("RegexMatcher") }),
		    false });

		registerRule(MatcherRule {
		    "Branch",
		    CombinationMatcher(Tuple {
		        SpaceMatcher(
		            ReferenceMatcher("Depth2Matcher"),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        RangeMatcher(
		            SpaceMatcher(
		                CombinationMatcher(Tuple {
		                    SpaceMatcher(
		                        TextMatcher(";"),
		                        false,
		                        ESpaceMethod::Whitespace,
		                        ESpaceDirection::Both),
		                    ReferenceMatcher("Depth2Matcher") }),
		                false,
		                ESpaceMethod::Whitespace,
		                ESpaceDirection::Both),
		            1) }) });

		registerRule(MatcherRule {
		    "CombinationMatcher",
		    CombinationMatcher(Tuple {
		        ReferenceMatcher("Depth3Matcher"),
		        RangeMatcher(
		            SpaceMatcher(
		                ReferenceMatcher("Depth3Matcher"),
		                true,
		                ESpaceMethod::Whitespace,
		                ESpaceDirection::Left),
		            1) }) });
		registerRule(MatcherRule {
		    "OrMatcher",
		    CombinationMatcher(Tuple {
		        SpaceMatcher(
		            ReferenceMatcher("Depth3Matcher"),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        RangeMatcher(
		            SpaceMatcher(
		                CombinationMatcher(Tuple {
		                    SpaceMatcher(
		                        TextMatcher("|"),
		                        false,
		                        ESpaceMethod::Whitespace,
		                        ESpaceDirection::Both),
		                    ReferenceMatcher("Depth3Matcher") }),
		                false,
		                ESpaceMethod::Whitespace,
		                ESpaceDirection::Both),
		            1) }) });

		registerRule(MatcherRule {
		    "ZeroOrMore",
		    CombinationMatcher(Tuple {
		        SpaceMatcher(
		            ReferenceMatcher("Depth4Matcher"),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        TextMatcher("*") }) });
		registerRule(MatcherRule {
		    "OneOrMore",
		    CombinationMatcher(Tuple {
		        SpaceMatcher(
		            ReferenceMatcher("Depth4Matcher"),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        TextMatcher("+") }) });
		registerRule(MatcherRule {
		    "ExactAmount",
		    CombinationMatcher(Tuple {
		        SpaceMatcher(
		            ReferenceMatcher("Depth4Matcher"),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        SpaceMatcher(
		            TextMatcher("{"),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        SpaceMatcher(
		            ReferenceMatcher("Integer"),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        TextMatcher("}") }) });
		registerRule(MatcherRule {
		    "RangeMatcher",
		    CombinationMatcher(Tuple {
		        SpaceMatcher(
		            ReferenceMatcher("Depth4Matcher"),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        SpaceMatcher(
		            TextMatcher("{"),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        SpaceMatcher(
		            ReferenceMatcher("Integer"),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        SpaceMatcher(
		            TextMatcher(","),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        SpaceMatcher(
		            ReferenceMatcher("Integer"),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        TextMatcher("}") }) });
		registerRule(MatcherRule {
		    "OptionalMatcher",
		    CombinationMatcher(Tuple {
		        SpaceMatcher(
		            ReferenceMatcher("Depth4Matcher"),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        SpaceMatcher(
		            TextMatcher("?"),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        NegativeMatcher(TextMatcher(":")) }) });
		registerRule(MatcherRule {
		    "NegativeMatcher",
		    CombinationMatcher(Tuple {
		        SpaceMatcher(
		            TextMatcher("~"),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        ReferenceMatcher("Depth4Matcher") }) });

		registerRule(MatcherRule {
		    "LenientSpaceMatcher",
		    CombinationMatcher(Tuple {
		        SpaceMatcher(
		            ReferenceMatcher("Depth5Matcher"),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        TextMatcher(",") }) });
		registerRule(MatcherRule {
		    "ForcedSpaceMatcher",
		    CombinationMatcher(Tuple {
		        SpaceMatcher(
		            ReferenceMatcher("Depth5Matcher"),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        TextMatcher(".") }) });

		registerRule(MatcherRule {
		    "Group",
		    CombinationMatcher(Tuple {
		        SpaceMatcher(
		            TextMatcher("("),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        SpaceMatcher(
		            ReferenceMatcher("Depth2Matcher"),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        TextMatcher(")") }) });
		registerRule(MatcherRule {
		    "NamedGroupMatcher",
		    CombinationMatcher(Tuple {
		        SpaceMatcher(
		            TextMatcher("("),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        SpaceMatcher(
		            TextMatcher("<"),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        SpaceMatcher(
		            ReferenceMatcher("Identifier"),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        SpaceMatcher(
		            TextMatcher(">"),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        SpaceMatcher(
		            TextMatcher(":"),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        SpaceMatcher(
		            ReferenceMatcher("Depth2Matcher"),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        TextMatcher(")") }) });
		registerRule(MatcherRule {
		    "NamedGroupReferenceMatcher",
		    CombinationMatcher(Tuple {
		        SpaceMatcher(
		            TextMatcher("\\"),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        SpaceMatcher(
		            ReferenceMatcher("Identifier"),
		            false,
		            ESpaceMethod::Whitespace,
		            ESpaceDirection::Both),
		        NegativeMatcher(
		            OrMatcher(Tuple {
		                CombinationMatcher(Tuple {
		                    SpaceMatcher(
		                        TextMatcher("!"),
		                        false,
		                        ESpaceMethod::Whitespace,
		                        ESpaceDirection::Both),
		                    TextMatcher(";") }),
		                CombinationMatcher(Tuple {
		                    OptionalMatcher(
		                        SpaceMatcher(
		                            TextMatcher("?"),
		                            false,
		                            ESpaceMethod::Whitespace,
		                            ESpaceDirection::Both)),
		                    TextMatcher(":") }) })) }) });
		registerRule(MatcherRule {
		    "ReferenceMatcher",
		    CombinationMatcher(Tuple {
		        ReferenceMatcher("Identifier"),
		        NegativeMatcher(
		            OrMatcher(Tuple {
		                CombinationMatcher(Tuple {
		                    SpaceMatcher(
		                        TextMatcher("!"),
		                        false,
		                        ESpaceMethod::Whitespace,
		                        ESpaceDirection::Both),
		                    TextMatcher(";") }),
		                CombinationMatcher(Tuple {
		                    OptionalMatcher(
		                        SpaceMatcher(
		                            TextMatcher("?"),
		                            false,
		                            ESpaceMethod::Whitespace,
		                            ESpaceDirection::Both)),
		                    TextMatcher(":") }) })) }) });
		registerRule(MatcherRule { "TextMatcher", RegexMatcher("\"(?:[^\"\\\\\n]|\\.|\\\\.)*\"") });
		registerRule(MatcherRule { "RegexMatcher", RegexMatcher("'(?:[^'\\\\\n]|\\.|\\\\.)*'") });
	}

	struct ScopeSettings
	{
	public:
		ESpaceMethod    m_SpaceMethod    = ESpaceMethod::Normal;
		ESpaceDirection m_SpaceDirection = ESpaceDirection::Right;
	};

	std::unique_ptr<IMatcher> handleMatcher(LexerLexerResult& result, const Lex& lex, const Node& node, ScopeSettings settings, std::size_t depth = 0)
	{
		auto source = lex.getSource();

		auto rule = node.getRule();
		auto span = node.getSpan();
		if (rule == "RegexMatcher")
		{
			return std::make_unique<RegexMatcher>(UnescapeText(source->getSpan(span)));
		}
		else if (rule == "TextMatcher")
		{
			return std::make_unique<TextMatcher>(UnescapeText(source->getSpan(span)));
		}
		else if (rule == "ReferenceMatcher")
		{
			auto ruleId = node.getChild(0);
			if (!ruleId || ruleId->getRule() != "Identifier")
			{
				result.m_Messages.emplace_back("Unexpected ReferenceMatcher missing Identifier", span.m_Begin, span);
				return nullptr;
			}

			return std::make_unique<ReferenceMatcher>(source->getSpan(ruleId->getSpan()));
		}
		else if (rule == "NamedGroupReferenceMatcher")
		{
			auto namedGroupId = node.getChild(0);
			if (!namedGroupId || namedGroupId->getRule() != "Identifier")
			{
				result.m_Messages.emplace_back("Unexpected NamedGroupReferenceMatcher missing Identifier", span.m_Begin, span);
				return nullptr;
			}

			return std::make_unique<ReferenceMatcher>(source->getSpan(namedGroupId->getSpan()));
		}
		else if (rule == "NamedGroupMatcher")
		{
			auto namedGroupId = node.getChild(0);
			if (!namedGroupId || namedGroupId->getRule() != "Identifier")
			{
				result.m_Messages.emplace_back("Unexpected NamedGroupMatcher missing Identifier", span.m_Begin, span);
				return nullptr;
			}
			auto namedGroupSpan = namedGroupId->getSpan();

			auto subMatcher = node.getChild(1);
			if (!subMatcher)
			{
				result.m_Messages.emplace_back("Unexpected NamedGroupMatcher missing Matcher", namedGroupSpan.m_End, span);
				return nullptr;
			}
			auto subMatcherSpan = subMatcher->getSpan();

			auto matcher = handleMatcher(result, lex, *subMatcher, settings, depth + 1);
			if (!matcher)
				return nullptr;

			return std::make_unique<NamedGroupMatcher>(source->getSpan(namedGroupSpan), std::move(matcher));
		}
		else if (rule == "Group")
		{
			auto subMatcher = node.getChild(0);
			if (!subMatcher)
			{
				result.m_Messages.emplace_back("Unexpected Group missing Matcher", span.m_Begin, span);
				return nullptr;
			}

			return handleMatcher(result, lex, *subMatcher, settings, depth);
		}
		else if (rule == "ForcedSpaceMatcher")
		{
			auto subMatcher = node.getChild(0);
			if (!subMatcher)
			{
				result.m_Messages.emplace_back("Unexpected ForcedSpaceMatcher missing Matcher", span.m_Begin, span);
				return nullptr;
			}

			auto matcher = handleMatcher(result, lex, *subMatcher, settings, depth + 1);
			if (!matcher)
				return nullptr;

			return std::make_unique<SpaceMatcher>(std::move(matcher), true, settings.m_SpaceMethod, settings.m_SpaceDirection);
		}
		else if (rule == "LenientSpaceMatcher")
		{
			auto subMatcher = node.getChild(0);
			if (!subMatcher)
			{
				result.m_Messages.emplace_back("Unexpected LenientSpaceMatcher missing Matcher", span.m_Begin, span);
				return nullptr;
			}

			auto matcher = handleMatcher(result, lex, *subMatcher, settings, depth + 1);
			if (!matcher)
				return nullptr;

			return std::make_unique<SpaceMatcher>(std::move(matcher), false, settings.m_SpaceMethod, settings.m_SpaceDirection);
		}
		else if (rule == "NegativeMatcher")
		{
			auto subMatcher = node.getChild(0);
			if (!subMatcher)
			{
				result.m_Messages.emplace_back("Unexpected NegativeMatcher missing Matcher", span.m_Begin, span);
				return nullptr;
			}

			auto matcher = handleMatcher(result, lex, *subMatcher, settings, depth + 1);
			if (!matcher)
				return nullptr;

			return std::make_unique<NegativeMatcher>(std::move(matcher));
		}
		else if (rule == "OptionalMatcher")
		{
			auto subMatcher = node.getChild(0);
			if (!subMatcher)
			{
				result.m_Messages.emplace_back("Unexpected OptionalMatcher missing Matcher", span.m_Begin, span);
				return nullptr;
			}

			auto matcher = handleMatcher(result, lex, *subMatcher, settings, depth + 1);
			if (!matcher)
				return nullptr;

			return std::make_unique<OptionalMatcher>(std::move(matcher));
		}
		else if (rule == "RangeMatcher")
		{
			auto subMatcher = node.getChild(0);
			if (!subMatcher)
			{
				result.m_Messages.emplace_back("Unexpected RangeMatcher missing Matcher", span.m_Begin, span);
				return nullptr;
			}
			auto subMatcherSpan = subMatcher->getSpan();

			auto lowerBoundsNode = node.getChild(1);
			if (!lowerBoundsNode || lowerBoundsNode->getRule() != "Integer")
			{
				result.m_Messages.emplace_back("Unexpected RangeMatcher missing lower bounds", subMatcherSpan.m_Begin, span);
				return nullptr;
			}
			auto lowerBoundsSpan = lowerBoundsNode->getSpan();
			auto lowerBounds     = ReadUInt(source->getSpan(lowerBoundsSpan));

			auto upperBoundsNode = node.getChild(2);
			if (!upperBoundsNode || upperBoundsNode->getRule() != "Integer")
			{
				result.m_Messages.emplace_back("Unexpected RangeMatcher missing upper bounds", subMatcherSpan.m_Begin, span);
				return nullptr;
			}
			auto upperBoundsSpan = upperBoundsNode->getSpan();
			auto upperBounds     = ReadUInt(source->getSpan(upperBoundsSpan));
			if (upperBounds < lowerBounds)
			{
				result.m_Messages.emplace_back("Range Matcher upper bounds is smaller than lower bounds, assuming UpperBounds=LowerBounds", upperBoundsSpan.m_Begin, span, EMessageSeverity::Warning);
				upperBounds = lowerBounds;
			}

			auto matcher = handleMatcher(result, lex, *subMatcher, settings, depth + 1);
			if (!matcher)
				return nullptr;

			return std::make_unique<RangeMatcher>(std::move(matcher), lowerBounds, upperBounds);
		}
		else if (rule == "ExactAmount")
		{
			auto subMatcher = node.getChild(0);
			if (!subMatcher)
			{
				result.m_Messages.emplace_back("Unexpected ExactAmount missing Matcher", span.m_Begin, span);
				return nullptr;
			}
			auto subMatcherSpan = subMatcher->getSpan();

			auto amountNode = node.getChild(1);
			if (!amountNode || amountNode->getRule() != "Integer")
			{
				result.m_Messages.emplace_back("Unexpected ExactAmount missing amount", subMatcherSpan.m_Begin, span);
				return nullptr;
			}
			auto amount = ReadUInt(source->getSpan(amountNode->getSpan()));

			auto matcher = handleMatcher(result, lex, *subMatcher, settings, depth + 1);
			if (!matcher)
				return nullptr;

			return std::make_unique<RangeMatcher>(std::move(matcher), amount, amount);
		}
		else if (rule == "OneOrMore")
		{
			auto subMatcher = node.getChild(0);
			if (!subMatcher)
			{
				result.m_Messages.emplace_back("Unexpected ExactAmount missing Matcher", span.m_Begin, span);
				return nullptr;
			}
			auto subMatcherSpan = subMatcher->getSpan();

			auto matcher = handleMatcher(result, lex, *subMatcher, settings, depth + 1);
			if (!matcher)
				return nullptr;

			return std::make_unique<RangeMatcher>(std::move(matcher), 1);
		}
		else if (rule == "ZeroOrMore")
		{
			auto subMatcher = node.getChild(0);
			if (!subMatcher)
			{
				result.m_Messages.emplace_back("Unexpected ExactAmount missing Matcher", span.m_Begin, span);
				return nullptr;
			}
			auto subMatcherSpan = subMatcher->getSpan();

			auto matcher = handleMatcher(result, lex, *subMatcher, settings, depth + 1);
			if (!matcher)
				return nullptr;

			return std::make_unique<RangeMatcher>(std::move(matcher));
		}
		else if (rule == "OrMatcher")
		{
			std::vector<std::unique_ptr<IMatcher>> matchers;
			matchers.reserve(node.getChildren().size());
			for (auto& child : node.getChildren())
			{
				auto matcher = handleMatcher(result, lex, child, settings, depth + 1);
				if (!matcher)
					return nullptr;
				matchers.push_back(std::move(matcher));
			}
			return std::make_unique<OrMatcher>(std::move(matchers));
		}
		else if (rule == "CombinationMatcher")
		{
			std::vector<std::unique_ptr<IMatcher>> matchers;
			matchers.reserve(node.getChildren().size());
			for (auto& child : node.getChildren())
			{
				auto matcher = handleMatcher(result, lex, child, settings, depth + 1);
				if (!matcher)
					return nullptr;
				matchers.push_back(std::move(matcher));
			}
			return std::make_unique<CombinationMatcher>(std::move(matchers));
		}
		else if (rule == "Branch")
		{
			std::vector<std::unique_ptr<IMatcher>> matchers;
			matchers.reserve(node.getChildren().size());
			for (auto& child : node.getChildren())
			{
				auto matcher = handleMatcher(result, lex, child, settings, depth + 1);
				if (!matcher)
					return nullptr;
				matchers.push_back(std::move(matcher));
			}
			return std::make_unique<OrMatcher>(std::move(matchers));
		}
		else
		{
			result.m_Messages.emplace_back(std::format("Rule '{}' is not a recognized matcher rule", rule), span.m_Begin, span, EMessageSeverity::Warning);
			return nullptr;
		}
	}

	void handleScope(LexerLexerResult& result, const Lex& lex, const Node& node, ScopeSettings settings, std::unordered_map<std::string, CallbackRule::Callback>& callbacks, std::size_t depth = 0)
	{
		auto source = lex.getSource();

		std::vector<std::string> declaredOptions;

		for (auto& declaration : node.getChildren())
		{
			auto rule            = declaration.getRule();
			auto declarationSpan = declaration.getSpan();
			if (rule == "OptionDeclaration")
			{
				auto optionIdNode = declaration.getChild(0);
				if (!optionIdNode || optionIdNode->getRule() != "Identifier")
				{
					result.m_Messages.emplace_back("Unexpected OptionDeclaration missing Identifier", declarationSpan.m_Begin, declarationSpan);
					continue;
				}
				auto optionIdSpan = optionIdNode->getSpan();

				auto valueNode = declaration.getChild(1);
				if (!valueNode)
				{
					result.m_Messages.emplace_back("Unexpected OptionDeclaration missing Value", optionIdSpan.m_End, declarationSpan);
					continue;
				}

				auto optionId = source->getSpan(optionIdSpan);
				if (std::find(declaredOptions.begin(), declaredOptions.end(), optionId) != declaredOptions.end())
				{
					result.m_Messages.emplace_back(std::format("OptionDeclaration {} already previously declared", optionId), optionIdSpan.m_Begin, optionIdSpan, EMessageSeverity::Warning);
					continue;
				}

				if (optionId == "MainRule")
				{
					if (depth > 0)
					{
						result.m_Messages.emplace_back("MainRule can only be set in the global scope", declarationSpan.m_Begin, declarationSpan);
						continue;
					}

					auto valueSpan = valueNode->getSpan();
					if (valueNode->getRule() != "Identifier")
					{
						result.m_Messages.emplace_back("MainRule value is not an Identifier", valueSpan.m_Begin, valueSpan);
						continue;
					}

					result.m_Lexer.setMainRule(source->getSpan(valueSpan));
				}
				else if (optionId == "SpaceMethod")
				{
					auto valueSpan = valueNode->getSpan();
					if (valueNode->getRule() != "TextMatcher")
					{
						result.m_Messages.emplace_back("SpaceMethod value is not a string", valueSpan.m_Begin, valueSpan);
						continue;
					}

					auto value = UnescapeText(source->getSpan(valueSpan));
					if (value == "Normal")
					{
						settings.m_SpaceMethod = ESpaceMethod::Normal;
					}
					else if (value == "Whitespace")
					{
						settings.m_SpaceMethod = ESpaceMethod::Whitespace;
					}
					else
					{
						result.m_Messages.emplace_back(std::format("SpaceMethod value \"{}\" is not valid, should be either \"Normal\" or \"Whitespace\" ", EscapeText(value)), valueSpan.m_Begin, valueSpan);
						continue;
					}
				}
				else if (optionId == "SpaceDirection")
				{
					auto valueSpan = valueNode->getSpan();
					if (valueNode->getRule() != "TextMatcher")
					{
						result.m_Messages.emplace_back("SpaceDirection value is not a string", valueSpan.m_Begin, valueSpan);
						continue;
					}

					auto value = UnescapeText(source->getSpan(valueSpan));
					if (value == "Left")
					{
						settings.m_SpaceDirection = ESpaceDirection::Left;
					}
					else if (value == "Right")
					{
						settings.m_SpaceDirection = ESpaceDirection::Right;
					}
					else if (value == "Both")
					{
						settings.m_SpaceDirection = ESpaceDirection::Both;
					}
					else
					{
						result.m_Messages.emplace_back(std::format("SpaceDirection value \"{}\" is not valid, should be either \"Left\", \"Right\" or \"Both\" ", EscapeText(value)), valueSpan.m_Begin, valueSpan);
						continue;
					}
				}

				declaredOptions.push_back(optionId);
			}
		}

		for (auto& declaration : node.getChildren())
		{
			auto rule            = declaration.getRule();
			auto declarationSpan = declaration.getSpan();
			if (rule == "OptionDeclaration")
			{
				continue;
			}
			else if (rule == "BlockDeclaration")
			{
				ScopeSettings blockScope = settings;
				handleScope(result, lex, declaration, blockScope, callbacks, depth + 1);
			}
			else if (rule == "RuleDeclaration")
			{
				auto ruleIdNode = declaration.getChild(0);
				if (!ruleIdNode || ruleIdNode->getRule() != "Identifier")
				{
					result.m_Messages.emplace_back("Unexpected RuleDeclaration missing Identifier", declarationSpan.m_Begin, declarationSpan);
					continue;
				}
				auto ruleIdSpan = ruleIdNode->getSpan();

				auto matcherNode = declaration.getChild(1);
				if (!matcherNode)
				{
					result.m_Messages.emplace_back("Unexpected RuleDeclaration missing Matcher", ruleIdSpan.m_End, declarationSpan);
					continue;
				}
				auto matcherSpan = matcherNode->getSpan();

				auto ruleId = source->getSpan(ruleIdSpan);
				if (result.m_Lexer.getRule(ruleId))
				{
					result.m_Messages.emplace_back(std::format("Rule {} has already been declared", ruleId), ruleIdSpan.m_Begin, declarationSpan);
					continue;
				}

				auto matcher = handleMatcher(result, lex, *matcherNode, settings);
				if (!matcher)
				{
					result.m_Messages.emplace_back(std::format("Rule {} failed to create its matchers", ruleId), matcherSpan.m_Begin, declarationSpan);
					continue;
				}
				result.m_Lexer.registerRule(std::make_unique<MatcherRule>(ruleId, std::move(matcher), true));
			}
			else if (rule == "NodelessRuleDeclaration")
			{
				auto ruleIdNode = declaration.getChild(0);
				if (!ruleIdNode || ruleIdNode->getRule() != "Identifier")
				{
					result.m_Messages.emplace_back("Unexpected RuleDeclaration missing Identifier", declarationSpan.m_Begin, declarationSpan);
					continue;
				}
				auto ruleIdSpan = ruleIdNode->getSpan();

				auto matcherNode = declaration.getChild(1);
				if (!matcherNode)
				{
					result.m_Messages.emplace_back("Unexpected RuleDeclaration missing Matcher", ruleIdSpan.m_End, declarationSpan);
					continue;
				}
				auto matcherSpan = matcherNode->getSpan();

				auto ruleId = source->getSpan(ruleIdSpan);
				if (result.m_Lexer.getRule(ruleId))
				{
					result.m_Messages.emplace_back(std::format("Rule {} has already been declared", ruleId), ruleIdSpan.m_Begin, declarationSpan);
					continue;
				}

				auto matcher = handleMatcher(result, lex, *matcherNode, settings);
				if (!matcher)
				{
					result.m_Messages.emplace_back(std::format("Rule {} failed to create its matchers", ruleId), matcherSpan.m_Begin, declarationSpan);
					continue;
				}
				result.m_Lexer.registerRule(std::make_unique<MatcherRule>(ruleId, std::move(matcher), false));
			}
			else if (rule == "CallbackRuleDeclaration")
			{
				auto ruleIdNode = declaration.getChild(0);
				if (!ruleIdNode || ruleIdNode->getRule() != "Identifier")
				{
					result.m_Messages.emplace_back("Unexpected CallbackRuleDeclaration missing Identifier", declarationSpan.m_Begin, declarationSpan);
					continue;
				}
				auto ruleIdSpan = ruleIdNode->getSpan();

				auto ruleId = source->getSpan(ruleIdSpan);
				if (result.m_Lexer.getRule(ruleId))
				{
					result.m_Messages.emplace_back(std::format("Rule {} has already been declared", ruleId), ruleIdSpan.m_Begin, declarationSpan);
					continue;
				}

				auto callbackItr = callbacks.find(ruleId);
				if (callbackItr == callbacks.end())
				{
					result.m_Messages.emplace_back(std::format("Missing callback for rule {}", ruleId), declarationSpan.m_Begin, declarationSpan);
					continue;
				}

				result.m_Lexer.registerRule(std::make_unique<CallbackRule>(ruleId, std::move(callbackItr->second)));
			}
			else
			{
				result.m_Messages.emplace_back(std::format("Rule {} is not a recognized declaration", rule), declarationSpan.m_Begin, declarationSpan, EMessageSeverity::Warning);
				continue;
			}
		}
	}

	LexerLexerResult LexerLexer::createLexer(const Lex& lex, std::unordered_map<std::string, CallbackRule::Callback> callbacks)
	{
		LexerLexerResult result;
		ScopeSettings    settings;
		handleScope(result, lex, lex.getRoot(), settings, callbacks);
		return result;
	}

	std::string handleMatcher(LexerCPPResult& result, const Lex& lex, const Node& node, ScopeSettings settings, std::string_view ruleId, std::string_view resultID, std::string_view stateID, std::string_view spanID, std::size_t depth = 1)
	{
		auto source = lex.getSource();

		auto rule = node.getRule();
		auto span = node.getSpan();
		if (rule == "RegexMatcher")
		{
			std::string indents = std::string(depth, '\t');

			std::ostringstream str;
			str << indents << "// Regex Matcher\n"
			    << indents << "{\n"
			    << indents << "\tstd::regex regex" << depth << " = std::regex(\"" << EscapeText(UnescapeText(source->getSpan(span))) << "\", std::regex_constants::ECMAScript | std::regex_constants::optimize);\n\n"
			    << indents << "\tstd::match_results<CommonLexer::SourceIterator> results;\n"
			    << indents << "\tif (std::regex_search(" << spanID << ".begin(" << stateID << ".m_Source), " << spanID << ".end(" << stateID << ".m_Source), results, regex" << depth << ", std::regex_constants::match_continuous))\n"
			    << indents << "\t{\n"
			    << indents << "\t\t" << resultID << " = { CommonLexer::EMatchStatus::Success, { results[0].first, results[0].second } };\n"
			    << indents << "\t}\n"
			    << indents << "\telse\n"
			    << indents << "\t{\n"
			    << indents << "\t\t" << stateID << ".m_Messages.emplace_back(\"Expected regex to succeed, but failed. Sadly I don't get regex error messages, maybe in the future ;), " << ruleId << "\", " << spanID << ".m_Begin, CommonLexer::SourceSpan { " << spanID << ".m_Begin, " << spanID << ".m_Begin });\n"
			    << indents << "\t\t" << resultID << " = { CommonLexer::EMatchStatus::Failure, { " << spanID << ".m_Begin, " << spanID << ".m_Begin } };\n"
			    << indents << "\t}\n"
			    << indents << "}";
			return str.str();
		}
		else if (rule == "TextMatcher")
		{
			std::string indents = std::string(depth, '\t');

			std::ostringstream str;
			str << indents << "// Text Matcher\n"
			    << indents << "{\n"
			    << indents << "\tauto itr = " << spanID << ".begin(" << stateID << ".m_Source);\n"
			    << indents << "\tauto end = " << spanID << ".end(" << stateID << ".m_Source);\n\n"
			    << indents << "\tstd::string_view text    = \"" << EscapeText(UnescapeText(source->getSpan(span))) << "\";\n"
			    << indents << "\tauto             textItr = text.begin();\n"
			    << indents << "\tauto             textEnd = text.end();\n\n"
			    << indents << "\twhile (textItr != textEnd)\n"
			    << indents << "\t{\n"
			    << indents << "\t\tif (itr == end)\n"
			    << indents << "\t\t{\n"
			    << indents << "\t\t\t" << stateID << ".m_Messages.emplace_back(std::format(\"Expected '{}' but got 'EOF', " << ruleId << "\", std::string_view { textItr, textEnd }), itr, CommonLexer::SourceSpan { " << spanID << ".m_Begin, itr });\n"
			    << indents << "\t\t\t" << resultID << " = { CommonLexer::EMatchStatus::Failure, { " << spanID << ".m_Begin, itr } };\n"
			    << indents << "\t\t\tbreak;\n"
			    << indents << "\t\t}\n\n"
			    << indents << "\t\tif (*itr != *textItr)\n"
			    << indents << "\t\t{\n"
			    << indents << "\t\t\t" << stateID << ".m_Messages.emplace_back(std::format(\"Expected '{}' but got '{}', " << ruleId << "\", *textItr, *itr), itr, CommonLexer::SourceSpan { " << spanID << ".m_Begin, itr });\n"
			    << indents << "\t\t\t" << resultID << " = { CommonLexer::EMatchStatus::Failure, { " << spanID << ".m_Begin, itr } };\n"
			    << indents << "\t\t}\n\n"
			    << indents << "\t\t++itr;\n"
			    << indents << "\t\t++textItr;\n"
			    << indents << "\t}\n"
			    << indents << "\tif (textItr == textEnd)\n"
			    << indents << "\t{\n"
			    << indents << "\t\t" << resultID << " = { CommonLexer::EMatchStatus::Success, { " << spanID << ".m_Begin, itr } };\n"
			    << indents << "\t}\n"
			    << indents << "}";
			return str.str();
		}
		else if (rule == "ReferenceMatcher")
		{
			auto ruleIdNode = node.getChild(0);
			if (!ruleIdNode || ruleIdNode->getRule() != "Identifier")
			{
				result.m_Messages.emplace_back("Unexpected ReferenceMatcher missing Identifier", span.m_Begin, span);
				return "/* ERROR: ReferenceMatcher failed */";
			}
			std::string indents = std::string(depth, '\t');

			std::ostringstream str;
			str << indents << "// ReferenceMatcher\n"
			    << indents << resultID << " = " << source->getSpan(ruleIdNode->getSpan()) << "Match(" << stateID << ", " << spanID << ");\n";
			return str.str();
		}
		else if (rule == "NamedGroupReferenceMatcher")
		{
			auto namedGroupId = node.getChild(0);
			if (!namedGroupId || namedGroupId->getRule() != "Identifier")
			{
				result.m_Messages.emplace_back("Unexpected NamedGroupReferenceMatcher missing Identifier", span.m_Begin, span);
				return "/* ERROR: NamedGroupReferenceMatcher failed */";
			}
			std::string indents = std::string(depth, '\t');

			std::ostringstream str;
			str << indents << "// NamedGroupReferenceMatcher\n"
			    << indents << "{\n"
			    << indents << "\tauto groupedSpan = " << stateID << ".m_Lexer->getGroupedValue(\"" << source->getSpan(namedGroupId->getSpan()) << "\");\n\n"
			    << indents << "\tauto itr = " << spanID << ".begin(" << stateID << ".m_Source);\n"
			    << indents << "\tauto end = " << spanID << ".end(" << stateID << ".m_Source);\n\n"
			    << indents << "\tauto textItr = groupedSpan.begin(" << stateID << ".m_Source);\n"
			    << indents << "\tauto textEnd = groupedSpan.end(" << stateID << ".m_Source);\n\n"
			    << indents << "\twhile (textItr != textEnd)\n"
			    << indents << "\t{\n"
			    << indents << "\t\tif (itr == end)\n"
			    << indents << "\t\t{\n"
			    << indents << "\t\t\t" << stateID << ".m_Messages.emplace_back(std::format(\"Expected '{}' but got 'EOF', " << ruleId << "\", std::string_view { textItr, textEnd }), itr, CommonLexer::SourceSpan { " << spanID << ".m_Begin, itr });\n"
			    << indents << "\t\t\t" << resultID << " = { CommonLexer::EMatchStatus::Failure, { " << spanID << ".m_Begin, itr } };\n"
			    << indents << "\t\t\tbreak;\n"
			    << indents << "\t\t}\n\n"
			    << indents << "\t\tif (*itr != *textItr)\n"
			    << indents << "\t\t{\n"
			    << indents << "\t\t\t" << stateID << ".m_Messages.emplace_back(std::format(\"Expected '{}' but got '{}', " << ruleId << "\", *textItr, *itr), itr, CommonLexer::SourceSpan { " << spanID << ".m_Begin, itr });\n"
			    << indents << "\t\t\t" << resultID << " = { CommonLexer::EMatchStatus::Failure, { " << spanID << ".m_Begin, itr } };\n"
			    << indents << "\t\t}\n\n"
			    << indents << "\t\t++itr;\n"
			    << indents << "\t\t++textItr;\n"
			    << indents << "\t}\n"
			    << indents << "\tif (textItr == textEnd)\n"
			    << indents << "\t{\n"
			    << indents << "\t\t" << resultID << " = { CommonLexer::EMatchStatus::Success, { " << spanID << ".m_Begin, itr } };\n"
			    << indents << "\t}\n"
			    << indents << "}";
			return str.str();
		}
		else if (rule == "NamedGroupMatcher")
		{
			auto namedGroupId = node.getChild(0);
			if (!namedGroupId || namedGroupId->getRule() != "Identifier")
			{
				result.m_Messages.emplace_back("Unexpected NamedGroupMatcher missing Identifier", span.m_Begin, span);
				return "/* ERROR: NamedGroupMatcher failed */";
			}
			auto namedGroupSpan = namedGroupId->getSpan();

			auto subMatcher = node.getChild(1);
			if (!subMatcher)
			{
				result.m_Messages.emplace_back("Unexpected NamedGroupMatcher missing Matcher", namedGroupSpan.m_End, span);
				return "/* ERROR: NamedGroupMatcher failed */";
			}
			auto subMatcherSpan = subMatcher->getSpan();

			std::string indents = std::string(depth, '\t');

			std::string newResultID = "result" + std::to_string(depth);

			std::ostringstream str;
			str << indents << "// NamedGroupMatcher\n"
			    << indents << "{\n"
			    << indents << "\tCommonLexer::MatchResult " << newResultID << " { CommonLexer::EMatchStatus::Failure, { " << spanID << ".m_Begin, " << spanID << ".m_Begin } };"
			    << handleMatcher(result, lex, *subMatcher, settings, ruleId, newResultID, stateID, spanID, depth + 1) << '\n'
			    << indents << "\tswitch (" << newResultID << ".m_Status)\n"
			    << indents << "\t{\n"
			    << indents << "\tcase CommonLexer::EMatchStatus::Failure:\n"
			    << indents << "\t\t" << resultID << " = { CommonLexer::EMatchStatus::Failure, " << newResultID << ".m_Span };\n"
			    << indents << "\t\tbreak;\n"
			    << indents << "\tcase CommonLexer::EMatchStatus::Skip:\n"
			    << indents << "\t\t" << resultID << " = { CommonLexer::EMatchStatus::Skip, " << newResultID << ".m_Span };\n"
			    << indents << "\t\tbreak;\n"
			    << indents << "\tcase CommonLexer::EMatchStatus::Success:\n"
			    << indents << "\t\t" << stateID << ".m_Lexer->setGroupedValue(\"" << source->getSpan(namedGroupSpan) << "\", " << newResultID << ".m_Span);\n"
			    << indents << "\t\t" << resultID << " = { CommonLexer::EMatchStatus::Success, " << newResultID << ".m_Span };\n"
			    << indents << "\t\tbreak;\n"
			    << indents << "\tdefault:\n"
			    << indents << "\t\t" << resultID << " = { CommonLexer::EMatchStatus::Failure, " << newResultID << ".m_Span };\n"
			    << indents << "\t\tbreak;\n"
			    << indents << "\t}\n"
			    << indents << "}";
			return str.str();
		}
		else if (rule == "Group")
		{
			auto subMatcher = node.getChild(0);
			if (!subMatcher)
			{
				result.m_Messages.emplace_back("Unexpected Group missing Matcher", span.m_Begin, span);
				return "/* ERROR: Group failed */";
			}

			return handleMatcher(result, lex, *subMatcher, settings, ruleId, resultID, stateID, spanID, depth);
		}
		else if (rule == "ForcedSpaceMatcher")
		{
			auto subMatcher = node.getChild(0);
			if (!subMatcher)
			{
				result.m_Messages.emplace_back("Unexpected ForcedSpaceMatcher missing Matcher", span.m_Begin, span);
				return "/* ERROR: ForcedSpaceMatcher failed */";
			}
			std::string indents = std::string(depth, '\t');

			std::string failedID    = "failed" + std::to_string(depth);
			std::string subSpanID   = "subSpan" + std::to_string(depth);
			std::string totalSpanID = "totalSpan" + std::to_string(depth);

			std::ostringstream str;
			str << indents << "// ForcedSpaceMatcher\n"
			    << indents << "{\n"
			    << indents << "\tbool " << failedID << " = false;\n\n"
			    << indents << "\tCommonLexer::SourceSpan " << subSpanID << " = " << spanID << ";\n"
			    << indents << "\tCommonLexer::SourceSpan " << totalSpanID << " = { " << spanID << ".m_Begin, " << spanID << ".m_Begin };\n";

			if (settings.m_SpaceDirection == ESpaceDirection::Left || settings.m_SpaceDirection == ESpaceDirection::Both)
			{
				str << indents << "\t{\n"
				    << indents << "\t\tstd::size_t i = 0;\n\n"
				    << indents << "\t\tauto begin = " << stateID << ".m_SourceSpan.begin(" << stateID << ".m_Source);\n"
				    << indents << "\t\tauto itr   = " << subSpanID << ".begin(" << stateID << ".m_Source);\n"
				    << indents << "\t\tauto end   = " << subSpanID << ".end(" << stateID << ".m_Source);\n"
				    << indents << "\t\t--itr;\n"
				    << indents << "\t\tif (itr < begin)\n"
				    << indents << "\t\t\t++itr;\n"
				    << indents << "\t\twhile (itr != end)\n"
				    << indents << "\t\t{\n"
				    << indents << "\t\t\tbool breakOut = true;\n"
				    << indents << "\t\t\tswitch (*itr)\n"
				    << indents << "\t\t\t{\n";

				switch (settings.m_SpaceMethod)
				{
				case ESpaceMethod::Normal:
					str << indents << "\t\t\tcase '\\t': [[fallthrough]];\n"
					    << indents << "\t\t\tcase ' ':\n";
					break;
				case ESpaceMethod::Whitespace:
					str << indents << "\t\t\tcase '\\t': [[fallthrough]];\n"
					    << indents << "\t\t\tcase '\\n': [[fallthrough]];\n"
					    << indents << "\t\t\tcase '\\v': [[fallthrough]];\n"
					    << indents << "\t\t\tcase '\\f': [[fallthrough]];\n"
					    << indents << "\t\t\tcase '\\r': [[fallthrough]];\n"
					    << indents << "\t\t\tcase ' ':\n";
					break;
				}

				str << indents << "\t\t\t\tbreakOut = false;\n"
				    << indents << "\t\t\t\tbreak;\n"
				    << indents << "\t\t\t}\n"
				    << indents << "\t\t\tif (i > 0 && breakOut)\n"
				    << indents << "\t\t\t\tbreak;\n"
				    << indents << "\t\t\t++itr;\n"
				    << indents << "\t\t\tif (!breakOut)\n"
				    << indents << "\t\t\t\t++i;\n"
				    << indents << "\t\t}\n\n"
				    << indents << "\t\tif (i == 0)\n"
				    << indents << "\t\t{\n"
				    << indents << "\t\t\t" << stateID << ".m_Messages.emplace_back(std::format(\"Expected ";
				switch (settings.m_SpaceMethod)
				{
				case ESpaceMethod::Normal:
					str << "space or tab";
					break;
				case ESpaceMethod::Whitespace:
					str << "spacee, tab, vertical tab, form feed, carriage return or line feed";
					break;
				}
				str << " but got '{}'\", itr != end ? std::string { *itr } : \"EOF\"), " << subSpanID << ".m_Begin, CommonLexer::SourceSpan { " << subSpanID << ".m_Begin, " << subSpanID << ".m_Begin });\n"
				    << indents << "\t\t\t" << resultID << " = { CommonLexer::EMatchStatus::Failure, { " << spanID << ".m_Begin, itr } };\n"
				    << indents << "\t\t\t" << failedID << " = true;\n"
				    << indents << "\t\t}\n"
				    << indents << "\t\telse\n"
				    << indents << "\t\t{\n"
				    << indents << "\t\t\t" << subSpanID << ".m_Begin = " << totalSpanID << ".m_End = itr;\n"
				    << indents << "\t\t}\n"
				    << indents << "\t}\n";
			}

			std::string newResultID = "result" + std::to_string(depth);

			str << indents << "\tif (!" << failedID << ")\n"
			    << indents << "\t{\n"
			    << indents << "\t\tCommonLexer::MatchResult " << newResultID << " { CommonLexer::EMatchStatus::Failure, { " << spanID << ".m_Begin, " << spanID << ".m_Begin } };\n"
			    << handleMatcher(result, lex, *subMatcher, settings, ruleId, newResultID, stateID, subSpanID, depth + 2) << '\n'
			    << indents << "\t\tswitch (" << newResultID << ".m_Status)\n"
			    << indents << "\t\t{\n"
			    << indents << "\t\tcase CommonLexer::EMatchStatus::Failure:\n"
			    << indents << "\t\t\t" << resultID << " = { CommonLexer::EMatchStatus::Failure, { " << spanID << ".m_Begin, " << newResultID << ".m_Span.m_End } };\n"
			    << indents << "\t\t\t" << failedID << " = true;\n"
			    << indents << "\t\t\tbreak;\n"
			    << indents << "\t\tcase CommonLexer::EMatchStatus::Success:\n"
			    << indents << "\t\t\t" << subSpanID << ".m_Begin = " << totalSpanID << ".m_End = " << resultID << ".m_Span.m_End;\n"
			    << indents << "\t\t\tbreak;\n"
			    << indents << "\t\tdefault: break;\n"
			    << indents << "\t\t}\n";

			if (settings.m_SpaceDirection == ESpaceDirection::Right || settings.m_SpaceDirection == ESpaceDirection::Both)
			{
				str << indents << "\t\tif (!" << failedID << ")\n"
				    << indents << "\t\t{\n"
				    << indents << "\t\t\tstd::size_t i = 0;\n\n"
				    << indents << "\t\t\tauto begin = " << stateID << ".m_SourceSpan.begin(" << stateID << ".m_Source);\n"
				    << indents << "\t\t\tauto itr   = " << subSpanID << ".begin(" << stateID << ".m_Source);\n"
				    << indents << "\t\t\tauto end   = " << subSpanID << ".end(" << stateID << ".m_Source);\n"
				    << indents << "\t\t\t--itr;\n"
				    << indents << "\t\t\tif (itr < begin)\n"
				    << indents << "\t\t\t\t++itr;\n"
				    << indents << "\t\t\twhile (itr != end)"
				    << indents << "\t\t\t{\n"
				    << indents << "\t\t\t\tbool breakOut = true;\n"
				    << indents << "\t\t\t\tswitch (*itr)\n"
				    << indents << "\t\t\t\t{\n";

				switch (settings.m_SpaceMethod)
				{
				case ESpaceMethod::Normal:
					str << indents << "\t\t\t\tcase '\\t': [[fallthrough]];\n"
					    << indents << "\t\t\t\tcase ' ':\n";
					break;
				case ESpaceMethod::Whitespace:
					str << indents << "\t\t\t\tcase '\\t': [[fallthrough]];\n"
					    << indents << "\t\t\t\tcase '\\n': [[fallthrough]];\n"
					    << indents << "\t\t\t\tcase '\\v': [[fallthrough]];\n"
					    << indents << "\t\t\t\tcase '\\f': [[fallthrough]];\n"
					    << indents << "\t\t\t\tcase '\\r': [[fallthrough]];\n"
					    << indents << "\t\t\t\tcase ' ':\n";
					break;
				}

				str << indents << "\t\t\t\t\tbreakOut = false;\n"
				    << indents << "\t\t\t\t\tbreak;\n"
				    << indents << "\t\t\t\t}\n"
				    << indents << "\t\t\t\tif (i > 0 && breakOut)\n"
				    << indents << "\t\t\t\t\tbreak;\n"
				    << indents << "\t\t\t\t++itr;\n"
				    << indents << "\t\t\t\tif (!breakOut)\n"
				    << indents << "\t\t\t\t\t++i;\n"
				    << indents << "\t\t\t}\n\n"
				    << indents << "\t\t\tif (i == 0)\n"
				    << indents << "\t\t\t{\n"
				    << indents << "\t\t\t\t" << stateID << ".m_Messages.emplace_back(std::format(\"Expected ";
				switch (settings.m_SpaceMethod)
				{
				case ESpaceMethod::Normal:
					str << "space or tab";
					break;
				case ESpaceMethod::Whitespace:
					str << "spacee, tab, vertical tab, form feed, carriage return or line feed";
					break;
				}
				str << " but got '{}'\", itr != end ? std::string { *itr } : \"EOF\"), " << subSpanID << ".m_Begin, CommonLexer::SourceSpan { " << subSpanID << ".m_Begin, " << subSpanID << ".m_Begin });\n"
				    << indents << "\t\t\t\t" << resultID << " = { CommonLexer::EMatchStatus::Failure, { " << spanID << ".m_Begin, itr } };\n"
				    << indents << "\t\t\t\t" << failedID << " = true;\n"
				    << indents << "\t\t\t}\n"
				    << indents << "\t\t\telse\n"
				    << indents << "\t\t\t{\n"
				    << indents << "\t\t\t\t" << subSpanID << ".m_Begin = " << totalSpanID << ".m_End = itr;\n"
				    << indents << "\t\t\t}\n"
				    << indents << "\t\t}\n";
			}

			str << indents << "\t}\n"
			    << indents << "\tif (!" << failedID << ")\n"
			    << indents << "\t\t" << resultID << " = { CommonLexer::EMatchStatus::Success, " << totalSpanID << " };\n"
			    << indents << "}";
			return str.str();
		}
		else if (rule == "LenientSpaceMatcher")
		{
			auto subMatcher = node.getChild(0);
			if (!subMatcher)
			{
				result.m_Messages.emplace_back("Unexpected ForcedSpaceMatcher missing Matcher", span.m_Begin, span);
				return "/* ERROR: LenientSpaceMatcher failed */";
			}
			std::string indents = std::string(depth, '\t');

			std::string failedID    = "failed" + std::to_string(depth);
			std::string subSpanID   = "subSpan" + std::to_string(depth);
			std::string totalSpanID = "totalSpan" + std::to_string(depth);

			std::ostringstream str;
			str << indents << "// LenientSpaceMatcher\n"
			    << indents << "{\n"
			    << indents << "\tbool " << failedID << " = false;\n\n"
			    << indents << "\tCommonLexer::SourceSpan " << subSpanID << " = " << spanID << ";\n"
			    << indents << "\tCommonLexer::SourceSpan " << totalSpanID << " = { " << spanID << ".m_Begin, " << spanID << ".m_Begin };\n";

			if (settings.m_SpaceDirection == ESpaceDirection::Left || settings.m_SpaceDirection == ESpaceDirection::Both)
			{
				str << indents << "\t{\n"
				    << indents << "\t\tstd::size_t i = 0;\n\n"
				    << indents << "\t\tauto begin = " << stateID << ".m_SourceSpan.begin(" << stateID << ".m_Source);\n"
				    << indents << "\t\tauto itr   = " << subSpanID << ".begin(" << stateID << ".m_Source);\n"
				    << indents << "\t\tauto end   = " << subSpanID << ".end(" << stateID << ".m_Source);\n"
				    << indents << "\t\t--itr;\n"
				    << indents << "\t\tif (itr < begin)\n"
				    << indents << "\t\t\t++itr;\n"
				    << indents << "\t\twhile (itr != end)\n"
				    << indents << "\t\t{\n"
				    << indents << "\t\t\tbool breakOut = true;\n"
				    << indents << "\t\t\tswitch (*itr)\n"
				    << indents << "\t\t\t{\n";

				switch (settings.m_SpaceMethod)
				{
				case ESpaceMethod::Normal:
					str << indents << "\t\t\tcase '\\t': [[tallthrough]];\n"
					    << indents << "\t\t\tcase ' ':\n";
					break;
				case ESpaceMethod::Whitespace:
					str << indents << "\t\t\tcase '\\t': [[fallthrough]];\n"
					    << indents << "\t\t\tcase '\\n': [[fallthrough]];\n"
					    << indents << "\t\t\tcase '\\v': [[fallthrough]];\n"
					    << indents << "\t\t\tcase '\\f': [[fallthrough]];\n"
					    << indents << "\t\t\tcase '\\r': [[fallthrough]];\n"
					    << indents << "\t\t\tcase ' ':\n";
					break;
				}

				str << indents << "\t\t\t\tbreakOut = false;\n"
				    << indents << "\t\t\t\tbreak;\n"
				    << indents << "\t\t\t}\n"
				    << indents << "\t\t\tif (i > 0 && breakOut)\n"
				    << indents << "\t\t\t\tbreak;\n"
				    << indents << "\t\t\t++itr;\n"
				    << indents << "\t\t}\n\n"
				    << indents << "\t\t" << subSpanID << ".m_Begin = " << totalSpanID << ".m_End = itr;\n"
				    << indents << "\t}\n";
			}

			std::string newResultID = "result" + std::to_string(depth);

			str << indents << "\tif (!" << failedID << ")\n"
			    << indents << "\t{\n"
			    << indents << "\t\tCommonLexer::MatchResult " << newResultID << " { CommonLexer::EMatchStatus::Failure, { " << spanID << ".m_Begin, " << spanID << ".m_Begin } };\n"
			    << handleMatcher(result, lex, *subMatcher, settings, ruleId, newResultID, stateID, subSpanID, depth + 2) << '\n'
			    << indents << "\t\tswitch (" << newResultID << ".m_Status)\n"
			    << indents << "\t\t{\n"
			    << indents << "\t\tcase CommonLexer::EMatchStatus::Failure:\n"
			    << indents << "\t\t\t" << resultID << " = { CommonLexer::EMatchStatus::Failure, { " << spanID << ".m_Begin, " << newResultID << ".m_Span.m_End } };\n"
			    << indents << "\t\t\t" << failedID << " = true;\n"
			    << indents << "\t\t\tbreak;\n"
			    << indents << "\t\tcase CommonLexer::EMatchStatus::Success:\n"
			    << indents << "\t\t\t" << subSpanID << ".m_Begin = " << totalSpanID << ".m_End = " << resultID << ".m_Span.m_End;\n"
			    << indents << "\t\t\tbreak;\n"
			    << indents << "\t\tdefault: break;\n"
			    << indents << "\t\t}\n";

			if (settings.m_SpaceDirection == ESpaceDirection::Right || settings.m_SpaceDirection == ESpaceDirection::Both)
			{
				str << indents << "\t\tif (!" << failedID << ")\n"
				    << indents << "\t\t{\n"
				    << indents << "\t\t\tstd::size_t i = 0;\n\n"
				    << indents << "\t\t\tauto begin = " << stateID << ".m_SourceSpan.begin(" << stateID << ".m_Source);\n"
				    << indents << "\t\t\tauto itr   = " << subSpanID << ".begin(" << stateID << ".m_Source);\n"
				    << indents << "\t\t\tauto end   = " << subSpanID << ".end(" << stateID << ".m_Source);\n"
				    << indents << "\t\t\t--itr;\n"
				    << indents << "\t\t\tif (itr < begin)\n"
				    << indents << "\t\t\t\t++itr;\n"
				    << indents << "\t\t\twhile (itr != end)\n"
				    << indents << "\t\t\t{\n"
				    << indents << "\t\t\t\tbool breakOut = true;\n"
				    << indents << "\t\t\t\tswitch (*itr)\n"
				    << indents << "\t\t\t\t{\n";

				switch (settings.m_SpaceMethod)
				{
				case ESpaceMethod::Normal:
					str << indents << "\t\t\t\tcase '\\t': [[fallthrough]];\n"
					    << indents << "\t\t\t\tcase ' ':\n";
					break;
				case ESpaceMethod::Whitespace:
					str << indents << "\t\t\t\tcase '\\t': [[fallthrough]];\n"
					    << indents << "\t\t\t\tcase '\\n': [[fallthrough]];\n"
					    << indents << "\t\t\t\tcase '\\v': [[fallthrough]];\n"
					    << indents << "\t\t\t\tcase '\\f': [[fallthrough]];\n"
					    << indents << "\t\t\t\tcase '\\r': [[fallthrough]];\n"
					    << indents << "\t\t\t\tcase ' ':\n";
					break;
				}

				str << indents << "\t\t\t\t\tbreakOut = false;\n"
				    << indents << "\t\t\t\t\tbreak;\n"
				    << indents << "\t\t\t\t}\n"
				    << indents << "\t\t\t\tif (i > 0 && breakOut)\n"
				    << indents << "\t\t\t\t\tbreak;\n"
				    << indents << "\t\t\t\t++itr;\n"
				    << indents << "\t\t\t}\n\n"
				    << indents << "\t\t\t" << subSpanID << ".m_Begin = " << totalSpanID << ".m_End = itr;\n"
				    << indents << "\t\t}\n";
			}

			str << indents << "\t}\n"
			    << indents << "\tif (!" << failedID << ")\n"
			    << indents << "\t\t" << resultID << " = { CommonLexer::EMatchStatus::Success, " << totalSpanID << " };\n"
			    << indents << "}";
			return str.str();
		}
		else if (rule == "NegativeMatcher")
		{
			auto subMatcher = node.getChild(0);
			if (!subMatcher)
			{
				result.m_Messages.emplace_back("Unexpected NegativeMatcher missing Matcher", span.m_Begin, span);
				return "/* ERROR: NegativeMatcher failed */";
			}
			std::string indents = std::string(depth, '\t');

			std::string tempNodeID  = "tempNode" + std::to_string(depth);
			std::string tempStateID = "tempState" + std::to_string(depth);

			std::ostringstream str;
			str << indents << "// NegativeMatcher\n"
			    << indents << "{\n"
			    << indents << "\tCommonLexer::Node         " << tempNodeID << " { *" << stateID << ".m_Lexer };\n"
			    << indents << "\tCommonLexer::MatcherState " << tempStateID << " { {}, &" << tempNodeID << ", " << stateID << ".m_Lexer, " << stateID << ".m_Source, " << stateID << ".m_SourceSpan, " << stateID << ".m_CurrentRule, " << stateID << ".m_RuleBegin };\n"
			    << handleMatcher(result, lex, *subMatcher, settings, ruleId, resultID, tempStateID, spanID, depth + 1) << '\n'
			    << indents << "\tswitch (" << resultID << ".m_Status)\n"
			    << indents << "\t{\n"
			    << indents << "\tcase CommonLexer::EMatchStatus::Failure: [[fallthrough]];\n"
			    << indents << "\tcase CommonLexer::EMatchStatus::Skip:\n"
			    << indents << "\t\t" << resultID << "= { CommonLexer::EMatchStatus::Success, { " << spanID << ".m_Begin, " << spanID << ".m_Begin } };\n"
			    << indents << "\t\tbreak;\n"
			    << indents << "\tcase CommonLexer::EMatchStatus::Success:\n"
			    << indents << "\t\t" << stateID << ".m_Messages.emplace_back(\"Did not expect following sequence\", " << spanID << ".m_Begin, " << resultID << ".m_Span);\n"
			    << indents << "\t\t" << resultID << "= { CommonLexer::EMatchStatus::Failure, " << resultID << ".m_Span };\n"
			    << indents << "\t\tbreak;\n"
			    << indents << "\tdefault:\n"
			    << indents << "\t\t" << resultID << "= { CommonLexer::EMatchStatus::Failure, " << resultID << ".m_Span };\n"
			    << indents << "\t}\n"
			    << indents << "}";
			return str.str();
		}
		else if (rule == "OptionalMatcher")
		{
			auto subMatcher = node.getChild(0);
			if (!subMatcher)
			{
				result.m_Messages.emplace_back("Unexpected OptionalMatcher missing Matcher", span.m_Begin, span);
				return "/* ERROR: OptionalMatcher failed */";
			}
			std::string indents = std::string(depth, '\t');

			std::ostringstream str;
			str << indents << "// OptionalMatcher\n"
			    << indents << "{\n"
			    << handleMatcher(result, lex, *subMatcher, settings, ruleId, resultID, stateID, spanID, depth + 1) << '\n'
			    << indents << "\tswitch (" << resultID << ".m_Status)\n"
			    << indents << "\t{\n"
			    << indents << "\tcase CommonLexer::EMatchStatus::Failure: [[fallthrough]];\n"
			    << indents << "\tcase CommonLexer::EMatchStatus::Skip:\n"
			    << indents << "\t\t" << resultID << ".m_Status = CommonLexer::EMatchStatus::Skip;\n"
			    << indents << "\t\tbreak;\n"
			    << indents << "\tdefault:\n"
			    << indents << "\t\t" << resultID << ".m_Status = CommonLexer::EMatchStatus::Skip;\n"
			    << indents << "\t}\n"
			    << indents << "}\n";
			return str.str();
		}
		else if (rule == "RangeMatcher")
		{
			auto subMatcher = node.getChild(0);
			if (!subMatcher)
			{
				result.m_Messages.emplace_back("Unexpected RangeMatcher missing Matcher", span.m_Begin, span);
				return "/* ERROR: RangeMatcher failed */";
			}
			auto subMatcherSpan = subMatcher->getSpan();

			auto lowerBoundsNode = node.getChild(1);
			if (!lowerBoundsNode || lowerBoundsNode->getRule() != "Integer")
			{
				result.m_Messages.emplace_back("Unexpected RangeMatcher missing lower bounds", subMatcherSpan.m_Begin, span);
				return "/* ERROR: RangeMatcher failed */";
			}
			auto lowerBoundsSpan = lowerBoundsNode->getSpan();
			auto lowerBounds     = ReadUInt(source->getSpan(lowerBoundsSpan));

			auto upperBoundsNode = node.getChild(2);
			if (!upperBoundsNode || upperBoundsNode->getRule() != "Integer")
			{
				result.m_Messages.emplace_back("Unexpected RangeMatcher missing upper bounds", subMatcherSpan.m_Begin, span);
				return "/* ERROR: RangeMatcher failed */";
			}
			auto upperBoundsSpan = upperBoundsNode->getSpan();
			auto upperBounds     = ReadUInt(source->getSpan(upperBoundsSpan));
			if (upperBounds < lowerBounds)
			{
				result.m_Messages.emplace_back("Range Matcher upper bounds is smaller than lower bounds, assuming UpperBounds=LowerBounds", upperBoundsSpan.m_Begin, span, EMessageSeverity::Warning);
				upperBounds = lowerBounds;
			}
			std::string indents = std::string(depth, '\t');

			std::string matchesID   = "matches" + std::to_string(depth);
			std::string subSpanID   = "subSpan" + std::to_string(depth);
			std::string totalSpanID = "totalSpan" + std::to_string(depth);
			std::string tempStateID = "tempState" + std::to_string(depth);
			std::string newResultID = "result" + std::to_string(depth);

			std::ostringstream str;
			str << indents << "// RangeMatcher\n"
			    << indents << "{\n";
			if (lowerBounds != 0 || upperBounds != ~0ULL)
				str << indents << "\tstd::size_t " << matchesID << " = 0;\n";

			str << indents << "\tCommonLexer::SourceSpan " << subSpanID << " = " << spanID << ";\n"
			    << indents << "\tCommonLexer::SourceSpan " << totalSpanID << " = { " << spanID << ".m_Begin, " << spanID << ".m_Begin };\n"
			    << indents << "\tCommonLexer::MatcherState " << tempStateID << " = { {}, " << stateID << ".m_ParentNode, " << stateID << ".m_Lexer, " << stateID << ".m_Source, " << stateID << ".m_SourceSpan, " << stateID << ".m_CurrentRule, " << stateID << ".m_RuleBegin };\n"
			    << indents << "\tCommonLexer::MatchResult " << newResultID << " { CommonLexer::EMatchStatus::Failure, { " << spanID << ".m_Begin, " << spanID << ".m_Begin } };\n";

			if (upperBounds != ~0ULL)
				str << indents << "\twhile (" << matchesID << " < " << std::to_string(upperBounds) << ")\n";
			else
				str << indents << "\twhile (true)\n";
			str << indents << "\t{\n"
			    << handleMatcher(result, lex, *subMatcher, settings, ruleId, newResultID, tempStateID, subSpanID, depth + 2) << '\n'
			    << indents << "\t\tif (" << newResultID << ".m_Status != CommonLexer::EMatchStatus::Success)\n"
			    << indents << "\t\t\tbreak;\n"
			    << indents << "\t\t" << subSpanID << ".m_Begin = " << totalSpanID << ".m_End = " << newResultID << ".m_Span.m_End;\n";
			if (lowerBounds != 0 || upperBounds != ~0ULL)
				str << indents << "\t\t++" << matchesID << ";\n";
			str << indents << "\t\tif (" << subSpanID << ".length() == 0)\n"
			    << indents << "\t\t\tbreak;\n"
			    << indents << "\t}\n"
			    << indents << "\tif (" << totalSpanID << ".m_End.m_Index < " << spanID << ".m_End.m_Index)\n"
			    << indents << "\t{\n"
			    << indents << "\t\t" << stateID << ".m_Messages.reserve(" << stateID << ".m_Messages.size() + " << tempStateID << ".m_Messages.size());\n"
			    << indents << "\t\tfor (auto& message : " << tempStateID << ".m_Messages)\n"
			    << indents << "\t\t\t" << tempStateID << ".m_Messages.push_back(std::move(message));\n"
			    << indents << "\t}\n";

			if (lowerBounds != 0)
				str << indents << "\tif (" << matchesID << " < " << std::to_string(lowerBounds) << ")\n"
				    << indents << "\t{\n"
				    << indents << "\t\t" << stateID << ".m_Messages.emplace_back(std::format(\"Expected at least " << std::to_string(lowerBounds) << " matches but only got {} matches\", " << matchesID << "), " << subSpanID << ".m_End, " << subSpanID << ");\n"
				    << indents << "\t\t" << resultID << " = { CommonLexer::EMatchResult::Failure, " << totalSpanID << " };\n"
				    << indents << "\t}\n"
				    << indents << "\telse\n"
				    << indents << "\t{\n"
				    << indents << "\t\t" << resultID << " = { CommonLexer::EMatchStatus::Success, " << totalSpanID << " };\n"
				    << indents << "\t}\n";
			else
				str << indents << "\t" << resultID << " = { CommonLexer::EMatchStatus::Success, " << totalSpanID << " };\n";

			str << indents << "}";
			return str.str();
		}
		else if (rule == "ExactAmount")
		{
			auto subMatcher = node.getChild(0);
			if (!subMatcher)
			{
				result.m_Messages.emplace_back("Unexpected ExactAmount missing Matcher", span.m_Begin, span);
				return "/* ERROR: ExactAmount failed */";
			}
			auto subMatcherSpan = subMatcher->getSpan();

			auto amountNode = node.getChild(1);
			if (!amountNode || amountNode->getRule() != "Integer")
			{
				result.m_Messages.emplace_back("Unexpected ExactAmount missing amount", subMatcherSpan.m_Begin, span);
				return "/* ERROR: ExactAmount failed */";
			}
			auto amount = ReadUInt(source->getSpan(amountNode->getSpan()));

			std::string indents = std::string(depth, '\t');

			std::string matchesID   = "matches" + std::to_string(depth);
			std::string subSpanID   = "subSpan" + std::to_string(depth);
			std::string totalSpanID = "totalSpan" + std::to_string(depth);
			std::string tempStateID = "tempState" + std::to_string(depth);
			std::string newResultID = "result" + std::to_string(depth);

			std::ostringstream str;
			str << indents << "// ExactAmount\n"
			    << indents << "{\n"
			    << indents << "\tstd::size_t " << matchesID << " = 0;\n"
			    << indents << "\tCommonLexer::SourceSpan " << subSpanID << " = " << spanID << ";\n"
			    << indents << "\tCommonLexer::SourceSpan " << totalSpanID << " = { " << spanID << ".m_Begin, " << spanID << ".m_Begin };\n"
			    << indents << "\tCommonLexer::MatcherState " << tempStateID << " = { {}, " << stateID << ".m_ParentNode, " << stateID << ".m_Lexer, " << stateID << ".m_Source, " << stateID << ".m_SourceSpan, " << stateID << ".m_CurrentRule, " << stateID << ".m_RuleBegin };\n"
			    << indents << "\tCommonLexer::MatchResult " << newResultID << " { CommonLexer::EMatchStatus::Failure, { " << spanID << ".m_Begin, " << spanID << ".m_Begin } };\n"
			    << indents << "\twhile (" << matchesID << " < " << std::to_string(amount) << ")\n"
			    << indents << "\t{\n"
			    << handleMatcher(result, lex, *subMatcher, settings, ruleId, newResultID, tempStateID, subSpanID, depth + 2) << '\n'
			    << indents << "\t\tif (" << newResultID << ".m_Status != CommonLexer::EMatchStatus::Success)\n"
			    << indents << "\t\t\tbreak;\n"
			    << indents << "\t\t" << subSpanID << ".m_Begin = " << totalSpanID << ".m_End = " << newResultID << ".m_Span.m_End;\n"
			    << indents << "\t\t++" << matchesID << ";\n"
			    << indents << "\t\tif (" << subSpanID << ".length() == 0)\n"
			    << indents << "\t\t\tbreak;\n"
			    << indents << "\t}\n"
			    << indents << "\tif (" << totalSpanID << ".m_End.m_Index < " << spanID << ".m_End.m_Index)\n"
			    << indents << "\t{\n"
			    << indents << "\t\t" << stateID << ".m_Messages.reserve(" << stateID << ".m_Messages.size() + " << tempStateID << ".m_Messages.size());\n"
			    << indents << "\t\tfor (auto& message : " << tempStateID << ".m_Messages)\n"
			    << indents << "\t\t\t" << tempStateID << ".m_Messages.push_back(std::move(message));\n"
			    << indents << "\t}\n"
			    << indents << "\tif (" << matchesID << " < " << std::to_string(amount) << ")\n"
			    << indents << "\t{\n"
			    << indents << "\t\t" << stateID << ".m_Messages.emplace_back(std::format(\"Expected exactly " << std::to_string(amount) << " matches but only got {} matches\", " << matchesID << "), " << subSpanID << ".m_End, " << subSpanID << ");\n"
			    << indents << "\t\t" << resultID << " = { CommonLexer::EMatchStatus::Failure, " << totalSpanID << " };\n"
			    << indents << "\t}\n"
			    << indents << "\telse\n"
			    << indents << "\t{\n"
			    << indents << "\t\t" << resultID << " = { CommonLexer::EMatchStatus::Success, " << totalSpanID << " };\n"
			    << indents << "\t}\n"
			    << indents << "}";
			return str.str();
		}
		else if (rule == "OneOrMore")
		{
			auto subMatcher = node.getChild(0);
			if (!subMatcher)
			{
				result.m_Messages.emplace_back("Unexpected ExactAmount missing Matcher", span.m_Begin, span);
				return "/* ERROR: OneOrMore failed */";
			}
			auto subMatcherSpan = subMatcher->getSpan();

			std::string indents = std::string(depth, '\t');

			std::string matchesID   = "matches" + std::to_string(depth);
			std::string subSpanID   = "subSpan" + std::to_string(depth);
			std::string totalSpanID = "totalSpan" + std::to_string(depth);
			std::string tempStateID = "tempState" + std::to_string(depth);
			std::string newResultID = "result" + std::to_string(depth);

			std::ostringstream str;
			str << indents << "// OneOrMore\n"
			    << indents << "{\n"
			    << indents << "\tstd::size_t " << matchesID << " = 0;\n"
			    << indents << "\tCommonLexer::SourceSpan " << subSpanID << " = " << spanID << ";\n"
			    << indents << "\tCommonLexer::SourceSpan " << totalSpanID << " = { " << spanID << ".m_Begin, " << spanID << ".m_Begin };\n"
			    << indents << "\tCommonLexer::MatcherState " << tempStateID << " = { {}, " << stateID << ".m_ParentNode, " << stateID << ".m_Lexer, " << stateID << ".m_Source, " << stateID << ".m_SourceSpan, " << stateID << ".m_CurrentRule, " << stateID << ".m_RuleBegin };\n"
			    << indents << "\tCommonLexer::MatchResult " << newResultID << " { CommonLexer::EMatchStatus::Failure, { " << spanID << ".m_Begin, " << spanID << ".m_Begin } };\n"
			    << indents << "\twhile (true)\n"
			    << indents << "\t{\n"
			    << handleMatcher(result, lex, *subMatcher, settings, ruleId, newResultID, tempStateID, subSpanID, depth + 2) << '\n'
			    << indents << "\t\tif (" << newResultID << ".m_Status != CommonLexer::EMatchStatus::Success)\n"
			    << indents << "\t\t\tbreak;\n"
			    << indents << "\t\t" << subSpanID << ".m_Begin = " << totalSpanID << ".m_End = " << newResultID << ".m_Span.m_End;\n"
			    << indents << "\t\t++" << matchesID << ";\n"
			    << indents << "\t\tif (" << subSpanID << ".length() == 0)\n"
			    << indents << "\t\t\tbreak;\n"
			    << indents << "\t}\n"
			    << indents << "\tif (" << totalSpanID << ".m_End.m_Index < " << spanID << ".m_End.m_Index)\n"
			    << indents << "\t{\n"
			    << indents << "\t\t" << stateID << ".m_Messages.reserve(" << stateID << ".m_Messages.size() + " << tempStateID << ".m_Messages.size());\n"
			    << indents << "\t\tfor (auto& message : " << tempStateID << ".m_Messages)\n"
			    << indents << "\t\t\t" << tempStateID << ".m_Messages.push_back(std::move(message));\n"
			    << indents << "\t}\n"
			    << indents << "\tif (" << matchesID << " < 1)\n"
			    << indents << "\t{\n"
			    << indents << "\t\t" << stateID << ".m_Messages.emplace_back(std::format(\"Expected one or more matches but only got {} matches\", " << matchesID << "), " << subSpanID << ".m_End, " << subSpanID << ");\n"
			    << indents << "\t\t" << resultID << " = { CommonLexer::EMatchStatus::Failure, " << totalSpanID << " };\n"
			    << indents << "\t}\n"
			    << indents << "\telse\n"
			    << indents << "\t{\n"
			    << indents << "\t\t" << resultID << " = { CommonLexer::EMatchStatus::Success, " << totalSpanID << " };\n"
			    << indents << "\t}\n"
			    << indents << "}";
			return str.str();
		}
		else if (rule == "ZeroOrMore")
		{
			auto subMatcher = node.getChild(0);
			if (!subMatcher)
			{
				result.m_Messages.emplace_back("Unexpected ExactAmount missing Matcher", span.m_Begin, span);
				return "/* ERROR: ZeroOrMore failed */";
			}
			auto subMatcherSpan = subMatcher->getSpan();

			std::string indents = std::string(depth, '\t');

			std::string matchesID   = "matches" + std::to_string(depth);
			std::string subSpanID   = "subSpan" + std::to_string(depth);
			std::string totalSpanID = "totalSpan" + std::to_string(depth);
			std::string tempStateID = "tempState" + std::to_string(depth);
			std::string newResultID = "result" + std::to_string(depth);

			std::ostringstream str;
			str << indents << "// ZeroOrMore\n"
			    << indents << "{\n"
			    << indents << "\tCommonLexer::SourceSpan " << subSpanID << " = " << spanID << ";\n"
			    << indents << "\tCommonLexer::SourceSpan " << totalSpanID << " = { " << spanID << ".m_Begin, " << spanID << ".m_Begin };\n"
			    << indents << "\tCommonLexer::MatcherState " << tempStateID << " = { {}, " << stateID << ".m_ParentNode, " << stateID << ".m_Lexer, " << stateID << ".m_Source, " << stateID << ".m_SourceSpan, " << stateID << ".m_CurrentRule, " << stateID << ".m_RuleBegin };\n"
			    << indents << "\tCommonLexer::MatchResult " << newResultID << " { CommonLexer::EMatchStatus::Failure, { " << spanID << ".m_Begin, " << spanID << ".m_Begin } };\n"
			    << indents << "\twhile (true)\n"
			    << indents << "\t{\n"
			    << handleMatcher(result, lex, *subMatcher, settings, ruleId, newResultID, tempStateID, subSpanID, depth + 2) << '\n'
			    << indents << "\t\tif (" << newResultID << ".m_Status != CommonLexer::EMatchStatus::Success)\n"
			    << indents << "\t\t\tbreak;\n"
			    << indents << "\t\t" << subSpanID << ".m_Begin = " << totalSpanID << ".m_End = " << newResultID << ".m_Span.m_End;\n"
			    << indents << "\t\tif (" << subSpanID << ".length() == 0)\n"
			    << indents << "\t\t\tbreak;\n"
			    << indents << "\t}\n"
			    << indents << "\tif (" << totalSpanID << ".m_End.m_Index < " << spanID << ".m_End.m_Index)\n"
			    << indents << "\t{\n"
			    << indents << "\t\t" << stateID << ".m_Messages.reserve(" << stateID << ".m_Messages.size() + " << tempStateID << ".m_Messages.size());\n"
			    << indents << "\t\tfor (auto& message : " << tempStateID << ".m_Messages)\n"
			    << indents << "\t\t\t" << tempStateID << ".m_Messages.push_back(std::move(message));\n"
			    << indents << "\t}\n"
			    << indents << "\t" << resultID << " = { CommonLexer::EMatchStatus::Success, " << totalSpanID << " };\n"
			    << indents << "}";
			return str.str();
		}
		else if (rule == "OrMatcher")
		{
			std::string indents = std::string(depth, '\t');

			std::string statusID      = "status" + std::to_string(depth);
			std::string messagesID    = "messages" + std::to_string(depth);
			std::string largestCopyID = "largestCopy" + std::to_string(depth);
			std::string largestSpanID = "largestSpan" + std::to_string(depth);
			std::string tempNodeID    = "tempNode" + std::to_string(depth);
			std::string tempStateID   = "tempState" + std::to_string(depth);

			std::ostringstream str;
			str << indents << "// OrMatcher\n"
			    << indents << "{\n"
			    << indents << "\tCommonLexer::EMatchStatus " << statusID << " = CommonLexer::EMatchStatus::Failure;\n"
			    << indents << "\tstd::vector<CommonLexer::Message> " << messagesID << ";\n"
			    << indents << "\tCommonLexer::Node " << largestCopyID << " { *" << stateID << ".m_Lexer };\n"
			    << indents << "\tCommonLexer::SourceSpan " << largestSpanID << " { " << spanID << ".m_Begin, " << spanID << ".m_Begin };\n";

			for (auto& child : node.getChildren())
			{
				str << indents << "\t{\n"
				    << indents << "\t\tCommonLexer::Node " << tempNodeID << " { *" << stateID << ".m_Lexer };\n"
				    << indents << "\t\tCommonLexer::MatcherState " << tempStateID << " { {}, &" << tempNodeID << ", " << stateID << ".m_Lexer, " << stateID << ".m_Source, " << stateID << ".m_SourceSpan, " << stateID << ".m_CurrentRule, " << stateID << ".m_RuleBegin };\n"
				    << handleMatcher(result, lex, child, settings, ruleId, resultID, tempStateID, spanID, depth + 2) << '\n'
				    << indents << "\t\tswitch (" << resultID << ".m_Status)\n"
				    << indents << "\t\t{\n"
				    << indents << "\t\tcase CommonLexer::EMatchStatus::Skip:\n"
				    << indents << "\t\t\tif (" << statusID << " == CommonLexer::EMatchStatus::Failure)\n"
				    << indents << "\t\t\t\t" << statusID << " = CommonLexer::EMatchStatus::Skip;\n"
				    << indents << "\t\t\t[[fallthrough]];\n"
				    << indents << "\t\tcase CommonLexer::EMatchStatus::Failure:\n"
				    << indents << "\t\t{\n"
				    << indents << "\t\t\tstd::size_t len = " << resultID << ".m_Span.length();\n"
				    << indents << "\t\t\tif (len == 0)\n"
				    << indents << "\t\t\t\tlen = 1;\n"
				    << indents << "\t\t\tif ((" << statusID << " == CommonLexer::EMatchStatus::Skip || " << statusID << " == CommonLexer::EMatchStatus::Failure) && len > " << largestSpanID << ".length())\n"
				    << indents << "\t\t\t{\n"
				    << indents << "\t\t\t\t" << largestSpanID << " = " << resultID << ".m_Span;\n"
				    << indents << "\t\t\t\t" << largestCopyID << " = std::move(" << tempNodeID << ");\n"
				    << indents << "\t\t\t\t" << messagesID << " = std::move(" << tempStateID << ".m_Messages);\n"
				    << indents << "\t\t\t}\n"
				    << indents << "\t\t\tbreak;\n"
				    << indents << "\t\t}\n"
				    << indents << "\t\tcase CommonLexer::EMatchStatus::Success:\n"
				    << indents << "\t\t\tif (" << statusID << " == CommonLexer::EMatchStatus::Skip || " << statusID << " == CommonLexer::EMatchStatus::Failure || " << resultID << ".m_Span.length() > " << largestSpanID << ".length())\n"
				    << indents << "\t\t\t{\n"
				    << indents << "\t\t\t\t" << largestSpanID << " = " << resultID << ".m_Span;\n"
				    << indents << "\t\t\t\t" << largestCopyID << " = std::move(" << tempNodeID << ");\n"
				    << indents << "\t\t\t\t" << messagesID << " = std::move(" << tempStateID << ".m_Messages);\n"
				    << indents << "\t\t\t\t" << statusID << " = CommonLexer::EMatchStatus::Success;\n"
				    << indents << "\t\t\t}\n"
				    << indents << "\t\t\tbreak;\n"
				    << indents << "\t\tdefault: break;\n"
				    << indents << "\t\t}\n"
				    << indents << "\t}\n";
			}

			str << indents << "\t" << stateID << ".m_Messages.reserve(" << stateID << ".m_Messages.size() + " << messagesID << ".size());\n"
			    << indents << "\tfor (auto& message : " << messagesID << ")\n"
			    << indents << "\t\t" << stateID << ".m_Messages.push_back(std::move(message));\n"
			    << indents << "\tswitch (" << statusID << ")\n"
			    << indents << "\t{\n"
			    << indents << "\tcase CommonLexer::EMatchStatus::Failure:\n"
			    << indents << "\t\t" << resultID << " = { CommonLexer::EMatchStatus::Failure, " << largestSpanID << " };\n"
			    << indents << "\t\tbreak;\n"
			    << indents << "\tcase CommonLexer::EMatchStatus::Skip:\n"
			    << indents << "\t\t" << resultID << " = { CommonLexer::EMatchStatus::Skip, " << largestSpanID << " };\n"
			    << indents << "\t\tbreak;\n"
			    << indents << "\tcase CommonLexer::EMatchStatus::Success:\n"
			    << indents << "\t\t" << stateID << ".m_ParentNode->addChildren(" << largestCopyID << ");\n"
			    << indents << "\t\t" << resultID << " = { CommonLexer::EMatchStatus::Success, " << largestSpanID << " };\n"
			    << indents << "\t\tbreak;\n"
			    << indents << "\tdefault:\n"
			    << indents << "\t\t" << resultID << " = { CommonLexer::EMatchStatus::Failure, " << largestSpanID << " };\n"
			    << indents << "\t\tbreak;\n"
			    << indents << "\t}\n"
			    << indents << "}";
			return str.str();
		}
		else if (rule == "CombinationMatcher")
		{
			std::string indents = std::string(depth, '\t');

			std::string failedID    = "failed" + std::to_string(depth);
			std::string subSpanID   = "subSpan" + std::to_string(depth);
			std::string totalSpanID = "totalSpan" + std::to_string(depth);

			std::ostringstream str;
			str << indents << "// CombinationMatcher\n"
			    << indents << "{\n"
			    << indents << "\tbool " << failedID << " = false;\n\n"
			    << indents << "\tCommonLexer::SourceSpan " << subSpanID << " = " << spanID << ";\n"
			    << indents << "\tCommonLexer::SourceSpan " << totalSpanID << " = { " << spanID << ".m_Begin, " << spanID << ".m_Begin };\n\n";
			for (auto& child : node.getChildren())
			{
				str << indents << "\tif (!" << failedID << ")\n"
				    << indents << "\t{\n"
				    << handleMatcher(result, lex, child, settings, ruleId, resultID, stateID, subSpanID, depth + 2) << '\n'
				    << indents << "\t\tswitch (" << resultID << ".m_Status)\n"
				    << indents << "\t\t{\n"
				    << indents << "\t\tcase CommonLexer::EMatchStatus::Failure:\n"
				    << indents << "\t\t\t" << failedID << " = true;\n"
				    << indents << "\t\t\tbreak;\n"
				    << indents << "\t\tcase CommonLexer::EMatchStatus::Skip: break;\n"
				    << indents << "\t\tcase CommonLexer::EMatchStatus::Success:\n"
				    << indents << "\t\t\t" << subSpanID << ".m_Begin = " << totalSpanID << ".m_End = " << resultID << ".m_Span.m_End;\n"
				    << indents << "\t\t\tbreak;\n"
				    << indents << "\t\t}\n"
				    << indents << "\t}\n";
			}
			str << indents << "\tif (" << failedID << ")\n"
			    << indents << "\t\t" << resultID << " = { CommonLexer::EMatchStatus::Failure, " << totalSpanID << " };\n"
			    << indents << "\telse\n"
			    << indents << "\t\t" << resultID << " = { CommonLexer::EMatchStatus::Success, " << totalSpanID << " };\n"
			    << indents << "}";
			return str.str();
		}
		else if (rule == "Branch")
		{
			std::string indents = std::string(depth, '\t');

			std::string statusID      = "status" + std::to_string(depth);
			std::string messagesID    = "messages" + std::to_string(depth);
			std::string largestCopyID = "largestCopy" + std::to_string(depth);
			std::string largestSpanID = "largestSpan" + std::to_string(depth);
			std::string tempNodeID    = "tempNode" + std::to_string(depth);
			std::string tempStateID   = "tempState" + std::to_string(depth);

			std::ostringstream str;
			str << indents << "// Branch\n"
			    << indents << "{\n"
			    << indents << "\tCommonLexer::EMatchStatus " << statusID << " = CommonLexer::EMatchStatus::Failure;\n"
			    << indents << "\tstd::vector<CommonLexer::Message> " << messagesID << ";\n"
			    << indents << "\tCommonLexer::Node " << largestCopyID << " { *" << stateID << ".m_Lexer };\n"
			    << indents << "\tCommonLexer::SourceSpan " << largestSpanID << " { " << spanID << ".m_Begin, " << spanID << ".m_Begin };\n";

			for (auto& child : node.getChildren())
			{
				str << indents << "\t{\n"
				    << indents << "\t\tCommonLexer::Node " << tempNodeID << " { *" << stateID << ".m_Lexer };\n"
				    << indents << "\t\tCommonLexer::MatcherState " << tempStateID << " { {}, &" << tempNodeID << ", " << stateID << ".m_Lexer, " << stateID << ".m_Source, " << stateID << ".m_SourceSpan, " << stateID << ".m_CurrentRule, " << stateID << ".m_RuleBegin };\n"
				    << handleMatcher(result, lex, child, settings, ruleId, resultID, tempStateID, spanID, depth + 2) << '\n'
				    << indents << "\t\tswitch (" << resultID << ".m_Status)\n"
				    << indents << "\t\t{\n"
				    << indents << "\t\tcase CommonLexer::EMatchStatus::Skip:\n"
				    << indents << "\t\t\tif (" << statusID << " == CommonLexer::EMatchStatus::Failure)\n"
				    << indents << "\t\t\t\t" << statusID << " = CommonLexer::EMatchStatus::Skip;\n"
				    << indents << "\t\t\t[[fallthrough]];\n"
				    << indents << "\t\tcase CommonLexer::EMatchStatus::Failure:\n"
				    << indents << "\t\t{\n"
				    << indents << "\t\t\tstd::size_t len = " << resultID << ".m_Span.length();\n"
				    << indents << "\t\t\tif (len == 0)\n"
				    << indents << "\t\t\t\tlen = 1;\n"
				    << indents << "\t\t\tif ((" << statusID << " == CommonLexer::EMatchStatus::Skip || " << statusID << " == CommonLexer::EMatchStatus::Failure) && len > " << largestSpanID << ".length())\n"
				    << indents << "\t\t\t{\n"
				    << indents << "\t\t\t\t" << largestSpanID << " = " << resultID << ".m_Span;\n"
				    << indents << "\t\t\t\t" << largestCopyID << " = std::move(" << tempNodeID << ");\n"
				    << indents << "\t\t\t\t" << messagesID << " = std::move(" << tempStateID << ".m_Messages);\n"
				    << indents << "\t\t\t}\n"
				    << indents << "\t\t\tbreak;\n"
				    << indents << "\t\t}\n"
				    << indents << "\t\tcase CommonLexer::EMatchStatus::Success:\n"
				    << indents << "\t\t\tif (" << statusID << " == CommonLexer::EMatchStatus::Skip || " << statusID << " == CommonLexer::EMatchStatus::Failure || " << resultID << ".m_Span.length() > " << largestSpanID << ".length())\n"
				    << indents << "\t\t\t{\n"
				    << indents << "\t\t\t\t" << largestSpanID << " = " << resultID << ".m_Span;\n"
				    << indents << "\t\t\t\t" << largestCopyID << " = std::move(" << tempNodeID << ");\n"
				    << indents << "\t\t\t\t" << messagesID << " = std::move(" << tempStateID << ".m_Messages);\n"
				    << indents << "\t\t\t\t" << statusID << " = CommonLexer::EMatchStatus::Success;\n"
				    << indents << "\t\t\t}\n"
				    << indents << "\t\t\tbreak;\n"
				    << indents << "\t\tdefault: break;\n"
				    << indents << "\t\t}\n"
				    << indents << "\t}\n";
			}

			str << indents << "\t" << stateID << ".m_Messages.reserve(" << stateID << ".m_Messages.size() + " << messagesID << ".size());\n"
			    << indents << "\tfor (auto& message : " << messagesID << ")\n"
			    << indents << "\t\t" << stateID << ".m_Messages.push_back(std::move(message));\n"
			    << indents << "\tswitch (" << statusID << ")\n"
			    << indents << "\t{\n"
			    << indents << "\tcase CommonLexer::EMatchStatus::Failure:\n"
			    << indents << "\t\t" << resultID << " = { CommonLexer::EMatchStatus::Failure, " << largestSpanID << " };\n"
			    << indents << "\t\tbreak;\n"
			    << indents << "\tcase CommonLexer::EMatchStatus::Skip:\n"
			    << indents << "\t\t" << resultID << " = { CommonLexer::EMatchStatus::Skip, " << largestSpanID << " };\n"
			    << indents << "\t\tbreak;\n"
			    << indents << "\tcase CommonLexer::EMatchStatus::Success:\n"
			    << indents << "\t\t" << stateID << ".m_ParentNode->addChildren(" << largestCopyID << ");\n"
			    << indents << "\t\t" << resultID << " = { CommonLexer::EMatchStatus::Success, " << largestSpanID << " };\n"
			    << indents << "\t\tbreak;\n"
			    << indents << "\tdefault:\n"
			    << indents << "\t\t" << resultID << " = { CommonLexer::EMatchStatus::Failure, " << largestSpanID << " };\n"
			    << indents << "\t\tbreak;\n"
			    << indents << "\t}\n"
			    << indents << "}";
			return str.str();
		}
		else
		{
			result.m_Messages.emplace_back(std::format("Rule '{}' is not a recognized matcher rule", rule), span.m_Begin, span, EMessageSeverity::Warning);
			return std::format("/* ERROR: {} is not a recognized matcher rule */", rule);
		}
	}

	void handleScope(LexerCPPResult& result, const Lex& lex, const Node& node, ScopeSettings settings, std::size_t depth = 0)
	{
		auto source = lex.getSource();

		std::vector<std::string> declaredOptions;

		for (auto& declaration : node.getChildren())
		{
			auto rule            = declaration.getRule();
			auto declarationSpan = declaration.getSpan();
			if (rule == "OptionDeclaration")
			{
				auto optionIdNode = declaration.getChild(0);
				if (!optionIdNode || optionIdNode->getRule() != "Identifier")
				{
					result.m_Messages.emplace_back("Unexpected OptionDeclaration missing Identifier", declarationSpan.m_Begin, declarationSpan);
					continue;
				}
				auto optionIdSpan = optionIdNode->getSpan();

				auto valueNode = declaration.getChild(1);
				if (!valueNode)
				{
					result.m_Messages.emplace_back("Unexpected OptionDeclaration missing Value", optionIdSpan.m_End, declarationSpan);
					continue;
				}

				auto optionId = source->getSpan(optionIdSpan);
				if (std::find(declaredOptions.begin(), declaredOptions.end(), optionId) != declaredOptions.end())
				{
					result.m_Messages.emplace_back(std::format("OptionDeclaration {} already previously declared", optionId), optionIdSpan.m_Begin, optionIdSpan, EMessageSeverity::Warning);
					continue;
				}

				if (optionId == "MainRule")
				{
					if (depth > 0)
					{
						result.m_Messages.emplace_back("MainRule can only be set in the global scope", declarationSpan.m_Begin, declarationSpan);
						continue;
					}

					auto valueSpan = valueNode->getSpan();
					if (valueNode->getRule() != "Identifier")
					{
						result.m_Messages.emplace_back("MainRule value is not an Identifier", valueSpan.m_Begin, valueSpan);
						continue;
					}

					result.m_MainRule = source->getSpan(valueSpan);
				}
				else if (optionId == "SpaceMethod")
				{
					auto valueSpan = valueNode->getSpan();
					if (valueNode->getRule() != "TextMatcher")
					{
						result.m_Messages.emplace_back("SpaceMethod value is not a string", valueSpan.m_Begin, valueSpan);
						continue;
					}

					auto value = UnescapeText(source->getSpan(valueSpan));
					if (value == "Normal")
					{
						settings.m_SpaceMethod = ESpaceMethod::Normal;
					}
					else if (value == "Whitespace")
					{
						settings.m_SpaceMethod = ESpaceMethod::Whitespace;
					}
					else
					{
						result.m_Messages.emplace_back(std::format("SpaceMethod value \"{}\" is not valid, should be either \"Normal\" or \"Whitespace\" ", EscapeText(value)), valueSpan.m_Begin, valueSpan);
						continue;
					}
				}
				else if (optionId == "SpaceDirection")
				{
					auto valueSpan = valueNode->getSpan();
					if (valueNode->getRule() != "TextMatcher")
					{
						result.m_Messages.emplace_back("SpaceDirection value is not a string", valueSpan.m_Begin, valueSpan);
						continue;
					}

					auto value = UnescapeText(source->getSpan(valueSpan));
					if (value == "Left")
					{
						settings.m_SpaceDirection = ESpaceDirection::Left;
					}
					else if (value == "Right")
					{
						settings.m_SpaceDirection = ESpaceDirection::Right;
					}
					else if (value == "Both")
					{
						settings.m_SpaceDirection = ESpaceDirection::Both;
					}
					else
					{
						result.m_Messages.emplace_back(std::format("SpaceDirection value \"{}\" is not valid, should be either \"Left\", \"Right\" or \"Both\" ", EscapeText(value)), valueSpan.m_Begin, valueSpan);
						continue;
					}
				}

				declaredOptions.push_back(optionId);
			}
		}

		for (auto& declaration : node.getChildren())
		{
			auto rule            = declaration.getRule();
			auto declarationSpan = declaration.getSpan();
			if (rule == "OptionDeclaration")
			{
				continue;
			}
			else if (rule == "BlockDeclaration")
			{
				ScopeSettings blockScope = settings;
				handleScope(result, lex, declaration, blockScope, depth + 1);
			}
			else if (rule == "RuleDeclaration")
			{
				auto ruleIdNode = declaration.getChild(0);
				if (!ruleIdNode || ruleIdNode->getRule() != "Identifier")
				{
					result.m_Messages.emplace_back("Unexpected RuleDeclaration missing Identifier", declarationSpan.m_Begin, declarationSpan);
					continue;
				}
				auto ruleIdSpan = ruleIdNode->getSpan();

				auto matcherNode = declaration.getChild(1);
				if (!matcherNode)
				{
					result.m_Messages.emplace_back("Unexpected RuleDeclaration missing Matcher", ruleIdSpan.m_End, declarationSpan);
					continue;
				}
				auto matcherSpan = matcherNode->getSpan();

				auto ruleId = source->getSpan(ruleIdSpan);
				if (result.m_Rules.find(ruleId) != result.m_Rules.end())
				{
					result.m_Messages.emplace_back(std::format("Rule {} has already been declared", ruleId), ruleIdSpan.m_Begin, declarationSpan);
					continue;
				}

				std::ostringstream str;
				str << "// Rule " << ruleId << '\n'
				    << "CommonLexer::MatchResult " << ruleId << "Match(CommonLexer::MatcherState& state, CommonLexer::SourceSpan span)\n"
				    << "{\n"
				    << "\tCommonLexer::Node         currentNode { *state.m_Lexer, \"" << ruleId << "\" };\n"
				    << "\tCommonLexer::MatcherState tempState { {}, &currentNode, state.m_Lexer, state.m_Source, state.m_SourceSpan, nullptr, span.m_Begin };\n"
				    << "\tCommonLexer::MatchResult  result { CommonLexer::EMatchStatus::Failure, { span.m_Begin, span.m_Begin } };\n\n"
				    << handleMatcher(result, lex, *matcherNode, settings, ruleId, "result", "tempState", "span") << '\n'
				    << "\tif (result.m_Status == CommonLexer::EMatchStatus::Success)\n"
				    << "\t{\n"
				    << "\t\tcurrentNode.setSourceSpan(state.m_Source, result.m_Span);\n"
				    << "\t\tstate.m_ParentNode->addChild(std::move(currentNode));\n"
				    << "\t}\n"
				    << "\tstate.m_Messages.reserve(state.m_Messages.size() + tempState.m_Messages.size());\n"
				    << "\tfor (auto& message : tempState.m_Messages)\n"
				    << "\t\tstate.m_Messages.push_back(std::move(message));\n"
				    << "\treturn result;\n"
				    << "}";
				result.m_Rules.insert({ std::move(ruleId), str.str() });
			}
			else if (rule == "NodelessRuleDeclaration")
			{
				auto ruleIdNode = declaration.getChild(0);
				if (!ruleIdNode || ruleIdNode->getRule() != "Identifier")
				{
					result.m_Messages.emplace_back("Unexpected RuleDeclaration missing Identifier", declarationSpan.m_Begin, declarationSpan);
					continue;
				}
				auto ruleIdSpan = ruleIdNode->getSpan();

				auto matcherNode = declaration.getChild(1);
				if (!matcherNode)
				{
					result.m_Messages.emplace_back("Unexpected RuleDeclaration missing Matcher", ruleIdSpan.m_End, declarationSpan);
					continue;
				}
				auto matcherSpan = matcherNode->getSpan();

				auto ruleId = source->getSpan(ruleIdSpan);
				if (result.m_Rules.find(ruleId) != result.m_Rules.end())
				{
					result.m_Messages.emplace_back(std::format("Rule {} has already been declared", ruleId), ruleIdSpan.m_Begin, declarationSpan);
					continue;
				}

				std::ostringstream str;
				str << "// Rule " << ruleId << '\n'
				    << "CommonLexer::MatchResult " << ruleId << "Match(CommonLexer::MatcherState& state, CommonLexer::SourceSpan span)\n"
				    << "{\n"
				    << "\tCommonLexer::MatcherState tempState { {}, state.m_ParentNode, state.m_Lexer, state.m_Source, state.m_SourceSpan, nullptr, span.m_Begin };\n"
				    << "\tCommonLexer::MatchResult  result { CommonLexer::EMatchStatus::Failure, { span.m_Begin, span.m_Begin } };\n\n"
				    << handleMatcher(result, lex, *matcherNode, settings, ruleId, "result", "tempState", "span") << '\n'
				    << "\tstate.m_Messages.reserve(state.m_Messages.size() + tempState.m_Messages.size());\n"
				    << "\tfor (auto& message : tempState.m_Messages)\n"
				    << "\t\tstate.m_Messages.push_back(std::move(message));\n"
				    << "\treturn result;\n"
				    << "}";
				result.m_Rules.insert({ std::move(ruleId), str.str() });
			}
			else if (rule == "CallbackRuleDeclaration")
			{
				auto ruleIdNode = declaration.getChild(0);
				if (!ruleIdNode || ruleIdNode->getRule() != "Identifier")
				{
					result.m_Messages.emplace_back("Unexpected CallbackRuleDeclaration missing Identifier", declarationSpan.m_Begin, declarationSpan);
					continue;
				}
				auto ruleIdSpan = ruleIdNode->getSpan();

				auto ruleId = source->getSpan(ruleIdSpan);
				if (result.m_Rules.find(ruleId) != result.m_Rules.end())
				{
					result.m_Messages.emplace_back(std::format("Rule {} has already been declared", ruleId), ruleIdSpan.m_Begin, declarationSpan);
					continue;
				}

				result.m_Rules.insert({ std::move(ruleId), "// Rule " + ruleId + " callback" });
			}
			else
			{
				result.m_Messages.emplace_back(std::format("Rule {} is not a recognized declaration", rule), declarationSpan.m_Begin, declarationSpan, EMessageSeverity::Warning);
				continue;
			}
		}
	}

	LexerCPPResult LexerLexer::createCPPLexer(const Lex& lex)
	{
		LexerCPPResult result;
		ScopeSettings  settings;
		handleScope(result, lex, lex.getRoot(), settings);
		return result;
	}

	std::string LexerLexer::compileLexer(LexerCPPResult& result, std::string_view namespaceName)
	{
		std::ostringstream str;
		str << "#include <CommonLexer/Lexer.h>\n"
		    << "#include <CommonLexer/Matcher.h>\n"
		    << "#include <CommonLexer/Message.h>\n"
		    << "#include <CommonLexer/Node.h>\n"
		    << "#include <CommonLexer/Rule.h>\n"
		    << "#include <CommonLexer/Source.h>\n\n"
		    << "#include <format>\n"
		    << "#include <regex>\n\n"
		    << "namespace " << namespaceName << "\n"
		    << "{\n\n"
		    << "//\n"
		    << "// Forward refs\n"
		    << "//\n\n";
		for (auto& rule : result.m_Rules)
			str << "CommonLexer::MatchResult " << rule.first << "Match(CommonLexer::MatcherState& state, CommonLexer::SourceSpan span);\n";
		str << "\n"
		    << "//\n"
		    << "// Declarations\n"
		    << "//\n\n";
		for (auto& rule : result.m_Rules)
			str << rule.second << "\n\n";
		str << "CommonLexer::Lex lexSource(CommonLexer::ISource* source, CommonLexer::SourceSpan span)\n"
		    << "{\n"
		    << "\tCommonLexer::Lexer lexer;\n\n"
		    << "\tCommonLexer::Lex lex { lexer, source };\n"
		    << "\tif (source)\n"
		    << "\t{\n"
		    << "\t\tauto& root = lex.getRoot();\n"
		    << "\t\troot.setRule(\"" << result.m_MainRule << "\");\n"
		    << "\t\tCommonLexer::MatcherState state { {}, &root, &lexer, source, span, nullptr, span.m_Begin };\n"
		    << "\t\tauto result = " << result.m_MainRule << "Match(state, span);\n"
		    << "\t\tif (result.m_Status == CommonLexer::EMatchStatus::Success)\n"
		    << "\t\t\troot.setSourceSpan(source, result.m_Span);\n"
		    << "\t\telse\n"
		    << "\t\t\troot.setSourceSpan(source, { span.m_Begin, span.m_Begin });\n"
		    << "\t\tlex.setMessages(std::move(state.m_Messages));\n"
		    << "\t}\n"
		    << "\treturn lex;\n"
		    << "}\n\n"
		    << "CommonLexer::Lex lexSource(CommonLexer::ISource* source)\n"
		    << "{\n"
		    << "\treturn lexSource(source, source->getCompleteSpan());\n"
		    << "}\n\n"
		    << "}";
		return str.str();
	}
} // namespace CommonLexer