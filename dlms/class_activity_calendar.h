/*
MIT License

Copyright (c) 2021 gremlin1206

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef DLMS_CLASS_ACTIVITY_CALENDAR_H_
#define DLMS_CLASS_ACTIVITY_CALENDAR_H_

#include <dlms/class.h>
#include <dlms/object.h>

/*
 * Class ID 20
 */

enum class_activity_calendar_attribute_t
{
	activity_calendar_zero_attribute,

	activity_calendar_logical_name,

	activity_calendar_name_active,
	activity_calendar_season_profile_active,
	activity_calendar_week_profile_table_active,
	activity_calendar_day_profile_table_active,

	activity_calendar_calendar_name_passive,
	activity_calendar_season_profile_passive,
	activity_calendar_week_profile_table_passive,
	activity_calendar_day_profile_table_passive,
	activity_calendar_activate_passive_calendar_time,
};

enum class_activity_calendar_method_t
{
	activity_calendar_zero_method,

	activity_calendar_activate_passive_calendar,
};

struct cosem_activity_calendar_object_t
{
	struct cosem_object_t base; // must be the first struct member
};

extern const struct cosem_class_t class_activity_calendar;

#endif /* DLMS_CLASS_ACTIVITY_CALENDAR_H_ */
