/*
datetimeFunctions.cpp - Date and time related Stutsk functions built into the interpreter are implemented here
Coded by Tibor Djurica Potpara and Maj Smerkol

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <builtinFunctions.h>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <ctime> 

using namespace boost::posix_time;

// We define UNIX epoch and difference between local time and UTC to aid conversion
const boost::gregorian::date unixEpoch(1970,1,1);
const time_duration TZoffset = second_clock::local_time() - second_clock::universal_time();

// Most of the functions here are using local system time, as provided by the localtime()
// libc facility. Timestamp is per standard stored in UTC format. If the timezone/DST settings
// are set wrongly or if the interface is broken, this may produce strange results.
// Nevertheless, `time_array make_time` should always return the same result as it is given.

inline boost::posix_time::ptime from_unix(const time_t time)
{
	// Seconds are only long, causing problems with 2038+ dates so we must use ticks.
	return ptime(unixEpoch, time_duration(0, 0, 0, time_duration::ticks_per_second() * 
	static_cast<time_duration::fractional_seconds_type>(time) ));
}

inline time_t to_unix(const boost::posix_time::ptime& time)
{
	time_duration dur = time - ptime(unixEpoch); 
	return dur.ticks() / dur.ticks_per_second();
}

void BuiltIns::_f_now(Context* context) {
	/* arguments: now
	   returnvalue: <T_INTEGER>
	   description: Returns current time as UNIX timestamp.
	   notes: 
	*/	
	pushInteger(to_unix(second_clock().universal_time()));
}

void BuiltIns::_f_utc_difference(Context* context) {
	/* arguments: utc_difference
	   returnvalue: <T_INTEGER>
	   description: Returns the difference of local time and UTC time 
	     (timezone and DST-adjusted) in seconds. (UTC + difference = localtime)
	   notes: May be negative.
	*/	
	pushInteger(TZoffset.total_seconds());
}

void BuiltIns::_f_make_time(Context* context) {
	/* arguments: <T_ARRAY datetime> make_time
	   returnvalue: <T_INTEGER>
	   description: Takes an array of time/date components and returns UNIX timestamp.
	     Syntax of the array: ( <second> <minute> <hour> <day> <month> <year> )
	   notes: 
	*/	
	Token token = stack_back_safe();
	stutskStack.pop_back();
	recurseVariables(token);
	if (token.tokenType != T_ARRAY)
		throw StutskException(ET_ERROR, "Token is not an array");
	if (token.asTokenList->size() != 6 )
		throw StutskException(ET_ERROR, "Array is of an invalid size");

	ptime timeRecord(boost::gregorian::date(giveInteger((*token.asTokenList)[5]),
								         	giveInteger((*token.asTokenList)[4]),
									        giveInteger((*token.asTokenList)[3])),
		                      time_duration(giveInteger((*token.asTokenList)[2]), 
				                            giveInteger((*token.asTokenList)[1]), 
		                                    giveInteger((*token.asTokenList)[0]), 0));
	pushInteger(to_unix(timeRecord-TZoffset));
}

void BuiltIns::_f_time_array(Context* context) {
	/* arguments: <T_INTEGER timestamp> time_array
	   returnvalue: <T_ARRAY>
	   description: Takes a UNIX timestamp and returns an array of time/date components.
	     Syntax of the array: ( <second> <minute> <hour> <day> <month> <year> )
	   notes: 
	*/	
	Token token = stack_back_safe();
	stutskStack.pop_back();
	ptime timeRecord = from_unix(giveInteger(token)) + TZoffset;
	TokenListPtr dateArray = TokenListPtr(new TokenList());
	Token addToken(T_INTEGER);
	for (short i=0; i<6; i++)
	{
		switch (i)
		{
		case 0: addToken.data.asInteger = timeRecord.time_of_day().seconds(); break;
		case 1: addToken.data.asInteger = timeRecord.time_of_day().minutes(); break;
		case 2: addToken.data.asInteger = timeRecord.time_of_day().hours(); break;
		case 3: addToken.data.asInteger = timeRecord.date().day(); break;
		case 4: addToken.data.asInteger = timeRecord.date().month(); break;
		case 5: addToken.data.asInteger = timeRecord.date().year(); break;
		}
		dateArray->push_back(addToken);
	}
	Token addArray(T_ARRAY);
	addArray.asTokenList = dateArray;
	stutskStack.push_back(addArray);
}

void BuiltIns::_f_day_of(Context* context) {
	/* arguments: <T_INTEGER timestamp> day_of
	   returnvalue: <T_INTEGER>
	   description: Returns the day number of `timestamp`.
	   notes: 
	*/	
	Token token = stack_back_safe();
	stutskStack.pop_back();
	ptime timeRecord = from_unix(giveInteger(token)) + TZoffset;	
	pushInteger(timeRecord.date().day());
}

void BuiltIns::_f_day_of_week(Context* context) {
	/* arguments: <T_INTEGER timestamp> day_of_week
	   returnvalue: <T_INTEGER>
	   description: Returns the day of the week number of `timestamp`.
	   notes: Monday-0, Tuesday-1, ..., Sunday-6
	*/	
	Token token = stack_back_safe();
	stutskStack.pop_back();
	ptime timeRecord = from_unix(giveInteger(token)) + TZoffset;	
	pushInteger((timeRecord.date().day_of_week() + 6) % 7);
}

void BuiltIns::_f_month_of(Context* context) {
	/* arguments: <T_INTEGER timestamp> month_of
	   returnvalue: <T_INTEGER>
	   description: Returns the month number of `timestamp`.
	   notes: 
	*/	
	Token token = stack_back_safe();
	stutskStack.pop_back();
	ptime timeRecord = from_unix(giveInteger(token)) + TZoffset;	
	pushInteger(timeRecord.date().month());
}

void BuiltIns::_f_year_of(Context* context) {
	/* arguments: <T_INTEGER timestamp> year_of
	   returnvalue: <T_INTEGER>
	   description: Returns the year number of `timestamp`.
	   notes: 
	*/	
	Token token = stack_back_safe();
	stutskStack.pop_back();
	ptime timeRecord = from_unix(giveInteger(token)) + TZoffset;	
	pushInteger(timeRecord.date().year());
}

void BuiltIns::_f_second_of(Context* context) {
	/* arguments: <T_INTEGER timestamp> second_of
	   returnvalue: <T_INTEGER>
	   description: Returns the seconds number `timestamp`.
	   notes: 
	*/	
	Token token = stack_back_safe();
	stutskStack.pop_back();
	ptime timeRecord = from_unix(giveInteger(token)) + TZoffset;	
	pushInteger(timeRecord.time_of_day().seconds());
}

void BuiltIns::_f_minute_of(Context* context) {
	/* arguments: <T_INTEGER timestamp> minute_of
	   returnvalue: <T_INTEGER>
	   description: Returns the minutes number `timestamp`.
	   notes: 
	*/	
	Token token = stack_back_safe();
	stutskStack.pop_back();
	ptime timeRecord = from_unix(giveInteger(token)) + TZoffset;	
	pushInteger(timeRecord.time_of_day().minutes());
}

void BuiltIns::_f_hour_of(Context* context) {
	/* arguments: <T_INTEGER timestamp> hour_of
	   returnvalue: <T_INTEGER>
	   description: Returns the hour number `timestamp`.
	   notes: 
	*/	
	Token token = stack_back_safe();
	stutskStack.pop_back();
	ptime timeRecord = from_unix(giveInteger(token)) + TZoffset;	
	pushInteger(timeRecord.time_of_day().hours());
}

void BuiltIns::_f_format_time(Context* context) {
	/* arguments: <T_INTEGER timestamp> <T_STRING format> format_time
	   returnvalue: <T_STRING>
	   description: Formats the date according to format string `format`.
	   notes: See http://www.boost.org/doc/libs/1_49_0/doc/html/date_time/date_time_io.html#date%5Ftime.format%5Fflags
	*/	
	Token token1 = stack_back_safe();
	stutskStack.pop_back();
	Token token2 = stack_back_safe();
	stutskStack.pop_back();
	time_t time = giveInteger(token2);

	string formatString = *giveString(token1);

	// The locale object takes over responsibility of deleting this facet object.
	time_facet *facet = new time_facet(formatString.c_str());
	
	stringstream ss;
	ss.imbue(std::locale(ss.getloc(), facet));

	ss << from_unix(time) + TZoffset;

	*pushString() = ss.str();
}

void BuiltIns::_f_parse_time(Context* context) {
	/* arguments: <T_INTEGER timestamp> <T_STRING format> parse_time
	   returnvalue: <T_STRING>
	   description: Formats the date according to format string `format`.
	   notes: See http://www.boost.org/doc/libs/1_49_0/doc/html/date_time/date_time_io.html#date%5Ftime.format%5Fflags
	*/	
	Token token1 = stack_back_safe();
	stutskStack.pop_back();
	Token token2 = stack_back_safe();
	stutskStack.pop_back();

	string formatString = *giveString(token1);
	string dateTime = *giveString(token2);

	// The locale object takes over responsibility of deleting this facet object.
	time_input_facet *facet = new time_input_facet(formatString.c_str());
	ptime timeObject;

	stringstream ss(dateTime);
	ss.imbue(std::locale(ss.getloc(), facet));
	ss >> timeObject;

	pushInteger(to_unix(timeObject-TZoffset));
}