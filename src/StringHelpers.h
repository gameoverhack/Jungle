/*
 *  StringHelpers.h
 *  Jungle
 *
 *  Created by ollie on 8/09/11.
 *  Copyright 2011 Oliver Marriott. All rights reserved.
 *
 */

#ifndef _STRINGHELPERS_H
#define _STRINGHELPERS_H

#include <boost/algorithm/string.hpp> // string splitting
#include <boost/regex.hpp> // regex
using boost::regex;
using boost::cmatch;
using boost::regex_search;
using boost::regex_match;
#include <string>
using std::string;

int findSequenceNumberFromString(string str);

#endif