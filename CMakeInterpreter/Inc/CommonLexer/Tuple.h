#pragma once

#include <cstddef>

#include <memory>
#include <utility>
#include <vector>

namespace CommonLexer
{
	template <class... Ts>
	struct Tuple;

	template <>
	struct Tuple<>
	{
	public:
		using Type = void;
		using Next = Tuple<>;

		static constexpr std::size_t Size = 0;
	};

	template <class T, class... Ts>
	struct Tuple<T, Ts...>
	{
	public:
		using Type = T;
		using Next = Tuple<Ts...>;

		static constexpr std::size_t Size = 1 + sizeof...(Ts);

	public:
		Tuple(T&& value, Ts&&... values)
		    : m_Value(std::forward<T>(value)), m_Next(std::forward<Ts>(values)...) {}

	public:
		Type m_Value;
		Next m_Next;
	};

	template <class... Ts>
	Tuple(Ts&&... values) -> Tuple<Ts...>;

	namespace Details
	{
		template <class Derived, class Base>
		concept DerivedFrom = std::is_base_of_v<Base, Derived>;

		template <class T, DerivedFrom<T>... Ts>
		void add_unique_ptr(std::vector<std::unique_ptr<T>>& ptrs, const Tuple<Ts...>& tuple)
		{
			using U = Tuple<Ts...>;

			if constexpr (U::Size > 0)
			{
				ptrs.push_back(std::make_unique<typename U::Type>(tuple.m_Value));
				if constexpr (U::Size > 1)
					add_unique_ptr(ptrs, tuple.m_Next);
			}
		}

		template <class T, DerivedFrom<T>... Ts>
		void add_unique_ptr(std::vector<std::unique_ptr<T>>& ptrs, Tuple<Ts...>&& tuple)
		{
			using U = Tuple<Ts...>;

			if constexpr (U::Size > 0)
			{
				ptrs.push_back(std::make_unique<typename U::Type>(std::move(tuple.m_Value)));
				if constexpr (U::Size > 1)
					add_unique_ptr(ptrs, std::move(tuple.m_Next));
			}
		}

		template <class T, DerivedFrom<T>... Ts>
		std::vector<std::unique_ptr<T>> make_unique_ptrs(const Tuple<Ts...>& tuple)
		{
			std::vector<std::unique_ptr<T>> ptrs;
			ptrs.reserve(sizeof...(Ts));
			add_unique_ptr(ptrs, tuple);
			return ptrs;
		}

		template <class T, DerivedFrom<T>... Ts>
		std::vector<std::unique_ptr<T>> make_unique_ptrs(Tuple<Ts...>&& tuple)
		{
			std::vector<std::unique_ptr<T>> ptrs;
			ptrs.reserve(sizeof...(Ts));
			add_unique_ptr(ptrs, std::move(tuple));
			return ptrs;
		}
	} // namespace Details
} // namespace CommonLexer