/*
 * CreateCharInfo.hpp
 *
 *  Created on: Nov 03, 2025
 *      Author: Timur
 */

#ifndef INC_PROJECT_SCENES_CREATE_CHAR_INFO_HPP_
#define INC_PROJECT_SCENES_CREATE_CHAR_INFO_HPP_

#include <Data/SourceData.hpp>
#include <Data/StaticText.hpp>
#include <Data/StaticTextView.hpp>

#include <stdio.h>

inline StaticText32 CreateCharInfo(StaticTextView name, StaticTextView value)
{
	char info[32]{};
	snprintf(info, sizeof(info), "%s: %s", name.ConstChar(), value.ConstChar());
	return info;
}

template <typename T>
inline StaticText32 CreateCharInfo(StaticTextView name, T&& value, size_t prec = 5)
{
	char info[32]{};
	snprintf(info, sizeof(info), value >= 0 ? "%s: %.*f" : "%s:%.*f", name.ConstChar(), prec, value);
	return info;
}

template <typename T>
inline StaticText32 CreateCharInfo(StaticTextView name, T& value, size_t prec = 5)
{
	char info[32]{};
	snprintf(info, sizeof(info), value >= 0 ? "%s: %.*f" : "%s:%.*f", name.ConstChar(), prec, value);
	return info;
}

template <typename T>
inline StaticText32 CreateCharInfo(StaticTextView name, SourceData<T>& Data, size_t prec = 5)
{
	T value{};
	Data.Get(value);
	return CreateCharInfo(name, value, prec);
}

#endif
