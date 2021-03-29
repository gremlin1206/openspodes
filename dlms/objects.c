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

#include <dlms/object.h>

#include <dlms/class_association_ln.h>
#include <dlms/class_data.h>
#include <dlms/class_profile_generic.h>
#include <dlms/class_clock.h>
#include <dlms/class_register.h>
#include <dlms/class_activity_calendar.h>
#include <dlms/class_limiter.h>
#include <dlms/class_demand.h>
#include <dlms/class_special_days_table.h>
#include <dlms/class_disconnect_control.h>
#include <dlms/class_push_setup.h>

#include <dlms/objects.h>

static union
{
	struct cosem_object_t                 base;

	struct cosem_data_object_t            data;
	struct cosem_association_ln_object_t  association;
	struct cosem_profile_generic_object_t profile_generic;
	struct cosem_clock_object_t           clock;
}
object;

#define OBJECT_ID(a, b, c, d, e, f) (((a) << 31) | ((d) << 24) | ((c) << 16) | (((e) | 0x80) << 8) | (f))

static unsigned int longname2id(struct cosem_longname_t name)
{
	return (name.A << 31) | (name.D << 24) | (name.C << 16) | ((name.E | 0x80) << 8) | name.F;
}

static struct cosem_longname_t id2longname(unsigned int id)
{
	struct cosem_longname_t result;

	result.F = (unsigned char)(id & 0xFF); id >>= 8;
	result.E = (unsigned char)(id & 0x7F); id >>= 8;
	result.C = (unsigned char)(id & 0xFF); id >>= 8;
	result.D = (unsigned char)(id & 0x7F); id >>= 7;
	result.A = (unsigned char)id;

	return result;
}

struct cosem_object_t *cosem_find_object_by_id(struct cosem_ctx_t *ctx, unsigned int id)
{
	switch (id)
	{
	case OBJECT_ID(0,0,40,0,0,255): // current association
	case __COUNTER__:
		id = OBJECT_ID(0,0,40,0,0,255);
		object.base.cosem_class = &class_association_ln;
		object.base.spodes_access_level = spodes_access_level_public;

		object.association.association = &ctx->association;
		break;

	case OBJECT_ID(0,0,42,0,0,255): // device logical name
	case __COUNTER__:
		id = OBJECT_ID(0,0,42,0,0,255);
		object.base.cosem_class = &class_data;
		object.base.spodes_access_level = spodes_access_level_public;

		object.data.value.type = dlms_octet_string;
		object.data.value.octet_string_value.string = ctx->device_logical_name.data;
		object.data.value.octet_string_value.length = ctx->device_logical_name.length;
		break;

	case OBJECT_ID(0,0,1,0,0,255): // clock
	case __COUNTER__:
		id = OBJECT_ID(0,0,1,0,0,255);
		object.base.cosem_class = &class_clock;
		break;

	case OBJECT_ID(1,0,11,7,0,255): // main current
	case __COUNTER__:
		id = OBJECT_ID(1,0,11,7,0,255);
		object.base.cosem_class = &class_register;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(1,0,91,7,0,255): // neutral current
	case __COUNTER__:
		id = OBJECT_ID(1,0,91,7,0,255);
		object.base.cosem_class = &class_register;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(1,0,12,7,0,255): // voltage
	case __COUNTER__:
		id = OBJECT_ID(1,0,12,7,0,255);
		object.base.cosem_class = &class_register;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(1,0,13,7,0,255): // power factor
	case __COUNTER__:
		id = OBJECT_ID(1,0,13,7,0,255);
		object.base.cosem_class = &class_register;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(1,0,14,7,0,255): // frequency
	case __COUNTER__:
		id = OBJECT_ID(1,0,14,7,0,255);
		object.base.cosem_class = &class_register;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(1,0,9,7,0,255): // apparent power
	case __COUNTER__:
		id = OBJECT_ID(1,0,9,7,0,255);
		object.base.cosem_class = &class_register;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(1,0,1,7,0,255): // active power
	case __COUNTER__:
		id = OBJECT_ID(1,0,1,7,0,255);
		object.base.cosem_class = &class_register;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(1,0,3,7,0,255): // reactive power
	case __COUNTER__:
		id = OBJECT_ID(1,0,3,7,0,255);
		object.base.cosem_class = &class_register;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(1,0,1,8,0,255): // active energy import
	case __COUNTER__:
		id = OBJECT_ID(1,0,1,8,0,255);
		object.base.cosem_class = &class_register;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(1,0,2,8,0,255): // active energy export
	case __COUNTER__:
		id = OBJECT_ID(1,0,2,8,0,255);
		object.base.cosem_class = &class_register;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(1,0,3,8,0,255): // reactive energy import
	case __COUNTER__:
		id = OBJECT_ID(1,0,3,8,0,255);
		object.base.cosem_class = &class_register;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(1,0,4,8,0,255): // reactive energy export
	case __COUNTER__:
		id = OBJECT_ID(1,0,4,8,0,255);
		object.base.cosem_class = &class_register;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(1,0,88,8,0,255): // power line energy losses
	case __COUNTER__:
		id = OBJECT_ID(1,0,88,8,0,255);
		object.base.cosem_class = &class_register;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(1,0,94,7,0,255): // snapshot
	case __COUNTER__:
		id = OBJECT_ID(1,0,94,7,0,255);
		object.base.cosem_class = &class_profile_generic;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(1,0,99,1,0,255): // load profile
	case __COUNTER__:
		id = OBJECT_ID(1,0,99,1,0,255);
		object.base.cosem_class = &class_profile_generic;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(1,0,0,8,4,255): // load profile interval
	case __COUNTER__:
		id = OBJECT_ID(1,0,0,8,4,255);
		object.base.cosem_class = &class_data;
		object.base.spodes_access_level = spodes_access_level_reader;
		object.data.value.type = dlms_unsigned;
		object.data.value.unsigned_value = 60;
		break;

	case OBJECT_ID(1,0,1,29,0,255): // active energy in load profile interval
	case __COUNTER__:
		id = OBJECT_ID(1,0,1,29,0,255);
		object.base.cosem_class = &class_register;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(1,0,3,29,0,255): // reactive energy import in load profile interval
	case __COUNTER__:
		id = OBJECT_ID(1,0,3,29,0,255);
		object.base.cosem_class = &class_register;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(1,0,4,29,0,255): // reactive energy export in load profile interval
	case __COUNTER__:
		id = OBJECT_ID(1,0,4,29,0,255);
		object.base.cosem_class = &class_register;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(1,0,94,7,4,255): // scale profile for load profile TODO: ???
	case __COUNTER__:
		id = OBJECT_ID(1,0,94,7,4,255);
		object.base.cosem_class = &class_profile_generic;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(1,0,99,2,0,255): // daily load profile
	case __COUNTER__:
		id = OBJECT_ID(1,0,99,2,0,255);
		object.base.cosem_class = &class_profile_generic;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(1,0,1,8,1,255): // active energy import T1
	case __COUNTER__:
		id = OBJECT_ID(1,0,1,8,1,255);
		object.base.cosem_class = &class_register;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(1,0,1,8,2,255): // active energy import T2
	case __COUNTER__:
		id = OBJECT_ID(1,0,1,8,2,255);
		object.base.cosem_class = &class_register;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(1,0,1,8,3,255): // active energy import T3
	case __COUNTER__:
		id = OBJECT_ID(1,0,1,8,3,255);
		object.base.cosem_class = &class_register;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(1,0,1,8,4,255): // active energy import T4
	case __COUNTER__:
		id = OBJECT_ID(1,0,1,8,4,255);
		object.base.cosem_class = &class_register;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(1,0,94,7,3,255): // scale profile for daily load profile TODO: ???
	case __COUNTER__:
		id = OBJECT_ID(1,0,94,7,3,255);
		object.base.cosem_class = &class_profile_generic;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(1,0,98,1,0,255): // monthly load profile
	case __COUNTER__:
		id = OBJECT_ID(1,0,98,1,0,255);
		object.base.cosem_class = &class_profile_generic;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(0,0,96,1,0,255): // meter serial number
	case __COUNTER__:
		id = OBJECT_ID(0,0,96,1,0,255);
		object.base.cosem_class = &class_data;
		object.base.spodes_access_level = spodes_access_level_reader;

		object.data.value.type = dlms_unsigned;
		object.data.value.unsigned_value = 60;
		break;

	case OBJECT_ID(0,0,96,1,1,255): // meter type
	case __COUNTER__:
		id = OBJECT_ID(0,0,96,1,1,255);
		object.base.cosem_class = &class_data;
		object.base.spodes_access_level = spodes_access_level_reader;

		object.data.value.type = dlms_unsigned;
		object.data.value.unsigned_value = 60;
		break;

	case OBJECT_ID(0,0,96,1,2,255): // metrology firmware version
	case __COUNTER__:
		id = OBJECT_ID(0,0,96,1,2,255);
		object.base.cosem_class = &class_data;
		object.base.spodes_access_level = spodes_access_level_reader;

		object.data.value.type = dlms_unsigned;
		object.data.value.unsigned_value = 60;
		break;

	case OBJECT_ID(0,0,96,1,3,255): // communication firmware version
	case __COUNTER__:
		id = OBJECT_ID(0,0,96,1,3,255);
		object.base.cosem_class = &class_data;
		object.base.spodes_access_level = spodes_access_level_reader;

		object.data.value.type = dlms_unsigned;
		object.data.value.unsigned_value = 60;
		break;

	case OBJECT_ID(0,0,96,1,4,255): // meter production date
	case __COUNTER__:
		id = OBJECT_ID(0,0,96,1,4,255);
		object.base.cosem_class = &class_data;
		object.base.spodes_access_level = spodes_access_level_reader;

		object.data.value.type = dlms_unsigned;
		object.data.value.unsigned_value = 60;
		break;

	case OBJECT_ID(0,0,13,0,0,255):
	case __COUNTER__:
		id = OBJECT_ID(0,0,13,0,0,255);
		object.base.cosem_class = &class_activity_calendar;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(0,0,40,0,2,255): // reader current association
	case __COUNTER__:
		id = OBJECT_ID(0,0,40,0,2,255);
		object.base.cosem_class = &class_association_ln;
		object.base.spodes_access_level = spodes_access_level_reader;

		object.association.association = &ctx->association;
		break;

	case OBJECT_ID(0,0,40,0,3,255): // configurator current association
	case __COUNTER__:
		id = OBJECT_ID(0,0,40,0,3,255);
		object.base.cosem_class = &class_association_ln;
		object.base.spodes_access_level = spodes_access_level_configurator;

		object.association.association = &ctx->association;
		break;

	case OBJECT_ID(0,0,96,1,10,255): // metering point data
	case __COUNTER__:
		id = OBJECT_ID(0,0,96,1,10,255);
		object.base.cosem_class = &class_data;
		object.base.spodes_access_level = spodes_access_level_reader;

		object.data.value.type = dlms_unsigned;
		object.data.value.unsigned_value = 60;
		break;

	case OBJECT_ID(0,0,17,0,0,255): // power limiter
	case __COUNTER__:
		id = OBJECT_ID(0,0,17,0,0,255);
		object.base.cosem_class = &class_limiter;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(0,0,17,0,1,255): // current limiter
	case __COUNTER__:
		id = OBJECT_ID(0,0,17,0,1,255);
		object.base.cosem_class = &class_limiter;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(0,0,17,0,2,255): // voltage limiter
	case __COUNTER__:
		id = OBJECT_ID(0,0,17,0,2,255);
		object.base.cosem_class = &class_limiter;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(0,0,17,0,3,255): // magnetic field limiter
	case __COUNTER__:
		id = OBJECT_ID(0,0,17,0,3,255);
		object.base.cosem_class = &class_limiter;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(1,0,131,35,0,255): // power tangent limit
	case __COUNTER__:
		id = OBJECT_ID(1,0,131,35,0,255);
		object.base.cosem_class = &class_register;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(1,0,0,6,4,255): // nominal voltage
	case __COUNTER__:
		id = OBJECT_ID(1,0,0,6,4,255);
		object.base.cosem_class = &class_register;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(1,0,1,4,0,255): // demand period
	case __COUNTER__:
		id = OBJECT_ID(1,0,1,4,0,255);
		object.base.cosem_class = &class_demand;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(1,0,0,8,5,255): // load profile 2 period
	case __COUNTER__:
		id = OBJECT_ID(1,0,0,8,5,255);
		object.base.cosem_class = &class_data;
		object.base.spodes_access_level = spodes_access_level_reader;

		object.data.value.type = dlms_unsigned;
		object.data.value.unsigned_value = 60;
		break;

	case OBJECT_ID(0,0,11,0,0,255): // special days table
	case __COUNTER__:
		id = OBJECT_ID(0,0,11,0,0,255);
		object.base.cosem_class = &class_special_days_table;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(0,0,96,3,10,255): // disconnector
	case __COUNTER__:
		id = OBJECT_ID(0,0,96,3,10,255);
		object.base.cosem_class = &class_disconnect_control;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(0,0,96,5,3,255): // push setup
	case __COUNTER__:
		id = OBJECT_ID(0,0,96,5,3,255);
		object.base.cosem_class = &class_push_setup;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(0,0,99,98,1,255): // event log (current)
	case __COUNTER__:
		id = OBJECT_ID(0,0,99,98,1,255);
		object.base.cosem_class = &class_profile_generic;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(0,0,99,98,2,255): // event log (disconnector)
	case __COUNTER__:
		id = OBJECT_ID(0,0,99,98,2,255);
		object.base.cosem_class = &class_profile_generic;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(0,0,99,98,3,255): // event log (data correction)
	case __COUNTER__:
		id = OBJECT_ID(0,0,99,98,3,255);
		object.base.cosem_class = &class_profile_generic;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(0,0,99,98,4,255): // event log (external influence)
	case __COUNTER__:
		id = OBJECT_ID(0,0,99,98,4,255);
		object.base.cosem_class = &class_profile_generic;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(0,0,99,98,5,255): // event log (communication events)
	case __COUNTER__:
		id = OBJECT_ID(0,0,99,98,5,255);
		object.base.cosem_class = &class_profile_generic;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(0,0,99,98,6,255): // event log (access control)
	case __COUNTER__:
		id = OBJECT_ID(0,0,99,98,6,255);
		object.base.cosem_class = &class_profile_generic;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(0,0,99,98,7,255): // event log (self diagnostics)
	case __COUNTER__:
		id = OBJECT_ID(0,0,99,98,7,255);
		object.base.cosem_class = &class_profile_generic;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(0,0,99,98,8,255): // event log (tangent limit)
	case __COUNTER__:
		id = OBJECT_ID(0,0,99,98,8,255);
		object.base.cosem_class = &class_profile_generic;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(0,0,99,98,9,255): // event log (PQM)
	case __COUNTER__:
		id = OBJECT_ID(0,0,99,98,9,255);
		object.base.cosem_class = &class_profile_generic;
		object.base.spodes_access_level = spodes_access_level_reader;
		break;

	case OBJECT_ID(0,0,96,2,0,255): // programming counter
	case __COUNTER__:
		id = OBJECT_ID(0,0,96,2,0,255);
		object.base.cosem_class = &class_data;
		object.base.spodes_access_level = spodes_access_level_reader;

		object.data.value.type = dlms_unsigned;
		object.data.value.unsigned_value = 60;
		break;

	case OBJECT_ID(0,0,96,2,1,255): // date of last programming
	case __COUNTER__:
		id = OBJECT_ID(0,0,96,2,1,255);
		object.base.cosem_class = &class_data;
		object.base.spodes_access_level = spodes_access_level_reader;

		object.data.value.type = dlms_unsigned;
		object.data.value.unsigned_value = 60;
		break;

	case OBJECT_ID(0,0,96,2,5,255): // data of last calibration
	case __COUNTER__:
		id = OBJECT_ID(0,0,96,2,5,255);
		object.base.cosem_class = &class_data;
		object.base.spodes_access_level = spodes_access_level_reader;

		object.data.value.type = dlms_unsigned;
		object.data.value.unsigned_value = 60;
		break;

	case OBJECT_ID(0,0,96,2,7,255): // data of last calendar activation
	case __COUNTER__:
		id = OBJECT_ID(0,0,96,2,7,255);
		object.base.cosem_class = &class_data;
		object.base.spodes_access_level = spodes_access_level_reader;

		object.data.value.type = dlms_unsigned;
		object.data.value.unsigned_value = 60;
		break;

	case OBJECT_ID(0,0,96,2,12,255): // data of last time change
	case __COUNTER__:
		id = OBJECT_ID(0,0,96,2,12,255);
		object.base.cosem_class = &class_data;
		object.base.spodes_access_level = spodes_access_level_reader;

		object.data.value.type = dlms_unsigned;
		object.data.value.unsigned_value = 60;
		break;

	case OBJECT_ID(0,0,96,2,13,255): // data of last firmware modification
	case __COUNTER__:
		id = OBJECT_ID(0,0,96,2,13,255);
		object.base.cosem_class = &class_data;
		object.base.spodes_access_level = spodes_access_level_reader;

		object.data.value.type = dlms_unsigned;
		object.data.value.unsigned_value = 60;
		break;

	case OBJECT_ID(0,0,96,20,0,255): // box open counter
	case __COUNTER__:
		id = OBJECT_ID(0,0,96,20,0,255);
		object.base.cosem_class = &class_data;
		object.base.spodes_access_level = spodes_access_level_reader;

		object.data.value.type = dlms_unsigned;
		object.data.value.unsigned_value = 60;
		break;

	case OBJECT_ID(0,0,96,20,1,255): // date of last box open
	case __COUNTER__:
		id = OBJECT_ID(0,0,96,20,1,255);
		object.base.cosem_class = &class_data;
		object.base.spodes_access_level = spodes_access_level_reader;

		object.data.value.type = dlms_unsigned;
		object.data.value.unsigned_value = 60;
		break;

	case OBJECT_ID(0,0,96,20,2,255): // duration of last box open
	case __COUNTER__:
		id = OBJECT_ID(0,0,96,20,2,255);
		object.base.cosem_class = &class_data;
		object.base.spodes_access_level = spodes_access_level_reader;

		object.data.value.type = dlms_unsigned;
		object.data.value.unsigned_value = 60;
		break;

	case OBJECT_ID(0,0,96,20,3,255): // total duration of box open
	case __COUNTER__:
		id = OBJECT_ID(0,0,96,20,3,255);
		object.base.cosem_class = &class_data;
		object.base.spodes_access_level = spodes_access_level_reader;

		object.data.value.type = dlms_unsigned;
		object.data.value.unsigned_value = 60;
		break;

	case OBJECT_ID(0,0,96,20,5,255): // terminal cover open counter
	case __COUNTER__:
		id = OBJECT_ID(0,0,96,20,5,255);
		object.base.cosem_class = &class_data;
		object.base.spodes_access_level = spodes_access_level_reader;

		object.data.value.type = dlms_unsigned;
		object.data.value.unsigned_value = 60;
		break;

	case OBJECT_ID(0,0,96,20,6,255): // date of last terminal cover open
	case __COUNTER__:
		id = OBJECT_ID(0,0,96,20,6,255);
		object.base.cosem_class = &class_data;
		object.base.spodes_access_level = spodes_access_level_reader;

		object.data.value.type = dlms_unsigned;
		object.data.value.unsigned_value = 60;
		break;

	case OBJECT_ID(0,0,96,20,7,255): // duration of last terminal cover open
	case __COUNTER__:
		id = OBJECT_ID(0,0,96,20,7,255);
		object.base.cosem_class = &class_data;
		object.base.spodes_access_level = spodes_access_level_reader;

		object.data.value.type = dlms_unsigned;
		object.data.value.unsigned_value = 60;
		break;

	case OBJECT_ID(0,0,96,20,8,255): // total duration of terminal cover open
	case __COUNTER__:
		id = OBJECT_ID(0,0,96,20,8,255);
		object.base.cosem_class = &class_data;
		object.base.spodes_access_level = spodes_access_level_reader;

		object.data.value.type = dlms_unsigned;
		object.data.value.unsigned_value = 60;
		break;

	case OBJECT_ID(0,0,96,20,15,255): // magnetic sensor events counter
	case __COUNTER__:
		id = OBJECT_ID(0,0,96,20,15,255);
		object.base.cosem_class = &class_data;
		object.base.spodes_access_level = spodes_access_level_reader;

		object.data.value.type = dlms_unsigned;
		object.data.value.unsigned_value = 60;
		break;

	case OBJECT_ID(0,0,96,20,16,255): // date of last magnetic sensor event
	case __COUNTER__:
		id = OBJECT_ID(0,0,96,20,16,255);
		object.base.cosem_class = &class_data;
		object.base.spodes_access_level = spodes_access_level_reader;

		object.data.value.type = dlms_unsigned;
		object.data.value.unsigned_value = 60;
		break;

	case OBJECT_ID(0,0,96,20,17,255): // duration of last magnetic sensor event
	case __COUNTER__:
		id = OBJECT_ID(0,0,96,20,17,255);
		object.base.cosem_class = &class_data;
		object.base.spodes_access_level = spodes_access_level_reader;

		object.data.value.type = dlms_unsigned;
		object.data.value.unsigned_value = 60;
		break;

	case OBJECT_ID(0,0,96,20,18,255): // total duration of magnetic sensor events
	case __COUNTER__:
		id = OBJECT_ID(0,0,96,20,18,255);
		object.base.cosem_class = &class_data;
		object.base.spodes_access_level = spodes_access_level_reader;

		object.data.value.type = dlms_unsigned;
		object.data.value.unsigned_value = 60;
		break;

	default:
		return 0;
	}

	object.base.logical_name = id2longname(id);

	return &object.base;
}

unsigned int cosem_objects_count(void)
{
	return __COUNTER__;
}

struct cosem_object_t *cosem_find_object_by_name(struct cosem_ctx_t *ctx, struct cosem_longname_t name)
{
	return cosem_find_object_by_id(ctx, longname2id(name));
}
