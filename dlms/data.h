
#ifndef DLMS_DATA_H_
#define DLMS_DATA_H_

#include "variant.h"



struct dlms_access_right_t
{

};

struct dlms_object_list_element_t
{
	unsigned short class_id;
	unsigned char version;
	struct cosem_longname_t logical_name;

};


#endif /* DLMS_DATA_H_ */
